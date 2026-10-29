#include <cstdio>
#include <cstdlib>
#include <cstring>
import xv.print;
extern "C" const char *captured_print();
extern "C" void reset_print();

static void check(const char *expected)
{
  if (std::strcmp(captured_print(), expected))
  {
    std::fprintf(stderr, "print_contract: expected [%s], got [%s]\n", expected,
                 captured_print());
    std::exit(1);
  }
  reset_print();
}

int main()
{
  xv::print::initialize();
  xv::print::printk("plain text");
  check("plain text");
  xv::print::printk("%d %ld %lld %u %lu %llu", -12, -34L, -56LL, 78U, 90UL, 123ULL);
  check("-12 -34 -56 78 90 123");
  xv::print::printk("%x %lx %llx %p %c %s %%", 0xabU, 0xcdUL, 0xefULL,
                    reinterpret_cast<void *>(0x1234UL), 'Q', "text");
  check("ab cd ef 0x0000000000001234 Q text %");
  // Nonliteral strings exercise the formatter's defined fallback paths.
  const char *unknown = "A%qB%";
  auto *formatter = &xv::print::printk;
  formatter(unknown);
  check("A%qB");
  const char *null_string = nullptr;
  xv::print::printk("[%s]", null_string);
  check("[(null)]");
  xv::print::printk("");
  check("");
}
