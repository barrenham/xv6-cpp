module;
#include "kernel/tty/termios.h"

module xv.drivers:tty;
import :character;
import :ldisc;
import xv.sync;

namespace xv::drivers
{
// Session lookup only calls retain() under its spinlock. Completion happens
// after that lock is released, so PTY/TTY locks cannot invert the group lock.
class TerminalLifetime
{
public:
  virtual void retain_terminal()=0;
  virtual CharacterDevice *open_retained_terminal()=0;
protected:
  constexpr TerminalLifetime()=default;
  ~TerminalLifetime()=default;
};
class Tty final : public CharacterDevice, public InputReceiver
{
public:

  constexpr Tty(TerminalOutput &output, LineDiscipline &discipline,TerminalLifetime *lifetime=nullptr)
      : output_(output), discipline_(discipline), lifetime_(lifetime)
  {
  }

  const char *name() const override
  {
    return "ttyS0";
  }

  Tty *pin_terminal()
  {if(lifetime_)lifetime_->retain_terminal();return this;}
  CharacterDevice *open_pinned_terminal()
  {return lifetime_ ? lifetime_->open_retained_terminal():this;}
  void initialize() override;
  int read(IoBuffer destination, int count) override;
  int write(IoBuffer source, int count) override;
  int ioctl(unsigned request, IoBuffer argument) override;
  void put_sync(int character) override;
  void receive(int character) override;
  // Periodic board timer notification; does not maintain the system tick clock.
  void timer_tick();
  // Software input can wait for buffer space; UART receive must never sleep.
  int inject(unsigned char character, bool nonblocking = false);
  void hangup();
  unsigned poll(xv::sync::PollTable *table = nullptr) override;
  bool input_writable(xv::sync::PollTable *table = nullptr);

private:

  struct InjectFlow;
  struct PollFlow;
  struct ReadFlow;
  struct WriteFlow;
  struct IoctlFlow;

  TerminalOutput &output_;
  LineDiscipline &discipline_;
  TerminalLifetime *lifetime_;
  xv::sync::SpinLock lock_;
  xv::sync::SleepLock read_lock_;
  xv::sync::SleepLock write_lock_;
  termios settings_{};
  winsize size_{24, 80, 0, 0};
  unsigned generation_ = 0;
  bool timed_wait_ = false;
  bool hungup_ = false;
};
} // namespace xv::drivers
