#include "presenter.h"

namespace fbterm_app
{
fbterm::Snapshot render_snapshot{};
unsigned char render_wire[fbterm::Snapshot::wire_capacity]{};
Presenter presenter{};

bool
Presenter::start(int framebuffer, int master) {
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

bool
Presenter::enqueue() {
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

bool
Presenter::writing() const {
  return busy && sent_ < wire_size_;
}

bool
Presenter::progress() {
  if (writing())
  {
    int n = write(writer, render_wire + sent_, wire_size_ - sent_);
    if (n > 0)
    {
      sent_ += n;
    }
    else if (!(n < 0 && errno == EAGAIN))
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
    else if (!(n < 0 && errno == EAGAIN))
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

bool
Presenter::finish(bool dirty) {
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

void
Presenter::close_worker() {
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
} // namespace fbterm_app
