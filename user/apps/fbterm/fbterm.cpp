#include "kernel/fs/fcntl.h"
#include "kernel/fs/io_status.h"
#include "kernel/fs/poll.h"
#include "kernel/io/framebuffer.h"
#include "kernel/process/wait/wait.h"
#include "kernel/support/types.h"
#include "kernel/tty/termios.h"
#include "keyboard.h"
#include "terminal.h"
#include "render.h"
#include "pacing.h"
#include "user/runtime/user.h"
#include "user/runtime/terminal_proxy.h"

namespace
{
fbterm::Terminal terminal{};
// Allocate exactly one screen of pixels; never put it on the user stack.
unsigned *pixels = nullptr;
FramebufferInfo screen{};
volatile sig_atomic_t stopping = 0;

// Opt-in diagnostic modes; normal launches retain the production display path.
struct Diagnostics
{
  uint64 poll_ticks = 0, read_ticks = 0, feed_ticks = 0;
  uint64 paint_ticks = 0, draw_ticks = 0, mirror_ticks = 0;
  uint64 bytes = 0, mirrored = 0, pixel_bytes = 0;
  uint64 polls = 0, reads = 0, writes = 0, frames = 0, draws = 0;
  uint64 max_draw_ticks = 0;
} diagnostics;

bool profiling = false, mirror_output = true, rasterize = true, present = true;
const char *profile_mode = nullptr;

uint64 timestamp()
{
  return profiling ? clockticks() : 0;
}

bool configure_profile()
{
  profile_mode = getenv("FBTERM_PROFILE");
  if (!profile_mode || !*profile_mode)
  {
    return true;
  }
  profiling = true;
  if (!strcmp(profile_mode, "normal"))
  {
    return true;
  }
  if (!strcmp(profile_mode, "no-mirror"))
  {
    mirror_output = false;
  }
  else if (!strcmp(profile_mode, "no-present"))
  {
    present = false;
  }
  else if (!strcmp(profile_mode, "no-paint"))
  {
    rasterize = false;
  }
  else if (!strcmp(profile_mode, "parse-only"))
  {
    mirror_output = rasterize = false;
  }
  else
  {
    return false;
  }
  return true;
}

void report(uint64 elapsed)
{
  if (!profiling)
  {
    return;
  }
  // Raw 10 MHz ticks retain precision; elapsed stages include scheduling time.
  printf("FBPROFILE mode=%s elapsed=%lu poll=%lu read=%lu feed=%lu raster=%lu "
         "draw=%lu mirror=%lu bytes=%lu mirrored=%lu pixels=%lu polls=%lu "
         "reads=%lu writes=%lu frames=%lu draws=%lu maxdraw=%lu\n",
         profile_mode, elapsed, diagnostics.poll_ticks, diagnostics.read_ticks,
         diagnostics.feed_ticks, diagnostics.paint_ticks - diagnostics.draw_ticks,
         diagnostics.draw_ticks, diagnostics.mirror_ticks, diagnostics.bytes,
         diagnostics.mirrored, diagnostics.pixel_bytes, diagnostics.polls,
         diagnostics.reads, diagnostics.writes, diagnostics.frames, diagnostics.draws,
         diagnostics.max_draw_ticks);
}

void decimal(char *output, unsigned value)
{
  char digits[12];
  unsigned count = 0;
  do
  {
    digits[count++] = '0' + value % 10;
    value /= 10;
  } while (value);
  for (unsigned i = 0; i < count; ++i)
  {
    output[i] = digits[count - 1 - i];
  }
  output[count] = 0;
}

void stop(int)
{
  stopping = 1;
}

bool flush(int framebuffer)
{
  if (!rasterize)
  {
    for (auto &row : terminal.dirty)
    {
      row = false;
    }
    terminal.scrolled = false;
    return true;
  }
  uint64 begin = timestamp();
  bool okay = fbterm::paint(terminal, screen.width, screen.height, pixels,
                            [&](unsigned top, unsigned height, unsigned *data)
                            {
                              FramebufferUpdate update{0,
                                                       top,
                                                       screen.width,
                                                       height,
                                                       screen.width * 4,
                                                       0,
                                                       reinterpret_cast<uint64>(data)};
                              if (profiling)
                              {
                                ++diagnostics.frames;
                              }
                              if (!present)
                              {
                                return true;
                              }
                              uint64 begin = timestamp();
                              bool okay =
                                  ioctl(framebuffer, XV_FB_DRAW,
                                        reinterpret_cast<unsigned long>(&update)) == 0;
                              if (profiling)
                              {
                                uint64 duration = timestamp() - begin;
                                diagnostics.draw_ticks += duration;
                                ++diagnostics.draws;
                                diagnostics.pixel_bytes += screen.width * height * 4;
                                if (duration > diagnostics.max_draw_ticks)
                                {
                                  diagnostics.max_draw_ticks = duration;
                                }
                              }
                              return okay;
                            });
  if (profiling)
  {
    diagnostics.paint_ticks += timestamp() - begin;
  }
  return okay;
}

bool demo(int framebuffer)
{
  const char *header = "fbterm: ASCII terminal\r\n";
  terminal.feed(header, strlen(header));
  for (unsigned i = 0; i < terminal.rows + 5; ++i)
  {
    char line[] =
        "scroll 00: ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789\r\n";
    line[7] = '0' + i / 10;
    line[8] = '0' + i % 10;
    terminal.feed(line, strlen(line));
  }
  const char *footer = "\033[32mfbterm demo complete\033[0m";
  terminal.feed(footer, strlen(footer));
  return flush(framebuffer);
}

bool transfer(int master, int framebuffer, int &keyboard)
{
  fbterm::Keyboard keymap{};
  KeyboardEvent key_events[32];
  char output[1024];
  unsigned char input[256];
  int used = 0, sent = 0, input_used = 0, input_sent = 0;
  bool hungup = false;
  bool needs_paint = false;
  fbterm::FramePacer pacer{};
  while (!stopping)
  {
    short events = sent == used ? POLLIN : 0;
    if (input_sent < input_used && !hungup)
    {
      events |= POLLOUT;
    }
    pollfd descriptors[] = {
        {input_sent < input_used || hungup ? -1 : 0, POLLIN, 0},
        {events ? master : -1, events, 0},
        {sent < used ? 1 : -1, POLLOUT, 0},
        {input_sent < input_used || hungup ? -1 : keyboard, POLLIN, 0},
    };
    uint64 poll_begin = timestamp();
    int ready = poll(descriptors, 4, pacer.timeout(clockticks()));
    if (profiling)
    {
      diagnostics.poll_ticks += timestamp() - poll_begin;
      ++diagnostics.polls;
    }
    if (ready < 0)
    {
      return stopping;
    }
    if (descriptors[3].revents & (POLLERR | POLLNVAL | POLLHUP))
    {
      close(keyboard);
      keyboard = -1;
      descriptors[3].revents = 0;
    }
    for (auto &descriptor : descriptors)
    {
      if (descriptor.revents & (POLLERR | POLLNVAL))
      {
        return false;
      }
    }
    if (descriptors[2].revents & POLLHUP)
    {
      return false;
    }
    if (descriptors[1].revents & POLLHUP)
    {
      hungup = true;
      input_sent = input_used;
    }
    if (sent == used && (descriptors[1].revents & (POLLIN | POLLHUP)))
    {
      uint64 read_begin = timestamp();
      int n = read(master, output, sizeof(output));
      if (profiling)
      {
        diagnostics.read_ticks += timestamp() - read_begin;
        ++diagnostics.reads;
      }
      if (n == 0)
      {
        return !needs_paint || flush(framebuffer);
      }
      if (n < 0 && n != -EAGAIN)
      {
        return false;
      }
      if (n == -EAGAIN && hungup)
      {
        return !needs_paint || flush(framebuffer);
      }
      if (n > 0)
      {
        uint64 feed_begin = timestamp();
        terminal.feed(output, n);
        if (profiling)
        {
          diagnostics.feed_ticks += timestamp() - feed_begin;
          diagnostics.bytes += n;
        }
        needs_paint = true;
        pacer.output(clockticks(), n, terminal.scrolled);
        used = n;
        sent = mirror_output ? 0 : n;
      }
    }
    if (needs_paint && pacer.timeout(clockticks()) == 0)
    {
      uint64 start = clockticks();
      if (!flush(framebuffer))
      {
        return false;
      }
      needs_paint = false;
      pacer.complete(start, clockticks());
    }
    if (sent < used && (descriptors[2].revents & POLLOUT))
    {
      uint64 write_begin = timestamp();
      int n = write(1, output + sent, used - sent);
      if (profiling)
      {
        diagnostics.mirror_ticks += timestamp() - write_begin;
        ++diagnostics.writes;
        if (n > 0)
        {
          diagnostics.mirrored += n;
        }
      }
      if (n > 0)
      {
        sent += n;
      }
      else if (n != -EAGAIN)
      {
        return false;
      }
    }
    if (descriptors[0].revents & (POLLIN | POLLHUP))
    {
      int n = read(0, input, sizeof(input));
      if (n == 0)
      {
        return !needs_paint || flush(framebuffer);
      }
      if (n < 0 && n != -EAGAIN)
      {
        return false;
      }
      if (n > 0)
      {
        for (int i = 0; i < n; ++i)
        {
          if (terminal_proxy::detach_requested(master, input[i]))
          {
            return !needs_paint || flush(framebuffer); // Ctrl-]
          }
        }
        input_used = n;
        input_sent = 0;
      }
    }
    if (input_sent == input_used && (descriptors[3].revents & POLLIN))
    {
      int bytes = read(keyboard, key_events, sizeof(key_events));
      if (bytes < 0 && bytes != -EAGAIN)
      {
        return false;
      }
      input_used = input_sent = 0;
      for (int i = 0; i < bytes / static_cast<int>(sizeof(KeyboardEvent)); ++i)
      {
        char translated[8];
        unsigned length = keymap.translate(key_events[i], translated);
        for (unsigned j = 0; j < length; ++j)
        {
          if (terminal_proxy::detach_requested(master, translated[j]))
          {
            return !needs_paint || flush(framebuffer);
          }
          input[input_used++] = translated[j];
        }
      }
    }
    if (input_sent < input_used && !hungup && (descriptors[1].revents & POLLOUT))
    {
      int n = write(master, input + input_sent, input_used - input_sent);
      if (n > 0)
      {
        input_sent += n;
      }
      else if (n != -EAGAIN)
      {
        return false;
      }
    }
  }
  return !needs_paint || flush(framebuffer);
}
} // namespace

extern "C" int main(int argc, char **argv)
{
  if (!configure_profile())
  {
    fprintf(2, "fbterm: FBTERM_PROFILE must be normal, no-mirror, no-present, "
               "no-paint or parse-only\n");
    return 1;
  }
  int first = 1;
  bool show_demo = false;
  while (first < argc)
  {
    const char *option = argv[first];
    if (!strcmp(option, "--"))
    {
      ++first;
      break;
    }
    if (!strcmp(option, "--no-serial-mirror"))
    {
      mirror_output = false;
    }
    else if (!strcmp(option, "--demo"))
    {
      show_demo = true;
    }
    else if (!strcmp(option, "--help") || !strcmp(option, "-h"))
    {
      printf("usage: fbterm [--no-serial-mirror] [--] [command [args...]]\n"
             "       fbterm --demo\n"
             "VNC/serial keyboard; Ctrl-] exits. --no-serial-mirror keeps serial "
             "input, displays command output only on screen.\n");
      return 0;
    }
    else if (option[0] == '-')
    {
      fprintf(2, "fbterm: unknown option %s\n", option);
      return 1;
    }
    else
    {
      break;
    }
    ++first;
  }
  if (show_demo && first < argc)
  {
    fprintf(2, "fbterm: --demo does not take a command\n");
    return 1;
  }
  int framebuffer = open("/dev/fb0", O_RDWR);
  FramebufferInfo info{};
  if (framebuffer < 0 ||
      ioctl(framebuffer, XV_FB_GET_INFO, reinterpret_cast<unsigned long>(&info)) < 0 ||
      info.version != 1 || info.bytes_per_pixel != 4 || info.format != 2 ||
      info.stride != info.width * 4 || !terminal.configure(info.width, info.height))
  {
    fprintf(
        2,
        "fbterm: supported BGRX framebuffer required (root, qemu-vnc or qemu-graphics)\n");
    if (framebuffer >= 0)
    {
      close(framebuffer);
    }
    return 1;
  }
  screen = info;
  pixels = static_cast<unsigned *>(malloc(info.width * info.height * sizeof(unsigned)));
  if (!pixels)
  {
    fprintf(2, "fbterm: cannot allocate screen buffer\n");
    close(framebuffer);
    return 1;
  }
  if (show_demo)
  {
    bool ok = demo(framebuffer);
    close(framebuffer);
    printf("fbterm: demo %s\n", ok ? "displayed" : "failed");
    return ok ? 0 : 1;
  }
  termios saved{};
  int input_flags = fcntl(0, F_GETFL), output_flags = fcntl(1, F_GETFL);
  if (input_flags < 0 || output_flags < 0 || tcgetattr(0, &saved) < 0 ||
      tcgetpgrp(0) != getpgrp())
  {
    fprintf(2, "fbterm: foreground serial terminal required\n");
    close(framebuffer);
    return 1;
  }
  int fds[2] = {-1, -1};
  char path[32];
  fds[0] = posix_openpt(O_RDWR);
  if (fds[0] >= 0 && grantpt(fds[0]) == 0 && unlockpt(fds[0]) == 0 &&
      ptsname_r(fds[0], path, sizeof(path)) == 0)
  {
    fds[1] = open(path, O_RDWR);
  }
  winsize size{static_cast<unsigned short>(terminal.rows),
               static_cast<unsigned short>(terminal.columns),
               static_cast<unsigned short>(info.width),
               static_cast<unsigned short>(info.height)};
  termios slave_modes{};
  bool slave_ready = fds[1] >= 0 && tcgetattr(fds[1], &slave_modes) == 0;
  slave_modes.c_oflag |= OPOST | ONLCR;
  if (!slave_ready || tcsetattr(fds[1], TCSANOW, &slave_modes) < 0 ||
      ioctl(fds[1], TIOCSWINSZ, reinterpret_cast<unsigned long>(&size)) < 0 ||
      !flush(framebuffer))
  {
    fprintf(2, "fbterm: PTY allocation or framebuffer draw failed\n");
    if (fds[0] >= 0)
    {
      close(fds[0]);
    }
    if (fds[1] >= 0)
    {
      close(fds[1]);
    }
    close(framebuffer);
    return 1;
  }
  int child = fork();
  if (child == 0)
  {
    close(framebuffer);
    close(fds[0]);
    if (setsid() < 0 || ioctl(fds[1], TIOCSCTTY, 0) < 0)
    {
      exit(1);
    }
    close(0);
    close(1);
    close(2);
    if (dup(fds[1]) != 0 || dup(fds[1]) != 1 || dup(fds[1]) != 2)
    {
      exit(1);
    }
    close(fds[1]);
    setenv("TERM", "xv-fbterm", 1);
    char columns[12], rows[12];
    decimal(columns, terminal.columns);
    decimal(rows, terminal.rows);
    setenv("COLUMNS", columns, 1);
    setenv("LINES", rows, 1);
    char *shell[] = {const_cast<char *>("sh"), nullptr};
    execvp(first < argc ? argv[first] : "sh", first < argc ? argv + first : shell);
    fprintf(2, "fbterm: exec failed\n");
    exit(1);
  }
  close(fds[1]);
  if (child < 0)
  {
    close(fds[0]);
    close(framebuffer);
    fprintf(2, "fbterm: fork failed\n");
    return 1;
  }
  // Restore shared serial state on catchable termination as well as normal EOF.
  struct sigaction action{stop, 0, 0};
  bool handlers = sigaction(SIGTERM, &action, nullptr) == 0 &&
                  sigaction(SIGHUP, &action, nullptr) == 0 &&
                  sigaction(SIGINT, &action, nullptr) == 0;
  signal(SIGTSTP, SIG_IGN); // Ctrl-Z is forwarded to the slave's foreground job.
  int keyboard = open("/dev/kbd0", O_RDONLY | O_NONBLOCK);
  printf("fbterm: connected (%dx%d); %s keyboard, Ctrl-] exits\n", terminal.columns,
         terminal.rows, keyboard >= 0 ? "VNC and serial" : "serial");
  termios raw = saved;
  cfmakeraw(&raw);
  raw.c_lflag |= XV_PROXY;
  bool configured = handlers && tcsetattr(0, TCSANOW, &raw) == 0 &&
                    fcntl(0, F_SETFL, input_flags | O_NONBLOCK) == 0 &&
                    fcntl(1, F_SETFL, output_flags | O_NONBLOCK) == 0 &&
                    fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0;
  diagnostics = {};
  uint64 transfer_begin = timestamp();
  bool ok = configured && transfer(fds[0], framebuffer, keyboard);
  uint64 transfer_ticks = timestamp() - transfer_begin;
  if (keyboard >= 0)
  {
    close(keyboard);
  }
  close(fds[0]); // Hang up the inner foreground group before reaping the shell.
  fcntl(0, F_SETFL, input_flags);
  fcntl(1, F_SETFL, output_flags);
  tcsetattr(0, TCSANOW, &saved);
  if (waitpid(child, nullptr, WNOHANG) == 0)
  {
    kill(child);
    waitpid(child, nullptr, 0);
  }
  close(framebuffer);
  report(transfer_ticks);
  printf("fbterm: disconnected%s\n", ok ? "" : " (I/O error)");
  return ok ? 0 : 1;
}
