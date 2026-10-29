module;
#include <cstdio>
#include <cstdlib>
#include <cstring>
module xv.drivers;
extern "C" int xv_test_interrupt_depth();
extern "C" void *stats_attached_device();
extern "C" unsigned stats_attached_major();
extern "C" void stats_copy_result(int);
extern "C" unsigned stats_copy_calls();
extern "C" unsigned long stats_copy_count();
extern "C" bool stats_copy_user();
namespace xv::drivers::detail
{
void statsinit();
namespace
{
unsigned page_fills = 0, lock_fills = 0;
int fill_size = 7;
void check(bool value, const char *reason)
{
  if (!value)
  {
    std::fprintf(stderr, "stats_contract: %s\n", reason);
    std::exit(1);
  }
}
} // namespace
int statscopyin(char *buffer, int capacity)
{
  check(xv_test_interrupt_depth() == 1 && capacity == 4096, "page fill protocol");
  check(page_fills == lock_fills, "page fill before lock fill");
  ++page_fills;
  std::memcpy(buffer, "PTE", 3);
  return 3;
}
int statslock(char *buffer, int capacity)
{
  check(xv_test_interrupt_depth() == 1 && capacity == 4096, "lock fill protocol");
  check(page_fills == lock_fills + 1, "both original fill calls execute");
  ++lock_fills;
  std::memcpy(buffer, "LOCKabc", 7);
  return fill_size;
}
extern "C" int run_stats_contract()
{
  statsinit();
  auto *device = static_cast<CharacterDevice *>(stats_attached_device());
  check(device && stats_attached_major() == 7 &&
            !std::strcmp(device->name(), "statistics"),
        "fixture registration");
  char output[32]{};
  auto read = [&](int count, bool user = true)
  {
    return device->read({user ? AddressSpace::User : AddressSpace::Kernel,
                         reinterpret_cast<unsigned long>(output)},
                        count);
  };
  check(read(3) == 3 && !std::memcmp(output, "LOC", 3), "initial bounded read");
  check(read(20, false) == 4 && !std::memcmp(output, "Kabc", 4) && !stats_copy_user(),
        "remaining kernel read");
  auto copies = stats_copy_calls();
  check(read(1) == -1 && stats_copy_calls() == copies && lock_fills == 1,
        "EOF resets without copy or refill");
  stats_copy_result(-1);
  output[0] = '!';
  check(read(2) == 2 && output[0] == '!' && lock_fills == 2,
        "copy failure preserves original positive return");
  stats_copy_result(0);
  check(read(2) == 2 && !std::memcmp(output, "LO", 2),
        "copy failure does not advance cursor");
  stats_copy_result(1);
  check(read(2) == 2 && !std::memcmp(output, "CK", 2),
        "every non-minus-one copy result advances");
  stats_copy_result(0);
  check(read(2) == 2 && !std::memcmp(output, "ab", 2),
        "cursor after nonzero copy success");
  copies = stats_copy_calls();
  check(read(0) == 0 && stats_copy_calls() == copies + 1 && stats_copy_count() == 0 &&
            stats_copy_user(),
        "zero length still calls copy on nonempty cache");
  check(read(1) == 1 && output[0] == 'c' && read(1) == -1, "last byte and next EOF");
  fill_size = 0;
  copies = stats_copy_calls();
  check(read(1) == -1 && stats_copy_calls() == copies && page_fills == 3 &&
            lock_fills == 3,
        "second fill result overrides first");
  check(device->write({AddressSpace::Kernel, 0}, 1) == -1, "write remains unsupported");
  device->put_sync('x');
  check(xv_test_interrupt_depth() == 0, "all reads release original lock");
  std::puts("stats_contract: passed");
  return 0;
}
} // namespace xv::drivers::detail
