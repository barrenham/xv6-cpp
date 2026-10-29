module;
#include "kernel/support/placement_new.hpp"
module xv.fs;
import :page_buffer_flow;
import :buffer;
import :platform;

namespace xv::fs
{
PageBuffer::~PageBuffer()
{
  clear();
}

void PageBuffer::take(PageBuffer &other)
{
  if (&other == this)
  {
    return;
  }
  clear();
  head_ = other.head_;
  tail_ = other.tail_;
  pages_ = other.pages_;
  size_ = other.size_;
  other.head_ = other.tail_ = nullptr;
  other.pages_ = other.size_ = 0;
}

PageBuffer::Page *PageBuffer::page(unsigned index, bool create)
{
  PageFlow::Context context{*this, index, create};
  (void)PageFlow::Graph::run<0>(context);
  return context.result;
}

const PageBuffer::Page *PageBuffer::page(unsigned index) const
{
  if (index >= pages_)
  {
    return nullptr;
  }
  if (index + 1 == pages_)
  {
    return tail_;
  }
  auto *result = head_;
  while (index--)
  {
    result = result->next;
  }
  return result;
}

bool PageBuffer::append(const void *source, unsigned count)
{
  return count <= 0x7fffffffU &&
         write(size_, false, reinterpret_cast<unsigned long>(source), count) ==
             static_cast<int>(count);
}

int PageBuffer::write(unsigned offset, bool user, unsigned long source, unsigned count)
{
  WriteFlow::Context context{*this, offset, user, source, count};
  (void)WriteFlow::Graph::run<0>(context);
  return context.result;
}

int PageBuffer::read(unsigned offset, bool user, unsigned long destination,
                     unsigned count) const
{
  ReadFlow::Context context{*this, offset, user, destination, count};
  (void)ReadFlow::Graph::run<0>(context);
  return context.result;
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
  tail_ = nullptr;
  pages_ = size_ = 0;
}

bool PageBuffer::resize(unsigned size)
{
  ResizeFlow::Context context{*this, size};
  (void)ResizeFlow::Graph::run<0>(context);
  return context.result;
}

} // namespace xv::fs
