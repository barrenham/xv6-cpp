module xv.memory;

import :freelist;
import :detail;

namespace xv::memory
{
bool FreeListPageAllocator::valid_page(PhysicalAddress pa) const
{
  return pa.is_aligned() && layout_.contains_usable_block(pa, PageSize);
}

void FreeListPageAllocator::initialize_layout()
{
  states_ = layout_.metadata().begin().pointer<unsigned char>();
  detail::fill(states_, 0, layout_.metadata().size());
  lock_.init("freelist");
  for (unsigned region = 0; region < layout_.usable_count(); ++region)
  {
    const auto &usable = layout_.usable(region);
    for (PageIndex index = 0; index < usable.page_count(); ++index)
    {
      auto page = usable.page_address(index);
      auto *run = page.pointer<Run>();
      run->next = head_;
      head_ = run;
      states_[layout_.ram().page_index(page)] = Free;
    }
  }
}

void *FreeListPageAllocator::allocate(unsigned order)
{
  if (!supports(order))
  {
    return nullptr;
  }

  lock_.acquire();
  Run *page = head_;
  if (page)
  {
    account_free(-1);
    head_ = page->next;
    states_[layout_.ram().page_index(PhysicalAddress::from_pointer(page))] = Allocated;
  }
  lock_.release();
  if (page)
  {
    detail::fill(page, 5, PageSize);
  }

  return page;
}

void FreeListPageAllocator::deallocate(void *address, unsigned order)
{

  PhysicalAddress pa = PhysicalAddress::from_pointer(address);
  if (order != 0 || !valid_page(pa))
  {
    detail::fail("freelist: range or order");
  }

  lock_.acquire();
  auto &state = states_[layout_.ram().page_index(pa)];
  if (state != Allocated)
  {
    detail::fail("freelist: double free or unowned page");
  }

  detail::fill(address, 1, PageSize);
  auto *page = static_cast<Run *>(address);
  page->next = head_;
  head_ = page;
  state = Free;
  account_free(1);
  lock_.release();
}

bool FreeListPageAllocator::split_to_pages(void *address, unsigned order)
{
  if (!supports(order))
  {
    return false;
  }

  PhysicalAddress pa = PhysicalAddress::from_pointer(address);
  if (!valid_page(pa))
  {
    detail::fail("freelist split: range");
  }

  lock_.acquire();
  bool owned = states_[layout_.ram().page_index(pa)] == Allocated;
  lock_.release();
  if (!owned)
  {
    detail::fail("freelist split: unowned page");
  }

  return true;
}
} // namespace xv::memory
