module;
#include <cstdlib>
#include <cstring>
module xv.net;
import :platform;
namespace
{
int owner_id = 1, pages = 0;
bool interrupt = false;
}
namespace xv::net::detail
{
void *allocate_page() { auto *p = std::malloc(4096); if (p) ++pages; return p; }
void free_page(void *page) { --pages; std::free(page); }
int owner() { return owner_id; }
bool interrupted() { return interrupt; }
int copy_in(void *to, unsigned long from, unsigned n)
{
  if (n && (!from || from == ~0UL)) return -1;
  if (n) std::memcpy(to, reinterpret_cast<void *>(from), n);
  return 0;
}
int copy_out(unsigned long to, const void *from, unsigned n)
{
  if (n && (!to || to == ~0UL)) return -1;
  if (n) std::memcpy(reinterpret_cast<void *>(to), from, n);
  return 0;
}
void prepare_sleep(void *) {}
void sleep() { interrupt = true; }
void wake(void *) {}
}
extern "C" void *net_test_allocate() { return xv::net::detail::allocate_page(); }
extern "C" void net_test_free(void *p) { xv::net::detail::free_page(p); }
extern "C" int net_test_pages() { return pages; }
extern "C" void net_test_owner(int id) { owner_id = id; interrupt = false; }
