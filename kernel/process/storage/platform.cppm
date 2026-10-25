module xv.process:storage_platform;

namespace xv::process::detail
{
void *allocate_process_page();
void free_process_page(void *page);
unsigned long allocate_kernel_stack();
void free_kernel_stack(unsigned long address);
[[noreturn]] void storage_fail(const char *message);
} // namespace xv::process::detail
