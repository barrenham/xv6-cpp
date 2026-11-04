module;
#include "kernel/fs/fcntl.h"
#include "kernel/fs/poll.h"
#include "kernel/io/posix_errno.h"
#include "kernel/support/placement_new.hpp"
module xv.ipc;
import xv.memory;
import xv.process;
import xv.runtime;

namespace xv::ipc
{
namespace
{
constexpr unsigned PageBytes = 4096;
constexpr unsigned MaxPages = 256;
constexpr unsigned Names = 32;

struct Object
{
  xv::sync::SpinLock lock;
  void *pages[MaxPages]{};
  unsigned allocated = 0, bytes = 0;
  unsigned references = 0, uid = 0, gid = 0, mode = 0;
  bool linked = true;
  char name[256]{};
};

static_assert(sizeof(Object) <= PageBytes);
constinit xv::sync::SpinLock names_lock;
Object *objects[Names]{};
unsigned initialized = 0;

void
initialize() {
  unsigned expected = 0;
  if (__atomic_compare_exchange_n(&initialized, &expected, 1U, false, __ATOMIC_ACQ_REL,
                                  __ATOMIC_ACQUIRE))
  {
    names_lock.init("POSIX shared memory names");
    __atomic_store_n(&initialized, 2U, __ATOMIC_RELEASE);
  }
  while (__atomic_load_n(&initialized, __ATOMIC_ACQUIRE) != 2)
  {
  }
}

void
release(Object *object) {
  names_lock.acquire();
  bool last = --object->references == 0;
  names_lock.release();
  if (last)
  {
    for (unsigned i = 0; i < object->allocated; ++i)
    {
      xv::memory::allocator().release_page(object->pages[i]);
    }
    object->~Object();
    xv::memory::allocator().deallocate(object);
  }
}

int
resize(Object &object, unsigned bytes) {
  if (bytes > MaxPages * PageBytes)
  {
    return -EFBIG;
  }
  unsigned needed = (bytes + PageBytes - 1) / PageBytes;
  unsigned allocated = object.allocated;
  while (allocated < needed)
  {
    void *page = xv::memory::allocator().allocate();
    if (!page)
    {
      while (allocated > object.allocated)
      {
        --allocated;
        xv::memory::allocator().release_page(object.pages[allocated]);
        object.pages[allocated] = nullptr;
      }
      return -ENOMEM;
    }
    xv::runtime::fill(page, 0, PageBytes);
    object.pages[allocated++] = page;
  }
  object.allocated = allocated;
  // Keep high-water pages until the object dies: existing mappings retain
  // their frames across shrink/grow, and newly exposed bytes are always zero.
  if (bytes < object.bytes)
  {
    unsigned offset = bytes;
    while (offset < object.allocated * PageBytes)
    {
      unsigned take = PageBytes - offset % PageBytes;
      xv::runtime::fill(static_cast<unsigned char *>(object.pages[offset / PageBytes]) +
                            offset % PageBytes,
                        0, take);
      offset += take;
    }
  }
  object.bytes = bytes;
  return 0;
}

class View final : public xv::io::IoObject, public SharedMemory
{
public:

  explicit View(Object &object, bool writable) : object_(object), writable_(writable) {
    position_lock_.init_single_waiter("shared object offset");
  }

  xv::io::Capability *
  capability(xv::io::CapabilityId id) override {
    return id == SharedMemory::Id ? static_cast<SharedMemory *>(this) : nullptr;
  }

  bool
  writable() const override {
    return writable_;
  }

  unsigned long
  length() const override {
    xv::sync::ScopedLock guard(object_.lock);
    return object_.bytes;
  }

  unsigned long
  position() const override {
    return __atomic_load_n(&position_, __ATOMIC_ACQUIRE);
  }

  int
  resize(unsigned bytes) override {
    if (!writable_)
    {
      return -EBADF;
    }
    xv::sync::ScopedLock guard(object_.lock);
    return ipc::resize(object_, bytes);
  }

  int
  pin(unsigned offset, unsigned count, void **pages) override {
    xv::sync::ScopedLock guard(object_.lock);
    unsigned available = (object_.bytes + PageBytes - 1) / PageBytes;
    if (!count || offset > available || count > available - offset)
    {
      return -EINVAL;
    }
    for (unsigned i = 0; i < count; ++i)
    {
      pages[i] = object_.pages[offset + i];
      xv::memory::allocator().retain_page(pages[i]);
    }
    return 0;
  }

  int
  copy(unsigned long offset, void *buffer, unsigned count, bool user,
       bool writing) override {
    if (writing && !writable_)
    {
      return -EBADF;
    }
    xv::sync::ScopedLock guard(object_.lock);
    if (offset > object_.bytes || count > object_.bytes - offset)
    {
      return -EINVAL;
    }
    unsigned done = 0;
    while (done < count)
    {
      unsigned at = offset + done;
      unsigned take = PageBytes - at % PageBytes;
      if (take > count - done)
      {
        take = count - done;
      }
      auto *physical =
          static_cast<unsigned char *>(object_.pages[at / PageBytes]) + at % PageBytes;
      auto address = reinterpret_cast<unsigned long>(buffer) + done;
      int result = writing ? xv::process::copy_in(physical, user, address, take)
                           : xv::process::copy_out(user, address, physical, take);
      if (result < 0)
      {
        return done ? int(done) : -EFAULT;
      }
      done += take;
    }
    return done;
  }

  int
  read(xv::io::Buffer buffer, int count) override {
    if (count < 0)
    {
      return -EINVAL;
    }
    xv::sync::ScopedLock guard(position_lock_);
    auto bytes = length();
    if (position_ >= bytes)
    {
      return 0;
    }
    if (unsigned(count) > bytes - position_)
    {
      count = bytes - position_;
    }
    int done =
        copy(position_, reinterpret_cast<void *>(buffer.address), count, true, false);
    if (done > 0)
    {
      __atomic_fetch_add(&position_, unsigned(done), __ATOMIC_RELEASE);
    }
    return done;
  }

  int
  write(xv::io::Buffer buffer, int count) override {
    if (count < 0)
    {
      return -EINVAL;
    }
    if (!writable_)
    {
      return -EBADF;
    }
    xv::sync::ScopedLock guard(position_lock_);
    if (position_ > MaxPages * PageBytes ||
        unsigned(count) > MaxPages * PageBytes - position_)
    {
      return -EFBIG;
    }
    if (position_ + count > length())
    {
      int error = resize(position_ + count);
      if (error < 0)
      {
        return error;
      }
    }
    int done =
        copy(position_, reinterpret_cast<void *>(buffer.address), count, true, true);
    if (done > 0)
    {
      __atomic_fetch_add(&position_, unsigned(done), __ATOMIC_RELEASE);
    }
    return done;
  }

  long
  seek(long delta, int origin) override {
    xv::sync::ScopedLock guard(position_lock_);
    if (origin < SEEK_SET || origin > SEEK_END)
    {
      return -EINVAL;
    }
    long start = origin == SEEK_SET   ? 0
                 : origin == SEEK_CUR ? long(position_)
                                      : long(length());
    if (delta < -start || delta > long(MaxPages * PageBytes) - start)
    {
      return -EINVAL;
    }
    __atomic_store_n(&position_, start + delta, __ATOMIC_RELEASE);
    return start + delta;
  }

  unsigned
  poll(xv::sync::PollTable *) override {
    return POLLIN | (writable_ ? POLLOUT : 0);
  }

  int
  sync() override {
    return 0;
  }

  int
  stat(xv::io::Metadata &value) override {
    xv::sync::ScopedLock guard(object_.lock);
    value = {};
    value.type = static_cast<short>(xv::io::MetadataKind::Regular);
    value.nlink = __atomic_load_n(&object_.linked, __ATOMIC_ACQUIRE) ? 1 : 0;
    value.size = object_.bytes;
    value.uid = object_.uid;
    value.gid = object_.gid;
    value.mode = object_.mode;
    return 0;
  }

  void
  close() override {
    auto *object = &object_;
    this->~View();
    xv::memory::allocator().deallocate(this);
    release(object);
  }

private:

  Object &object_;
  bool writable_;
  unsigned long position_ = 0;
  xv::sync::SleepLock position_lock_;
};

static_assert(sizeof(View) <= PageBytes);

bool
valid_name(const char *name) {
  if (name[0] != '/' || !name[1])
  {
    return false;
  }
  for (unsigned i = 1; name[i]; ++i)
  {
    if (name[i] == '/')
    {
      return false;
    }
  }
  return true;
}
} // namespace

xv::io::IoObject *
shared_open(const char *name, unsigned flags, unsigned mode, int &error) {
  error = -EINVAL;
  if (!valid_name(name) ||
      (flags & ~(O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_CLOEXEC)) ||
      ((flags & O_ACCMODE) != O_RDONLY && (flags & O_ACCMODE) != O_RDWR))
  {
    return nullptr;
  }
  bool writable = (flags & O_ACCMODE) == O_RDWR;
  if ((flags & O_TRUNC) && !writable)
  {
    return nullptr;
  }
  initialize();
  auto credentials = xv::process::current()->credentials().snapshot();
  names_lock.acquire();
  Object *object = nullptr;
  unsigned slot = Names;
  for (unsigned i = 0; i < Names; ++i)
  {
    if (objects[i] && !xv::runtime::compare_text(objects[i]->name, name, 256))
    {
      object = objects[i];
    }
    if (!objects[i] && slot == Names)
    {
      slot = i;
    }
  }
  if (object)
  {
    if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    {
      error = -EEXIST;
    }
    else
    {
      bool group = object->gid == credentials.effective_gid;
      for (unsigned i = 0; i < credentials.group_count; ++i)
      {
        group |= object->gid == credentials.groups[i];
      }
      unsigned shift = object->uid == credentials.effective_uid ? 6 : group ? 3 : 0;
      unsigned requested = writable ? 6 : 4;
      if (credentials.effective_uid &&
          (((object->mode >> shift) & requested) != requested))
      {
        error = -EACCES;
      }
      else
      {
        ++object->references;
        error = 0;
      }
    }
    if (error)
    {
      object = nullptr;
    }
  }
  else if (!(flags & O_CREAT))
  {
    error = -ENOENT;
  }
  else if (slot == Names)
  {
    error = -ENOSPC;
  }
  else
  {
    void *memory = xv::memory::allocator().allocate();
    if (!memory)
    {
      error = -ENOMEM;
    }
    else
    {
      object = new (memory) Object;
      object->lock.init("shared object pages");
      object->uid = credentials.effective_uid;
      object->gid = credentials.effective_gid;
      object->mode = mode & 0777 & ~credentials.mask;
      object->references = 2; // name plus this open view
      xv::runtime::copy_fixed(object->name, name, sizeof(object->name));
      objects[slot] = object;
      error = 0;
    }
  }
  names_lock.release();
  if (!object)
  {
    return nullptr;
  }
  if (flags & O_TRUNC)
  {
    xv::sync::ScopedLock guard(object->lock);
    resize(*object, 0);
  }
  void *page = xv::memory::allocator().allocate();
  if (!page)
  {
    release(object);
    error = -ENOMEM;
    return nullptr;
  }
  return new (page) View(*object, writable);
}

int
shared_unlink(const char *name) {
  if (!valid_name(name))
  {
    return -EINVAL;
  }
  initialize();
  auto uid = xv::process::current()->credentials().snapshot().effective_uid;
  names_lock.acquire();
  Object *object = nullptr;
  for (unsigned i = 0; i < Names; ++i)
  {
    if (objects[i] && !xv::runtime::compare_text(objects[i]->name, name, 256))
    {
      object = objects[i];
      if (uid && object->uid != uid)
      {
        names_lock.release();
        return -EACCES;
      }
      objects[i] = nullptr;
      __atomic_store_n(&object->linked, false, __ATOMIC_RELEASE);
      break;
    }
  }
  names_lock.release();
  if (!object)
  {
    return -ENOENT;
  }
  release(object);
  return 0;
}
} // namespace xv::ipc
