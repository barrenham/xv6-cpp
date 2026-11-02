module;
#include "kernel/fs/poll.h"
module xv.fs;
import :file;
import :platform;

namespace xv::fs
{
namespace
{
class MemoryDevice final : public xv::drivers::CharacterDevice
{
public:
  enum class Kind {Null,Zero,Full};
  constexpr explicit MemoryDevice(Kind kind) : kind_(kind)
  {
  }

  const char *name() const override
  {
    return kind_==Kind::Null ? "null":kind_==Kind::Zero ? "zero":"full";
  }

  void initialize() override
  {
  }

  void put_sync(int) override
  {
  }

  int read(xv::drivers::IoBuffer destination, int count) override
  {
    if (count < 0)
    {
      return -1;
    }
    if (kind_==Kind::Null)
    {
      return 0;
    }
    const unsigned char zeros[128]{};
    int done = 0;
    while (done < count)
    {
      unsigned part = count - done;
      if (part > sizeof(zeros))
      {
        part = sizeof(zeros);
      }
      if (detail::copy_out(destination.space == xv::drivers::AddressSpace::User,
                           destination.address + done, zeros, part) < 0)
      {
        return done ? done : -1;
      }
      done += part;
    }
    return done;
  }

  int write(xv::drivers::IoBuffer, int count) override
  {
    return count < 0 || (kind_==Kind::Full && count) ? -1 : count;
  }

  unsigned poll(xv::sync::PollTable *) override
  {
    return POLLIN | POLLOUT;
  }

private:

  Kind kind_;
};

constinit MemoryDevice null_device(MemoryDevice::Kind::Null);
constinit MemoryDevice zero_device(MemoryDevice::Kind::Zero);
constinit MemoryDevice full_device(MemoryDevice::Kind::Full);
} // namespace

void initialize_device_registry()
{
  if (devices().get(2, 0)) return;
  devices().attach("null", 2, 0, null_device);
  devices().attach("zero", 3, 0, zero_device);
  devices().attach("full", 2, 1, full_device);
  devices().seal();
}
} // namespace xv::fs
