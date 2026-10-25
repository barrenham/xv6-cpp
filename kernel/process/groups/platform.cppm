module xv.process:group_platform;

namespace xv::process::detail
{
// Non-sleeping physical page operations; may be called under the groups lock.
void *allocate_group_page();
void free_group_page(void *page);
[[noreturn]] void group_fail(const char *message);
} // namespace xv::process::detail
