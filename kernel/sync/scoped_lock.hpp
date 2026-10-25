#pragma once
namespace xv::sync {
// A guard must never span sched()/sleep(), which transfer lock ownership.
template<class Lock>
class ScopedLock final {
public:
  explicit ScopedLock(Lock &lock) : lock_(lock) { lock_.acquire(); }
  ~ScopedLock() { lock_.release(); }
  ScopedLock(const ScopedLock &) = delete;
  ScopedLock &operator=(const ScopedLock &) = delete;
private:
  Lock &lock_;
};
}
