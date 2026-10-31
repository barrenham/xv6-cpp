module xv.drivers:controlling;
import :tty;
import :platform;
namespace xv::drivers
{
class ControllingTerminal final : public CharacterDevice
{
public:
  const char *name() const override {return "controlling tty";}
  void initialize() override {}
  CharacterDevice *open(bool) override
  {
    auto *tty=static_cast<Tty *>(detail::pin_controlling_terminal(+[](void *terminal)->void *
      {return static_cast<Tty *>(terminal)->pin_terminal();}));
    return tty ? tty->open_pinned_terminal():nullptr;
  }
  int read(IoBuffer,int) override {return -1;}
  int write(IoBuffer,int) override {return -1;}
  void put_sync(int) override {}
};
}
