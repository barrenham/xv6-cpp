module xv.net;
import :platform;
import xv.memory;
import xv.arch;
import xv.process;
import xv.print;
namespace xv::net::detail
{
[[noreturn]] void fail(const char *message) { xv::print::panic(message); }
void *allocate_page() { return xv::memory::allocator().allocate(); }
void free_page(void *page) { xv::memory::allocator().deallocate(page); }
unsigned long milliseconds() { return xv::arch::r_time() / 10000; }
int owner() { return xv::process::current()->id(); }
bool interrupted() { return xv::process::current()->is_killed(); }
int copy_in(void *destination, unsigned long source, unsigned count)
{ return xv::process::copy_in(destination, true, source, count); }
int copy_out(unsigned long destination, const void *source, unsigned count)
{ return xv::process::copy_out(true, destination, source, count); }
void prepare_sleep(void *channel) { xv::process::prepare_interruptible_sleep(channel); }
void sleep() { xv::process::sleep(); }
void wake(void *channel) { xv::process::wakeup(channel); }
}
