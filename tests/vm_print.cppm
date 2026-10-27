module;
#include <cstdio>
#include <cstdlib>
export module xv.print;
export namespace xv::print
{
int printk(const char *, ...) { return 0; }
[[noreturn]] void panic(const char *message)
{
  std::fprintf(stderr, "unexpected VM panic: %s\n", message);
  std::abort();
}
}
