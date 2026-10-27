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
  return xv::memory::kernel_page_table().allocate_process_stack();
}
void free_kernel_stack(unsigned long address)
{
  xv::memory::kernel_page_table().free_process_stack(address);
}

[[noreturn]] void storage_fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::process::detail
