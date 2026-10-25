module;
#include "kernel/tty/termios.h"

export module xv.drivers:ldisc;
export import :serial;

export namespace xv::drivers
{
enum class InputAction
{
  None,
  Interrupt,
  Suspend
};

struct TerminalByte
{
  unsigned char value;
  bool end;
  bool eof;
};

// Caller holds the TTY state lock. No method may sleep; receive runs in IRQ
// context. Discipline storage is owned by the board, independently of the UART.
class LineDiscipline
{
public:

  LineDiscipline(const LineDiscipline &) = delete;
  LineDiscipline &operator=(const LineDiscipline &) = delete;
  virtual InputAction receive(unsigned char byte, const termios &settings,
                              SerialPort &output) = 0;
  virtual unsigned available() const = 0;
  virtual unsigned readable_bytes() const = 0;
  virtual TerminalByte front() const = 0;
  virtual void consume() = 0;
  virtual void flush() = 0;
  virtual void change_mode(bool canonical) = 0;

protected:

  constexpr LineDiscipline() = default;
  ~LineDiscipline() = default;
};
} // namespace xv::drivers
