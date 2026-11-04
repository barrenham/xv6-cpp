#pragma once
#include "user/runtime/user.h"
#include "kernel/support/placement_new.hpp"
namespace userfs::detail
{
template <class T> class Workspace
{
public:
  Workspace()
  {
    void *storage = malloc(sizeof(T));
    if (storage) pointer_ = new (storage) T{};
  }
  ~Workspace() { if (pointer_) { pointer_->~T(); free(pointer_); } }
  Workspace(const Workspace &) = delete;
  Workspace &operator=(const Workspace &) = delete;
  explicit operator bool() const { return pointer_ != nullptr; }
  T *operator->() const { return pointer_; }
  T &operator*() const { return *pointer_; }
private:
  T *pointer_ = nullptr;
};
}
