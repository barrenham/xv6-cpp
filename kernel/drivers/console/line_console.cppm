module xv.drivers:console;

import :character;
import :serial;
import xv.sync;

namespace xv::drivers
{
class LineConsole final : public CharacterDevice, public InputReceiver
{
public:

  constexpr explicit LineConsole(SerialPort &serial) : serial_(serial)
  {
  }

  const char *name() const override
  {
    return "line console";
  }

  void initialize() override;
  int read(IoBuffer destination, int count) override;
  int write(IoBuffer source, int count) override;
  void put_sync(int character) override;
  void receive(int character) override;

private:

  static constexpr unsigned InputSize = 128;
  SerialPort &serial_;
  xv::sync::SpinLock lock_;
  char buffer_[InputSize]{};
  unsigned read_ = 0;
  unsigned written_ = 0;
  unsigned edited_ = 0;
};
} // namespace xv::drivers
