module xv.process;

import :storage_platform;
import xv.memory;
import xv.arch;
import xv.print;

namespace xv::process::detail
{
unsigned long scheduling_time() { return xv::arch::r_time(); }

namespace
{
#if XV_DEFAULT_SLAB_BITMAP
constinit xv::memory::BitmapSlabCache descriptors;
#else
constinit xv::memory::FreeListSlabCache descriptors;
#endif
int descriptors_ready = 0;
unsigned long descriptor_bytes = 0, descriptor_alignment = 0;
}
void initialize_process_objects(unsigned long bytes, unsigned long alignment)
{
  int expected = 0;
  if (__atomic_compare_exchange_n(&descriptors_ready, &expected, 1, false,
                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
  {
    if (!descriptors.initialize(xv::memory::allocator(), bytes, alignment, 0))
      xv::print::panic("process descriptors: invalid slab layout");
    descriptor_bytes = bytes;
    descriptor_alignment = alignment;
    __atomic_store_n(&descriptors_ready, 2, __ATOMIC_RELEASE);
  }
  else while (__atomic_load_n(&descriptors_ready, __ATOMIC_ACQUIRE) != 2) {}
  if (descriptor_bytes != bytes || descriptor_alignment != alignment)
    xv::print::panic("process descriptors: inconsistent slab layout");
}
void *allocate_process_object() { return descriptors.allocate(); }
void free_process_object(void *object)
{
  if (!descriptors.deallocate(object)) xv::print::panic("process descriptors: invalid free");
}

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
