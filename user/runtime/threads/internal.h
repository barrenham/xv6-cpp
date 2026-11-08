#pragma once
#include "user/include/pthread.h"
#include "kernel/io/posix_errno.h"
extern "C"
{
  long __xv_thread_create(unsigned long, unsigned long, unsigned long, unsigned long);
  long __xv_thread_join(int, unsigned long *);
  [[noreturn]]
  void __xv_thread_exit(unsigned long);
  int __xv_gettid();
  int __xv_thread_word_wait(unsigned *, unsigned);
  int __xv_thread_word_timedwait(unsigned *, unsigned, unsigned long);
  int __xv_thread_word_wake(unsigned *, int);
  int __xv_thread_signal(int, int);
}

namespace xv_threads
{
constexpr unsigned long Page = 4096;
constexpr unsigned long DefaultStack = 64 * 1024;

struct Control
{
  Control *next;
  void *mapping;
  unsigned long mapping_bytes;
  unsigned long stack_top;
  unsigned long thread_pointer;
  void *(*entry)(void *);
  void *argument;
  int tid;
  unsigned initial_mask;
  bool joining;
};

inline Control *
current() {
  unsigned long tp;
  asm volatile("mv %0, tp" : "=r"(tp));
  return tp ? *reinterpret_cast<Control **>(tp - sizeof(Control *)) : nullptr;
}

void lock(unsigned &);
void unlock(unsigned &);
Control *initialize_main();
Control *allocate(unsigned long stack);
void release(Control *);
} // namespace xv_threads
