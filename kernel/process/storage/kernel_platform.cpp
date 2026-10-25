module xv.process;

import :storage_platform;
import xv.memory;
import xv.print;

namespace xv::process::detail
{
void *allocate_process_page()
{
  return xv::memory::allocator().allocate();
}

void free_process_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

unsigned long allocate_kernel_stack()
{
  auto &memory = xv::memory::allocator();
  void *physical = memory.allocate();
  if (!physical)
  {
    return 0;
  }
  const auto slot = memory.layout().ram().page_index(
      xv::memory::PhysicalAddress::from_pointer(physical));
  auto address = xv::memory::kernel_page_table().map_process_stack(slot, physical);
  if (!address)
  {
    memory.deallocate(physical);
  }
  return address;
}

void free_kernel_stack(unsigned long address)
{
  void *physical = xv::memory::kernel_page_table().unmap_process_stack(address);
  xv::memory::allocator().deallocate(physical);
}

[[noreturn]] void storage_fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::process::detail
