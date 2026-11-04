module;
#include "kernel/io/fs_mount.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"
module xv.fs;
import :file;
import :platform;

namespace xv::fs
{
namespace
{
class MountManager final
{
  class Endpoint final : public xv::io::IoObject
  {
  public:
    constexpr explicit Endpoint(MountManager &owner) : owner_(owner) {}

    int read(xv::io::Buffer out, int count) override
    {
      auto &o = owner_;
      if (count != sizeof(xv::fsmount::Request)) return -1;
      o.state_.acquire();
      if (detail::current_pid() != o.pid_) { o.state_.release(); return -1; }
      while (o.connected_ && (!o.pending_ || o.delivered_))
      {
        if (out.nonblocking || detail::killed())
        {
          o.state_.release();
          return out.nonblocking ? -EAGAIN : -1;
        }
        detail::prepare_interruptible_sleep(&o.pending_);
        o.state_.release();
        detail::sleep();
        o.state_.acquire();
      }
      int result = 0;
      if (o.connected_ && o.pending_ && !o.delivered_)
      {
        result = detail::copy_out(true, out.address, &o.request_, count) < 0 ? -1 : count;
        if (result > 0) o.delivered_ = true;
      }
      o.state_.release();
      return result;
    }

    int write(xv::io::Buffer in, int count) override
    {
      auto &o = owner_;
      if (count != sizeof(xv::fsmount::Request)) return -1;
      xv::fsmount::Request response;
      if (detail::copy_in(&response, true, in.address, count) < 0) return -1;
      xv::sync::ScopedLock guard(o.state_);
      if (detail::current_pid() != o.pid_ || !o.connected_ || !o.pending_ || !o.delivered_ || o.replied_ ||
          response.version != xv::fsmount::Version || response.id != o.request_.id ||
          response.pid != o.request_.pid ||
          (response.result != 0 && response.result != -1 && response.result != xv::fsmount::Progress)) return -1;
      if (response.result == xv::fsmount::Progress)
      {
        o.deadline_ = detail::monotonic_milliseconds() + 30000;
        detail::wakeup(&o.replied_);
        return count;
      }
      o.request_.result = response.result;
      o.replied_ = true;
      detail::wakeup(&o.replied_);
      detail::wakeup(&o.pending_);
      return count;
    }

    unsigned poll(xv::sync::PollTable *table) override
    {
      auto &o = owner_;
      xv::sync::ScopedLock guard(o.state_);
      if (table) table->watch(&o.pending_);
      if (!o.connected_) return POLLHUP;
      return (o.pending_ && !o.delivered_ ? POLLIN : 0) |
             (o.pending_ && o.delivered_ && !o.replied_ ? POLLOUT : 0);
    }

    void close() override
    {
      xv::sync::ScopedLock guard(owner_.state_);
      owner_.endpoint_open_ = false;
      owner_.disconnect();
    }

  private:
    MountManager &owner_;
  };

public:
  constexpr MountManager() : endpoint_(*this) {}
  void initialize()
  {
    state_.init("mount manager state");
    serial_.init("mount manager request");
  }
  bool claim()
  {
    {
      xv::sync::ScopedLock guard(state_);
      if (endpoint_open_) return false;
    }
    // Wait for any caller from a previous, closed connection to finish before
    // publishing a new server. The static endpoint cannot be freed underneath it.
    xv::sync::ScopedLock serial(serial_);
    xv::sync::ScopedLock guard(state_);
    if (endpoint_open_) return false;
    pid_ = detail::current_pid();
    connected_ = endpoint_open_ = true;
    pending_ = delivered_ = replied_ = false;
    return true;
  }
  xv::io::IoObject &endpoint() { return endpoint_; }
  bool available()
  {
    xv::sync::ScopedLock guard(state_);
    return connected_ && detail::current_pid() != pid_;
  }
  int request(const char *type, const char *path, const char *source, const char *options)
  {
    if (!type || !path) return -1;
    unsigned length = 0;
    while (length < sizeof(request_.path) && path[length]) ++length;
    if (length == sizeof(request_.path)) return -1;
    for (unsigned i = 0; source && source[i]; ++i) if (i + 1 >= sizeof(request_.source)) return -1;
    for (unsigned i = 0; options && options[i]; ++i) if (i + 1 >= sizeof(request_.options)) return -1;
    {
      xv::sync::ScopedLock guard(state_);
      if (!connected_ || detail::current_pid() == pid_) return -1;
    }
    // No namespace, inode, or registry lock is held across this RPC. In
    // particular the server can register a backend and call mount itself.
    xv::sync::ScopedLock serial(serial_);
    state_.acquire();
    if (!connected_ || detail::current_pid() == pid_ || detail::identity().uid ||
        detail::killed())
    {
      state_.release();
      return -1;
    }
    request_ = {};
    request_.id = ++sequence_;
    request_.pid = detail::current_pid();
    detail::copy_fixed(request_.type, type, sizeof(request_.type));
    detail::copy_fixed(request_.path, path, sizeof(request_.path));
    if (source) detail::copy_fixed(request_.source, source, sizeof(request_.source));
    if (options) detail::copy_fixed(request_.options, options, sizeof(request_.options));
    pending_ = true;
    delivered_ = replied_ = false;
    detail::wakeup(&pending_);
    deadline_ = detail::monotonic_milliseconds() + 30000;
    while (connected_ && !replied_ && !detail::killed() &&
           detail::monotonic_milliseconds() < deadline_)
    {
      const auto observed = deadline_;
      if (!detail::wait_until(&replied_, state_, observed) && deadline_ == observed) break;
    }
    bool success = connected_ && replied_;
    int result = success ? request_.result : -1;
    pending_ = false;
    // A canceled request may have mounted already; retire this channel so a
    // late answer cannot complete a different caller's request.
    if (!success) disconnect();
    detail::wakeup(&pending_);
    state_.release();
    return result;
  }

private:
  void disconnect()
  {
    connected_ = false;
    detail::wakeup(&pending_);
    detail::wakeup(&replied_);
  }
  Endpoint endpoint_;
  xv::sync::SpinLock state_;
  xv::sync::SleepLock serial_;
  xv::fsmount::Request request_{};
  unsigned sequence_ = 0;
  unsigned long deadline_ = 0;
  int pid_ = 0;
  bool connected_ = false, endpoint_open_ = false;
  bool pending_ = false, delivered_ = false, replied_ = false;
};
constinit MountManager manager;
}

void initialize_mount_manager() { manager.initialize(); }

File *create_mount_manager()
{
  auto *file = files().allocate();
  if (!file) return nullptr;
  if (!manager.claim()) { files().release(file); return nullptr; }
  files().bind_object(*file, manager.endpoint());
  return file;
}

int request_managed_mount(const char *type, const char *path, const char *source, const char *options)
{
  return manager.request(type, path, source, options);
}

bool managed_mounts_available() { return manager.available(); }
} // namespace xv::fs
