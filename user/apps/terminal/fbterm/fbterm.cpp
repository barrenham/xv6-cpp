#include "user/runtime/ui.h"
#include "application.h"
#include "presenter.h"

using namespace fbterm_app;

extern "C" int
app_main(int argc, char **argv) {
  if (!configure_profile())
  {
    ui::error("fbterm: FBTERM_PROFILE must be normal, frames, no-mirror, no-present, "
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
      ui::heading(
          "usage: fbterm [--no-serial-mirror] [--] [command [args...]]\n"
          "       fbterm --demo | --benchmark-scroll\n"
          "PageUp/PageDown: history; mouse wheel: scroll; typing returns to live.\n"
          "VNC/serial keyboard; Ctrl-] exits. --no-serial-mirror keeps serial "
          "input, displays command output only on screen.\n");
      return 0;
    }
    else if (option[0] == '-')
    {
      ui::error("fbterm: unknown option %s\n", option);
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
    ui::error("fbterm: --demo does not take a command\n");
    return 1;
  }
  int framebuffer = open("/dev/fb0", O_RDWR);
  FramebufferInfo info{};
  if (framebuffer < 0 ||
      ioctl(framebuffer, XV_FB_GET_INFO, reinterpret_cast<unsigned long>(&info)) < 0 ||
      info.version != 1 || info.bytes_per_pixel != 4 || info.format != 2 ||
      info.stride != info.width * 4 || !terminal.configure(info.width, info.height))
  {
    ui::error(
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
      ui::error("fbterm: shared framebuffer mapping failed\n");
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
    ui::error("fbterm: cannot allocate screen/history buffers\n");
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
    ui::info("fbterm: benchmark start\n");
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
    ui::print(1, okay ? ui::Tone::Success : ui::Tone::Error, "fbterm: benchmark %s\n",
              okay ? "complete" : "failed");
    return okay ? 0 : 1;
  }
  if (show_demo)
  {
    bool ok = demo(framebuffer);
    close(framebuffer);
    ui::print(1, ok ? ui::Tone::Success : ui::Tone::Error, "fbterm: demo %s\n",
              ok ? "displayed" : "failed");
    return ok ? 0 : 1;
  }
  termios saved{};
  int input_flags = fcntl(0, F_GETFL), output_flags = fcntl(1, F_GETFL);
  if (input_flags < 0 || output_flags < 0 || tcgetattr(0, &saved) < 0 ||
      tcgetpgrp(0) != getpgrp())
  {
    ui::error("fbterm: foreground serial terminal required\n");
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
    ui::error("fbterm: PTY allocation or framebuffer draw failed\n");
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
    ui::error("fbterm: exec failed\n");
    exit(1);
  }
  close(fds[1]);
  if (child < 0)
  {
    close(fds[0]);
    close(framebuffer);
    ui::error("fbterm: fork failed\n");
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
  ui::info("fbterm: connected (%dx%d); %s keyboard, Ctrl-] exits\n", terminal.columns,
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
  ui::print(1, ok ? ui::Tone::Muted : ui::Tone::Error, "fbterm: disconnected%s\n",
            ok ? "" : " (I/O error)");
  return ok ? 0 : 1;
}
