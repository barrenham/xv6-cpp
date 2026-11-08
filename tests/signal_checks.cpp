module;
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <atomic>
#include <vector>
#include "kernel/process/signals/signal.h"

module xv.process;
import :signals;

namespace xv::process::signal_test
{
void check(bool ok, const char *message)
{
  if (!ok)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

void pending()
{
  PendingSignals signals;
  check(signals.pending() == 0 && signals.terminating_signal() == 0,
        "empty pending set");
  check(!signals.post(0) && !signals.post(-1) && !signals.post(3) &&
            !signals.post(32) && signals.pending() == 0,
        "unsupported signal validation");
  check(signals.post(SIGTERM) && signals.post(SIGTERM) &&
            signals.pending() == (1U << SIGTERM),
        "standard signals coalesce");
  check(signals.terminating_signal() == SIGTERM, "termination selection");
  signals.post(SIGINT);
  signals.post(SIGKILL);
  check(signals.terminating_signal() == SIGKILL, "SIGKILL takes priority");
  signals.reset();
  check(signals.pending() == 0, "reap reset");
  check(signals.post(SIGHUP) && signals.terminating_signal() == SIGHUP,
        "hangup default termination");
  check(signals.disposition(SIGHUP, SIGNAL_IGNORE) == SIGNAL_DEFAULT &&
        signals.pending() == 0 && signals.post(SIGHUP) && signals.pending() == 0,
        "hangup ignore disposition");
  signals.reset();
  std::vector<std::thread> senders;
  for (int i = 0; i < 6; ++i)
  {
    senders.emplace_back(
        [&, i]
        {
          for (int round = 0; round < 10000; ++round)
          {
            signals.post(i % 3 == 0 ? SIGINT : i % 3 == 1 ? SIGTERM : SIGKILL);
          }
        });
  }
  for (auto &sender : senders)
  {
    sender.join();
  }
  check(signals.pending() == ((1U << SIGINT) | (1U << SIGKILL) | (1U << SIGTERM)),
        "concurrent posting loses no signal bits");
}

void dispositions()
{
  PendingSignals parent, child;
  parent.post(SIGINT);
  check(parent.disposition(SIGINT, SIGNAL_IGNORE) == SIGNAL_DEFAULT &&
            parent.pending() == 0,
        "ignore clears pending atomically");
  parent.post(SIGINT);
  parent.post(SIGTERM);
  child.inherit_dispositions(parent);
  check(child.pending() == 0 && child.post(SIGINT) && child.pending() == 0,
        "fork inherits ignored disposition but no pending signals");
  check(child.disposition(SIGINT, SIGNAL_DEFAULT) == SIGNAL_IGNORE &&
            child.post(SIGINT) && child.terminating_signal() == SIGINT,
        "command restores default action");
  check(child.disposition(SIGKILL, SIGNAL_IGNORE) == -1 &&
            child.disposition(SIGINT, 99) == -1,
        "SIGKILL and invalid actions rejected");
  PendingSignals race;
  std::thread sender(
      [&]
      {
        for (int i = 0; i < 10000; ++i)
        {
          race.post(SIGINT);
        }
      });
  for (int i = 0; i < 10000; ++i)
  {
    race.disposition(SIGINT, SIGNAL_IGNORE);
    race.disposition(SIGINT, SIGNAL_DEFAULT);
  }
  sender.join();
  race.disposition(SIGINT, SIGNAL_IGNORE);
  check(race.pending() == 0, "concurrent posting cannot survive final ignore");
}

void stop_continue()
{
  PendingSignals signals;
  check(signals.post(SIGTSTP) && signals.stopping_signal() == SIGTSTP, "stop request");
  signals.post(SIGCONT);
  check(signals.stopping_signal() == 0 && (signals.pending() & (1U << SIGCONT)),
        "continue cancels pending stop");
  signals.post(SIGSTOP);
  check(!(signals.pending() & (1U << SIGCONT)) && signals.take_stop() == SIGSTOP &&
            signals.stopping_signal() == 0,
        "stop cancels continue and is consumed once");
  check(signals.disposition(SIGSTOP, SIGNAL_IGNORE) == -1, "SIGSTOP cannot be ignored");
  signals.disposition(SIGTSTP, SIGNAL_IGNORE);
  signals.post(SIGTSTP);
  check(signals.stopping_signal() == 0, "shell may ignore SIGTSTP");
  signals.post(SIGTTIN);
  check(signals.take_stop() == SIGTTIN, "background read stop");
}
void masks_and_handlers()
{
  PendingSignals state, child;
  state.set_action(SIGUSR1, {0x1000, 1U << SIGUSR2, 0, 0x2000});
  state.set_mask((1U << SIGUSR1) | PendingSignals::Unmaskable);
  check(state.mask() == (1U << SIGUSR1), "SIGKILL/SIGSTOP cannot be blocked");
  state.post(SIGUSR1);
  check(!state.interrupting() && state.pending() == (1U << SIGUSR1),
        "masked caught signal remains pending without interrupting waits");
  child.inherit_dispositions(state);
  check(!child.pending() && child.mask() == state.mask() &&
        child.action(SIGUSR1).handler == 0x1000, "fork inherits handlers and mask");
  state.set_mask(0);
  check(state.caught_signal() == SIGUSR1 && !state.terminating_signal(),
        "caught signal interrupts without default termination");
  state.consume(SIGUSR1);
  check(!state.interrupting(), "delivered signal consumed");
  state.post(SIGUSR1);
  state.set_mask(1U << SIGUSR1);
  state.disposition(SIGTERM, SIGNAL_IGNORE);
  state.on_exec();
  check(state.pending() == (1U << SIGUSR1) && state.mask() == (1U << SIGUSR1) &&
        state.action(SIGUSR1).handler == 0 &&
        state.action(SIGTERM).handler == SIGNAL_IGNORE, "exec disposition rules");
  state.set_mask(0);
  check(state.terminating_signal() == SIGUSR1, "exec pending uses new default action");
  state.reset();
  state.disposition(SIGCONT, SIGNAL_IGNORE);
  state.post(SIGSTOP);
  state.post(SIGCONT);
  check(state.continuing() && !state.pending() && !state.stopping_signal(),
        "ignored SIGCONT still resumes and cancels stops");
  state.disposition(SIGCONT, SIGNAL_DEFAULT);
  state.set_action(SIGTSTP, {0x1000, 0, 0, 0x2000});
  state.set_mask(1U << SIGTTIN);
  state.post(SIGTTIN);
  state.post(SIGTSTP);
  state.post(SIGSTOP);
  check(state.take_stop() == SIGSTOP && state.caught_signal() == SIGTSTP &&
        (state.pending() & (1U << SIGTTIN)), "default stop preserves blocked/caught stops");
}

void
shared_dispositions() {
  PendingSignals leader, first, second;
  leader.set_action(SIGUSR1, {0x1000, 1U << SIGUSR2, 0, 0x2000});
  leader.set_mask(1U << SIGUSR1);
  first.share_dispositions(leader);
  second.share_dispositions(first);
  check(first.action(SIGUSR1).handler == 0x1000 && second.mask() == leader.mask(),
        "new threads share actions and inherit the calling mask");
  first.set_mask(0);
  check(second.mask() == leader.mask(), "thread masks remain independent");
  first.set_action(SIGUSR2, {0x3000, 0, 0, 0x4000});
  check(leader.action(SIGUSR2).handler == 0x3000 &&
            second.action(SIGUSR2).restorer == 0x4000,
        "handler changes are visible throughout a thread group");
  first.post(SIGUSR2);
  second.post(SIGUSR2);
  leader.disposition(SIGUSR2, SIGNAL_IGNORE);
  check(!first.pending() && !second.pending() && first.post(SIGUSR2) &&
            !first.pending(),
        "shared ignore clears pending bits in every member");
  second.on_exec();
  check(!leader.action(SIGUSR1).handler &&
            first.action(SIGUSR2).handler == SIGNAL_IGNORE,
        "exec resets shared caught actions and preserves ignored actions");
  first.set_action(SIGUSR1, {0x5000, 0, 0x80000000U, 0x6000});
  first.post(SIGUSR1);
  int signal = 0;
  PendingSignals::Action selected;
  check(first.take_handler(signal, selected) && signal == SIGUSR1 &&
            selected.handler == 0x5000 && !first.pending() &&
            !leader.action(SIGUSR1).handler,
        "handler snapshot, pending consume and one-shot reset are atomic");
  second.post(SIGUSR1);
  second.set_mask(0);
  check(second.terminating_signal() == SIGUSR1,
        "one-shot reset is visible to another thread with a pending signal");
  second.consume(SIGUSR1);
  std::atomic<bool> done{false};
  std::thread changing([&] {
    for (unsigned round = 0; round < 2000; ++round)
    {
      first.set_action(SIGUSR1, round & 1
                                    ? PendingSignals::Action{0x1000, 7, 9, 0x2000}
                                    : PendingSignals::Action{0x3000, 11, 13, 0x4000});
    }
    done.store(true);
  });
  while (!done.load())
  {
    auto action = leader.action(SIGUSR1);
    check(!action.handler ||
              (action.handler == 0x1000 && action.mask == 7 && action.flags == 9 &&
               action.restorer == 0x2000) ||
              (action.handler == 0x3000 && action.mask == 11 && action.flags == 13 &&
               action.restorer == 0x4000),
          "concurrent disposition reads never tear a multiword handler action");
  }
  changing.join();
  second.reset();
  first.reset();
  leader.reset();
}

void
process_delivery() {
  PendingSignals leader, first, second;
  leader.set_action(SIGUSR1, {0x1000, 0, 0, 0x2000});
  unsigned long generation;
  auto previous = leader.action(SIGUSR1, &generation);
  leader.set_action(SIGUSR2, {});
  check(!leader.set_action_if(SIGUSR1, {}, generation) &&
            leader.action(SIGUSR1).handler == previous.handler,
        "sigaction retries a stale old-value snapshot without changing the action");
  leader.action(SIGUSR1, &generation);
  check(leader.set_action_if(SIGUSR1, previous, generation),
        "sigaction commits against its exact old-value snapshot");
  leader.set_mask(1U << SIGUSR1);
  first.share_dispositions(leader);
  second.share_dispositions(first);
  check(leader.post_process(SIGUSR1) && !leader.caught_signal() &&
            !first.caught_signal() && !second.caught_signal() &&
            leader.process_pending() == (1U << SIGUSR1),
        "process signal remains pending when every thread blocks it");
  first.set_mask(0);
  second.set_mask(0);
  check(first.caught_signal() == SIGUSR1 && !second.caught_signal() &&
            !leader.process_pending(),
        "one eligible thread claims each process signal");
  leader.deactivate();
  check(first.deactivate() && second.caught_signal() == SIGUSR1,
        "an exiting selected thread returns undelivered process signals");
  second.consume(SIGUSR1);
  first.post(SIGUSR1);
  check(!first.deactivate() && !second.caught_signal(),
        "thread-directed signals never migrate when their target exits");
  leader.post_process(SIGUSR1);
  check(!leader.caught_signal() && !first.caught_signal() &&
            second.caught_signal() == SIGUSR1,
        "process routing survives an exited group leader");
  second.consume(SIGUSR1);
  second.set_mask(1U << SIGUSR1);
  leader.post_process(SIGUSR1);
  first.disposition(SIGUSR1, SIGNAL_IGNORE);
  check(!leader.process_pending(), "shared ignore discards process-pending signals");
  second.reset();
  first.reset();
  leader.reset();
}

void
group_stop_delivery() {
  PendingSignals leader, worker;
  leader.set_mask(1U << SIGTSTP);
  worker.share_dispositions(leader);
  leader.post_process(SIGTSTP);
  check(!leader.stopping_signal(), "blocked group stop remains pending");
  worker.set_mask(0);
  check(worker.stopping_signal() == SIGTSTP && leader.stopping_signal() == SIGTSTP &&
            worker.take_stop() == SIGTSTP && worker.take_stop() == SIGTSTP,
        "a selected default stop persists across every thread's individual mask");
  leader.post_process(SIGCONT);
  check(!worker.stopping_signal() && !leader.stopping_signal() && worker.continuing() &&
            leader.continuing(),
        "continue cancels the group stop and resumes every member");
  worker.consume_continue();
  leader.consume_continue();
  leader.set_action(SIGTSTP, {0x1000, 0, 0, 0x2000});
  worker.post(SIGTSTP);
  check(!worker.stopping_signal() && worker.caught_signal() == SIGTSTP &&
            !leader.group_stop_signal(),
        "a caught stop signal remains an ordinary per-thread handler");
  worker.reset();
  leader.reset();
}
void child_notifications()
{
  PendingSignals state;
  check(state.post(SIGCHLD) && !state.interrupting() &&
        !state.terminating_signal(), "SIGCHLD defaults to ignoring, not termination");
  state.set_action(SIGCHLD, {0x1000, 0, 0, 0x2000});
  check(state.caught_signal() == SIGCHLD, "blocked/default pending can be caught");
  state.set_mask(1U << SIGCHLD);
  state.post(SIGCHLD);
  state.post(SIGCHLD);
  check(!state.interrupting() && state.pending() == (1U << SIGCHLD),
        "blocked child notifications coalesce");
  state.set_action(SIGCHLD, {0x3000, 0, 0, 0x2000});
  check(state.pending() == (1U << SIGCHLD), "replacing handler preserves pending");
  state.disposition(SIGCHLD, SIGNAL_DEFAULT);
  check(!state.pending(), "restoring default ignore discards pending SIGCHLD");
  state.disposition(SIGCHLD, SIGNAL_IGNORE);
  state.post(SIGCHLD);
  check(!state.pending(), "explicit ignore drops SIGCHLD");
}
} // namespace xv::process::signal_test

extern "C" int run_signal_contracts()
{
  xv::process::signal_test::pending();
  xv::process::signal_test::dispositions();
  xv::process::signal_test::stop_continue();
  xv::process::signal_test::masks_and_handlers();
  xv::process::signal_test::shared_dispositions();
  xv::process::signal_test::process_delivery();
  xv::process::signal_test::group_stop_delivery();
  xv::process::signal_test::child_notifications();
  std::puts("pending signal contracts: passed");
  return 0;
}
