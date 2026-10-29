#include "kernel/process/scheduler/policies/scheduling_policy.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace xv::scheduling;
static void check(bool value, const char *message)
{
  if (!value) { std::fprintf(stderr, "scheduling_policy: %s\n", message); std::abort(); }
}
static Entity *take(RunQueue &queue, Time now)
{
  return queue.take(now, [](const Entity &) { return true; });
}
static void fairness(Kind kind, unsigned weight, bool short_weighted = false)
{
  RunQueue queue(kind, 10);
  Entity first, second;
  second.weight = weight;
  check(queue.enqueue(first, 0) && queue.enqueue(second, 0), "initial enqueue");
  check(!queue.enqueue(first, 0), "duplicate enqueue rejected");
  Time now = 0;
  for (unsigned i = 0; i < 30000; ++i)
  {
    auto *entity = take(queue, now);
    check(entity && queue.start(*entity, now), "dispatch once");
    now += (entity == &first) != short_weighted ? 1 : 10;
    check(queue.enqueue(*entity, now), "charge on voluntary yield");
  }
  if (kind == Kind::Fair)
  {
    auto expected = first.runtime * weight / 1024;
    check(second.runtime + 20 >= expected && second.runtime <= expected + 20,
          "CPU time follows weight despite different burst lengths");
  }
  else check(second.runtime == first.runtime * 10, "round-robin preserves dispatch fairness");
  while (take(queue, now)) {}
  check(!queue.size(), "drain queue");
}
static void sleepers_and_smp()
{
  RunQueue queue(Kind::Fair, 10);
  Entity a, b, sleeper;
  check(queue.enqueue(a, 0) && queue.enqueue(b, 0), "SMP enqueue");
  check(take(queue, 0) == &a && queue.start(a, 0), "first hart dispatch");
  check(take(queue, 0) == &b && queue.start(b, 0), "second hart dispatch");
  check(!queue.tick(a, 100), "no redundant preemption without waiting work");
  check(queue.enqueue(sleeper, 100), "new task joins active entitlement");
  check(queue.minimum_runtime() == 100 && sleeper.virtual_runtime == 100,
        "floor includes actual running work on every hart");
  check(queue.tick(a, 100), "backlog makes expired quantum preempt");
  queue.stop(a, 100); queue.stop(b, 100);
  check(queue.enqueue(a, 100) && queue.enqueue(b, 100), "requeue running tasks");
  check(queue.take(100, [&a](const Entity &e) { return &e == &a; }) == &a,
        "CPU affinity may skip the tree minimum");
  check(queue.start(a, 100), "affinity dispatch starts once");
  queue.stop(a, 200);
  while (take(queue, 200)) {}

  RunQueue wrapped(Kind::Fair, 10);
  Entity entity;
  check(wrapped.enqueue(entity, ~Time(0) - 4) && take(wrapped, ~Time(0) - 4) == &entity &&
        wrapped.start(entity, ~Time(0) - 4), "wrapped clock fixture");
  wrapped.stop(entity, 5);
  check(entity.runtime == 10, "unsigned clock subtraction across wrap");
}
static void feedback()
{
  RunQueue queue(Kind::Feedback, 10);
  Entity cpu, interactive;
  check(queue.enqueue(cpu, 0) && queue.enqueue(interactive, 0), "MLFQ fixture");
  check(take(queue, 0) == &cpu && queue.start(cpu, 0), "top-level FIFO");
  check(!queue.tick(cpu, 9) && queue.tick(cpu, 10), "first quantum boundary");
  check(queue.enqueue(cpu, 10) && cpu.level == 1, "CPU-bound task demoted");
  check(take(queue, 10) == &interactive && queue.start(interactive, 10), "interactive task precedes demoted work");
  queue.stop(interactive, 12);
  Time now = 12;
  for (unsigned i = 0; i < 3; ++i)
  {
    check(take(queue, now) == &cpu && queue.start(cpu, now), "resume CPU task");
    now += 7;
    check(queue.enqueue(cpu, now), "short voluntary yield");
  }
  check(cpu.level == 2, "voluntary yields cannot reset CPU allotment");
  check(take(queue, 250) == &cpu && cpu.level == 0, "periodic boost prevents starvation");
  check(queue.enqueue(interactive, 250) && interactive.level == 0, "sleeping task observes boost epoch");
  while (take(queue, 250)) {}
}
int main()
{
  fairness(Kind::Fair, 1024); fairness(Kind::Fair, 2048); fairness(Kind::Fair, 2048, true);
  RunQueue invalid(Kind::Fair, 0), overflow(Kind::Fair, ~Time(0));
  Entity invalid_entity;
  check(!invalid.valid() && !overflow.valid() && !invalid.enqueue(invalid_entity, 0) &&
        !invalid.start(invalid_entity, 0) && !take(invalid, 0), "invalid quantum rejected");
  RunQueue legacy(Kind::RoundRobin, 10);
  Entity alone;
  check(legacy.enqueue(alone, 0) && take(legacy, 0) == &alone && legacy.start(alone, 0) &&
        !legacy.tick(alone, 9) && legacy.tick(alone, 10), "legacy quantum preempts even without backlog");
  legacy.stop(alone, 10);
  fairness(Kind::RoundRobin, 1024); sleepers_and_smp(); feedback();
  std::puts("scheduling policy: CPU fairness, weights, SMP, affinity, quanta and MLFQ boosting passed");
}
