#pragma once
#include "kernel/support/types.h"
struct cpu;
// Explicit init preserves xv6's boot ordering; no global constructors run.
class SpinLock {
public:
  SpinLock() = default;
  SpinLock(const SpinLock &) = delete;
  SpinLock &operator=(const SpinLock &) = delete;
  void init(const char *label);
  void acquire();
  void release();
  int holding();
private:
  uint locked;
  const char *name;
  struct cpu *cpu;
};
