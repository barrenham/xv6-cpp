module;
#include "kernel/fs/userfs/protocol.h"
#include "kernel/io/fs_control.h"
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

void
UserFileSystem::initialize_inodes() {
  tree_.init("userfs metadata");
  serial_.init("userfs request");
  state_.init("userfs channel");
  references_.init("userfs references");
}

bool
UserFileSystem::reserve(unsigned device) {
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
    n.data_lock_.init("inode data");
    n.cache_state_.init("inode page cache");
    n.write_epoch_ = 0;
    n.ref = 0;
    n.inum = 0;
  }
  apply(nodes_[0], registration_.root);
  nodes_[0]->ref = 1; // Backend pin, independent of the VFS mount pin.
  return true;
}

void
UserFileSystem::attached(bool value) {
  xv::sync::ScopedLock guard(state_);
  mounted_ = value;
  mounting_ = false;
}

void
UserFileSystem::apply(Inode *n, const xv::userfs::Attributes &a) {
  bool held = n->data_lock_.holding();
  if (!held)
  {
    n->data_lock_.acquire();
  }
  // A lookup can finish before a concurrent write releases the data lock.
  // Keep the size committed by that later write instead of replaying stale attrs.
  bool newer_data = n->inum == a.node && n->write_epoch_ > request_epoch_;
  unsigned size = newer_data ? n->size : a.size;
  n->owner_ = this;
  n->dev = device_;
  n->inum = a.node;
  n->type = a.type;
  n->size = size;
  n->uid_ = a.uid;
  n->gid_ = a.gid;
  n->mode_ = a.mode & 07777;
  n->nlink = a.links;
  n->major = a.major;
  n->minor = a.minor;
  if (!newer_data)
  {
    __atomic_store_n(&n->metadata_valid_, true, __ATOMIC_RELEASE);
  }
  if (!held)
  {
    n->data_lock_.release();
  }
}

Inode *
UserFileSystem::node(const xv::userfs::Attributes &source) {
  auto attributes = source; // May alias response_, which a later RPC replaces.
  const auto &a = attributes;
  if (!a.node ||
      (a.type != T_DIR && a.type != T_FILE && a.type != T_SYMLINK &&
       a.type != T_DEVICE && a.type != T_BLOCK && a.type != T_SOCKET) ||
      a.links > 32767 || a.mode > 07777 || a.major > 32767 || a.minor > 32767)
  {
    return nullptr;
  }
  Inode *selected = nullptr, *empty = nullptr;
  references_.acquire();
  for (auto *n : nodes_)
  {
    if (n->inum == a.node)
    {
      selected = n;
      break;
    }
    if (!n->ref && !empty)
    {
      empty = n;
    }
  }
  if (!selected)
  {
    selected = empty;
  }
  if (selected)
  {
    ++selected->ref;
    ++live_references_;
  }
  references_.release();
  if (selected)
  {
    apply(selected, a);
    return selected;
  }
  begin(Operation::Forget, a.node);
  (void)exchange();
  return nullptr;
}

void
UserFileSystem::lock_data(Inode *node) {
  tree_.acquire();
  node->data_lock_.acquire();
  // Set-id clearing remains one metadata/data transaction. Ordinary file I/O
  // drops the instance metadata lock before waiting for its private RPC lane.
  if (node->type == T_FILE && !(node->mode_ & 06000))
  {
    tree_.release();
  }
}

void
UserFileSystem::lock_read(Inode *node) {
  tree_.acquire();
  if (node->type == T_FILE)
  {
    node->data_lock_.acquire_shared();
    tree_.release();
  }
  else
  {
    // Non-regular files retain the existing metadata/exclusive protection.
    node->data_lock_.acquire();
  }
}

void
UserFileSystem::unlock_read(Inode *node) {
  if (node->type == T_FILE)
  {
    node->data_lock_.release_shared();
  }
  else
  {
    node->data_lock_.release();
    tree_.release();
  }
}

void
UserFileSystem::unlock_data(Inode *node) {
  node->data_lock_.release();
  if (tree_.holding())
  {
    tree_.release();
  }
}

bool
UserFileSystem::operation_held() {
  if (tree_.holding() || serial_.holding())
  {
    return true;
  }
  xv::sync::ScopedLock guard(state_);
  int pid = detail::current_tid();
  for (const auto &slot : data_slots_)
  {
    if (slot.owner == pid)
    {
      return true;
    }
  }
  return false;
}

Inode *
UserFileSystem::root() {
  return retain(nodes_[0]);
}

Inode *
UserFileSystem::retain(Inode *n) {
  xv::sync::ScopedLock guard(references_);
  ++n->ref;
  ++live_references_;
  return n;
}

bool
UserFileSystem::pin_cached_inode(Inode *node) {
  if (references_.holding() || !references_.try_acquire())
  {
    return false;
  }
  bool pinned = node->ref > 0;
  if (pinned)
  {
    ++node->ref;
    ++live_references_;
  }
  references_.release();
  return pinned;
}

void
UserFileSystem::release(Inode *n) {
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
    discard_file_cache(n);
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

bool
UserFileSystem::busy() {
  // A last close can still be sending Forget after dropping its inode ref.
  // Wait for that metadata operation before permitting instance retirement.
  xv::sync::ScopedLock metadata(tree_);
  invalidate_lookup();
  xv::sync::ScopedLock guard(references_);
  return live_references_ != 2;
}

void
UserFileSystem::begin(Operation operation, unsigned node) {
  // Clear the complete wire buffer, including ABI padding, before delivery.
  detail::fill(&request_, 0, sizeof(request_));
  request_.version = xv::userfs::Version;
  request_.result = -1;
  request_.id = __atomic_add_fetch(&sequence_, 1UL, __ATOMIC_RELAXED);
  request_epoch_ = __atomic_load_n(&data_epoch_, __ATOMIC_ACQUIRE);
  request_.operation = operation;
  request_.node = node;
  auto caller = detail::identity();
  request_.context = {unsigned(detail::current_pid()), caller.uid, caller.gid};
}

bool
UserFileSystem::exchange() {
  __atomic_fetch_add(&statistics_.exchanges, 1UL, __ATOMIC_RELAXED);
  const auto deadline = detail::monotonic_milliseconds() + 5000;
  state_.acquire();
  if (!connected_ || (!mounted_ && !mounting_) ||
      !(registration_.operations & xv::userfs::operation_bit(request_.operation)))
  {
    state_.release();
    return false;
  }
  if (peer_)
  {
    auto *peer = files().retain(peer_);
    state_.release();
    __atomic_fetch_add(&statistics_.waits, 1UL, __ATOMIC_RELAXED);
    bool success = detail::exchange_transport(peer, &request_, &response_,
                                              sizeof(request_), deadline);
    files().release(peer);
    state_.acquire();
    success = success && connected_ && response_.version == xv::userfs::Version &&
              response_.id == request_.id &&
              response_.operation == request_.operation &&
              response_.count <= registration_.max_payload &&
              response_.result <= int(registration_.max_payload);
    if (!success)
    {
      disconnect();
    }
    state_.release();
    __atomic_fetch_add(&statistics_.copied_bytes, 2UL * sizeof(request_),
                       __ATOMIC_RELAXED);
    return success && response_.result >= 0;
  }
  pending_ = true;
  delivered_ = replied_ = false;
  detail::wakeup(&pending_);
  while (connected_ && !replied_ && detail::monotonic_milliseconds() < deadline)
  {
    __atomic_fetch_add(&statistics_.waits, 1UL, __ATOMIC_RELAXED);
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

void
UserFileSystem::disconnect() {
  connected_ = false;
  detail::cancel_transport(peer_);
  for (auto *peer : data_peers_)
  {
    detail::cancel_transport(peer);
  }
  detail::wakeup(data_slots_);
  detail::wakeup(&pending_);
  detail::wakeup(&replied_);
}

void
UserFileSystem::shutdown() {
  xv::sync::ScopedLock guard(state_);
  mounted_ = false;
  completed_ = true;
  detail::cancel_transport(peer_);
  for (auto *peer : data_peers_)
  {
    detail::cancel_transport(peer);
  }
  detail::wakeup(data_slots_);
  if (!endpoint_open_)
  {
    reserved_ = false;
  }
  pending_ = false;
  detail::wakeup(&pending_);
  // Keep the slot reserved until the last endpoint descriptor is closed.
}

int
UserFileSystem::bulk_copy(unsigned long address, unsigned bytes, bool user,
                          bool writing, unsigned offset) {
  state_.acquire();
  auto *bulk = bulk_ ? files().retain(bulk_) : nullptr;
  state_.release();
  if (!bulk)
  {
    return -1;
  }
  int result = detail::copy_transport(bulk, address, bytes, user, writing, offset);
  files().release(bulk);
  return result;
}

int
UserFileSystem::Endpoint::ioctl(unsigned request, xv::io::Buffer argument) {
  if (request != xv::userfs::BindTransport)
  {
    return -1;
  }
  unsigned header[2];
  if (detail::copy_in(header, true, argument.address, sizeof(header)) < 0)
  {
    return -1;
  }
  unsigned lanes = 1, payload = 0, cache_flags = 0;
  int sockets[xv::userfs::TransportLanes]{-1, -1, -1}, memory = -1;
  if (header[0] == sizeof(xv::userfs::TransportBinding) && header[1] == 1)
  {
    xv::userfs::TransportBinding binding;
    if (detail::copy_in(&binding, true, argument.address, sizeof(binding)) < 0 ||
        binding.reserved)
    {
      return -1;
    }
    payload = binding.payload;
    sockets[0] = binding.socket;
    memory = binding.memory;
  }
  else if (header[0] == sizeof(xv::userfs::TransportBindingV2) && header[1] == 2)
  {
    xv::userfs::TransportBindingV2 binding;
    if (detail::copy_in(&binding, true, argument.address, sizeof(binding)) < 0 ||
        binding.lanes < 2 || binding.lanes > xv::userfs::TransportLanes ||
        (binding.cache_flags & ~xv::userfs::CacheMetadata) || binding.reserved)
    {
      return -1;
    }
    lanes = binding.lanes;
    payload = binding.payload;
    memory = binding.memory;
    cache_flags = binding.cache_flags;
    for (unsigned i = 0; i < lanes; ++i)
    {
      sockets[i] = binding.sockets[i];
    }
  }
  else
  {
    return -1;
  }
  if (payload <= xv::userfs::DataSize || payload > xv::userfs::BulkSize)
  {
    return -1;
  }
  File *peers[xv::userfs::TransportLanes]{}, *bulk = nullptr;
  bool valid = true;
  for (unsigned i = 0; i < lanes; ++i)
  {
    File *buffer = nullptr;
    if (!detail::bind_transport(sockets[i], memory, payload * lanes, peers[i], buffer))
    {
      valid = false;
      break;
    }
    if (!i)
    {
      bulk = buffer;
    }
    else
    {
      valid = buffer == bulk;
      files().release(buffer);
      for (unsigned j = 0; j < i; ++j)
      {
        valid &= peers[j] != peers[i];
      }
      if (!valid)
      {
        break;
      }
    }
  }
  auto &o = owner_;
  o.state_.acquire();
  valid =
      valid && o.connected_ && !o.mounted_ && !o.mounting_ && !o.peer_ && !o.pending_;
  if (valid)
  {
    o.peer_ = peers[0];
    for (unsigned i = 1; i < lanes; ++i)
    {
      o.data_peers_[i - 1] = peers[i];
    }
    o.bulk_ = bulk;
    o.lanes_ = lanes;
    o.cache_flags_ = cache_flags;
    o.concurrency_.lanes = lanes;
    o.registration_.max_payload = payload;
    o.statistics_.payload = payload;
  }
  o.state_.release();
  if (!valid)
  {
    for (auto *peer : peers)
    {
      if (peer)
      {
        files().release(peer);
      }
    }
    if (bulk)
    {
      files().release(bulk);
    }
  }
  return valid ? 0 : -1;
}

int
UserFileSystem::Endpoint::read(xv::io::Buffer out, int count) {
  if (count != sizeof(xv::userfs::Message))
  {
    return -1;
  }
  auto &o = owner_;
  o.state_.acquire();
  if (o.peer_)
  {
    o.state_.release();
    return -1;
  }
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
      __atomic_fetch_add(&o.statistics_.copied_bytes, static_cast<unsigned long>(count),
                         __ATOMIC_RELAXED);
      o.delivered_ = true;
      detail::wakeup(&o.pending_);
    }
  }
  o.state_.release();
  return result;
}

int
UserFileSystem::Endpoint::write(xv::io::Buffer in, int count) {
  if (count != sizeof(xv::userfs::Message))
  {
    return -1;
  }
  auto &o = owner_;
  xv::sync::ScopedLock guard(o.state_);
  if (o.peer_ || !o.connected_ || (!o.mounted_ && !o.mounting_) || !o.pending_ ||
      !o.delivered_ || o.replied_)
  {
    return -1;
  }
  if (detail::copy_in(&o.response_, true, in.address, count) < 0)
  {
    return -1;
  }
  __atomic_fetch_add(&o.statistics_.copied_bytes, static_cast<unsigned long>(count),
                     __ATOMIC_RELAXED);
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

unsigned
UserFileSystem::Endpoint::poll(xv::sync::PollTable *table) {
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

void
UserFileSystem::Endpoint::close() {
  auto &o = owner_;
  o.state_.acquire();
  o.disconnect();
  auto *peer = o.peer_, *bulk = o.bulk_;
  File *data[xv::userfs::TransportLanes - 1]{};
  for (unsigned i = 0; i < xv::userfs::TransportLanes - 1; ++i)
  {
    data[i] = o.data_peers_[i];
    o.data_peers_[i] = nullptr;
  }
  o.peer_ = o.bulk_ = nullptr;
  o.endpoint_open_ = false;
  if (!o.mounted_)
  {
    o.reserved_ = false;
  }
  o.state_.release();
  if (peer)
  {
    files().release(peer);
  }
  if (bulk)
  {
    files().release(bulk);
  }
  for (auto *channel : data)
  {
    if (channel)
    {
      files().release(channel);
    }
  }
}

struct UserFileSystem::LookupCache
{
  static constexpr unsigned Capacity = 12;

  struct Entry
  {
    Inode *parent = nullptr, *target = nullptr;
    unsigned long stamp = 0;
    char name[256]{};
  } entries[Capacity];

  unsigned long clock = 0;
};

bool
UserFileSystem::cache_active() {
  xv::sync::ScopedLock guard(state_);
  return connected_ && (mounted_ || mounting_) && !completed_;
}

Inode *
UserFileSystem::cached_lookup(Inode *parent, const char *name) {
  if (!lookup_cache_ || !cache_active())
  {
    return nullptr;
  }
  for (auto &entry : lookup_cache_->entries)
  {
    if (entry.parent == parent &&
        !detail::compare(entry.name, name, sizeof(entry.name)))
    {
      entry.stamp = ++lookup_cache_->clock;
      __atomic_fetch_add(&concurrency_.lookup_hits, 1UL, __ATOMIC_RELAXED);
      return retain(entry.target);
    }
  }
  return nullptr;
}

void
UserFileSystem::cache_lookup(Inode *parent, const char *name, Inode *target) {
  if (!(cache_flags_ & xv::userfs::CacheMetadata) || !target ||
      !detail::compare(name, ".", 256) || !detail::compare(name, "..", 256))
  {
    return;
  }
  static_assert(sizeof(LookupCache) <= 4096);
  if (!lookup_cache_)
  {
    auto *page = detail::allocate_page();
    if (!page)
    {
      return;
    }
    lookup_cache_ = new (page) LookupCache;
  }
  auto *selected = &lookup_cache_->entries[0];
  for (auto &entry : lookup_cache_->entries)
  {
    if (!entry.target || entry.stamp < selected->stamp)
    {
      selected = &entry;
    }
    if (!entry.target)
    {
      break;
    }
  }
  auto *old_parent = selected->parent, *old_target = selected->target;
  selected->parent = retain(parent);
  selected->target = retain(target);
  selected->stamp = ++lookup_cache_->clock;
  detail::copy_fixed(selected->name, name, sizeof(selected->name));
  // Never evict while holding serial_: the last reference sends Forget.
  if (old_target)
  {
    release(old_target);
    release(old_parent);
  }
}

void
UserFileSystem::invalidate_lookup() {
  if (cache_flags_ & xv::userfs::CacheMetadata)
  {
    for (auto *node : nodes_)
    {
      __atomic_store_n(&node->metadata_valid_, false, __ATOMIC_RELEASE);
    }
  }
  if (!lookup_cache_)
  {
    return;
  }
  for (auto &entry : lookup_cache_->entries)
  {
    auto *parent = entry.parent, *target = entry.target;
    entry = {};
    if (target)
    {
      release(target);
      release(parent);
    }
  }
}

void
UserFileSystem::cache_pressure() {
  if (!lookup_cache_)
  {
    return;
  }
  bool available = false;
  references_.acquire();
  for (auto *node : nodes_)
  {
    available |= !node->ref;
  }
  references_.release();
  if (!available)
  {
    invalidate_lookup();
  }
}

Inode *
UserFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset) {
  if (directory->type != T_DIR)
  {
    return nullptr;
  }
  if (offset)
  {
    *offset = 0;
  }
  if (auto *hit = cached_lookup(directory, name))
  {
    return hit;
  }
  cache_pressure();
  Inode *result;
  {
    xv::sync::ScopedLock guard(serial_);
    begin(Operation::Lookup, directory->inum);
    detail::copy_fixed(request_.name, name, sizeof(request_.name));
    result = exchange() ? node(response_.attributes) : nullptr;
  }
  cache_lookup(directory, name, result);
  return result;
}

Inode *
UserFileSystem::create_at(Inode *directory, const char *name, short type, short major,
                          short minor) {
  if (type != T_FILE && type != T_DIR && type != T_SYMLINK && type != T_DEVICE &&
      type != T_BLOCK && type != T_SOCKET)
  {
    return nullptr;
  }
  if (directory->type != T_DIR || !directory->linked() ||
      !detail::allowed(directory, 3))
  {
    return nullptr;
  }
  tree_.acquire();
  invalidate_lookup();
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
                         unsigned(((type == T_DIR || type == T_SOCKET)     ? 0777
                                   : (type == T_DEVICE || type == T_BLOCK) ? 0600
                                                                           : 0666) &
                                  ~identity.mask),
                         unsigned(type),
                         1,
                         unsigned(major),
                         unsigned(minor)};
  auto *result = exchange() ? node(response_.attributes) : nullptr;
  serial_.release();
  if (!result)
  {
    tree_.release();
  }
  if (result)
  {
    result->data_lock_.acquire();
  }
  return result; // Both inode data and metadata locks transfer to the caller.
}

class UserFileSystem::Request
{
public:

  Request(UserFileSystem &owner, bool parallel) : owner_(owner) {
    if (!parallel || owner_.lanes_ < 2)
    {
      owner_.serial_.acquire();
      request_ = &owner_.request_;
      response_ = &owner_.response_;
      valid_ = true;
      return;
    }
    const auto deadline = detail::monotonic_milliseconds() + 5000;
    owner_.state_.acquire();
    while (owner_.connected_ && owner_.mounted_ && !owner_.completed_)
    {
      for (unsigned i = 0; i + 1 < owner_.lanes_; ++i)
      {
        if (!owner_.data_slots_[i].owner)
        {
          lane_ = i + 1;
          owner_.data_slots_[i].owner = detail::current_tid();
          peer_ = files().retain(owner_.data_peers_[i]);
          bulk_ = files().retain(owner_.bulk_);
          break;
        }
      }
      if (lane_)
      {
        break;
      }
      if (detail::monotonic_milliseconds() >= deadline)
      {
        break;
      }
      __atomic_fetch_add(&owner_.concurrency_.lane_waits, 1UL, __ATOMIC_RELAXED);
      if (!detail::wait_until(owner_.data_slots_, owner_.state_, deadline))
      {
        break;
      }
    }
    owner_.state_.release();
    if (lane_)
    {
      auto *storage = detail::allocate_page();
      if (storage)
      {
        page_ = new (storage) Messages;
      }
      if (page_)
      {
        request_ = &page_->request;
        response_ = &page_->response;
        valid_ = true;
      }
    }
  }

  ~Request() {
    if (!lane_)
    {
      if (valid_)
      {
        owner_.serial_.release();
      }
      return;
    }
    // Keep the owner marker until descriptor callbacks finish: reclamation
    // must not recursively enter metadata while this operation holds data_lock_.
    files().release(peer_);
    files().release(bulk_);
    owner_.state_.acquire();
    owner_.data_slots_[lane_ - 1].owner = 0;
    detail::wakeup(owner_.data_slots_);
    owner_.state_.release();
    if (page_)
    {
      page_->~Messages();
      detail::free_page(page_);
    }
  }

  bool
  valid() const {
    return valid_;
  }

  xv::userfs::Message &
  request() {
    return *request_;
  }

  xv::userfs::Message &
  response() {
    return *response_;
  }

  void
  begin(Operation operation, unsigned node) {
    if (!lane_)
    {
      owner_.begin(operation, node);
      return;
    }
    detail::fill(request_, 0, sizeof(*request_));
    request_->version = xv::userfs::Version;
    request_->result = -1;
    request_->id = __atomic_add_fetch(&owner_.sequence_, 1UL, __ATOMIC_RELAXED);
    request_->operation = operation;
    request_->node = node;
    auto caller = detail::identity();
    request_->context = {unsigned(detail::current_pid()), caller.uid, caller.gid};
  }

  bool
  exchange() {
    if (!lane_)
    {
      return owner_.exchange();
    }
    auto &statistics = owner_.statistics_;
    __atomic_fetch_add(&statistics.exchanges, 1UL, __ATOMIC_RELAXED);
    owner_.state_.acquire();
    bool active = owner_.connected_ && owner_.mounted_ && !owner_.completed_ &&
                  (owner_.registration_.operations &
                   xv::userfs::operation_bit(request_->operation));
    owner_.state_.release();
    if (!active)
    {
      return false;
    }
    auto &concurrency = owner_.concurrency_;
    __atomic_fetch_add(&concurrency.data_requests, 1UL, __ATOMIC_RELAXED);
    auto count = __atomic_add_fetch(&concurrency.in_flight, 1UL, __ATOMIC_ACQ_REL);
    auto maximum = __atomic_load_n(&concurrency.maximum_in_flight, __ATOMIC_RELAXED);
    while (count > maximum &&
           !__atomic_compare_exchange_n(&concurrency.maximum_in_flight, &maximum, count,
                                        false, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
    {
    }
    __atomic_fetch_add(&statistics.waits, 1UL, __ATOMIC_RELAXED);
    bool good =
        detail::exchange_transport(peer_, request_, response_, sizeof(*request_),
                                   detail::monotonic_milliseconds() + 5000);
    __atomic_fetch_sub(&concurrency.in_flight, 1UL, __ATOMIC_RELEASE);
    __atomic_fetch_add(&statistics.copied_bytes, 2UL * sizeof(*request_),
                       __ATOMIC_RELAXED);
    owner_.state_.acquire();
    good = good && owner_.connected_ && response_->version == xv::userfs::Version &&
           response_->id == request_->id &&
           response_->operation == request_->operation &&
           response_->count <= owner_.registration_.max_payload &&
           response_->result <= int(owner_.registration_.max_payload);
    if (!good)
    {
      owner_.disconnect();
    }
    owner_.state_.release();
    return good && response_->result >= 0;
  }

  int
  copy(unsigned long address, unsigned bytes, bool user, bool writing,
       unsigned offset = 0) {
    return lane_ ? detail::copy_transport(bulk_, address, bytes, user, writing,
                                          lane_ * owner_.statistics_.payload + offset)
                 : owner_.bulk_copy(address, bytes, user, writing, offset);
  }

  int
  publish(Inode *node, unsigned old_size, unsigned offset, unsigned count) {
    return owner_.cache_written(
        node, old_size, offset, count,
        [](void *opaque, void *output, unsigned begin, unsigned bytes) {
          auto &context = *static_cast<Request *>(opaque);
          if (context.owner_.statistics_.payload > xv::userfs::DataSize)
          {
            return context.copy(reinterpret_cast<unsigned long>(output), bytes, false,
                                false, begin);
          }
          detail::move(output, context.request_->data + begin, bytes);
          return 0;
        },
        this);
  }

private:

  struct Messages
  {
    xv::userfs::Message request, response;
  };

  static_assert(sizeof(Messages) <= 4096);
  UserFileSystem &owner_;
  Messages *page_ = nullptr;
  xv::userfs::Message *request_ = nullptr, *response_ = nullptr;
  File *peer_ = nullptr, *bulk_ = nullptr;
  unsigned lane_ = 0;
  bool valid_ = false;
};

int
UserFileSystem::read(Inode *node, int user, unsigned long address, unsigned offset,
                     unsigned count) {
  return read_impl(node, user, address, offset, count, false);
}

int
UserFileSystem::write(Inode *node, int user, unsigned long address, unsigned offset,
                      unsigned count) {
  return write_impl(node, user, address, offset, count, false);
}

int
UserFileSystem::read_data(Inode *node, int user, unsigned long address, unsigned offset,
                          unsigned count) {
  return read_impl(node, user, address, offset, count, node->kind() == T_FILE);
}

int
UserFileSystem::write_data(Inode *node, int user, unsigned long address,
                           unsigned offset, unsigned count) {
  return write_impl(node, user, address, offset, count, node->kind() == T_FILE);
}

int
UserFileSystem::write_cached_data(Inode *node, int user, unsigned long address,
                                  unsigned offset, unsigned count) {
  return write_impl(node, user, address, offset, count, node->kind() == T_FILE, true);
}

int
UserFileSystem::read_impl(Inode *n, int user, unsigned long out, unsigned offset,
                          unsigned count, bool parallel) {
  if (offset + count < offset)
  {
    return -1;
  }
  if (!count)
  {
    return 0;
  }
  Request context(*this, parallel);
  if (!context.valid())
  {
    return -1;
  }
  auto &request = context.request();
  auto &response = context.response();
  unsigned done = 0;
  while (done < count)
  {
    context.begin(Operation::Read, n->inum);
    request.offset = offset + done;
    request.count = count - done;
    if (request.count > registration_.max_payload)
    {
      request.count = registration_.max_payload;
    }
    if (!context.exchange() || response.count > request.count ||
        response.result != int(response.count) ||
        (statistics_.payload > xv::userfs::DataSize
             ? context.copy(out + done, response.count, user, false) < 0
             : detail::copy_out(user, out + done, response.data, response.count) < 0))
    {
      return done ? int(done) : -1;
    }
    done += response.count;
    if (response.count < request.count)
    {
      break;
    }
  }
  return done;
}

int
UserFileSystem::write_impl(Inode *n, int user, unsigned long in, unsigned offset,
                           unsigned count, bool parallel, bool publish) {
  if (offset + count < offset || n->type == T_DIR)
  {
    return -1;
  }
  if (!count)
  {
    return 0;
  }
  Request context(*this, parallel);
  if (!context.valid())
  {
    return -1;
  }
  auto &request = context.request();
  auto &response = context.response();
  if (statistics_.payload > xv::userfs::DataSize)
  {
    unsigned done = 0;
    while (done < count)
    {
      unsigned take = count - done;
      if (take > registration_.max_payload)
      {
        take = registration_.max_payload;
      }
      if (context.copy(in + done, take, user, true) < 0)
      {
        break;
      }
      context.begin(Operation::Write, n->inum);
      request.offset = offset + done;
      request.count = take;
      if (!context.exchange() || response.result > int(take))
      {
        n->write_epoch_ = __atomic_add_fetch(&data_epoch_, 1UL, __ATOMIC_RELEASE);
        __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
        return done ? int(done) : -1;
      }
      unsigned written = response.result;
      unsigned old_size = n->size;
      if (written)
      {
        n->write_epoch_ = __atomic_add_fetch(&data_epoch_, 1UL, __ATOMIC_RELEASE);
      }
      if (offset + done + written > n->size)
      {
        n->size = offset + done + written;
      }
      if (publish && written &&
          context.publish(n, old_size, offset + done, written) < 0)
      {
        __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
        return done ? int(done) : -1;
      }
      done += written;
      if (written < take)
      {
        return done;
      }
    }
    if (done == count)
    {
      return done;
    }
    // Continue at the first faulted batch using block-level legacy accounting.
    int tail = write_fault_blocks(context, n, user, in + done, offset + done,
                                  count - done, publish);
    return tail < 0 ? (done ? int(done) : -1) : int(done) + tail;
  }
  return write_fault_blocks(context, n, user, in, offset, count, publish);
}

int
UserFileSystem::write_fault_blocks(Request &context, Inode *n, int user,
                                   unsigned long in, unsigned offset, unsigned count,
                                   bool publish_cache) {
  auto &request = context.request();
  auto &response = context.response();
  unsigned char bytes[BlockSize];
  unsigned done = 0;
  while (done < count)
  {
    unsigned part = BlockSize - (offset + done) % BlockSize;
    if (part > count - done)
    {
      part = count - done;
    }
    unsigned copied = 0;
    while (copied < part)
    {
      unsigned take = part - copied;
      if (user)
      {
        unsigned tail = 4096 - (in + done + copied) % 4096;
        if (take > tail)
        {
          take = tail;
        }
      }
      if (detail::copy_in(bytes + copied, user, in + done + copied, take) < 0)
      {
        break;
      }
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
      if (publish > existing)
      {
        publish = existing;
      }
    }
    unsigned sent = 0;
    while (sent < publish)
    {
      context.begin(Operation::Write, n->inum);
      request.offset = offset + done + sent;
      request.count = publish - sent;
      if (request.count > registration_.max_payload)
      {
        request.count = registration_.max_payload;
      }
      if (statistics_.payload > xv::userfs::DataSize)
      {
        if (context.copy(reinterpret_cast<unsigned long>(bytes + sent), request.count,
                         false, true) < 0)
        {
          return done ? int(done) : -1;
        }
      }
      else
      {
        detail::move(request.data, bytes + sent, request.count);
      }
      if (!context.exchange() || response.result > int(request.count))
      {
        n->write_epoch_ = __atomic_add_fetch(&data_epoch_, 1UL, __ATOMIC_RELEASE);
        __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
        unsigned progress = done + (fault ? 0 : sent);
        return progress ? int(progress) : -1;
      }
      unsigned take = response.result;
      unsigned old_size = n->size;
      if (take)
      {
        n->write_epoch_ = __atomic_add_fetch(&data_epoch_, 1UL, __ATOMIC_RELEASE);
      }
      if (offset + done + sent + take > n->size)
      {
        n->size = offset + done + sent + take;
      }
      if (publish_cache && take &&
          context.publish(n, old_size, offset + done + sent, take) < 0)
      {
        __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
        return done ? int(done) : -1;
      }
      sent += take;
      if (take < request.count)
      {
        return fault ? (done ? int(done) : -1) : int(done + sent);
      }
    }
    if (fault)
    {
      return done ? int(done) : -1;
    }
    done += part;
  }
  return done;
}

int
UserFileSystem::resize(Inode *n, unsigned size) {
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Resize, n->inum);
  request_.attributes.size = size;
  if (!exchange())
  {
    __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
    return -1;
  }
  n->size = size;
  n->write_epoch_ = __atomic_add_fetch(&data_epoch_, 1UL, __ATOMIC_RELEASE);
  return 0;
}

void
UserFileSystem::stat(Inode *n, Stat *out) {
  *out = {};
  out->dev = device_;
  out->ino = n->inum;
  out->type = n->type;
  out->nlink = n->nlink;
  out->size = n->size;
  n->security_stat(out);
}

int
UserFileSystem::next_directory(Inode *n, unsigned long &cookie,
                               DirectoryRecord &entry) {
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
               : response_.attributes.type == T_SOCKET  ? 12
                                                        : 8;
  detail::copy_fixed(entry.name, response_.name, sizeof(entry.name));
  cookie = response_.offset;
  return 1;
}

int
UserFileSystem::link_at(Inode *directory, const char *name, Inode *target) {
  xv::sync::ScopedLock tree(tree_);
  invalidate_lookup();
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

int
UserFileSystem::unlink_at(Inode *directory, const char *name) {
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
  invalidate_lookup();
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

int
UserFileSystem::rename_at(Inode *from, const char *old, Inode *to, const char *next) {
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
  invalidate_lookup();
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::Rename, from->inum);
  request_.other = to->inum;
  detail::copy_fixed(request_.name, old, sizeof(request_.name));
  detail::copy_fixed(request_.target, next, sizeof(request_.target));
  return exchange() ? 0 : -1;
}

} // namespace xv::fs

namespace xv::fs
{
int
UserFileSystem::status(Inode *n, Stat *out) {
  if (!cache_active())
  {
    return -1;
  }
  if ((cache_flags_ & xv::userfs::CacheMetadata) &&
      __atomic_load_n(&n->metadata_valid_, __ATOMIC_ACQUIRE))
  {
    stat(n, out);
    return 0;
  }
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

int
UserFileSystem::synchronize(Inode *n) {
  bool tree_held = tree_.holding(), data_held = n->data_lock_.holding();
  if (!tree_held)
  {
    tree_.acquire();
  }
  if (!data_held)
  {
    n->data_lock_.acquire();
  }
  int result = 0;
  // A mount-level sync must publish mapped stores before asking the backend
  // to commit its filesystem journal. No serial RPC lock is held during flush.
  if (n == nodes_[0])
  {
    for (auto *node : nodes_)
    {
      if (!node->mapped_pages_)
      {
        continue;
      }
      bool held = node->data_lock_.holding();
      if (!held)
      {
        node->data_lock_.acquire();
      }
      result = flush_mapped(node);
      if (!held)
      {
        node->data_lock_.release();
      }
      if (result < 0)
      {
        break;
      }
    }
  }
  if (!result)
  {
    xv::sync::ScopedLock guard(serial_);
    begin(Operation::Sync, n->inum);
    if (!exchange())
    {
      __atomic_store_n(&n->metadata_valid_, false, __ATOMIC_RELEASE);
      result = -1;
    }
  }
  if (!data_held)
  {
    n->data_lock_.release();
  }
  if (!tree_held)
  {
    tree_.release();
  }
  return result;
}

bool
UserFileSystem::attributes(Inode *n, unsigned uid, unsigned gid, unsigned mode) {
  invalidate_lookup();
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
UserFileSystem *
UserFileSystem::create(unsigned device, const xv::userfs::Registration &registration,
                       bool anonymous) {
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
    fs->registration_.operations &=
        xv::userfs::ReadOperations |
        (registration.operations & xv::userfs::operation_bit(Operation::Control)) |
        ((registration.flags & xv::userfs::Snapshots) ? xv::userfs::SnapshotOperations
                                                      : 0);
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

void
UserFileSystem::destroy() {
  if (lookup_cache_)
  {
    lookup_cache_->~LookupCache();
    detail::free_page(lookup_cache_);
  }
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

bool
UserFileSystem::claim_mount() {
  xv::sync::ScopedLock guard(state_);
  if (!connected_ || !endpoint_open_ || mounted_ || mounting_ || completed_)
  {
    return false;
  }
  mounting_ = true;
  return true;
}

bool
UserFileSystem::retired() {
  xv::sync::ScopedLock guard(state_);
  return !endpoint_open_ && !mounted_ && !mounting_;
}

bool
UserFileSystem::describe_provider(xv::userfs::ProviderInfo &info) {
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
bool
UserFileSystem::snapshot(Inode *node, PageBuffer &buffer) {
  // One contiguous transaction keeps the backend capture stable. Each open
  // receives its own kernel-owned bytes, which outlive process exit and fd reuse.
  xv::sync::ScopedLock guard(serial_);
  begin(Operation::SnapshotBegin, node->inum);
  bool good = exchange();
  unsigned size = good ? response_.attributes.size : 0;
  unsigned offset = 0;
  unsigned char snapshot_bytes[xv::userfs::DataSize];
  while (good && offset < size)
  {
    begin(Operation::SnapshotRead, node->inum);
    request_.offset = offset;
    request_.count = size - offset;
    if (request_.count > registration_.max_payload)
    {
      request_.count = registration_.max_payload;
    }
    if (request_.count > xv::userfs::DataSize)
    {
      request_.count = xv::userfs::DataSize;
    }
    good = exchange() && response_.result > 0 &&
           response_.result == int(response_.count) &&
           response_.count <= request_.count &&
           (statistics_.payload > xv::userfs::DataSize
                ? bulk_copy(reinterpret_cast<unsigned long>(snapshot_bytes),
                            response_.count, false, false) >= 0 &&
                      buffer.append(snapshot_bytes, response_.count)
                : buffer.append(response_.data, response_.count));
    if (good)
    {
      offset += response_.count;
    }
  }
  begin(Operation::SnapshotEnd, node->inum);
  bool ended = exchange();
  if (!good || !ended)
  {
    buffer.clear();
  }
  return good && ended;
}
} // namespace xv::fs

namespace xv::fs
{
int
UserFileSystem::control(Inode *node, unsigned long address, unsigned access) {
  auto *page = detail::allocate_page();
  if (!page)
  {
    return -1;
  }
  auto &control = *static_cast<xv::fscontrol::Request *>(page);
  int result = -1;
  if (detail::copy_in(&control, true, address, sizeof(control)) == 0 &&
      control.size == sizeof(control) && control.version == xv::fscontrol::Version &&
      control.driver_version && control.interface_name[0] &&
      !control.interface_name[31] && control.input_size <= xv::userfs::DataSize &&
      control.input_size <= registration_.max_payload &&
      control.capacity <= xv::fscontrol::DataSize)
  {
    if (!detail::compare(control.interface_name, "xvfs.transport", 32))
    {
      if (control.driver_version == 1 && control.command == 1 && !control.input_size &&
          control.capacity >= sizeof(xv::userfs::TransportStatistics))
      {
        xv::userfs::TransportStatistics stats;
        stats.payload = statistics_.payload;
        stats.exchanges = __atomic_load_n(&statistics_.exchanges, __ATOMIC_RELAXED);
        stats.copied_bytes =
            __atomic_load_n(&statistics_.copied_bytes, __ATOMIC_RELAXED);
        stats.waits = __atomic_load_n(&statistics_.waits, __ATOMIC_RELAXED);
        detail::move(control.data, &stats, sizeof(stats));
        control.output_size = sizeof(stats);
        result = detail::copy_out(true, address, &control, sizeof(control));
      }
      if (control.driver_version == 1 && control.command == 2 && !control.input_size &&
          control.capacity >= sizeof(xv::userfs::ConcurrencyStatistics))
      {
        xv::userfs::ConcurrencyStatistics stats;
        stats.lanes = lanes_;
        stats.data_requests =
            __atomic_load_n(&concurrency_.data_requests, __ATOMIC_RELAXED);
        stats.lane_waits = __atomic_load_n(&concurrency_.lane_waits, __ATOMIC_RELAXED);
        stats.in_flight = __atomic_load_n(&concurrency_.in_flight, __ATOMIC_ACQUIRE);
        stats.maximum_in_flight =
            __atomic_load_n(&concurrency_.maximum_in_flight, __ATOMIC_RELAXED);
        stats.lookup_hits =
            __atomic_load_n(&concurrency_.lookup_hits, __ATOMIC_RELAXED);
        stats.read_hits = __atomic_load_n(&concurrency_.read_hits, __ATOMIC_RELAXED);
        detail::move(control.data, &stats, sizeof(stats));
        control.output_size = sizeof(stats);
        result = detail::copy_out(true, address, &control, sizeof(control));
      }
      detail::free_page(page);
      return result;
    }
    if (!(registration_.operations & xv::userfs::operation_bit(Operation::Control)))
    {
      detail::free_page(page);
      return -1;
    }
    invalidate_lookup();
    xv::sync::ScopedLock guard(serial_);
    begin(Operation::Control, node->inum);
    request_.other = control.command;
    request_.offset = control.driver_version;
    request_.count = control.input_size;
    request_.attributes.size = control.capacity < registration_.max_payload
                                   ? control.capacity
                                   : registration_.max_payload;
    request_.attributes.mode = access | (read_only() ? xv::fscontrol::ReadOnly : 0);
    detail::move(request_.name, control.interface_name, sizeof(control.interface_name));
    detail::move(request_.data, control.data, control.input_size);
    if (exchange() && response_.count <= request_.attributes.size &&
        response_.result == int(response_.count) &&
        response_.attributes.node == node->inum &&
        response_.attributes.type == unsigned(node->type) &&
        response_.attributes.links <= 32767 && response_.attributes.mode <= 07777 &&
        response_.attributes.major <= 32767 && response_.attributes.minor <= 32767)
    {
      apply(node, response_.attributes);
      control.output_size = response_.count;
      detail::fill(control.data, 0, sizeof(control.data));
      detail::move(control.data, response_.data, control.output_size);
      if (detail::copy_out(true, address, &control, sizeof(control)) == 0)
      {
        result = control.output_size;
      }
    }
  }
  detail::free_page(page);
  return result;
}
} // namespace xv::fs
