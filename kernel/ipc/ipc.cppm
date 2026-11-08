module;
#include "kernel/net/socket/socket.h"
#include "kernel/io/mqueue.h"
#include "kernel/io/posix_errno.h"
export module xv.ipc;
export import xv.io;
import xv.vm_types;

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
  virtual int bind_path(const char *) { return -EOPNOTSUPP; }
  virtual int connect_path(const char *, bool) { return -EOPNOTSUPP; }
  virtual int listen(int) { return -EOPNOTSUPP; }
  virtual xv::io::IoObject *accept(bool, int &error)
  {
    error = -EOPNOTSUPP;
    return nullptr;
  }
  virtual int address(bool peer, char *path)
  {
    path[0] = 0;
    return peer && !connected() ? -ENOTCONN : 0;
  }

protected:

  constexpr LocalSocket() = default;
  ~LocalSocket() = default;
};

xv::io::IoObject *local_socket(int type);
bool socket_pair(int type, xv::io::IoObject *out[2]);
void release_rights(Rights &);
void start_socket_collector();
void request_socket_reclamation();
void collect_socket_cycles();
void collect_deferred_socket_cycles();
bool reclaim_named_sockets();
} // namespace xv::ipc

export namespace xv::ipc
{
class SharedMemory : public xv::io::Capability
{
public:

  static constexpr auto Id = xv::io::CapabilityId::SharedMemory;
  virtual bool writable() const = 0;
  virtual unsigned long length() const = 0;
  // The returned lease owns the object independently of names and open FDs.
  virtual xv::memory::MappingBacking *mapping(unsigned offset_pages) = 0;
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

export namespace xv::ipc
{
class MessageQueue : public xv::io::Capability
{
public:
  static constexpr auto Id = xv::io::CapabilityId::MessageQueue;
  virtual int send(unsigned long, unsigned long, unsigned, bool, unsigned long) = 0;
  virtual int receive(unsigned long, unsigned long, unsigned long, bool, unsigned long) = 0;
  virtual void attributes(mq_attr &) = 0;
protected:
  constexpr MessageQueue() = default;
  ~MessageQueue() = default;
};
xv::io::IoObject *queue_open(const char *, unsigned, unsigned, const mq_attr *, int &);
int queue_unlink(const char *);
}
