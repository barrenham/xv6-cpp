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
#include "navigation.h"
#include "snapshot.h"
#include "user/runtime/user.h"
#include "user/runtime/terminal_proxy.h"

namespace
{
fbterm::Terminal terminal{};
fbterm::Renderer renderer{};
// Allocate exactly one screen of pixels; never put it on the user stack.
unsigned *pixels = nullptr;
FramebufferInfo screen{};
FramebufferMapping mapping{};
bool mapped_buffer = false;
FramebufferMappedStats mapped_stats{}, mapped_stats_begin{};
volatile sig_atomic_t stopping = 0;

// Opt-in diagnostic modes; normal launches retain the production display path.
struct Diagnostics
{
  uint64 poll_ticks = 0, read_ticks = 0, feed_ticks = 0;
  uint64 paint_ticks = 0, draw_ticks = 0, mirror_ticks = 0;
  uint64 bytes = 0, mirrored = 0, pixel_bytes = 0, busy_bytes = 0;
  uint64 polls = 0, reads = 0, writes = 0, frames = 0, draws = 0;
  uint64 max_draw_ticks = 0, max_raster_ticks = 0;
} diagnostics;

struct FrameSample
{
  uint64 raster, draw, rows, reused;
};

FrameSample frame_samples[256]{};
FramebufferStats gpu_stats{};
FramebufferTiming gpu_timing{}, gpu_timing_begin{};
bool reference_raster = false;
uint64 sample_count = 0;

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
  const char *raster = getenv("FBTERM_RASTER");
  if (raster && strcmp(raster, "cached") && strcmp(raster, "full"))
  {
    return false;
  }
  reference_raster = raster && !strcmp(raster, "full");
  const char *damage = getenv("FBTERM_DAMAGE");
  renderer.full_damage = damage && !strcmp(damage, "full");
  if (!strcmp(profile_mode, "normal") || !strcmp(profile_mode, "frames"))
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
         "reads=%lu writes=%lu frames=%lu draws=%lu maxdraw=%lu maxraster=%lu "
         "rasterrows=%lu reusedrows=%lu busybytes=%lu\n",
         profile_mode, elapsed, diagnostics.poll_ticks, diagnostics.read_ticks,
         diagnostics.feed_ticks, diagnostics.paint_ticks - diagnostics.draw_ticks,
         diagnostics.draw_ticks, diagnostics.mirror_ticks, diagnostics.bytes,
         diagnostics.mirrored, diagnostics.pixel_bytes, diagnostics.polls,
         diagnostics.reads, diagnostics.writes, diagnostics.frames, diagnostics.draws,
         diagnostics.max_draw_ticks, diagnostics.max_raster_ticks,
         static_cast<uint64>(renderer.raster_rows),
         static_cast<uint64>(renderer.reused_rows), diagnostics.busy_bytes);
  printf("GPUSTATS interrupts=%lu waits=%lu bootpolls=%lu failed=%lu\n",
         static_cast<uint64>(gpu_stats.interrupts),
         static_cast<uint64>(gpu_stats.runtime_waits),
         static_cast<uint64>(gpu_stats.boot_polls),
         static_cast<uint64>(gpu_stats.failed_waits));
  printf("FBSTAGES move=%lu copy_ms=%lu transfer_ms=%lu flush_ms=%lu "
         "copied=%lu gpudraws=%lu\n",
         static_cast<uint64>(renderer.move_ticks),
         gpu_timing.copy_ms - gpu_timing_begin.copy_ms,
         gpu_timing.transfer_ms - gpu_timing_begin.transfer_ms,
         gpu_timing.flush_ms - gpu_timing_begin.flush_ms,
         gpu_timing.copied_bytes - gpu_timing_begin.copied_bytes,
         gpu_timing.draws - gpu_timing_begin.draws);
  printf("FBMAPPED enabled=%d presents=%lu uploaded=%lu scanouts=%lu\n", mapped_buffer,
         mapped_stats.presents - mapped_stats_begin.presents,
         mapped_stats.uploaded_bytes - mapped_stats_begin.uploaded_bytes,
         mapped_stats.scanout_changes - mapped_stats_begin.scanout_changes);
  if (!strcmp(profile_mode, "frames"))
  {
    uint64 first = sample_count > 256 ? sample_count - 256 : 0;
    for (uint64 i = first; i < sample_count; ++i)
    {
      const auto &sample = frame_samples[i % 256];
      printf("FBFRAME index=%lu raster=%lu draw=%lu rows=%lu reused=%lu\n", i,
             sample.raster, sample.draw, sample.rows, sample.reused);
    }
  }
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
  uint64 draw_before = diagnostics.draw_ticks, frames_before = diagnostics.frames;
  uint64 rows_before = renderer.raster_rows, reuse_before = renderer.reused_rows;
  auto submit = [&](unsigned top, unsigned height, unsigned *data)
  {
    unsigned width = reference_raster ? screen.width : renderer.damage_width;
    FramebufferUpdate update{
        0, top, width, height, screen.width * 4, 0, reinterpret_cast<uint64>(data)};
    if (profiling)
    {
      ++diagnostics.frames;
      if (reference_raster)
      {
        renderer.raster_rows +=
            (height + fbterm::cell_height - 1) / fbterm::cell_height;
      }
    }
    if (!present)
    {
      return true;
    }
    uint64 begin = timestamp();
    FramebufferPresent region{
        0, top, height ? width : 0, height, reference_raster ? 0 : renderer.origin_y,
        0};
    bool okay = mapped_buffer ? ioctl(framebuffer, XV_FB_PRESENT,
                                      reinterpret_cast<unsigned long>(&region)) == 0
                              : ioctl(framebuffer, XV_FB_DRAW,
                                      reinterpret_cast<unsigned long>(&update)) == 0;
    if (profiling)
    {
      uint64 duration = timestamp() - begin;
      diagnostics.draw_ticks += duration;
      ++diagnostics.draws;
      diagnostics.pixel_bytes += width * height * 4;
      if (duration > diagnostics.max_draw_ticks)
      {
        diagnostics.max_draw_ticks = duration;
      }
    }
    return okay;
  };
  bool okay =
      reference_raster
          ? fbterm::paint(terminal, screen.width, screen.height, pixels, submit)
          : renderer.paint(terminal, screen.width, screen.height, pixels, submit);
  if (profiling)
  {
    uint64 total = timestamp() - begin;
    diagnostics.paint_ticks += total;
    uint64 draw = diagnostics.draw_ticks - draw_before;
    uint64 raster = total - draw;
    if (raster > diagnostics.max_raster_ticks)
    {
      diagnostics.max_raster_ticks = raster;
    }
    if (diagnostics.frames != frames_before && !strcmp(profile_mode, "frames"))
    {
      frame_samples[sample_count++ % 256] = {raster, draw,
                                             renderer.raster_rows - rows_before,
                                             renderer.reused_rows - reuse_before};
    }
  }
  return okay;
}

fbterm::Snapshot render_snapshot{};
unsigned char render_wire[fbterm::Snapshot::wire_capacity]{};

// A renderer is the only process allowed to draw after start(). The I/O
// process sends at most one cell snapshot and continues servicing the PTY.
class Presenter
{
public:

  int writer = -1, reader = -1, pid = -1;
  bool busy = false;
  uint64 completed_cost = 0, completed_rows = 0;

  bool start(int framebuffer, int master)
  {
    framebuffer_ = framebuffer;
    const char *mode = profiling ? getenv("FBTERM_DISPLAY") : nullptr;
    synchronous_ = mode && !strcmp(mode, "sync");
    if (synchronous_)
    {
      return true;
    }
    int requests[2] = {-1, -1}, replies[2] = {-1, -1};
    if (pipe(requests) < 0 || pipe(replies) < 0)
    {
      for (int fd : requests)
      {
        if (fd >= 0)
        {
          close(fd);
        }
      }
      for (int fd : replies)
      {
        if (fd >= 0)
        {
          close(fd);
        }
      }
      return false;
    }
    pid = fork();
    if (pid == 0)
    {
      close(requests[1]);
      close(replies[0]);
      close(master);
      close(0);
      close(1);
      close(2);
      signal(SIGTERM, SIG_DFL);
      signal(SIGHUP, SIG_DFL);
      signal(SIGINT, SIG_IGN);
      diagnostics = {};
      sample_count = 0;
      renderer.raster_rows = renderer.reused_rows = renderer.move_ticks = 0;
      for (;;)
      {
        unsigned received = 0;
        unsigned target = sizeof(fbterm::Snapshot::Header);
        while (received < target)
        {
          int n = read(requests[0], render_wire + received, target - received);
          if (n <= 0)
          {
            exit(received ? 1 : 0);
          }
          received += n;
          if (received == sizeof(fbterm::Snapshot::Header))
          {
            fbterm::Snapshot::Header header{};
            memcpy(&header, render_wire, sizeof(header));
            if (header.bytes < sizeof(header) || header.bytes > sizeof(render_wire))
            {
              exit(1);
            }
            target = header.bytes;
          }
        }
        if (!render_snapshot.decode(render_wire, received, terminal.rows,
                                    terminal.columns))
        {
          exit(1);
        }
        render_snapshot.restore(terminal);
        uint64 begin = clockticks();
        bool okay = flush(framebuffer);
        reply_.cost = clockticks() - begin;
        reply_.okay = okay;
        reply_.diagnostics = diagnostics;
        reply_.raster_rows = renderer.raster_rows;
        reply_.reused_rows = renderer.reused_rows;
        reply_.move_ticks = renderer.move_ticks;
        reply_.sample =
            sample_count ? frame_samples[(sample_count - 1) % 256] : FrameSample{};
        reply_.sample_count = sample_count;
        if (write(replies[1], &reply_, sizeof(reply_)) != sizeof(reply_))
        {
          exit(1);
        }
        if (!okay)
        {
          exit(1);
        }
      }
    }
    close(requests[0]);
    close(replies[1]);
    if (pid < 0)
    {
      close(requests[1]);
      close(replies[0]);
      return false;
    }
    writer = requests[1];
    reader = replies[0];
    return fcntl(writer, F_SETFL, O_NONBLOCK) == 0 &&
           fcntl(reader, F_SETFL, O_NONBLOCK) == 0;
  }

  bool enqueue()
  {
    if (synchronous_)
    {
      uint64 begin = clockticks(), rows = renderer.raster_rows;
      bool okay = flush(framebuffer_);
      completed_cost = clockticks() - begin;
      completed_rows = renderer.raster_rows - rows;
      return okay;
    }
    render_snapshot.capture(terminal);
    // Scrolling after this snapshot belongs to the next batch. The renderer
    // owns a different Terminal, so its flush cannot clear the producer flag.
    terminal.scrolled = false;
    wire_size_ = render_snapshot.encode(render_wire, terminal.rows, terminal.columns);
    sent_ = received_ = 0;
    busy = true;
    return true;
  }

  bool writing() const
  {
    return busy && sent_ < wire_size_;
  }

  bool progress()
  {
    if (writing())
    {
      int n = write(writer, render_wire + sent_, wire_size_ - sent_);
      if (n > 0)
      {
        sent_ += n;
      }
      else if (n != -EAGAIN)
      {
        return false;
      }
    }
    if (busy)
    {
      int n = read(reader, reinterpret_cast<char *>(&reply_) + received_,
                   sizeof(reply_) - received_);
      if (n > 0)
      {
        received_ += n;
      }
      else if (n != -EAGAIN)
      {
        return false;
      }
      if (received_ == sizeof(reply_))
      {
        if (!reply_.okay)
        {
          return false;
        }
        busy = false;
        completed_cost = reply_.cost;
        completed_rows = reply_.raster_rows - renderer.raster_rows;
        // Parsing, mirroring and poll counters belong to this process.
        const auto &d = reply_.diagnostics;
        diagnostics.paint_ticks = d.paint_ticks;
        diagnostics.draw_ticks = d.draw_ticks;
        diagnostics.pixel_bytes = d.pixel_bytes;
        diagnostics.frames = d.frames;
        diagnostics.draws = d.draws;
        diagnostics.max_draw_ticks = d.max_draw_ticks;
        diagnostics.max_raster_ticks = d.max_raster_ticks;
        renderer.raster_rows = reply_.raster_rows;
        renderer.reused_rows = reply_.reused_rows;
        renderer.move_ticks = reply_.move_ticks;
        if (reply_.sample_count > sample_count)
        {
          frame_samples[sample_count++ % 256] = reply_.sample;
        }
      }
    }
    return true;
  }

  bool finish(bool dirty)
  {
    if (synchronous_)
    {
      return !dirty || flush(framebuffer_);
    }
    // Publish the last state on EOF/detach, even if an earlier frame is in flight.
    for (unsigned phase = 0; phase < (dirty ? 2U : 1U); ++phase)
    {
      if (phase)
      {
        if (!enqueue())
        {
          return false;
        }
      }
      uint64 deadline = clockticks() + 5000 * fbterm::FramePacer::Millisecond;
      while (busy)
      {
        pollfd fds[] = {{reader, POLLIN, 0}, {writing() ? writer : -1, POLLOUT, 0}};
        if (clockticks() >= deadline || poll(fds, 2, 100) < 0 || !progress())
        {
          return false;
        }
      }
    }
    return true;
  }

  void close_worker()
  {
    if (writer >= 0)
    {
      close(writer);
    }
    if (reader >= 0)
    {
      close(reader);
    }
    if (pid > 0)
    {
      if (waitpid(pid, nullptr, WNOHANG) == 0)
      {
        kill(pid);
      }
      waitpid(pid, nullptr, 0);
    }
  }

private:

  int framebuffer_ = -1;
  bool synchronous_ = false;
  unsigned wire_size_ = 0;

  struct Reply
  {
    Diagnostics diagnostics;
    FrameSample sample;
    uint64 raster_rows, reused_rows, move_ticks, sample_count, cost;
    bool okay;
  } reply_{};

  unsigned sent_ = 0, received_ = 0;
};

Presenter presenter{};

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

bool transfer(int master, int &keyboard, int &mouse)
{
  fbterm::Keyboard keymap{};
  fbterm::Navigation navigation{};
  KeyboardEvent key_events[32];
  char output[1024];
  unsigned char input[264], serial[256];
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
        {mouse, POLLIN, 0},
        {presenter.busy ? presenter.reader : -1, POLLIN, 0},
        {presenter.writing() ? presenter.writer : -1, POLLOUT, 0},
    };
    uint64 now = clockticks();
    if (input_sent == input_used && navigation.timeout(now) == 0)
    {
      input_used = navigation.flush(reinterpret_cast<char *>(input));
      input_sent = 0;
      terminal.live();
      needs_paint = true;
      pacer.output(now, 0, false);
      continue;
    }
    int timeout = presenter.busy ? -1 : pacer.timeout(now);
    int navigation_timeout = navigation.timeout(now);
    if (navigation_timeout >= 0 && (timeout < 0 || navigation_timeout < timeout))
    {
      timeout = navigation_timeout;
    }
    uint64 poll_begin = timestamp();
    int ready = poll(descriptors, 7, timeout);
    if (profiling)
    {
      diagnostics.poll_ticks += timestamp() - poll_begin;
      ++diagnostics.polls;
    }
    if (ready < 0)
    {
      return stopping;
    }
    bool was_busy = presenter.busy;
    if (!presenter.progress())
    {
      return false;
    }
    if (was_busy && !presenter.busy)
    {
      pacer.feedback(clockticks(), presenter.completed_cost,
                     presenter.completed_rows == terminal.rows);
    }
    if (descriptors[3].revents & (POLLERR | POLLNVAL | POLLHUP))
    {
      close(keyboard);
      keyboard = -1;
      descriptors[3].revents = 0;
    }
    if (descriptors[4].revents & (POLLERR | POLLNVAL | POLLHUP))
    {
      close(mouse);
      mouse = -1;
      descriptors[4].revents = 0;
    }
    if (descriptors[4].revents & POLLIN)
    {
      KeyboardEvent motion[32];
      int bytes = read(mouse, motion, sizeof(motion));
      if (bytes < 0 && bytes != -EAGAIN)
      {
        return false;
      }
      for (int i = 0; i < bytes / static_cast<int>(sizeof(KeyboardEvent)); ++i)
      {
        if (motion[i].type == XV_INPUT_REL && motion[i].code == XV_REL_WHEEL)
        {
          int steps = static_cast<int>(motion[i].value);
          steps = steps < -120 ? -120 : steps > 120 ? 120 : steps;
          terminal.scroll_history(steps * 3);
          needs_paint = true;
          pacer.output(clockticks(), 0, false);
        }
      }
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
        return presenter.finish(needs_paint);
      }
      if (n < 0 && n != -EAGAIN)
      {
        return false;
      }
      if (n == -EAGAIN && hungup)
      {
        return presenter.finish(needs_paint);
      }
      if (n > 0)
      {
        uint64 feed_begin = timestamp();
        terminal.feed(output, n);
        if (profiling)
        {
          diagnostics.feed_ticks += timestamp() - feed_begin;
          diagnostics.bytes += n;
          if (presenter.busy)
          {
            diagnostics.busy_bytes += n;
          }
        }
        needs_paint = true;
        pacer.output(clockticks(), n, terminal.scrolled);
        used = n;
        sent = mirror_output ? 0 : n;
      }
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
      int n = read(0, serial, sizeof(serial));
      if (n == 0)
      {
        return presenter.finish(needs_paint);
      }
      if (n < 0 && n != -EAGAIN)
      {
        return false;
      }
      if (n > 0)
      {
        input_used = input_sent = 0;
        uint64 input_time = clockticks();
        for (int i = 0; i < n; ++i)
        {
          if (terminal_proxy::detach_requested(master, serial[i]))
          {
            return presenter.finish(needs_paint);
          }
          char forwarded[4];
          int page = 0;
          unsigned length = navigation.feed(serial[i], input_time, forwarded, page);
          if (page)
          {
            terminal.scroll_history(page * static_cast<int>(terminal.rows - 1));
            needs_paint = true;
            pacer.output(clockticks(), 0, false);
          }
          if (length)
          {
            terminal.live();
            needs_paint = true;
            pacer.output(clockticks(), 0, false);
          }
          for (unsigned j = 0; j < length; ++j)
          {
            input[input_used++] = forwarded[j];
          }
        }
      }
    }
    if (input_sent == input_used && (descriptors[3].revents & POLLIN))
    {
      int bytes = read(keyboard, key_events, sizeof(key_events));
      if (bytes < 0 && bytes != -EAGAIN)
      {
        return false;
      }
      input_sent = 0;
      input_used = navigation.flush(reinterpret_cast<char *>(input));
      for (int i = 0; i < bytes / static_cast<int>(sizeof(KeyboardEvent)); ++i)
      {
        char translated[8];
        unsigned length = keymap.translate(key_events[i], translated);
        if (key_events[i].type == XV_KEY_EVENT && key_events[i].value &&
            (key_events[i].code == 104 || key_events[i].code == 109))
        {
          int page = terminal.rows - 1;
          terminal.scroll_history(key_events[i].code == 104 ? page : -page);
          needs_paint = true;
          pacer.output(clockticks(), 0, false);
          continue;
        }
        if (length)
        {
          terminal.live();
          needs_paint = true;
          pacer.output(clockticks(), 0, false);
        }
        for (unsigned j = 0; j < length; ++j)
        {
          if (terminal_proxy::detach_requested(master, translated[j]))
          {
            return presenter.finish(needs_paint);
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
    if (needs_paint && !presenter.busy && pacer.timeout(clockticks()) == 0)
    {
      uint64 start = clockticks();
      if (!presenter.enqueue())
      {
        return false;
      }
      needs_paint = false;
      uint64 end = clockticks();
      pacer.complete(start, end);
      if (!presenter.busy)
      {
        pacer.feedback(end, presenter.completed_cost,
                       presenter.completed_rows == terminal.rows);
      }
    }
  }
  return presenter.finish(needs_paint);
}
} // namespace

extern "C" int main(int argc, char **argv)
{
  if (!configure_profile())
  {
    fprintf(2, "fbterm: FBTERM_PROFILE must be normal, frames, no-mirror, no-present, "
               "no-paint or parse-only\n");
    return 1;
  }
  int first = 1;
  bool show_demo = false, benchmark = false;
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
    else if (!strcmp(option, "--benchmark-scroll"))
    {
      benchmark = true;
    }
    else if (!strcmp(option, "--demo"))
    {
      show_demo = true;
    }
    else if (!strcmp(option, "--help") || !strcmp(option, "-h"))
    {
      printf("usage: fbterm [--no-serial-mirror] [--] [command [args...]]\n"
             "       fbterm --demo | --benchmark-scroll\n"
             "PageUp/PageDown: history; mouse wheel: scroll; typing returns to live.\n"
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
  if ((show_demo || benchmark) && first < argc)
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
  renderer.clock = profiling ? timestamp : nullptr;
  if (profiling)
  {
    ioctl(framebuffer, XV_FB_GET_TIMING,
          reinterpret_cast<unsigned long>(&gpu_timing_begin));
    ioctl(framebuffer, XV_FB_GET_MAPPED_STATS,
          reinterpret_cast<unsigned long>(&mapped_stats_begin));
  }
  const char *buffer_mode = profiling ? getenv("FBTERM_BUFFER") : nullptr;
  bool copied_buffer = buffer_mode && !strcmp(buffer_mode, "copied");
  if (!copied_buffer)
  {
    mapped_buffer =
        ioctl(framebuffer, XV_FB_MAP, reinterpret_cast<unsigned long>(&mapping)) == 0;
    if (!mapped_buffer || mapping.version != 1 || !mapping.address ||
        mapping.backing_height < info.height * 2 ||
        mapping.bytes < static_cast<uint64>(info.stride) * mapping.backing_height)
    {
      fprintf(2, "fbterm: shared framebuffer mapping failed\n");
      close(framebuffer);
      return 1;
    }
    pixels = reinterpret_cast<unsigned *>(mapping.address);
    const char *scroll = profiling ? getenv("FBTERM_SCROLL") : nullptr;
    renderer.backing_height = !reference_raster && (!scroll || strcmp(scroll, "copied"))
                                  ? mapping.backing_height
                                  : 0;
  }
  else
  {
    pixels =
        static_cast<unsigned *>(malloc(info.width * info.height * sizeof(unsigned)));
  }
  auto *history = static_cast<fbterm::Cell(*)[fbterm::max_columns]>(malloc(
      fbterm::Terminal::history_capacity * fbterm::max_columns * sizeof(fbterm::Cell)));
  if (!pixels || !history)
  {
    fprintf(2, "fbterm: cannot allocate screen/history buffers\n");
    close(framebuffer);
    return 1;
  }
  terminal.set_history(history);
  if (benchmark)
  {
    for (unsigned i = 0; i < terminal.rows + 1; ++i)
    {
      const char *line = "scroll benchmark: unchanged row pixels 0123456789\r\n";
      terminal.feed(line, strlen(line));
    }
    if (!flush(framebuffer))
    {
      close(framebuffer);
      return 1;
    }
    printf("fbterm: benchmark start\n");
    ioctl(framebuffer, XV_FB_GET_TIMING,
          reinterpret_cast<unsigned long>(&gpu_timing_begin));
    ioctl(framebuffer, XV_FB_GET_MAPPED_STATS,
          reinterpret_cast<unsigned long>(&mapped_stats_begin));
    diagnostics = {};
    sample_count = 0;
    renderer.raster_rows = renderer.reused_rows = 0;
    uint64 begin = timestamp();
    bool okay = true;
    for (unsigned i = 0; i < 60 && okay; ++i)
    {
      const char *line = "scroll benchmark: unchanged row pixels 0123456789\r\n";
      terminal.feed(line, strlen(line));
      okay = flush(framebuffer);
    }
    uint64 elapsed = timestamp() - begin;
    if (profiling)
    {
      ioctl(framebuffer, XV_FB_GET_STATS, reinterpret_cast<unsigned long>(&gpu_stats));
    }
    ioctl(framebuffer, XV_FB_GET_TIMING, reinterpret_cast<unsigned long>(&gpu_timing));
    ioctl(framebuffer, XV_FB_GET_MAPPED_STATS,
          reinterpret_cast<unsigned long>(&mapped_stats));
    report(elapsed);
    close(framebuffer);
    printf("fbterm: benchmark %s\n", okay ? "complete" : "failed");
    return okay ? 0 : 1;
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
  bool display_ready = handlers && presenter.start(framebuffer, fds[0]);
  int keyboard = open("/dev/kbd0", O_RDONLY | O_NONBLOCK);
  int mouse = open("/dev/mouse0", O_RDONLY | O_NONBLOCK);
  printf("fbterm: connected (%dx%d); %s keyboard, Ctrl-] exits\n", terminal.columns,
         terminal.rows, keyboard >= 0 ? "VNC and serial" : "serial");
  termios raw = saved;
  cfmakeraw(&raw);
  raw.c_lflag |= XV_PROXY;
  bool configured = display_ready && tcsetattr(0, TCSANOW, &raw) == 0 &&
                    fcntl(0, F_SETFL, input_flags | O_NONBLOCK) == 0 &&
                    fcntl(1, F_SETFL, output_flags | O_NONBLOCK) == 0 &&
                    fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0;
  ioctl(framebuffer, XV_FB_GET_TIMING,
        reinterpret_cast<unsigned long>(&gpu_timing_begin));
  ioctl(framebuffer, XV_FB_GET_MAPPED_STATS,
        reinterpret_cast<unsigned long>(&mapped_stats_begin));
  diagnostics = {};
  sample_count = 0;
  renderer.raster_rows = renderer.reused_rows = 0;
  uint64 transfer_begin = timestamp();
  bool ok = configured && transfer(fds[0], keyboard, mouse);
  uint64 transfer_ticks = timestamp() - transfer_begin;
  presenter.close_worker();
  if (mouse >= 0)
  {
    close(mouse);
  }
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
  if (profiling)
  {
    ioctl(framebuffer, XV_FB_GET_STATS, reinterpret_cast<unsigned long>(&gpu_stats));
  }
  ioctl(framebuffer, XV_FB_GET_TIMING, reinterpret_cast<unsigned long>(&gpu_timing));
  ioctl(framebuffer, XV_FB_GET_MAPPED_STATS,
        reinterpret_cast<unsigned long>(&mapped_stats));
  close(framebuffer);
  report(transfer_ticks);
  printf("fbterm: disconnected%s\n", ok ? "" : " (I/O error)");
  return ok ? 0 : 1;
}
