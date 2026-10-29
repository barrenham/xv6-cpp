module;
#include "kernel/tty/termios.h"
#include "kernel/support/flow.h"

module xv.drivers;
import :n_tty;

namespace xv::drivers
{
namespace
{
bool matches(unsigned char byte, const termios &settings, unsigned index)
{
  return settings.c_cc[index] != 0 && byte == settings.c_cc[index];
}

bool word_space(unsigned char byte)
{
  return byte == ' ' || byte == '\t';
}

void echo(unsigned char byte, const termios &settings, TerminalOutput &serial)
{
  if ((settings.c_lflag & ECHOCTL) && (byte < 32 || byte == 127) && byte != '\n' &&
      byte != '\t')
  {
    serial.put_sync('^');
    serial.put_sync(byte == 127 ? '?' : byte + '@');
    return;
  }
  if (byte == '\n' && (settings.c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR))
  {
    serial.put_sync('\r');
  }
  serial.put_sync(byte);
}
} // namespace

namespace flow = xv::flow;

struct NttyDiscipline::ReceiveFlow
{
  struct Context
  {
    NttyDiscipline &owner;
    unsigned char byte;
    const termios &settings;
    TerminalOutput &output;
    bool ignored = false, canonical = false, quoted = false, eof = false, end = false;
    InputAction result = InputAction::None;
  };

  static void translate(Context &c) noexcept
  {
    if (c.settings.c_iflag & ISTRIP)
    {
      c.byte &= 0x7f;
    }
    if (c.byte == '\r')
    {
      if (c.settings.c_iflag & IGNCR)
      {
        c.ignored = true;
        return;
      }
      if (c.settings.c_iflag & ICRNL)
      {
        c.byte = '\n';
      }
    }
    else if (c.byte == '\n' && (c.settings.c_iflag & INLCR))
    {
      c.byte = '\r';
    }
  }

  static bool ignored(Context &c) noexcept
  {
    return c.ignored;
  }

  static void state(Context &c) noexcept
  {
    c.canonical = c.settings.c_lflag & ICANON;
    c.quoted = c.owner.literal_;
    c.owner.literal_ = false;
  }

  static bool signal(Context &c) noexcept
  {
    return !c.quoted && (c.settings.c_lflag & ISIG) &&
           (matches(c.byte, c.settings, VINTR) || matches(c.byte, c.settings, VSUSP));
  }

  static void post_signal(Context &c) noexcept
  {
    bool suspend =
        matches(c.byte, c.settings, VSUSP) && !matches(c.byte, c.settings, VINTR);
    c.owner.flush();
    if (c.settings.c_lflag & ECHO)
    {
      echo(c.byte, c.settings, c.output);
      echo('\n', c.settings, c.output);
    }
    c.result = suspend ? InputAction::Suspend : InputAction::Interrupt;
  }

  static bool editing(Context &c) noexcept
  {
    return c.canonical && !c.quoted;
  }

  static bool quoted_next(Context &c) noexcept
  {
    return (c.settings.c_lflag & IEXTEN) && matches(c.byte, c.settings, VLNEXT);
  }

  static void quote(Context &c) noexcept
  {
    c.owner.literal_ = true;
  }

  static bool erasing(Context &c) noexcept
  {
    return matches(c.byte, c.settings, VERASE);
  }

  static void erase(Context &c) noexcept
  {
    c.owner.erase(c.settings, c.output);
  }

  static bool killing(Context &c) noexcept
  {
    return matches(c.byte, c.settings, VKILL);
  }

  static void kill(Context &c) noexcept
  {
    if ((c.settings.c_lflag & (ECHO | ECHOE | ECHOKE)) == (ECHO | ECHOE | ECHOKE))
    {
      while (c.owner.head_ != c.owner.committed_)
      {
        c.owner.erase(c.settings, c.output);
      }
    }
    else
    {
      c.owner.head_ = c.owner.committed_;
      if (c.settings.c_lflag & ECHO)
      {
        echo(c.byte, c.settings, c.output);
        if (c.settings.c_lflag & ECHOK)
        {
          echo('\n', c.settings, c.output);
        }
      }
    }
  }

  static bool word_erasing(Context &c) noexcept
  {
    return (c.settings.c_lflag & IEXTEN) && matches(c.byte, c.settings, VWERASE);
  }

  static void word_erase(Context &c) noexcept
  {
    while (c.owner.head_ != c.owner.committed_ &&
           word_space(c.owner.ring_[(c.owner.head_ - 1) % Capacity].value))
    {
      c.owner.erase(c.settings, c.output);
    }
    while (c.owner.head_ != c.owner.committed_ &&
           !word_space(c.owner.ring_[(c.owner.head_ - 1) % Capacity].value))
    {
      c.owner.erase(c.settings, c.output);
    }
  }

  static void delimiter(Context &c) noexcept
  {
    c.eof = c.canonical && !c.quoted && matches(c.byte, c.settings, VEOF);
    c.end = c.canonical && !c.quoted &&
            (c.eof || c.byte == '\n' || matches(c.byte, c.settings, VEOL) ||
             ((c.settings.c_lflag & IEXTEN) && matches(c.byte, c.settings, VEOL2)));
  }

  static bool room(Context &c) noexcept
  {
    return c.owner.head_ - c.owner.tail_ < Capacity &&
           (!c.canonical || c.end || c.owner.head_ - c.owner.tail_ < Capacity - 1);
  }

  static void full(Context &c) noexcept
  {
    if (c.owner.committed_ != c.owner.tail_)
    {
      c.owner.literal_ = c.quoted;
      c.result = InputAction::Retry;
    }
  }

  static void publish(Context &c) noexcept
  {
    c.owner.ring_[c.owner.head_++ % Capacity] = {c.byte, c.end, c.eof};
    if (!c.eof && ((c.settings.c_lflag & ECHO) ||
                   (c.canonical && c.byte == '\n' && (c.settings.c_lflag & ECHONL))))
    {
      echo(c.byte, c.settings, c.output);
    }
    if (!c.canonical || c.end)
    {
      c.owner.committed_ = c.owner.head_;
    }
  }
  struct Translate;
  struct Ignored;
  struct State;
  struct Signal;
  struct Quote;
  struct Erase;
  struct Kill;
  struct WordErase;
  struct Delimiter;
  struct Full;
  struct Publish;
  using Graph = flow::graph<
      Translate,
      flow::types<
          flow::node<Translate, flow::action<&translate>,
                     flow::branch<flow::condition<&ignored>, flow::go<Ignored>,
                                  flow::go<State>>>,
          flow::node<Ignored, flow::noop, flow::stop>,
          flow::node<
              State, flow::action<&state>,
              flow::branch<
                  flow::condition<&signal>,
                  flow::go<Signal>,
                  flow::branch<
                      flow::condition<&editing>,
                      flow::branch<flow::condition<&quoted_next>, flow::go<Quote>,
                                   flow::branch<
                                       flow::condition<&erasing>, flow::go<Erase>,
                                       flow::branch<
                                           flow::condition<&killing>, flow::go<Kill>,
                                           flow::branch<flow::condition<&word_erasing>,
                                                        flow::go<WordErase>,
                                                        flow::go<Delimiter>>>>>,
                      flow::go<Delimiter>>>>,
          flow::node<Signal, flow::action<&post_signal>, flow::stop>,
          flow::node<Quote, flow::action<&quote>, flow::stop>,
          flow::node<Erase, flow::action<&erase>, flow::stop>,
          flow::node<Kill, flow::action<&kill>, flow::stop>,
          flow::node<WordErase, flow::action<&word_erase>, flow::stop>,
          flow::node<
              Delimiter, flow::action<&delimiter>,
              flow::branch<flow::condition<&room>, flow::go<Publish>, flow::go<Full>>>,
          flow::node<Full, flow::action<&full>, flow::stop>,
          flow::node<Publish, flow::action<&publish>, flow::stop>>>;
};

bool NttyDiscipline::input_room(bool canonical) const
{
  return head_ - tail_ < Capacity &&
         (!canonical || head_ - tail_ < Capacity - 1 || committed_ == tail_);
}

unsigned NttyDiscipline::available() const
{
  return committed_ - tail_;
}

unsigned NttyDiscipline::readable_bytes() const
{
  unsigned count = 0;
  for (unsigned i = tail_; i != committed_; ++i)
  {
    if (!ring_[i % Capacity].eof)
    {
      ++count;
    }
  }
  return count;
}

TerminalByte NttyDiscipline::front() const
{
  return ring_[tail_ % Capacity];
}

void NttyDiscipline::consume()
{
  ++tail_;
}

void NttyDiscipline::flush()
{
  tail_ = committed_ = head_ = 0;
  literal_ = false;
}

void NttyDiscipline::change_mode(bool canonical)
{
  // Preserve pending bytes across a mode switch, removing synthetic EOF slots.
  unsigned destination = tail_;
  for (unsigned i = tail_; i != head_; ++i)
  {
    auto byte = ring_[i % Capacity];
    if (!byte.eof)
    {
      byte.end = false;
      ring_[destination++ % Capacity] = byte;
    }
  }
  head_ = committed_ = destination;
  if (canonical && destination != tail_)
  {
    ring_[(destination - 1) % Capacity].end = true;
  }
  literal_ = false;
}

void NttyDiscipline::erase(const termios &settings, TerminalOutput &output)
{
  if (head_ == committed_)
  {
    return;
  }
  auto byte = ring_[--head_ % Capacity].value;
  if (!(settings.c_lflag & ECHO))
  {
    return;
  }
  if (settings.c_lflag & ECHOE)
  {
    unsigned width =
        ((settings.c_lflag & ECHOCTL) && (byte < 32 || byte == 127) && byte != '\t')
            ? 2
            : 1;
    // Tab display width depends on the emulator's cursor; erase visibly as one
    // cell. UTF-8 and tab-column tracking are outside this byte discipline.
    while (width--)
    {
      output.put_sync('\b');
      output.put_sync(' ');
      output.put_sync('\b');
    }
  }
  else
  {
    echo(settings.c_cc[VERASE], settings, output);
  }
}

InputAction NttyDiscipline::receive(unsigned char byte, const termios &settings,
                                    TerminalOutput &output)
{
  ReceiveFlow::Context context{*this, byte, settings, output};
  (void)ReceiveFlow::Graph::run<0>(context);
  return context.result;
}
} // namespace xv::drivers
