module;
#include "kernel/net/socket/socket.h"
export module xv.ipc;
export import xv.io;

export namespace xv::ipc
{
inline constexpr unsigned MaxVectors = 16;
inline constexpr unsigned MaxRights = 4;
inline constexpr unsigned MaxPacket = 3072;
// Internal bounded RPCs finish before delivering a client signal. Never accepted from user calls.
inline constexpr unsigned KernelWait = 1U << 31;

struct Rights
{
  void *files[MaxRights]{};
  unsigned count = 0;
};

class LocalSocket : public xv::io::Capability
{
public:

  static constexpr auto Id = xv::io::CapabilityId::LocalSocket;
  // iovec storage is kernel-owned; user selects the address space of its data.
  // Received rights are ordinary owned File references, released by the caller.
  virtual int send(const iovec *, unsigned, bool user, unsigned flags, const Rights &,
                   unsigned long deadline = 0) = 0;
  virtual int receive(const iovec *, unsigned, bool user, unsigned flags, Rights &,
                      unsigned &output_flags, unsigned long deadline = 0) = 0;
  virtual int shutdown(int how) = 0;
  virtual void cancel() = 0;
  virtual bool connected() const = 0;
  virtual int type() const = 0;
  virtual void attach_file(void *) = 0;

protected:

  constexpr LocalSocket() = default;
  ~LocalSocket() = default;
};

bool socket_pair(int type, xv::io::IoObject *out[2]);
void release_rights(Rights &);
void collect_socket_cycles();
} // namespace xv::ipc

export namespace xv::ipc
{
class SharedMemory : public xv::io::Capability
{
public:

  static constexpr auto Id = xv::io::CapabilityId::SharedMemory;
  virtual bool writable() const = 0;
  virtual unsigned long length() const = 0;
  // pin() returns temporary physical-page references; callers release each.
  virtual int pin(unsigned offset_pages, unsigned count, void **pages) = 0;
  virtual int copy(unsigned long offset, void *buffer, unsigned count, bool user,
                   bool writing) = 0;

protected:

  constexpr SharedMemory() = default;
  ~SharedMemory() = default;
};

xv::io::IoObject *shared_open(const char *, unsigned flags, unsigned mode, int &error);
int shared_unlink(const char *);
} // namespace xv::ipc
