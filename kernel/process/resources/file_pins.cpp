module;
#include "kernel/support/placement_new.hpp"
module xv.process;
import xv.memory;
import :resource_platform;

namespace xv::process
{
struct Process::FilePins
{
  FilePins *next = nullptr;
  unsigned count = 0;
  xv::fs::File *files[(4096 - 16) / sizeof(xv::fs::File *)]{};
};

xv::fs::File *
Process::pin_file(int descriptor) {
  auto *file = files().retain(descriptor);
  if (!file)
  {
    return nullptr;
  }
  for (unsigned i = 0; i < inline_file_pin_count_; ++i)
  {
    if (inline_file_pins_[i] == file)
    {
      detail::release_file(file);
      return file;
    }
  }
  for (auto *page = extra_file_pins_; page; page = page->next)
  {
    for (unsigned i = 0; i < page->count; ++i)
    {
      if (page->files[i] == file)
      {
        detail::release_file(file);
        return file;
      }
    }
  }
  if (inline_file_pin_count_ < 8)
  {
    inline_file_pins_[inline_file_pin_count_++] = file;
    return file;
  }
  if (!extra_file_pins_ ||
      extra_file_pins_->count == sizeof(extra_file_pins_->files) / sizeof(file))
  {
    static_assert(sizeof(FilePins) <= 4096);
    auto *memory = xv::memory::allocator().allocate();
    if (!memory)
    {
      detail::release_file(file);
      return nullptr;
    }
    auto *page = new (memory) FilePins{};
    page->next = extra_file_pins_;
    extra_file_pins_ = page;
  }
  extra_file_pins_->files[extra_file_pins_->count++] = file;
  return file;
}

void
Process::clear_file_pins() {
  while (inline_file_pin_count_)
  {
    auto *file = inline_file_pins_[--inline_file_pin_count_];
    inline_file_pins_[inline_file_pin_count_] = nullptr;
    detail::release_file(file);
  }
  while (auto *page = extra_file_pins_)
  {
    extra_file_pins_ = page->next;
    for (unsigned i = 0; i < page->count; ++i)
    {
      detail::release_file(page->files[i]);
    }
    page->~FilePins();
    xv::memory::allocator().deallocate(page);
  }
}
} // namespace xv::process
