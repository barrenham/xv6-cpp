module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
module xv.process;
extern "C" int xv_test_interrupt_depth();
namespace
{
int copy_result = 0;
unsigned calls = 0;
unsigned long last_count = 0;
bool last_user = false;
} // namespace
namespace xv::process
{
int copy_out(bool user, unsigned long destination, const void *source,
             unsigned long count)
{
  if (xv_test_interrupt_depth() != 1 || count > 32)
  {
    std::fputs("stats: copy outside lock or invalid length\n", stderr);
    std::abort();
  }
  ++calls;
  last_count = count;
  last_user = user;
  if (copy_result != -1)
  {
    std::memcpy(reinterpret_cast<void *>(destination), source, count);
  }
  return copy_result;
}
} // namespace xv::process
extern "C" void stats_copy_result(int value) { copy_result = value; }
extern "C" unsigned stats_copy_calls() { return calls; }
extern "C" unsigned long stats_copy_count() { return last_count; }
extern "C" bool stats_copy_user() { return last_user; }
