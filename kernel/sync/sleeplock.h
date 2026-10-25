#pragma once
#include "kernel/sync/spinlock.h"
class SleepLock {
public:
  SleepLock() = default;
  SleepLock(const SleepLock &) = delete;
  SleepLock &operator=(const SleepLock &) = delete;
  void init(const char *label);
  void acquire();
  void release();
  int holding();
private:
  uint locked;
  SpinLock lk;
  const char *name;
  int pid;
};
