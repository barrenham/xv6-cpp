module;

#include <cstring>
#include <cstdlib>

module xv.memory;

import :detail;

namespace xv::memory::detail
{
void fill(void *p, int value, MemorySize length)
{
  std::memset(p, value, length);
}

[[noreturn]] void fail(const char *)
{
  std::_Exit(99);
}
} // namespace xv::memory::detail
