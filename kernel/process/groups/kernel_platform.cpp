module xv.process;
import :group_platform;
import xv.memory;
import xv.print;

namespace xv::process::detail
{
void *allocate_group_object(unsigned long bytes, unsigned long alignment)
{
  return xv::memory::allocate_object(bytes, alignment);
}

void free_group_object(void *page)
{
  if (!xv::memory::deallocate_object(page)) xv::print::panic("groups: invalid slab object");
}

[[noreturn]] void group_fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::process::detail
