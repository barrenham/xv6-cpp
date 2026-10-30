module xv.process:group_platform;

namespace xv::process::detail
{
// Non-sleeping slab object operations; may be called under the groups lock.
void *allocate_group_object(unsigned long bytes, unsigned long alignment);
void free_group_object(void *page);
[[noreturn]] void group_fail(const char *message);
} // namespace xv::process::detail
