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
  check(!signals.post(0) && !signals.post(-1) && !signals.post(1) &&
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
} // namespace xv::process::signal_test

extern "C" int run_signal_contracts()
{
  xv::process::signal_test::pending();
  xv::process::signal_test::dispositions();
  xv::process::signal_test::stop_continue();
  std::puts("pending signal contracts: passed");
  return 0;
}
