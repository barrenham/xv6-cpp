module;
#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/trap/arch/riscv/entry.h"

module xv.trap;
import xv.net;
import xv.memory;
import xv.sync;

import xv.arch;

import xv.print;
import :platform;
import xv.process;
import xv.drivers;
import xv.syscall;

namespace xv::trap::detail
{
alignas(4096) unsigned char entry_stacks[NCPU][16384]{};
unsigned long scheduler_stacks[NCPU]{};
constexpr unsigned StackReservePages = 64;
struct StackReserve { void *pages[StackReservePages]{}; unsigned count = 0; };
StackReserve stack_reserves[NCPU];

void refill_stack_reserve()
{
  if (xv::process::current_cpu()->noff) return;
  auto &reserve = stack_reserves[xv::process::cpu_id()];
  while (reserve.count < StackReservePages)
  {
    auto *page = xv::memory::allocator().allocate();
    if (!page) break;
    reserve.pages[reserve.count++] = page;
  }
}
void *take_stack_frame()
{
  if (!xv::process::current_cpu()->noff)
    if (auto *page = xv::memory::allocator().allocate()) return page;
  auto &reserve = stack_reserves[xv::process::cpu_id()];
  return reserve.count ? reserve.pages[--reserve.count] : nullptr;
}
#if XV_VM_SELF_TEST
__attribute__((noinline)) unsigned long stack_probe(unsigned depth)
{
  volatile unsigned char page[4096];
  for (unsigned i = 0; i < sizeof(page); ++i) page[i] = depth;
  page[4095] = depth + 1;
  auto next = depth ? stack_probe(depth - 1) : 0;
  return next + page[0] + page[4095];
}
void run_vm_kernel_probe()
{
  auto base = xv::process::current()->kernel_stack();
#if XV_VM_SELF_TEST == 2
  auto *entry = xv::memory::walk_entry(static_cast<xv::memory::PageTable>(
      const_cast<void *>(xv::memory::kernel_page_table().root())), base + KSTACK_BYTES - PGSIZE, 0);
  *entry &= ~PTE_W;
  xv::arch::sfence_vma();
  *reinterpret_cast<volatile char *>(base + KSTACK_BYTES - PGSIZE) = 0;
#elif XV_VM_SELF_TEST == 4
  *reinterpret_cast<volatile char *>(base - 8) = 0;
#else
  (void)base;
  xv::sync::SpinLock lock;
  lock.init("VM locked stack probe");
  lock.acquire();
  auto &reserve = stack_reserves[xv::process::cpu_id()];
  auto before = __atomic_load_n(&reserve.count, __ATOMIC_ACQUIRE);
  auto checksum = stack_probe(XV_VM_SELF_TEST == 3 ? 200 : 48);
  auto after = __atomic_load_n(&reserve.count, __ATOMIC_ACQUIRE);
  lock.release();
  xv::print::printk("stack probe checksum=%lu reserve=%u->%u\n", checksum, before, after);
  if (checksum != 2401 || after >= before) xv::print::panic("VM locked stack probe failed");
  xv::print::printk("kernel stack locked growth: OK\n");
#endif
}
#endif

static_assert(TrapEvent::SupervisorPrevious == SSTATUS_SPP);
static_assert(TrapEvent::InterruptEnable == SSTATUS_SIE);

TrapEvent snapshot()
{
  return {TrapCause(xv::arch::r_scause()), xv::arch::r_sepc(), xv::arch::r_sstatus(),
          xv::arch::r_stval()};
}

void install_kernel_vector()
{
  auto top = reinterpret_cast<unsigned long>(&entry_stacks[xv::process::cpu_id()][16384]);
  asm volatile("csrw sscratch, %0" : : "r"(top));
  xv::arch::w_stvec(reinterpret_cast<Word>(kernelvec));
}

bool has_process()
{
  return xv::process::current() != nullptr;
}

void deliver_signals()
{
  xv::process::deliver_signals();
}

bool interrupted()
{
  return xv::process::current()->is_killed();
}

bool killed()
{
  return xv::process::current()->termination_status() != 0;
}

void request_kill()
{
  xv::process::current()->request_kill();
}

[[noreturn]] void exit_current(int status)
{
  xv::process::exit_process(status);
}

void yield_current()
{
  xv::process::scheduler().yield();
}

void save_user_pc(Word pc)
{
  xv::process::current()->registers()->epc = pc;
}

void advance_user_pc(Word bytes)
{
  xv::process::current()->registers()->epc += bytes;
}

void enable_interrupts()
{
  xv::arch::intr_on();
}

void dispatch_system_call()
{
  xv::syscall::dispatch();
}

void dispatch_external_interrupt()
{
  xv::drivers::handle_external_interrupt(xv::process::cpu_id());
  // UART/TTY locks have been released and the PLIC source completed.
  xv::process::service_signal_notifications();
}

bool resolve_page_fault(Word address, MemoryAccess access)
{
  return xv::process::current()->memory().handle_fault(address, access == MemoryAccess::Read ? xv::memory::FaultAccess::Read :
      access == MemoryAccess::Write ? xv::memory::FaultAccess::Write : xv::memory::FaultAccess::Execute).result == xv::memory::FaultResult::Resolved;
}

void report_user_fault(const TrapEvent &event)
{
  xv::print::printk("usertrap(): unexpected scause 0x%lx pid=%d\n", event.cause.raw(),
                    xv::process::current()->id());
  xv::print::printk("            sepc=0x%lx stval=0x%lx\n", event.pc, event.value);
}

[[noreturn]] void fatal(const char *message, const TrapEvent &event)
{
  xv::print::printk("scause=0x%lx sepc=0x%lx stval=0x%lx\n", event.cause.raw(),
                    event.pc, event.value);
  xv::print::panic(message);
}

void prepare_user_return()
{
#if XV_VM_SELF_TEST
  static bool tested = false;
  if (!tested) { tested = true; run_vm_kernel_probe(); }
#endif
  auto *process = xv::process::current();
  xv::arch::intr_off();
  const auto vector = TRAMPOLINE + (reinterpret_cast<Word>(uservec) -
                                    reinterpret_cast<Word>(trampoline));
  xv::arch::w_stvec(vector);
  auto *frame = process->registers();
  frame->kernel_satp = xv::arch::r_satp();
  frame->kernel_sp = process->kernel_stack() + KSTACK_PAGES * PGSIZE;
  frame->kernel_trap = reinterpret_cast<Word>(usertrap);
  frame->kernel_hartid = xv::arch::r_tp();
  auto status = xv::arch::r_sstatus();
  status &= ~SSTATUS_SPP;
  status |= SSTATUS_SPIE;
  xv::arch::w_sstatus(status);
  xv::arch::w_sepc(frame->epc);
}

Word user_page_table_token()
{
  return MAKE_SATP(xv::process::current()->memory().page_table());
}

void restore_kernel(const TrapEvent &event)
{
  xv::arch::w_sepc(event.pc);
  xv::arch::w_sstatus(event.status);
}

[[noreturn]] void enter_user(Word token)
{
  const auto address = TRAMPOLINE + (reinterpret_cast<Word>(userret) -
                                     reinterpret_cast<Word>(trampoline));
  reinterpret_cast<void (*)(Word)>(address)(token);
  __builtin_unreachable();
}

int hart_id()
{
  return xv::process::cpu_id();
}

Word time()
{
  return xv::arch::r_time();
}

void arm_timer(Word deadline)
{
  xv::arch::w_stimecmp(deadline);
}

void service_timer_devices()
{
  if (hart_id() == 0)
  {
    xv::process::poll_timer_tick(xv::arch::r_time() / 10000);
    xv::drivers::terminal_timer_tick();
    xv::net::timer_tick();
  }
}

void prepare_interruptible_sleep(void *channel)
{
  xv::process::prepare_interruptible_sleep(channel);
}

void prepare_sleep(void *channel)
{
  xv::process::prepare_sleep(channel);
}

void sleep()
{
  xv::process::sleep();
}

void wakeup(void *channel)
{
  xv::process::wakeup(channel);
}
} // namespace xv::trap::detail

extern "C" int kernel_stack_prepare(unsigned long stack, unsigned long cause, unsigned long fault)
{
  auto *process = xv::process::current();
  auto base = process ? process->kernel_stack() : xv::trap::detail::scheduler_stacks[xv::process::cpu_id()];
  if (!base) return 0;
  if (stack < base || stack > base + KSTACK_BYTES)
  {
    if (stack >= KSTACK_BOTTOM && stack < KSTACK_TOP) xv::print::panic("kernel stack reserve exhausted");
    return 0;
  }
  bool page_fault = cause == 12 || cause == 13 || cause == 15;
  bool in_stack = fault >= base && fault < base + KSTACK_BYTES;
  bool in_guard = fault >= base - KSTACK_BYTES && fault < base;
  if (page_fault && (in_stack || in_guard))
  {
    auto access = cause == 12 ? xv::memory::FaultAccess::Execute :
                  cause == 15 ? xv::memory::FaultAccess::Write : xv::memory::FaultAccess::Read;
    auto result = xv::memory::kernel_page_table().handle_stack_fault(
        base, stack, fault, access, xv::trap::detail::take_stack_frame);
    if (result != xv::memory::FaultResult::Resolved)
    {
      if (result == xv::memory::FaultResult::Permission) xv::print::panic("kernel stack: protection fault");
      if (result == xv::memory::FaultResult::AlreadyMapped) xv::print::panic("kernel stack: unexpected mapped-page fault");
      if (result == xv::memory::FaultResult::OutsideRegion) xv::print::panic("kernel stack: guard overflow");
      if (result == xv::memory::FaultResult::OutOfMemory) xv::print::panic("kernel stack: emergency frames exhausted");
      xv::print::panic("kernel stack: invalid mapping");
    }
    // No registry/allocator lock is acquired when interrupted code owns a lock.
    return 1;
  }
  if (!xv::process::current_cpu()->noff)
  {
    xv::trap::detail::refill_stack_reserve();
    auto lowest = stack > base + 32768 ? stack - 32768 : base;
    xv::memory::kernel_page_table().grow_process_stack(base, lowest);
  }
  return 0;
}

namespace xv::trap
{
unsigned long scheduler_stack_top()
{
  auto &base = detail::scheduler_stacks[xv::process::cpu_id()];
  if (base) xv::print::panic("scheduler stack already initialized");
  detail::refill_stack_reserve();
  base = xv::memory::kernel_page_table().allocate_process_stack();
  if (!base) xv::print::panic("scheduler stack allocation failed");
  return base + KSTACK_BYTES;
}
}
