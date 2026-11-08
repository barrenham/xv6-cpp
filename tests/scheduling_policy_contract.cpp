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
static void cfs_slices()
{
  RunQueue queue(Kind::Cfs, 12);
  Entity light, heavy;
  heavy.weight = 3072;
  check(queue.enqueue(light, 0) && queue.enqueue(heavy, 0), "CFS weighted enqueue");
  check(queue.runnable_weight() == 4096, "queued weights counted");
  check(take(queue, 0) == &light && queue.start(light, 0), "CFS light dispatch");
  check(light.slice == 12, "CFS light proportional slice");
  check(take(queue, 0) == &heavy && queue.start(heavy, 0), "CFS heavy dispatch");
  check(heavy.slice == 36 && queue.runnable_weight() == 4096, "CFS heavy proportional slice and running load");
  queue.stop(light, 1);
  check(!queue.tick(heavy, 40), "CFS no switch without eligible work");
  check(queue.enqueue(light, 40), "CFS waking contender");
  check(!queue.tick(heavy, 40, [](const Entity &) { return false; }),
        "CFS affinity rejects irrelevant queued work");
  check(queue.tick(heavy, 40), "CFS load change triggers expired slice");
  queue.stop(heavy, 40);
  check(take(queue, 40) == &light && !queue.runnable_weight(), "CFS removal subtracts load");

  RunQueue large(Kind::Cfs, ~Time(0) / 20);
  Entity a, b;
  b.weight = 4096;
  check(large.enqueue(a, 0) && large.enqueue(b, 0) && take(large, 0) == &a &&
        large.start(a, 0) && take(large, 0) == &b && large.start(b, 0), "large CFS clock scale");
  check(b.slice > a.slice && b.slice <= 4 * (~Time(0) / 20), "weighted slice multiplication does not overflow");
  large.stop(a, 1); large.stop(b, 1);

  RunQueue busy(Kind::Cfs, 10);
  Entity tasks[6];
  for (auto &task : tasks) check(busy.enqueue(task, 0), "CFS oversubscribed enqueue");
  auto *first = take(busy, 0);
  check(first && busy.start(*first, 0) && first->slice == 10, "CFS expands period to preserve minimum granularity");
  busy.stop(*first, 1);
  while (take(busy, 1)) {}
  check(!busy.runnable_weight(), "CFS drained load");
}

static void cfs_timer_fairness()
{
  RunQueue queue(Kind::Cfs, 10);
  Entity a, b;
  b.weight = 3072;
  check(queue.enqueue(a, 0) && queue.enqueue(b, 0), "CFS timer fairness setup");
  Entity *running = nullptr;
  for (Time now = 0; now < 20000; ++now)
  {
    if (!running)
    {
      running = take(queue, now);
      check(running && queue.start(*running, now), "CFS timer dispatch");
    }
    if (queue.tick(*running, now + 1))
    {
      check(queue.enqueue(*running, now + 1), "CFS timer preemption requeue");
      running = nullptr;
    }
  }
  if (running) queue.stop(*running, 20000);
  check(b.runtime + 100 >= 3 * a.runtime && b.runtime <= 3 * a.runtime + 100,
        "CFS timer-driven CPU share follows 3:1 weights");
  while (take(queue, 20000)) {}
  check(!queue.runnable_weight(), "CFS timer fairness load drains");
}

static void feedback_wakeup_preemption()
{
  RunQueue queue(Kind::Feedback, 10);
  Entity cpu, interactive;
  check(queue.enqueue(cpu, 0) && take(queue, 0) == &cpu && queue.start(cpu, 0), "MLFQ preemption setup");
  check(queue.enqueue(cpu, 10) && cpu.level == 1 && take(queue, 10) == &cpu &&
        queue.start(cpu, 10), "MLFQ lower-priority execution");
  check(queue.enqueue(interactive, 11), "MLFQ high-priority wakeup");
  check(!queue.tick(cpu, 11, [](const Entity &) { return false; }),
        "MLFQ does not preempt for tasks pinned elsewhere");
  check(queue.tick(cpu, 11), "MLFQ high priority preempts before low quantum expires");
  check(queue.enqueue(cpu, 11) && cpu.used == 1 && cpu.level == 1,
        "MLFQ preemption retains consumed allotment");
  check(take(queue, 11) == &interactive && queue.start(interactive, 11), "MLFQ dispatches woken task first");
  check(!queue.tick(interactive, 12), "MLFQ lower priority cannot preempt early");
  queue.stop(interactive, 12);
  while (take(queue, 12)) {}
}

static void live_policy_changes()
{
  for (auto initial : {Kind::Cfs, Kind::Feedback, Kind::RoundRobin})
    for (auto desired : {Kind::Cfs, Kind::Feedback, Kind::RoundRobin})
    {
      RunQueue queue(initial, 10);
      Entity a, b, pending, waiting, sleeper;
      b.weight = 2048;
      check(queue.enqueue(sleeper, 0) && take(queue, 0) == &sleeper &&
            queue.start(sleeper, 0), "switch sleeper setup");
      queue.stop(sleeper, 5);
      check(queue.enqueue(a, 5) && queue.enqueue(b, 5) &&
            queue.enqueue(pending, 5) && queue.enqueue(waiting, 5), "switch work setup");
      check(take(queue, 5) == &a && queue.start(a, 5) && take(queue, 5) == &b &&
            queue.start(b, 5) && take(queue, 5) == &pending, "switch has running and in-flight dispatch");
      check(queue.change_policy(desired, 12), "live switch accepted");
      check(queue.kind() == desired && queue.changes() == unsigned(initial != desired), "switch snapshot and no-op semantics");
      check(a.running && b.running && waiting.queued && !pending.running &&
            !pending.queued && queue.size() == 1 && queue.running_count() == 2 &&
            queue.runnable_weight() == 4096, "migration conserves ownership and load");
      if (initial != desired) check(a.runtime == 7 && b.runtime == 7, "migration charges outgoing CPU time once");
      check(!queue.change_policy(static_cast<Kind>(99), 12) && queue.kind() == desired,
            "invalid policy leaves active queue intact");
      check(queue.start(pending, 12), "in-flight dispatch starts under new policy");
      check(queue.enqueue(sleeper, 12) && sleeper.policy_generation == queue.changes() + 1,
            "sleeping task joins new generation lazily");
      queue.stop(a, 20); queue.stop(b, 20); queue.stop(pending, 20);
      check(a.runtime == 15 && b.runtime == 15 && pending.runtime == 8 &&
            sleeper.runtime == 5, "CPU totals survive policy changes");
      while (take(queue, 20)) {}
      check(!queue.size() && !queue.running_count() && !queue.runnable_weight(), "migration complete drain");
    }

  RunQueue queue(Kind::Cfs, 10);
  Entity entities[8];
  for (auto &entity : entities) check(queue.enqueue(entity, 0), "repeated switch setup");
  check(take(queue, 0) == &entities[0] && queue.start(entities[0], 0) &&
        take(queue, 0) == &entities[1] && queue.start(entities[1], 0), "repeated switch active harts");
  Kind choices[] = {Kind::Feedback, Kind::RoundRobin, Kind::Cfs};
  for (unsigned i = 0; i < 240; ++i)
  {
    check(queue.change_policy(choices[i % 3], (i + 1) * 250), "repeated live policy change");
    check(queue.size() == 6 && queue.running_count() == 2 &&
          queue.runnable_weight() == 8192 && queue.changes() == i + 1,
          "repeated migration never loses or duplicates work");
  }
  queue.stop(entities[0], 60000); queue.stop(entities[1], 60000);
  check(entities[0].runtime == 60000 && entities[1].runtime == 60000,
        "repeated migration preserves exact accumulated CPU time");
  unsigned seen = 0;
  while (auto *entity = take(queue, 60000))
  {
    unsigned index = entity - entities;
    check(index >= 2 && index < 8 && !(seen & (1U << index)), "migrated queue contains each waiter once");
    seen |= 1U << index;
  }
  check(seen == 252 && !queue.runnable_weight(), "repeated migration full reclamation");
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
  fairness(Kind::RoundRobin, 1024); sleepers_and_smp(); feedback(); cfs_slices(); cfs_timer_fairness(); feedback_wakeup_preemption(); live_policy_changes();
  std::puts("scheduling policy: CPU fairness, weights, SMP, affinity, quanta and MLFQ boosting passed");
}
