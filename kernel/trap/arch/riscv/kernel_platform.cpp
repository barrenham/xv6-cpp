module;
#include "kernel/arch/riscv/register_constants.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/support/flow.h"
#include "kernel/support/param.h"
#include "kernel/support/types.h"
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
struct StackReserve
{
  void *pages[StackReservePages]{};
  unsigned count = 0;
};
StackReserve stack_reserves[NCPU];

namespace flow = xv::flow;
struct RefillReserveFlow
{
  struct Context
  {
    StackReserve &reserve;
    void *page = nullptr;
  };
  static bool space(Context &c) noexcept { return c.reserve.count < StackReservePages; }
  static bool allocate(Context &c) noexcept
  {
    c.page = xv::memory::allocator().allocate();
    return c.page;
  }
  static void publish(Context &c) noexcept
  {
    c.reserve.pages[c.reserve.count++] = c.page;
  }
  struct Space;
  struct Allocate;
  struct Publish;
  using Graph = flow::graph<
      Space, flow::types<flow::node<Space, flow::noop,
                                    flow::branch<flow::condition<&space>,
                                                 flow::go<Allocate>, flow::stop>>,
                         flow::node<Allocate, flow::noop,
                                    flow::branch<flow::condition<&allocate>,
                                                 flow::go<Publish>, flow::stop>>,
                         flow::node<Publish, flow::action<&publish>, flow::go<Space>>>>;
};

void refill_stack_reserve()
{
  if (xv::process::current_cpu()->noff)
  {
    return;
  }
  auto &reserve = stack_reserves[xv::process::cpu_id()];
  RefillReserveFlow::Context context{reserve};
  (void)RefillReserveFlow::Graph::run<0>(context);
}
void *take_stack_frame()
{
  if (!xv::process::current_cpu()->noff)
  {
    if (auto *page = xv::memory::allocator().allocate())
    {
      return page;
    }
  }
  auto &reserve = stack_reserves[xv::process::cpu_id()];
  return reserve.count ? reserve.pages[--reserve.count] : nullptr;
}
#if XV_VM_SELF_TEST
#if XV_VM_SELF_TEST == 7
#include "tests/slab_kernel_probe.h"
#endif
__attribute__((noinline)) unsigned long stack_probe(unsigned depth)
{
  volatile unsigned char page[4096];
  for (unsigned i = 0; i < sizeof(page); ++i)
  {
    page[i] = depth;
  }
  page[4095] = depth + 1;
  auto next = depth ? stack_probe(depth - 1) : 0;
  return next + page[0] + page[4095];
}
void run_vm_kernel_probe()
{
  auto base = xv::process::current()->kernel_stack();
#if XV_VM_SELF_TEST == 7
  slab_kernel_probe(base);
#elif XV_VM_SELF_TEST == 6
  (void)base;
  auto &table = xv::memory::kernel_page_table();
  auto fixture = table.allocate_process_stack();
  if (!fixture) xv::print::panic("kernel virtual pool: OOM fixture failed");
  table.free_process_stack(fixture);
  auto initial = xv::memory::allocator().free_pages();
  void *held = nullptr;
  while (auto *page = xv::memory::allocator().allocate())
  {
    *static_cast<void **>(page) = held;
    held = page;
  }
  for (unsigned budget = 0; budget <= 12; ++budget)
  {
    if (budget && held)
    {
      auto *page = held;
      held = *static_cast<void **>(page);
      xv::memory::allocator().deallocate(page);
    }
    auto available = xv::memory::allocator().free_pages();
    auto address = table.allocate_process_stack();
    if ((budget < KSTACK_INITIAL_PAGES + KSTACK_BYTES / (512 * PGSIZE)) == bool(address))
      xv::print::panic("kernel virtual pool: unexpected allocation budget");
    if (address)
    {
      if (address != fixture) xv::print::panic("kernel virtual pool: rollback lost range");
      table.free_process_stack(address);
    }
    else
    {
      xv::memory::VirtualRegion region;
      if (table.region(fixture, region) || table.region(fixture - 1, region))
        xv::print::panic("kernel virtual pool: rollback retained regions");
    }
    if (xv::memory::allocator().free_pages() != available)
      xv::print::panic("kernel virtual pool: OOM rollback leaked frames");
  }
  while (held)
  {
    auto *page = held;
    held = *static_cast<void **>(page);
    xv::memory::allocator().deallocate(page);
  }
  if (xv::memory::allocator().free_pages() != initial)
    xv::print::panic("kernel virtual pool: OOM fixture leaked frames");
  if (table.allocate_process_stack() != fixture)
    xv::print::panic("kernel virtual pool: OOM recovery lost range");
  table.free_process_stack(fixture);
  xv::print::printk("kernel virtual pool: OOM rollback and recovery OK\n");
#elif XV_VM_SELF_TEST == 5
  static unsigned long addresses[1000];
  auto &table = xv::memory::kernel_page_table();
  // Stabilize the running probe's own stack/margin before measuring resources
  // belonging to the 1000 independent test stacks.
  if (!table.grow_process_stack(base, base + KSTACK_BYTES - 32 * PGSIZE))
    xv::print::panic("kernel virtual pool: probe stack preparation failed");
  auto initial = xv::memory::allocator().free_pages();
  for (auto &address : addresses)
  {
    address = table.allocate_process_stack();
    if (!address) xv::print::panic("kernel virtual pool: 1000 stack allocation failed");
    xv::memory::VirtualRegion region;
    if (!table.region(address, region) || region.kind != xv::memory::RegionKind::KernelStack ||
        !table.region(address - 1, region) || region.kind != xv::memory::RegionKind::Guard)
      xv::print::panic("kernel virtual pool: region mismatch");
  }
  for (unsigned i = 0; i < 1000; i += 2) table.free_process_stack(addresses[i]);
  if (table.handle_stack_fault(addresses[0], addresses[0] + KSTACK_BYTES,
                               addresses[0], xv::memory::FaultAccess::Write,
                               take_stack_frame) != xv::memory::FaultResult::OutsideRegion)
    xv::print::panic("kernel virtual pool: freed stack remained published");
  for (unsigned i = 0; i < 1000; i += 2)
    if (table.allocate_process_stack() != addresses[i])
      xv::print::panic("kernel virtual pool: fragmented range not reused");
  for (auto address : addresses) table.free_process_stack(address);
  auto warm = xv::memory::allocator().free_pages();
  if (warm != initial)
  {
    xv::print::printk("kernel virtual pool: free pages %lu -> %lu\n", initial, warm);
    xv::print::panic("kernel virtual pool: empty metadata or directory retained");
  }
  // A second complete cycle must reuse ranges and reclaim all extra metadata.
  for (auto &address : addresses)
  {
    address = table.allocate_process_stack();
    if (!address) xv::print::panic("kernel virtual pool: repeat allocation failed");
  }
  for (auto address : addresses) table.free_process_stack(address);
  if (xv::memory::allocator().free_pages() != warm)
    xv::print::panic("kernel virtual pool: stack resource leak");
  xv::print::printk("kernel virtual pool: 1000 stacks, fragmentation and reuse OK\n");
#elif XV_VM_SELF_TEST == 2
  auto *entry =
      xv::memory::walk_entry(static_cast<xv::memory::PageTable>(const_cast<void *>(
                                 xv::memory::kernel_page_table().root())),
                             base + KSTACK_BYTES - PGSIZE, 0);
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
  xv::print::printk("stack probe checksum=%lu reserve=%u->%u\n", checksum, before,
                    after);
  if (checksum != 2401 || after >= before)
  {
    xv::print::panic("VM locked stack probe failed");
  }
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
  auto top =
      reinterpret_cast<unsigned long>(&entry_stacks[xv::process::cpu_id()][16384]);
  asm volatile("csrw sscratch, %0" : : "r"(top));
  xv::arch::w_stvec(reinterpret_cast<Word>(kernelvec));
}

bool has_process() { return xv::process::current() != nullptr; }

void deliver_signals() { xv::process::deliver_signals(); }

bool interrupted() { return xv::process::current()->is_killed(); }

bool killed() { return xv::process::current()->termination_status() != 0; }

void request_kill() { xv::process::current()->request_kill(); }

[[noreturn]] void exit_current(int status) { xv::process::exit_process(status); }

void yield_current() { xv::process::scheduler().yield(); }
bool scheduler_timer_tick() { return xv::process::scheduler().timer_tick(); }

void save_user_pc(Word pc) { xv::process::current()->registers()->epc = pc; }

void advance_user_pc(Word bytes) { xv::process::current()->registers()->epc += bytes; }

void enable_interrupts() { xv::arch::intr_on(); }

void dispatch_system_call() { xv::syscall::dispatch(); }

void dispatch_external_interrupt()
{
  xv::drivers::handle_external_interrupt(xv::process::cpu_id());
  // UART/TTY locks have been released and the PLIC source completed.
  xv::process::service_signal_notifications();
}

bool resolve_page_fault(Word address, MemoryAccess access)
{
  return xv::process::current()
             ->memory()
             .handle_fault(address, access == MemoryAccess::Read
                                        ? xv::memory::FaultAccess::Read
                                    : access == MemoryAccess::Write
                                        ? xv::memory::FaultAccess::Write
                                        : xv::memory::FaultAccess::Execute)
             .result == xv::memory::FaultResult::Resolved;
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

struct UserReturnFlow
{
  struct Context
  {
    xv::process::Process *process;
    xv::process::TrapFrame *frame = nullptr;
  };
  static void mask(Context &) noexcept { xv::arch::intr_off(); }
  static void vector(Context &c) noexcept
  {
    const auto address = TRAMPOLINE + (reinterpret_cast<Word>(uservec) -
                                       reinterpret_cast<Word>(trampoline));
    xv::arch::w_stvec(address);
    c.frame = c.process->registers();
  }
  static void frame(Context &c) noexcept
  {
    c.frame->kernel_satp = xv::arch::r_satp();
    c.frame->kernel_sp = c.process->kernel_stack() + KSTACK_PAGES * PGSIZE;
    c.frame->kernel_trap = reinterpret_cast<Word>(usertrap);
    c.frame->kernel_hartid = xv::arch::r_tp();
  }
  static void commit(Context &c) noexcept
  {
    auto status = xv::arch::r_sstatus();
    status &= ~SSTATUS_SPP;
    status |= SSTATUS_SPIE;
    xv::arch::w_sstatus(status);
    xv::arch::w_sepc(c.frame->epc);
  }
  struct Mask;
  struct Vector;
  struct Frame;
  struct Commit;
  using Graph = flow::graph<
      Mask, flow::types<flow::node<Mask, flow::action<&mask>, flow::go<Vector>>,
                        flow::node<Vector, flow::action<&vector>, flow::go<Frame>>,
                        flow::node<Frame, flow::action<&frame>, flow::go<Commit>>,
                        flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

void prepare_user_return()
{
#if XV_VM_SELF_TEST
  static bool tested = false;
  if (!tested)
  {
    tested = true;
    run_vm_kernel_probe();
  }
#endif
  auto *process = xv::process::current();
  UserReturnFlow::Context context{process};
  (void)UserReturnFlow::Graph::run<0>(context);
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

int hart_id() { return xv::process::cpu_id(); }

Word time() { return xv::arch::r_time(); }

void arm_timer(Word deadline) { xv::arch::w_stimecmp(deadline); }

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

void prepare_sleep(void *channel) { xv::process::prepare_sleep(channel); }

void sleep() { xv::process::sleep(); }

void wakeup(void *channel) { xv::process::wakeup(channel); }
struct KernelStackPrepareFlow
{
  struct Context
  {
    unsigned long base, stack, cause, fault;
    xv::memory::FaultResult fault_result = xv::memory::FaultResult::Resolved;
    unsigned long lowest = 0;
    xv::memory::KernelPageTable *table = nullptr;
    pte_t *entry = nullptr;
    int result = 0;
  };
  static bool within(Context &c) noexcept
  {
    return c.stack >= c.base && c.stack <= c.base + KSTACK_BYTES;
  }
  static void outside(Context &c) noexcept
  {
    if (xv::memory::kernel_page_table().in_stack_window(c.stack))
    {
      xv::print::panic("kernel stack reserve exhausted");
    }
  }
  static bool stack_fault(Context &c) noexcept
  {
    bool page_fault = c.cause == 12 || c.cause == 13 || c.cause == 15;
    bool in_stack = c.fault >= c.base && c.fault < c.base + KSTACK_BYTES;
    bool in_guard = c.fault >= c.base - KSTACK_BYTES && c.fault < c.base;
    return page_fault && (in_stack || in_guard);
  }
  static void resolve(Context &c) noexcept
  {
    auto access = c.cause == 12   ? xv::memory::FaultAccess::Execute
                  : c.cause == 15 ? xv::memory::FaultAccess::Write
                                  : xv::memory::FaultAccess::Read;
    c.fault_result = xv::memory::kernel_page_table().handle_stack_fault(
        c.base, c.stack, c.fault, access, take_stack_frame);
  }
  static bool resolved(Context &c) noexcept
  {
    return c.fault_result == xv::memory::FaultResult::Resolved;
  }
  // Return directly to the trapped instruction, without registry or refill work.
  static void handled(Context &c) noexcept { c.result = 1; }
  static void diagnose(Context &c) noexcept
  {
    if (c.fault_result == xv::memory::FaultResult::Permission)
    {
      xv::print::panic("kernel stack: protection fault");
    }
    if (c.fault_result == xv::memory::FaultResult::AlreadyMapped)
    {
      xv::print::panic("kernel stack: unexpected mapped-page fault");
    }
    if (c.fault_result == xv::memory::FaultResult::OutsideRegion)
    {
      xv::print::panic("kernel stack: guard overflow");
    }
    if (c.fault_result == xv::memory::FaultResult::OutOfMemory)
    {
      xv::print::panic("kernel stack: emergency frames exhausted");
    }
    xv::print::panic("kernel stack: invalid mapping");
  }
  static bool unlocked(Context &) noexcept { return !xv::process::current_cpu()->noff; }
  static void margin(Context &c) noexcept
  {
    refill_stack_reserve();
    c.lowest = c.stack > c.base + 32768 ? c.stack - 32768 : c.base;
    c.table = &xv::memory::kernel_page_table();
    // Running ownership pins the leaf tables. A committed contiguous suffix means
    // a valid lowest margin page proves the entire requested margin exists.
    c.entry = xv::memory::walk_entry(
        static_cast<xv::memory::PageTable>(const_cast<void *>(c.table->root())),
        c.lowest & ~(PGSIZE - 1UL), 0);
  }
  // Preserve the mapped-margin fast path: no global stack lock on a hit.
  static bool needs_growth(Context &c) noexcept
  {
    return !c.entry || !(__atomic_load_n(c.entry, __ATOMIC_ACQUIRE) & PTE_V);
  }
  static void grow(Context &c) noexcept
  {
    c.table->grow_process_stack(c.base, c.lowest);
  }
  struct Within;
  struct Fault;
  struct Resolve;
  struct Handled;
  struct Diagnose;
  struct Outside;
  struct Unlocked;
  struct Margin;
  struct Grow;
  using Graph = flow::graph<
      Within,
      flow::types<flow::node<Within, flow::noop,
                             flow::branch<flow::condition<&within>, flow::go<Fault>,
                                          flow::go<Outside>>>,
                  flow::node<Fault, flow::noop,
                             flow::branch<flow::condition<&stack_fault>,
                                          flow::go<Resolve>, flow::go<Unlocked>>>,
                  flow::node<Resolve, flow::action<&resolve>,
                             flow::branch<flow::condition<&resolved>, flow::go<Handled>,
                                          flow::go<Diagnose>>>,
                  flow::node<Handled, flow::action<&handled>, flow::stop>,
                  flow::node<Diagnose, flow::action<&diagnose>, flow::stop>,
                  flow::node<Outside, flow::action<&outside>, flow::stop>,
                  flow::node<Unlocked, flow::noop,
                             flow::branch<flow::condition<&unlocked>, flow::go<Margin>,
                                          flow::stop>>,
                  flow::node<Margin, flow::action<&margin>,
                             flow::branch<flow::condition<&needs_growth>,
                                          flow::go<Grow>, flow::stop>>,
                  flow::node<Grow, flow::action<&grow>, flow::stop>>>;
};
struct SchedulerStackFlow
{
  struct Context
  {
    unsigned long &base;
  };
  static bool exists(Context &c) noexcept { return c.base != 0; }
  static void duplicate(Context &) noexcept
  {
    xv::print::panic("scheduler stack already initialized");
  }
  static void refill(Context &) noexcept { refill_stack_reserve(); }
  static bool allocate(Context &c) noexcept
  {
    c.base = xv::memory::kernel_page_table().allocate_process_stack();
    return c.base != 0;
  }
  static void failed(Context &) noexcept
  {
    xv::print::panic("scheduler stack allocation failed");
  }
  struct Exists;
  struct Duplicate;
  struct Refill;
  struct Allocate;
  struct Failed;
  using Graph = flow::graph<
      Exists,
      flow::types<flow::node<Exists, flow::noop,
                             flow::branch<flow::condition<&exists>, flow::go<Duplicate>,
                                          flow::go<Refill>>>,
                  flow::node<Duplicate, flow::action<&duplicate>, flow::stop>,
                  flow::node<Refill, flow::action<&refill>, flow::go<Allocate>>,
                  flow::node<Allocate, flow::noop,
                             flow::branch<flow::condition<&allocate>, flow::stop,
                                          flow::go<Failed>>>,
                  flow::node<Failed, flow::action<&failed>, flow::stop>>>;
};

} // namespace xv::trap::detail

extern "C" int kernel_stack_prepare(unsigned long stack, unsigned long cause,
                                    unsigned long fault)
{
  auto *process = xv::process::current();
  auto base = process ? process->kernel_stack()
                      : xv::trap::detail::scheduler_stacks[xv::process::cpu_id()];
  if (!base)
  {
    return 0;
  }
  xv::trap::detail::KernelStackPrepareFlow::Context context{base, stack, cause, fault};
  (void)xv::trap::detail::KernelStackPrepareFlow::Graph::run<0>(context);
  return context.result;
}

namespace xv::trap
{
unsigned long scheduler_stack_top()
{
  auto &base = detail::scheduler_stacks[xv::process::cpu_id()];
  detail::SchedulerStackFlow::Context context{base};
  (void)detail::SchedulerStackFlow::Graph::run<0>(context);
  return base + KSTACK_BYTES;
}
} // namespace xv::trap
