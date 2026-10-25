module;
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include "native_wait.h"
#include "kernel/trap/arch/riscv/entry.h"

module xv.trap;
import :platform;

namespace xv::trap::test
{
static_assert(TrapCause(8).kind() == CauseKind::UserSystemCall);
static_assert(TrapCause(TrapCause::InterruptBit | 8).kind() == CauseKind::Other);
static_assert(TrapCause(TrapCause::InterruptBit | 9).kind() ==
              CauseKind::ExternalInterrupt);
static_assert(TrapCause(TrapCause::InterruptBit | 5).kind() ==
              CauseKind::TimerInterrupt);
static_assert(TrapCause(13).kind() == CauseKind::LoadPageFault);
static_assert(TrapCause(15).kind() == CauseKind::StorePageFault);
static_assert(TrapCause(12).kind() == CauseKind::Other);
static_assert(TrapCause(~Word{0}).code() == TrapCause::InterruptBit - 1);
static_assert(std::is_abstract_v<TrapHandler>);
static_assert(!std::is_copy_constructible_v<TrapController>);
static_assert(!std::is_copy_constructible_v<TickClock>);

void check(bool condition, const char *message)
{
  if (!condition)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

struct State
{
  TrapEvent event{TrapCause(8), 0x1000, 0, 0x9000};
  bool has_process = true;
  bool interrupts = false;
  bool resolve_success = true;
  bool exec_in_syscall = false;
  bool kill_in_syscall = false;
  Word user_pc = 0;
  Word prepared_pc = 0;
  Word token = 0x8000000000012345UL;
  Word deadline = 0;
  Word fault_address = 0;
  bool fault_read = false;
  int installs = 0;
  int syscalls = 0;
  int external = 0;
  int faults = 0;
  int yields = 0;
  int reports = 0;
  int marked = 0;
  int preparations = 0;
  int restores = 0;
  std::vector<std::string> trace;
} state;

std::atomic<bool> killed_flag{false};
std::atomic<unsigned> registrations{0};
std::atomic<unsigned> wakeups{0};
std::atomic<Word> current_time{1234};
thread_local int current_hart = 0;
std::mutex trace_mutex;
void (*exit_check)() = nullptr;
void (*fatal_check)(const char *, const TrapEvent &) = nullptr;
void (*return_check)(Word) = nullptr;

void log(const char *step)
{
  std::lock_guard guard(trace_mutex);
  state.trace.emplace_back(step);
}

void reset(Word cause = 8, Word status = 0)
{
  state = State{};
  state.event.cause = TrapCause(cause);
  state.event.status = status;
  killed_flag = false;
  registrations = 0;
  wakeups = 0;
  current_time = 1234;
  current_hart = 0;
  exit_check = nullptr;
  fatal_check = nullptr;
  return_check = nullptr;
}

unsigned position(const char *step)
{
  for (unsigned i = 0; i < state.trace.size(); ++i)
  {
    if (state.trace[i] == step)
    {
      return i;
    }
  }
  check(false, "missing execution step");
  return 0;
}

template <class Operation> void expect_exit(int expected, Operation operation)
{
  auto child = fork();
  check(child >= 0, "fork subprocess");
  if (child == 0)
  {
    operation();
    std::_Exit(0);
  }
  int status;
  check(waitpid(child, &status, 0) == child, "wait subprocess");
  check(WIFEXITED(status) && WEXITSTATUS(status) == expected,
        "expected termination path");
}

class ProbeHandler final : public TrapHandler
{
public:

  int calls = 0;

  const char *name() const override
  {
    return "probe handler";
  }

  bool accepts(const TrapEvent &) const override
  {
    return true;
  }

  TrapAction handle(const TrapEvent &) override
  {
    ++calls;
    return TrapAction::Resume;
  }
};

void dispatch_contract()
{
  ProbeHandler first, second;
  TrapHandler *entries[] = {&first, &second};
  TrapDispatcher dispatcher(entries, 2);
  check(dispatcher.dispatch(state.event) == TrapAction::Resume && first.calls == 1 &&
            second.calls == 0,
        "dispatches to the first accepting handler");
  TrapDispatcher empty(nullptr, 0);
  check(empty.dispatch(state.event) == TrapAction::Unhandled,
        "unrecognized event is explicit");
  expect_exit(99,
              []
              {
                TrapDispatcher invalid(nullptr, 1);
                invalid.dispatch(state.event);
              });
}

void syscall_contract()
{
  reset();
  const auto token = ::usertrap(); // Exercise the actual C ABI entry wrapper.
  check(token == state.token && state.syscalls == 1 && state.user_pc == 0x1004,
        "system call advances the saved user PC once");
  check(state.prepared_pc == 0x1004 && !state.interrupts && state.installs == 1,
        "return uses the updated PC with interrupts off");
  check(position("kernel_vector") < position("save_pc") &&
            position("advance_pc") < position("enable") &&
            position("enable") < position("syscall"),
        "vector and PC established before enabling interrupts");
  reset();
  state.exec_in_syscall = true;
  ::usertrap();
  check(state.prepared_pc == 0x4000, "exec-installed PC survives trap return");

  reset();
  killed_flag = true;
  exit_check = []
  {
    check(state.syscalls == 0 && state.user_pc == 0x1000 && !state.interrupts &&
              state.preparations == 0,
          "already killed process exits before syscall/PC advance");
  };
  expect_exit(42, [] { ::usertrap(); });
  reset();
  state.kill_in_syscall = true;
  exit_check = []
  {
    check(state.syscalls == 1 && state.preparations == 0, "post-syscall killed check");
  };
  expect_exit(42, [] { ::usertrap(); });
}

void fault_contract()
{
  for (bool read : {true, false})
  {
    reset(read ? 13 : 15);
    ::usertrap();
    check(
        state.faults == 1 && state.fault_address == state.event.value &&
            state.fault_read == read,
        "load/store page faults reach the address space with the correct access mode");
    check(state.syscalls == 0 && state.prepared_pc == 0x1000 && state.marked == 0,
          "resolved fault retries the faulting instruction");
  }
  reset(15);
  state.resolve_success = false;
  exit_check = []
  {
    check(state.faults == 1 && state.reports == 1 && state.marked == 1 &&
              state.preparations == 0,
          "unresolved user fault is diagnosed and terminated");
  };
  expect_exit(42, [] { ::usertrap(); });
  reset(2);
  exit_check = []
  {
    check(state.reports == 1 && state.marked == 1 && state.faults == 0,
          "unknown user exception");
  };
  expect_exit(42, [] { ::usertrap(); });
  reset(12);
  exit_check = []
  {
    check(state.faults == 0 && state.marked == 1,
          "instruction page faults remain unsupported");
  };
  expect_exit(42, [] { ::usertrap(); });
}

void interrupt_contract()
{
  reset(TrapCause::InterruptBit | 9);
  ::usertrap();
  check(state.external == 1 && state.yields == 0 && state.syscalls == 0,
        "external user interrupt dispatches without a forced yield");
  reset(TrapCause::InterruptBit | 5);
  const auto before = clock().uptime();
  ::usertrap();
  check(clock().uptime() == before + 1 && state.yields == 1 &&
            state.deadline == current_time + TimerInterval,
        "timer interrupt counts, rearms and requests preemption");
  check(state.prepared_pc == 0x1000,
        "timer preemption preserves the interrupted user PC");

  reset(TrapCause::InterruptBit | 5, TrapEvent::SupervisorPrevious);
  const auto original = state.event;
  ::kerneltrap();
  check(state.yields == 1 && state.restores == 1 && state.event.pc == original.pc &&
            state.event.status == original.status,
        "kernel trap restores PC/status after a context switch clobbers CSRs");
  check(position("yield") < position("restore"), "restore follows scheduling");
  reset(TrapCause::InterruptBit | 5, TrapEvent::SupervisorPrevious);
  state.has_process = false;
  ::kerneltrap();
  check(state.yields == 0 && state.restores == 1,
        "idle hart handles a timer without yielding a process");
  reset(TrapCause::InterruptBit | 9, TrapEvent::SupervisorPrevious);
  ::kerneltrap();
  check(state.external == 1 && state.restores == 1,
        "kernel external IRQ is completed through the router");
}

void privilege_contract()
{
  reset(8, TrapEvent::SupervisorPrevious);
  expect_exit(99, [] { ::usertrap(); });
  reset(8);
  state.has_process = false;
  expect_exit(99, [] { ::usertrap(); });
  reset(TrapCause::InterruptBit | 9, 0);
  expect_exit(99, [] { ::kerneltrap(); });
  reset(TrapCause::InterruptBit | 9,
        TrapEvent::SupervisorPrevious | TrapEvent::InterruptEnable);
  expect_exit(99, [] { ::kerneltrap(); });
  for (Word cause : {Word{8}, Word{13}, Word{15}, TrapCause::InterruptBit | 3})
  {
    reset(cause, TrapEvent::SupervisorPrevious);
    fatal_check = [](const char *, const TrapEvent &event)
    {
      check(event.from_supervisor() && state.syscalls == 0 && state.faults == 0,
            "kernel exceptions never enter user syscall/demand-paging handlers");
    };
    expect_exit(99, [] { ::kerneltrap(); });
  }
}

void return_contract()
{
  reset();
  initialize_hart();
  check(state.installs == 1, "per-hart vector initialization");
  state.user_pc = 0x7890;
  return_check = [](Word token)
  {
    check(token == state.token && state.preparations == 1 && !state.interrupts &&
              state.prepared_pc == 0x7890,
          "initial process return prepares state before entering trampoline");
  };
  expect_exit(43, [] { return_to_user(); });
}

void clock_contract()
{
  reset();
  TickClock local(17);
  local.initialize();
  current_hart = 1;
  local.interrupt();
  check(local.uptime() == 17 && wakeups == 0 &&
            state.deadline == current_time + TimerInterval,
        "secondary harts rearm without incrementing global uptime");
  current_hart = 0;
  local.interrupt();
  check(local.uptime() == 18 && wakeups == 1,
        "hart 0 advances global ticks and wakes sleepers");
  killed_flag = true;
  check(local.pause(1) == -1 && local.pause(0) == 0 && local.pause(-1) == 0,
        "pause handles killed, zero, and negative intervals");
  killed_flag = false;

  TickClock wrapped(~0U - 1);
  wrapped.initialize();
  registrations = 0;
  std::atomic<int> result{-2};
  std::thread sleeper([&] { result = wrapped.pause(3); });
  for (unsigned tick = 1; tick <= 3; ++tick)
  {
    while (registrations.load() < tick)
    {
      std::this_thread::yield();
    }
    ++current_time;
    wrapped.interrupt();
  }
  sleeper.join();
  check(result == 0 && wrapped.uptime() == 1,
        "pause survives unsigned tick wraparound and actual wait/wakeup");
}
} // namespace xv::trap::test

namespace xv::trap::detail
{
TrapEvent snapshot()
{
  test::log("snapshot");
  return test::state.event;
}

void install_kernel_vector()
{
  ++test::state.installs;
  test::log("kernel_vector");
}

bool has_process()
{
  return test::state.has_process;
}

bool killed()
{
  return test::killed_flag.load();
}

void request_kill()
{
  ++test::state.marked;
  test::killed_flag = true;
  test::log("kill");
}

[[noreturn]] void exit_current(int status)
{
  test::check(status == -1, "trap termination status");
  if (test::exit_check)
  {
    test::exit_check();
  }
  std::_Exit(42);
}

void yield_current()
{
  ++test::state.yields;
  test::log("yield");
  test::state.event.pc = 0xbad;
  test::state.event.status = 0;
}

void save_user_pc(Word pc)
{
  test::state.user_pc = pc;
  test::log("save_pc");
}

void advance_user_pc(Word bytes)
{
  test::state.user_pc += bytes;
  test::log("advance_pc");
}

void enable_interrupts()
{
  test::state.interrupts = true;
  test::log("enable");
}

void dispatch_system_call()
{
  test::check(test::state.interrupts, "syscall runs with interrupts enabled");
  ++test::state.syscalls;
  test::log("syscall");
  if (test::state.exec_in_syscall)
  {
    test::state.user_pc = 0x4000;
  }
  if (test::state.kill_in_syscall)
  {
    test::killed_flag = true;
  }
}

void dispatch_external_interrupt()
{
  ++test::state.external;
  test::log("external");
}

bool resolve_page_fault(Word address, bool read)
{
  ++test::state.faults;
  test::state.fault_address = address;
  test::state.fault_read = read;
  test::log("page_fault");
  return test::state.resolve_success;
}

void report_user_fault(const TrapEvent &)
{
  ++test::state.reports;
  test::log("report");
}

[[noreturn]] void fatal(const char *message, const TrapEvent &event)
{
  if (test::fatal_check)
  {
    test::fatal_check(message, event);
  }
  std::_Exit(99);
}

void prepare_user_return()
{
  ++test::state.preparations;
  test::state.prepared_pc = test::state.user_pc;
  test::state.interrupts = false;
  test::log("prepare_return");
}

Word user_page_table_token()
{
  return test::state.token;
}

void restore_kernel(const TrapEvent &event)
{
  ++test::state.restores;
  test::state.event.pc = event.pc;
  test::state.event.status = event.status;
  test::log("restore");
}

[[noreturn]] void enter_user(Word token)
{
  if (test::return_check)
  {
    test::return_check(token);
  }
  std::_Exit(43);
}

int hart_id()
{
  return test::current_hart;
}

Word time()
{
  return test::current_time.load();
}

void arm_timer(Word deadline)
{
  test::state.deadline = deadline;
  test::log("arm");
}

void prepare_sleep(void *channel)
{
  xv_test_prepare_sleep(channel);
  ++test::registrations;
}

void sleep()
{
  xv_test_sleep();
}

void wakeup(void *channel)
{
  ++test::wakeups;
  xv_test_wakeup(channel);
}
} // namespace xv::trap::detail

extern "C" int run_trap_contracts()
{
  using namespace xv::trap;
  initialize();
  test::dispatch_contract();
  test::syscall_contract();
  test::fault_contract();
  test::interrupt_contract();
  test::privilege_contract();
  test::return_contract();
  test::clock_contract();
  std::puts("trap contracts: passed");
  return 0;
}
