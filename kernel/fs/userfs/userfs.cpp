module;
#include "kernel/fs/userfs/protocol.h"
#include "kernel/fs/stat.h"
#include "kernel/fs/poll.h"
#include "kernel/fs/io_status.h"
#include "kernel/support/placement_new.hpp"
module xv.fs;
import :userfs;
import :platform;

namespace xv::fs
{
using xv::userfs::Operation;

void UserFileSystem::initialize_inodes()
{
  tree_.init("userfs metadata");
  serial_.init("userfs request");
  state_.init("userfs channel");
  references_.init("userfs references");
}

bool UserFileSystem::reserve(unsigned device)
{
  xv::sync::ScopedLock guard(state_);
  if (reserved_)
  {
    return false;
  }
  reserved_ = connected_ = endpoint_open_ = true;
  mounted_ = pending_ = delivered_ = replied_ = false;
  device_ = device;
  live_references_ = 1;
  for (auto *slot : nodes_)
  {
    auto &n = *slot;
    n.owner_ = this;
    n.ref = 0;
    n.inum = 0;
  }
  apply(nodes_[0], registration_.root);
  nodes_[0]->ref = 1; // Backend pin, independent of the VFS mount pin.
  return true;
}

void UserFileSystem::attached(bool value)
{
  xv::sync::ScopedLock guard(state_);
  mounted_ = value;
  mounting_ = false;
}

void UserFileSystem::apply(Inode *n, const xv::userfs::Attributes &a)
{
  n->owner_ = this;
  n->dev = device_;
  n->inum = a.node;
  n->type = a.type;
  n->size = a.size;
  n->uid_ = a.uid;
  n->gid_ = a.gid;
  n->mode_ = a.mode & 07777;
  n->nlink = a.links;
  n->major = a.major;
  n->minor = a.minor;
  n->valid = 1;
}

Inode *UserFileSystem::node(const xv::userfs::Attributes &a)
{
  if (!a.node || (a.type != T_DIR && a.type != T_FILE && a.type != T_SYMLINK &&
                  a.type != T_DEVICE && a.type != T_BLOCK) ||
      a.links > 32767 || a.mode > 07777 || a.major > 32767 || a.minor > 32767)
  {
    return nullptr;
  }
  {
    xv::sync::ScopedLock guard(references_);
    Inode *empty = nullptr;
    for (auto *slot : nodes_)
    {
      auto &n = *slot;
      if (n.inum == a.node)
      {
        apply(&n, a);
        ++n.ref;
        ++live_references_;
        return &n;
      }
      if (!n.ref && !empty)
      {
        empty = &n;
      }
    }
    if (empty)
    {
      apply(empty, a);
      empty->ref = 1;
      ++live_references_;
      return empty;
    }
  }
  // The server has handed us a reference, but all local slots are pinned.
  // Undo that reference without holding the spinlock. Callers own serial_.
  unsigned identity = a.node;
  begin(Operation::Forget, identity);
  (void)exchange();
  return nullptr;
}

Inode *UserFileSystem::root()
{
  return retain(nodes_[0]);
}

Inode *UserFileSystem::retain(Inode *n)
{
  xv::sync::ScopedLock guard(references_);
  ++n->ref;
  ++live_references_;
  return n;
}

void UserFileSystem::release(Inode *n)
{
  // Path operations sometimes release another inode while holding tree_.
  // Take the metadata lock before making the last reference reusable, so a
  // new lookup cannot race a Forget for the previous identity.
  bool held = tree_.holding();
  if (!held)
  {
    tree_.acquire();
  }
  references_.acquire();
  if (!n->ref)
  {
    detail::fail("userfs: reference underflow");
  }
  --n->ref;
  --live_references_;
  bool last = n->ref == 0;
  references_.release();
  if (last)
  {
    xv::sync::ScopedLock guard(serial_);
    begin(Operation::Forget, n->inum);
    (void)exchange();
    n->inum = 0;
  }
  if (!held)
  {
    tree_.release();
  }
}

bool UserFileSystem::busy()
{
  // A last close can still be sending Forget after dropping its inode ref.
  // Wait for that metadata operation before permitting instance retirement.
  xv::sync::ScopedLock metadata(tree_);
  xv::sync::ScopedLock guard(references_);
  return live_references_ != 2;
}

void UserFileSystem::begin(Operation operation, unsigned node)
{
  // Clear the complete wire buffer, including ABI padding, before delivery.
  detail::fill(&request_, 0, sizeof(request_));
  request_.version = xv::userfs::Version;
  request_.result = -1;
  request_.id = ++sequence_;
  request_.operation = operation;
  request_.node = node;
  auto caller = detail::identity();
  request_.context = {unsigned(detail::current_pid()), caller.uid, caller.gid};
}

bool UserFileSystem::exchange()
{
  const auto deadline = detail::monotonic_milliseconds() + 5000;
  state_.acquire();
  if (!connected_ || (!mounted_ && !mounting_) || detail::killed() ||
      !(registration_.operations & xv::userfs::operation_bit(request_.operation)))
  {
    state_.release();
    return false;
  }
  pending_ = true;
  delivered_ = replied_ = false;
  detail::wakeup(&pending_);
  while (connected_ && !replied_ && !detail::killed() &&
         detail::monotonic_milliseconds() < deadline)
  {
    if (!detail::wait_until(&replied_, state_, deadline))
    {
      break;
    }
  }
  bool success = connected_ && replied_;
  pending_ = false;
  // An interrupted/timed-out operation may already have modified server state.
  // Retire this connection instead of allowing a late reply to cross requests.
  if (!success)
  {
    disconnect();
  }
  state_.release();
  return success && response_.result >= 0;
}

void UserFileSystem::disconnect()
{
  connected_ = false;
  detail::wakeup(&pending_);
  detail::wakeup(&replied_);
}

void UserFileSystem::shutdown()
{
  xv::sync::ScopedLock guard(state_);
  mounted_ = false;
  completed_ = true;
  if (!endpoint_open_)
  {
    reserved_ = false;
  }
  pending_ = false;
  detail::wakeup(&pending_);
  // Keep the slot reserved until the last endpoint descriptor is closed.
}

int UserFileSystem::Endpoint::read(xv::io::Buffer out, int count)
{
  if (count != sizeof(xv::userfs::Message))
  {
    return -1;
  }
  auto &o = owner_;
  o.state_.acquire();
  while (o.connected_ && !o.completed_ && (!o.pending_ || o.delivered_))
  {
    if (out.nonblocking)
    {
      o.state_.release();
      return -EAGAIN;
    }
    if (detail::killed())
    {
      o.state_.release();
      return -1;
    }
    detail::prepare_interruptible_sleep(&o.pending_);
    o.state_.release();
    detail::sleep();
    o.state_.acquire();
  }
  int result = 0;
  if (o.connected_ && (o.mounted_ || o.mounting_) && o.pending_ && !o.delivered_)
  {
    result = detail::copy_out(true, out.address, &o.request_, count) < 0 ? -1 : count;
    if (result > 0)
    {
      o.delivered_ = true;
      detail::wakeup(&o.pending_);
    }
  }
  o.state_.release();
  return result;
}

int UserFileSystem::Endpoint::write(xv::io::Buffer in, int count)
{
  if (count != sizeof(xv::userfs::Message))
  {
    return -1;
  }
  auto &o = owner_;
  xv::sync::ScopedLock guard(o.state_);
  if (!o.connected_ || (!o.mounted_ && !o.mounting_) || !o.pending_ || !o.delivered_ ||
      o.replied_)
  {
    return -1;
  }
  if (detail::copy_in(&o.response_, true, in.address, count) < 0)
  {
    return -1;
  }
  auto &r = o.response_;
  if (r.version != xv::userfs::Version || r.id != o.request_.id ||
      r.operation != o.request_.operation || r.count > xv::userfs::DataSize ||
      r.result > int(xv::userfs::DataSize))
  {
    return -1;
  }
  o.replied_ = true;
  detail::wakeup(&o.replied_);
  detail::wakeup(&o.pending_);
  return count;
}

unsigned UserFileSystem::Endpoint::poll(xv::sync::PollTable *table)
{
  auto &o = owner_;
  xv::sync::ScopedLock guard(o.state_);
  if (table)
  {
    table->watch(&o.pending_);
  }
  if (!o.connected_ || o.completed_)
  {
    return POLLHUP;
  }
  return (o.pending_ && !o.delivered_ ? POLLIN : 0) |
         (o.pending_ && o.delivered_ && !o.replied_ ? POLLOUT : 0);
}

void UserFileSystem::Endpoint::close()
{
  auto &o = owner_;
  xv::sync::ScopedLock guard(o.state_);
  o.disconnect();
  o.endpoint_open_ = false;
  if (!o.mounted_)
  {
    o.reserved_ = false;
  }
}

Inode *UserFileSystem::lookup_entry(Inode *directory, const char *name,
                                    unsigned *offset)
{
  if (directory->type != T_DIR)
  {
    return nullptr;
  }
  if (offset)
  {
    *offset = 0;
  }
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Lookup, directory->inum);
  detail::copy_fixed(request_.name, name, sizeof(request_.name));
  return exchange() ? node(response_.attributes) : nullptr;
}

Inode *UserFileSystem::create_at(Inode *directory, const char *name, short type, short major,
                                 short minor)
{
  if (type != T_FILE && type != T_DIR && type != T_SYMLINK &&
      type != T_DEVICE && type != T_BLOCK)
  {
    return nullptr;
  }
  if (directory->type != T_DIR || !directory->linked() ||
      !detail::allowed(directory, 3))
  {
    return nullptr;
  }
  tree_.acquire();
  serial_.acquire();
  begin(Operation::Create, directory->inum);
  detail::copy_fixed(request_.name, name, sizeof(request_.name));
  auto identity = detail::identity();
  if ((type == T_DEVICE || type == T_BLOCK) && identity.uid)
  {
    serial_.release();
    tree_.release();
    return nullptr;
  }
  request_.attributes = {0,
                         0,
                         identity.uid,
                         identity.gid,
                         unsigned((type == T_DIR ? 0777 :
                                   (type == T_DEVICE || type == T_BLOCK) ? 0600 : 0666) & ~identity.mask),
                         unsigned(type),
                         1, unsigned(major), unsigned(minor)};
  auto *result = exchange() ? node(response_.attributes) : nullptr;
  serial_.release();
  if (!result)
  {
    tree_.release();
  }
  return result; // create_at returns a locked inode, matching existing backends.
}

int UserFileSystem::read(Inode *n, int user, unsigned long out, unsigned offset,
                         unsigned count)
{
  if (offset + count < offset)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(serial_);
  unsigned done = 0;
  while (done < count)
  {
    begin(Operation::Read, n->inum);
    request_.offset = offset + done;
    request_.count = count - done;
    if (request_.count > registration_.max_payload)
    {
      request_.count = registration_.max_payload;
    }
    if (!exchange() || response_.count > request_.count ||
        response_.result != int(response_.count) ||
        detail::copy_out(user, out + done, response_.data, response_.count) < 0)
    {
      return done ? int(done) : -1;
    }
    done += response_.count;
    if (response_.count < request_.count)
    {
      break;
    }
  }
  return done;
}

int UserFileSystem::write(Inode *n, int user, unsigned long in, unsigned offset,
                          unsigned count)
{
  if (offset + count < offset || n->type == T_DIR)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(serial_);
  unsigned char bytes[BlockSize];
  unsigned done = 0;
  while (done < count)
  {
    unsigned part = BlockSize - (offset + done) % BlockSize;
    if (part > count - done) part = count - done;
    unsigned copied = 0;
    while (copied < part)
    {
      unsigned take = part - copied;
      if (user)
      {
        unsigned tail = 4096 - (in + done + copied) % 4096;
        if (take > tail) take = tail;
      }
      if (detail::copy_in(bytes + copied, user, in + done + copied, take) < 0) break;
      copied += take;
    }
    bool fault = copied != part;
    unsigned publish = copied;
    if (fault)
    {
      // Preserve the existing block-write ABI: a faulted part contributes no
      // count or file growth, but valid overwrites of existing bytes survive.
      unsigned position = offset + done;
      unsigned existing = position < n->size ? n->size - position : 0;
      if (publish > existing) publish = existing;
    }
    unsigned sent = 0;
    while (sent < publish)
    {
      begin(Operation::Write, n->inum);
      request_.offset = offset + done + sent;
      request_.count = publish - sent;
      if (request_.count > registration_.max_payload) request_.count = registration_.max_payload;
      detail::move(request_.data, bytes + sent, request_.count);
      if (!exchange() || response_.result > int(request_.count))
      {
        unsigned progress = done + (fault ? 0 : sent);
        return progress ? int(progress) : -1;
      }
      unsigned take = response_.result;
      sent += take;
      if (offset + done + sent > n->size) n->size = offset + done + sent;
      if (take < request_.count) return fault ? (done ? int(done) : -1) : int(done + sent);
    }
    if (fault) return done ? int(done) : -1;
    done += part;
  }
  return done;
}

int UserFileSystem::resize(Inode *n, unsigned size)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Resize, n->inum);
  request_.attributes.size = size;
  if (!exchange())
  {
    return -1;
  }
  n->size = size;
  return 0;
}

void UserFileSystem::update(Inode *n)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Setattr, n->inum);
  request_.attributes = {n->inum,  n->size,           n->uid_,           n->gid_,
                         n->mode_, unsigned(n->type), unsigned(n->nlink)};
  (void)exchange();
}

void UserFileSystem::stat(Inode *n, Stat *out)
{
  *out = {};
  out->dev = device_;
  out->ino = n->inum;
  out->type = n->type;
  out->nlink = n->nlink;
  out->size = n->size;
  n->security_stat(out);
}

int UserFileSystem::next_directory(Inode *n, unsigned long &cookie,
                                   DirectoryRecord &entry)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Readdir, n->inum);
  request_.offset = cookie;
  if (!exchange() || response_.result > 1)
  {
    return -1;
  }
  if (!response_.result)
  {
    return 0;
  }
  if (!response_.attributes.node || response_.offset <= cookie || response_.name[255])
  {
    return -1;
  }
  entry = {};
  entry.inode = response_.attributes.node;
  entry.type = response_.attributes.type == T_DIR       ? 4
               : response_.attributes.type == T_SYMLINK ? 10
               : response_.attributes.type == T_DEVICE  ? 2
               : response_.attributes.type == T_BLOCK   ? 6
               : response_.attributes.type == T_SOCKET  ? 12 : 8;
  detail::copy_fixed(entry.name, response_.name, sizeof(entry.name));
  cookie = response_.offset;
  return 1;
}

int UserFileSystem::link_at(Inode *directory, const char *name, Inode *target)
{
  xv::sync::ScopedLock tree(tree_);
  xv::sync::ScopedLock guard(serial_);
  if (!detail::allowed(directory, 3) || target->kind() == T_DIR)
  {
    return -1;
  }
  begin(Operation::Link, directory->inum);
  request_.other = target->inum;
  detail::copy_fixed(request_.name, name, sizeof(request_.name));
  if (!exchange())
  {
    return -1;
  }
  ++target->nlink;
  return 0;
}

int UserFileSystem::unlink_at(Inode *directory, const char *name)
{
  xv::sync::ScopedLock tree(tree_);
  auto *target = lookup_entry(directory, name, nullptr);
  if (!target)
  {
    return -1;
  }
  bool permitted = detail::may_remove(directory, target);
  release(target);
  if (!permitted)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Unlink, directory->inum);
  detail::copy_fixed(request_.name, name, sizeof(request_.name));
  if (!exchange())
  {
    return -1;
  }
  for (auto *cached : nodes_)
  {
    if (cached->inum == response_.attributes.node)
    {
      cached->nlink = response_.attributes.links;
    }
  }
  return 0;
}

int UserFileSystem::rename_at(Inode *from, const char *old, Inode *to, const char *next)
{
  xv::sync::ScopedLock tree(tree_);
  auto *source = lookup_entry(from, old, nullptr);
  auto *target = lookup_entry(to, next, nullptr);
  bool permitted = source && detail::may_remove(from, source) &&
                   detail::allowed(to, 3) &&
                   (!target || detail::may_remove(to, target));
  if (source)
  {
    release(source);
  }
  if (target)
  {
    release(target);
  }
  if (!permitted)
  {
    return -1;
  }
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Rename, from->inum);
  request_.other = to->inum;
  detail::copy_fixed(request_.name, old, sizeof(request_.name));
  detail::copy_fixed(request_.target, next, sizeof(request_.target));
  return exchange() ? 0 : -1;
}

void UserFileSystem::flush()
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Sync);
  (void)exchange();
}
} // namespace xv::fs

namespace xv::fs
{
int UserFileSystem::status(Inode *n, Stat *out)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Getattr, n->inum);
  if (!exchange() || response_.attributes.node != n->inum ||
      response_.attributes.type != unsigned(n->type) ||
      response_.attributes.links > 32767 || response_.attributes.mode > 07777)
  {
    return -1;
  }
  apply(n, response_.attributes);
  stat(n, out);
  return 0;
}

int UserFileSystem::synchronize(Inode *n)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Sync, n->inum);
  return exchange() ? 0 : -1;
}

bool UserFileSystem::attributes(Inode *n, unsigned uid, unsigned gid, unsigned mode)
{
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Setattr, n->inum);
  request_.attributes = {n->inum,           n->size,           uid, gid, mode,
                         unsigned(n->type), unsigned(n->nlink)};
  if (!exchange())
  {
    return false;
  }
  n->uid_ = uid;
  n->gid_ = gid;
  n->mode_ = mode;
  return true;
}
} // namespace xv::fs

namespace xv::fs
{
UserFileSystem *UserFileSystem::create(unsigned device,
                                       const xv::userfs::Registration &registration,
                                       bool anonymous)
{
  static_assert(sizeof(UserFileSystem) <= 4096 && sizeof(NodePage) <= 4096);
  void *memory = detail::allocate_page();
  if (!memory)
  {
    return nullptr;
  }
  auto *fs = new (memory) UserFileSystem{};
  fs->initialize_inodes();
  fs->registration_ = registration;
  if (registration.flags & xv::userfs::ReadOnly)
  {
    fs->registration_.operations &= xv::userfs::ReadOperations |
        ((registration.flags & xv::userfs::Snapshots) ? xv::userfs::SnapshotOperations : 0);
  }
  fs->provider_pid_ = detail::current_pid();
  fs->anonymous_ = anonymous;
  unsigned allocated = 0;
  while (allocated < 64)
  {
    void *storage = detail::allocate_page();
    if (!storage)
    {
      fs->destroy();
      return nullptr;
    }
    auto *page = new (storage) NodePage{};
    page->next = fs->node_pages_;
    fs->node_pages_ = page;
    for (auto &node : page->nodes)
    {
      fs->nodes_[allocated++] = &node;
      if (allocated == 64)
      {
        break;
      }
    }
  }
  fs->reserve(device);
  return fs;
}

void UserFileSystem::destroy()
{
  while (node_pages_)
  {
    auto *page = node_pages_;
    node_pages_ = page->next;
    page->~NodePage();
    detail::free_page(page);
  }
  this->~UserFileSystem();
  detail::free_page(this);
}

bool UserFileSystem::claim_mount()
{
  xv::sync::ScopedLock guard(state_);
  if (!connected_ || !endpoint_open_ || mounted_ || mounting_ || completed_)
  {
    return false;
  }
  mounting_ = true;
  return true;
}

bool UserFileSystem::retired()
{
  xv::sync::ScopedLock guard(state_);
  return !endpoint_open_ && !mounted_ && !mounting_;
}

bool UserFileSystem::describe_provider(xv::userfs::ProviderInfo &info)
{
  xv::sync::ScopedLock guard(state_);
  if (!endpoint_open_ && !mounted_ && !mounting_)
  {
    return false;
  }
  info = {};
  detail::copy_fixed(info.type, registration_.type, sizeof(info.type));
  info.pid = provider_pid_;
  info.device = device_;
  info.version = registration_.version;
  info.flags = registration_.flags;
  info.state = mounting_    ? xv::userfs::ProviderState::Mounting
               : completed_ ? xv::userfs::ProviderState::Stopped
               : mounted_   ? (connected_ ? xv::userfs::ProviderState::Mounted
                                          : xv::userfs::ProviderState::Disconnected)
                            : xv::userfs::ProviderState::Registered;
  return true;
}
} // namespace xv::fs

namespace xv::fs
{
bool UserFileSystem::snapshot(Inode *node, PageBuffer &buffer)
{
  // One contiguous transaction keeps the backend capture stable. Each open
  // receives its own kernel-owned bytes, which outlive process exit and fd reuse.
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::SnapshotBegin, node->inum);
  bool good = exchange();
  unsigned size = good ? response_.attributes.size : 0;
  unsigned offset = 0;
  while (good && offset < size)
  {
    begin(Operation::SnapshotRead, node->inum);
    request_.offset = offset;
    request_.count = size - offset;
    if (request_.count > registration_.max_payload) request_.count = registration_.max_payload;
    good = exchange() && response_.result > 0 &&
           response_.result == int(response_.count) && response_.count <= request_.count &&
           buffer.append(response_.data, response_.count);
    if (good) offset += response_.count;
  }
  begin(Operation::SnapshotEnd, node->inum);
  bool ended = exchange();
  if (!good || !ended) buffer.clear();
  return good && ended;
}
} // namespace xv::fs
