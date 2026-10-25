module;
#include "kernel/tty/termios.h"

module xv.drivers:n_tty;
import :ldisc;

namespace xv::drivers
{
class NttyDiscipline final : public LineDiscipline
{
public:

  constexpr NttyDiscipline() = default;
  InputAction receive(unsigned char byte, const termios &settings,
                      TerminalOutput &output) override;
  unsigned available() const override;
  bool input_room(bool canonical) const override;
  unsigned readable_bytes() const override;
  TerminalByte front() const override;
  void consume() override;
  void flush() override;
  void change_mode(bool canonical) override;

private:

  static constexpr unsigned Capacity = 128;
  TerminalByte ring_[Capacity]{};
  unsigned tail_ = 0;
  unsigned committed_ = 0;
  unsigned head_ = 0;
  bool literal_ = false;
  void erase(const termios &settings, TerminalOutput &output);
};
} // namespace xv::drivers
