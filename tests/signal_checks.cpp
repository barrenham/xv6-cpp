module;
#include <cstdio>
#include <cstdlib>
#include <thread>
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
} // namespace xv::process::signal_test

extern "C" int run_signal_contracts()
{
  xv::process::signal_test::pending();
  xv::process::signal_test::dispositions();
  xv::process::signal_test::stop_continue();
  xv::process::signal_test::masks_and_handlers();
  std::puts("pending signal contracts: passed");
  return 0;
}
