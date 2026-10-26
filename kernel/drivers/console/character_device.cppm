export module xv.drivers:character;

export import :device;
export import xv.sync;

export namespace xv::drivers
{
enum class AddressSpace
{
  Kernel,
  User
};

struct IoBuffer
{
  AddressSpace space;
  unsigned long address;
  bool nonblocking = false;
};

class CharacterDevice : public Device
{
public:

  virtual CharacterDevice *open(bool) { return this; }
  virtual int read(IoBuffer destination, int count) = 0;
  virtual int write(IoBuffer source, int count) = 0;
  virtual void put_sync(int character) = 0;
  // Nonblocking readiness snapshot. Unsupported devices report no events.
  virtual unsigned poll(xv::sync::PollTable * = nullptr) { return 0; }

  // Called once when the final open-file reference disappears.
  virtual void close() {}

  virtual int ioctl(unsigned, IoBuffer)
  {
    return -1;
  }

protected:

  constexpr CharacterDevice() = default;
  ~CharacterDevice() = default;
};
} // namespace xv::drivers
