module;
#include "kernel/support/placement_new.hpp"
module xv.fs;
import :buffer;
import :platform;
namespace xv::fs
{
PageBuffer::~PageBuffer() { clear(); }
void PageBuffer::take(PageBuffer &other)
{
  if (&other == this) return;
  clear();
  head_ = other.head_; tail_ = other.tail_; pages_ = other.pages_; size_ = other.size_;
  other.head_ = other.tail_ = nullptr; other.pages_ = other.size_ = 0;
}
PageBuffer::Page *PageBuffer::page(unsigned index, bool create)
{
  while (create && pages_ <= index)
  {
    auto *memory = detail::allocate_page();
    if (!memory) return nullptr;
    auto *next = new (memory) Page{};
    if (tail_) tail_->next = next;
    else head_ = next;
    tail_ = next;
    ++pages_;
  }
  if (index >= pages_) return nullptr;
  if (index + 1 == pages_) return tail_;
  auto *result = head_;
  while (index--) result = result->next;
  return result;
}
const PageBuffer::Page *PageBuffer::page(unsigned index) const
{
  if (index >= pages_) return nullptr;
  if (index + 1 == pages_) return tail_;
  auto *result = head_;
  while (index--) result = result->next;
  return result;
}
bool PageBuffer::append(const void *source, unsigned count)
{ return count <= 0x7fffffffU && write(size_, false, reinterpret_cast<unsigned long>(source), count) == static_cast<int>(count); }
int PageBuffer::write(unsigned offset, bool user, unsigned long source, unsigned count)
{
  if (count > 0x7fffffffU || offset > 0x7fffffffU - count) return -1;
  if (count && offset > size_)
  {
    unsigned gap = size_;
    while (gap < offset)
    {
      unsigned inside = gap % sizeof(Page::bytes);
      auto *block = page(gap / sizeof(Page::bytes), true);
      if (!block) return -1;
      unsigned part = sizeof(Page::bytes) - inside;
      if (part > offset - gap) part = offset - gap;
      detail::fill(block->bytes + inside, 0, part);
      gap += part;
    }
  }
  unsigned done = 0;
  while (done < count)
  {
    unsigned position = offset + done, inside = position % sizeof(Page::bytes);
    auto *block = page(position / sizeof(Page::bytes), true);
    if (!block) return done ? static_cast<int>(done) : -1;
    unsigned part = sizeof(Page::bytes) - inside;
    if (part > count - done) part = count - done;
    if (detail::copy_in(block->bytes + inside, user, source + done, part) < 0)
      return done ? static_cast<int>(done) : -1;
    done += part;
    if (offset + done > size_) size_ = offset + done;
  }
  return done;
}
int PageBuffer::read(unsigned offset, bool user, unsigned long destination, unsigned count) const
{
  if (offset >= size_) return 0;
  if (count > size_ - offset) count = size_ - offset;
  unsigned done = 0;
  while (done < count)
  {
    unsigned position = offset + done, inside = position % sizeof(Page::bytes);
    auto *block = page(position / sizeof(Page::bytes));
    if (!block) detail::fail("page buffer: missing data page");
    unsigned part = sizeof(Page::bytes) - inside;
    if (part > count - done) part = count - done;
    if (detail::copy_out(user, destination + done, block->bytes + inside, part) < 0)
      return done ? static_cast<int>(done) : -1;
    done += part;
  }
  return done;
}
void PageBuffer::clear()
{
  while (head_)
  {
    auto *next = head_->next;
    head_->~Page();
    detail::free_page(head_);
    head_ = next;
  }
  tail_ = nullptr; pages_ = size_ = 0;
}
}
