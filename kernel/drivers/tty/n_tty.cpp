module;
#include "kernel/tty/termios.h"

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

void echo(unsigned char byte, const termios &settings, SerialPort &serial)
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

void NttyDiscipline::erase(const termios &settings, SerialPort &output)
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

void NttyDiscipline::receive(unsigned char byte, const termios &settings,
                             SerialPort &output)
{
  if (settings.c_iflag & ISTRIP)
  {
    byte &= 0x7f;
  }
  if (byte == '\r')
  {
    if (settings.c_iflag & IGNCR)
    {
      return;
    }
    if (settings.c_iflag & ICRNL)
    {
      byte = '\n';
    }
  }
  else if (byte == '\n' && (settings.c_iflag & INLCR))
  {
    byte = '\r';
  }

  const bool canonical = settings.c_lflag & ICANON;
  const bool quoted = literal_;
  literal_ = false;
  if (canonical && !quoted)
  {
    if ((settings.c_lflag & IEXTEN) && matches(byte, settings, VLNEXT))
    {
      literal_ = true;
      return;
    }
    if (matches(byte, settings, VERASE))
    {
      erase(settings, output);
      return;
    }
    if (matches(byte, settings, VKILL))
    {
      if ((settings.c_lflag & (ECHO | ECHOE | ECHOKE)) == (ECHO | ECHOE | ECHOKE))
      {
        while (head_ != committed_)
        {
          erase(settings, output);
        }
      }
      else
      {
        head_ = committed_;
        if (settings.c_lflag & ECHO)
        {
          echo(byte, settings, output);
          if (settings.c_lflag & ECHOK)
          {
            echo('\n', settings, output);
          }
        }
      }
      return;
    }
    if ((settings.c_lflag & IEXTEN) && matches(byte, settings, VWERASE))
    {
      while (head_ != committed_ && word_space(ring_[(head_ - 1) % Capacity].value))
      {
        erase(settings, output);
      }
      while (head_ != committed_ && !word_space(ring_[(head_ - 1) % Capacity].value))
      {
        erase(settings, output);
      }
      return;
    }
  }
  bool eof = canonical && !quoted && matches(byte, settings, VEOF);
  bool end = canonical && !quoted &&
             (eof || byte == '\n' || matches(byte, settings, VEOL) ||
              ((settings.c_lflag & IEXTEN) && matches(byte, settings, VEOL2)));
  // Reserve the last slot for the line delimiter; overflow never publishes a
  // partial canonical line. Erase/kill remain usable while the ring is full.
  if (head_ - tail_ >= Capacity || (canonical && !end && head_ - tail_ >= Capacity - 1))
  {
    return;
  }
  ring_[head_++ % Capacity] = {byte, end, eof};
  if (!eof && ((settings.c_lflag & ECHO) ||
               (canonical && byte == '\n' && (settings.c_lflag & ECHONL))))
  {
    echo(byte, settings, output);
  }
  if (!canonical || end)
  {
    committed_ = head_;
  }
}
} // namespace xv::drivers
