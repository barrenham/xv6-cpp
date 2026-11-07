#include "presenter.h"

namespace fbterm_app
{
bool
transfer(int master, int &keyboard, int &mouse) {
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
      if (bytes < 0 && !(bytes < 0 && errno == EAGAIN))
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
      if (n < 0 && !(n < 0 && errno == EAGAIN))
      {
        return false;
      }
      if ((n < 0 && errno == EAGAIN) && hungup)
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
      else if (!(n < 0 && errno == EAGAIN))
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
      if (n < 0 && !(n < 0 && errno == EAGAIN))
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
      if (bytes < 0 && !(bytes < 0 && errno == EAGAIN))
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
      else if (!(n < 0 && errno == EAGAIN))
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
} // namespace fbterm_app
