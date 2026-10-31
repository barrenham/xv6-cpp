module;
#include "kernel/io/stat.h"
export module xv.io;
export import xv.sync;
export namespace xv::io
{
using Metadata = ::stat;
enum class MetadataKind : short { Directory = T_DIR, Regular = T_FILE, Device = T_DEVICE, Socket = T_SOCKET, Block = T_BLOCK };
enum class CapabilityId { DeviceControl, Socket };
struct Buffer { unsigned long address; bool nonblocking = false; bool append = false; };
class Capability
{
protected:
  constexpr Capability() = default;
  ~Capability() = default;
};
class DeviceControl : public Capability
{
public:
  static constexpr CapabilityId Id = CapabilityId::DeviceControl;
  virtual int ioctl(unsigned request, Buffer argument) = 0;
protected:
  constexpr DeviceControl() = default;
  ~DeviceControl() = default;
};
class IoObject
{
public:
  IoObject(const IoObject &) = delete;
  IoObject &operator=(const IoObject &) = delete;
  virtual int read(Buffer destination, int count) = 0;
  virtual int write(Buffer source, int count) = 0;
  virtual unsigned poll(xv::sync::PollTable *table) = 0;
  virtual int getdents64(Buffer,int) {return -1;}
  virtual long seek(long, int) { return -1; }
  // Lock-free observation only; descriptor inspection must not acquire I/O locks.
  virtual unsigned long position() const {return 0;}
  virtual int resize(unsigned) { return -1; }
  virtual int sync() { return -1; }
  virtual int stat(Metadata &) { return -1; }
  virtual Capability *capability(CapabilityId) { return nullptr; }
  virtual void close() = 0;
protected:
  constexpr IoObject() = default;
  ~IoObject() = default;
};
template <class Interface> Interface *query(IoObject *object)
{
  auto *capability = object ? object->capability(Interface::Id) : nullptr;
  return capability ? static_cast<Interface *>(capability) : nullptr;
}
}
