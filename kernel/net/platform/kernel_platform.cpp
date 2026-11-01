module xv.net;
import :platform;
import xv.memory;
import xv.arch;
import xv.process;
import xv.print;

namespace
{
unsigned long timer_generation = 1;
}

namespace xv::net::detail
{
[[noreturn]] void fail(const char *message)
{
  xv::print::panic(message);
}

void *allocate_page()
{
  return xv::memory::allocator().allocate();
}

void free_page(void *page)
{
  xv::memory::allocator().deallocate(page);
}

unsigned long milliseconds()
{
  return xv::arch::r_time() / 10000;
}

bool interrupted()
{
  return xv::process::current()->is_killed();
}

int copy_in(void *destination, unsigned long source, unsigned count)
{
  return xv::process::copy_in(destination, true, source, count);
}

int copy_out(unsigned long destination, const void *source, unsigned count)
{
  return xv::process::copy_out(true, destination, source, count);
}

void prepare_sleep(void *channel)
{
  xv::process::prepare_interruptible_sleep(channel);
}

void sleep()
{
  xv::process::sleep();
}

void timers_changed()
{
  __atomic_add_fetch(&timer_generation, 1UL, __ATOMIC_RELEASE);
  xv::process::wakeup(&timer_generation);
  // A local one-shot kick also wakes an idle hart and provides a scheduling
  // opportunity after RX interrupts. No remote CSR write or periodic 1 kHz tick.
  const auto at = xv::arch::r_time() + 10000;
  if (at < xv::arch::r_stimecmp())
  {
    xv::arch::w_stimecmp(at);
  }
}

void wake(void *channel)
{
  xv::process::wakeup(channel);
}
} // namespace xv::net::detail

namespace xv::net
{
namespace
{
void timer_worker()
{
  xv::arch::intr_on();
  unsigned passes = 0;
  auto wait = xv::process::make_poll_wait();
  wait.watch(&timer_generation);
  if (wait.failed())
  {
    xv::print::panic("network timer subscription");
  }
  for (;;)
  {
    // Subscribe before observing generation. A change during computation or
    // between the last comparison and sleep cannot be lost.
    wait.cancel_deadline();
    auto generation = __atomic_load_n(&timer_generation, __ATOMIC_ACQUIRE);
    timer_tick();
    auto next = next_timer_deadline();
    if (++passes == 8)
    {
      passes = 0;
      xv::process::scheduler().yield();
    }
    if (generation != __atomic_load_n(&timer_generation, __ATOMIC_ACQUIRE))
    {
      continue;
    }
    if (next != ~0UL)
    {
      wait.set_deadline(next);
    }
    wait.sleep();
  }
}
} // namespace

void start_timer_worker()
{
  xv::process::create_kernel_task("net-timers", timer_worker);
}
} // namespace xv::net
