#include "user/runtime/user.h"
#include "internal.h"

extern "C"
{
  // Set before lifecycle APIs can mutate the registry. Ordinary single-thread
  // fork needs no registry repair and must not dirty the child's COW stack.
  unsigned __xv_threads_fork_needed;
}

namespace
{
unsigned registry_lock;
xv_threads::Control *registry;

int
tid(xv_threads::Control *control) {
  if (!control->mapping)
  {
    // The image-owned initial thread always has TID == process PID, including
    // after a fork which intentionally did not modify its inherited header.
#ifdef LAB_PGTBL
    return ugetpid();
#else
    return getpid();
#endif
  }
  return __atomic_load_n(&control->tid, __ATOMIC_ACQUIRE);
}

[[noreturn]]
void
enter(xv_threads::Control *control) {
  __atomic_store_n(&control->tid, __xv_gettid(), __ATOMIC_RELEASE);
  if (sigprocmask(SIG_SETMASK, &control->initial_mask, nullptr) < 0)
  {
    exit(127);
  }
  pthread_exit(control->entry(control->argument));
}
} // namespace

extern "C" int
__xv_threads_init() {
  auto *control = xv_threads::initialize_main();
  if (!control)
  {
    return -1;
  }
  control->tid = __xv_gettid();
  registry = control;
  asm volatile("mv tp, %0" ::"r"(control->thread_pointer) : "memory");
  return 0;
}

extern "C" void
__xv_threads_after_fork() {
  auto *control = xv_threads::current();
  // In the common single-thread fork these already have the right values.
  // Avoid dirtying an unrelated global page merely to store the same bits.
  if (registry_lock)
  {
    registry_lock = 0;
  }
  if (registry != control)
  {
    registry = control;
  }
  if (control)
  {
    control->next = nullptr;
#ifdef LAB_PGTBL
    // fork creates a fresh, single-thread process: its PID is also its TID.
    control->tid = ugetpid();
#else
    control->tid = __xv_gettid();
#endif
    control->joining = false;
  }
}

extern "C" int
pthread_attr_init(pthread_attr_t *attr) {
  if (!attr)
  {
    return EINVAL;
  }
  attr->stack_size = xv_threads::DefaultStack;
  return 0;
}

extern "C" int
pthread_attr_destroy(pthread_attr_t *attr) {
  return attr ? 0 : EINVAL;
}

extern "C" int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t bytes) {
  if (!attr || bytes < PTHREAD_STACK_MIN || bytes > 512 * 1024)
  {
    return EINVAL;
  }
  attr->stack_size = bytes;
  return 0;
}

extern "C" int
pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *bytes) {
  if (!attr || !bytes)
  {
    return EINVAL;
  }
  *bytes = attr->stack_size;
  return 0;
}

extern "C" pthread_t
pthread_self() {
  return reinterpret_cast<pthread_t>(xv_threads::current());
}

extern "C" int
pthread_equal(pthread_t left, pthread_t right) {
  return left == right;
}

extern "C" int
pthread_create(pthread_t *handle, const pthread_attr_t *attr, void *(*entry)(void *),
               void *argument) {
  if (!handle || !entry)
  {
    return EINVAL;
  }
  auto stack = attr ? attr->stack_size : xv_threads::DefaultStack;
  if (stack < PTHREAD_STACK_MIN || stack > 512 * 1024)
  {
    return EINVAL;
  }
  __atomic_store_n(&__xv_threads_fork_needed, 1U, __ATOMIC_RELEASE);
  int saved_errno = errno;
  auto *control = xv_threads::allocate(stack);
  errno = saved_errno;
  if (!control)
  {
    return EAGAIN;
  }
  control->entry = entry;
  control->argument = argument;
  xv_threads::lock(registry_lock);
  // A handler must never see an uninitialized thread ID. The new task starts
  // with asynchronous signals blocked, publishes its TID, then restores the
  // mask inherited from its creator before calling user code.
  sigset_t all;
  sigfillset(&all);
  if (sigprocmask(SIG_BLOCK, &all, &control->initial_mask) < 0)
  {
    xv_threads::unlock(registry_lock);
    xv_threads::release(control);
    errno = saved_errno;
    return EAGAIN;
  }
  auto tid = __xv_thread_create(reinterpret_cast<unsigned long>(enter),
                                reinterpret_cast<unsigned long>(control),
                                control->stack_top, control->thread_pointer);
  if (tid >= 0)
  {
    __atomic_store_n(&control->tid, static_cast<int>(tid), __ATOMIC_RELEASE);
  }
  if (sigprocmask(SIG_SETMASK, &control->initial_mask, nullptr) < 0)
  {
    exit(127);
  }
  if (tid < 0)
  {
    xv_threads::unlock(registry_lock);
    xv_threads::release(control);
    errno = saved_errno;
    return -tid;
  }
  control->next = registry;
  registry = control;
  *handle = reinterpret_cast<pthread_t>(control);
  xv_threads::unlock(registry_lock);
  return 0;
}

extern "C" int
pthread_join(pthread_t handle, void **result) {
  if (handle && pthread_equal(handle, pthread_self()))
  {
    return EDEADLK;
  }
  __atomic_store_n(&__xv_threads_fork_needed, 1U, __ATOMIC_RELEASE);
  xv_threads::lock(registry_lock);
  auto *control = registry;
  while (control && reinterpret_cast<pthread_t>(control) != handle)
  {
    control = control->next;
  }
  if (!control)
  {
    xv_threads::unlock(registry_lock);
    return ESRCH;
  }
  if (control->joining)
  {
    xv_threads::unlock(registry_lock);
    return EINVAL;
  }
  control->joining = true;
  auto target_tid = tid(control);
  xv_threads::unlock(registry_lock);
  unsigned long value = 0;
  long status;
  do
  {
    status = __xv_thread_join(target_tid, &value);
  } while (status == -EINTR);
  xv_threads::lock(registry_lock);
  if (status)
  {
    control->joining = false;
    xv_threads::unlock(registry_lock);
    return -status;
  }
  auto **link = &registry;
  while (*link != control)
  {
    link = &(*link)->next;
  }
  *link = control->next;
  xv_threads::unlock(registry_lock);
  if (result)
  {
    *result = reinterpret_cast<void *>(value);
  }
  int saved_errno = errno;
  xv_threads::release(control);
  errno = saved_errno;
  return 0;
}

extern "C" [[noreturn]]
void
pthread_exit(void *result) {
  __xv_thread_exit(reinterpret_cast<unsigned long>(result));
}

extern "C" int
pthread_kill(pthread_t handle, int signal) {
  if (!handle)
  {
    return ESRCH;
  }
  // As with POSIX pthread IDs, a handle must be used within its lifetime. This
  // direct lookup also permits self-signalling from a signal handler without
  // recursively acquiring the runtime's thread-registry lock.
  auto *control = reinterpret_cast<xv_threads::Control *>(handle);
  auto result = __xv_thread_signal(tid(control), signal);
  return result < 0 ? -result : 0;
}
