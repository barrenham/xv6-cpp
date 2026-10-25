export module xv.drivers:character;

export import :device;

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
};

class CharacterDevice : public Device
{
public:

  virtual int read(IoBuffer destination, int count) = 0;
  virtual int write(IoBuffer source, int count) = 0;
  virtual void put_sync(int character) = 0;

protected:

  constexpr CharacterDevice() = default;
  ~CharacterDevice() = default;
};
} // namespace xv::drivers
