module xv.process;
import :group_platform;
import xv.memory;
import xv.print;

namespace xv::process::detail
{
void *allocate_group_page()
{
  return xv::memory::allocator().allocate();
}

void free_group_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

[[noreturn]] void group_fail(const char *message)
{
  xv::print::panic(message);
}
} // namespace xv::process::detail
