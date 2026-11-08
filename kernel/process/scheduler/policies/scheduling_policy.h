#pragma once
#include "kernel/support/rb_tree.h"
#include "kernel/support/intrusive_list.h"

namespace xv::scheduling
{
enum class Kind { Cfs, Fair = Cfs, RoundRobin, Feedback };
using Time = unsigned long;
inline constexpr Time DefaultQuantum = 1000000; // QEMU virt timebase units

// No Process, architecture, allocator or lock dependency. Queue owner serializes
// all access. An entity lives until it is neither queued nor running.
struct Entity : support::RbLinks<Entity>, support::ListLinks<Entity>
{
  void *owner = nullptr;
  Time virtual_runtime = 0, runtime = 0, started = 0, budget_started = 0;
  Time used = 0, slice = 0, ticket = 0, epoch = 0, fractional = 0;
  Time policy_generation = 0;
  unsigned weight = 1024, level = 0;
  bool queued = false, running = false, initialized = false;
};

class RunQueue
{
public:
  constexpr explicit RunQueue(Kind kind, Time quantum = DefaultQuantum)
      : kind_(kind), unit_(quantum) {}
  RunQueue(const RunQueue &) = delete;
  RunQueue &operator=(const RunQueue &) = delete;
  bool valid() const
  {
    return unit_ && unit_ <= ~Time(0) / 20 &&
           (kind_ == Kind::Cfs || kind_ == Kind::RoundRobin || kind_ == Kind::Feedback);
  }
  unsigned long size() const { return count_; }
  Kind kind() const { return kind_; }
  Time changes() const { return generation_ - 1; }
  unsigned long running_count() const { return running_count_; }
  Time minimum_runtime() const { return floor_; }
  Time runnable_weight() const { return queued_weight_ + running_weight_; }

  // Caller holds queue exclusion. The queue object and entities stay stable;
  // no Process locks, context switches or allocations occur during migration.
  bool change_policy(Kind desired, Time now)
  {
    if (!valid() || (desired != Kind::Cfs && desired != Kind::RoundRobin &&
                     desired != Kind::Feedback)) return false;
    if (desired == kind_) return true;
    support::IntrusiveList<Entity> active, waiting;
    while (auto *node = running_.first())
    {
      stop(*node, now); // Charge all actual CPU time under the outgoing policy.
      active.push_back(*node);
    }
    while (auto *node = take(now, [](const Entity &) { return true; }))
      waiting.push_back(*node);
    kind_ = desired;
    ++generation_;
    floor_ = sequence_ = epoch_ = 0;
    last_boost_ = now;
    // Every entity is detached first. Re-enqueue preserves old selection order;
    // queued and running weights are unchanged when migration finishes.
    while (auto *node = waiting.pop_front())
      if (!enqueue(*node, now)) __builtin_trap();
    while (auto *node = active.pop_front())
      if (!start(*node, now)) __builtin_trap();
    return true;
  }

  bool enqueue(Entity &entity, Time now)
  {
    if (!valid() || entity.queued || !entity.weight || entity.weight > 4096) return false;
    if (!entity.running && !can_add(entity)) return false;
    bool yielding = entity.running;
    stop(entity, now);
    boost(now);
    update_floor(now, yielding ? &entity : nullptr);
    refresh(entity, now);
    if (!entity.initialized || entity.virtual_runtime < floor_) entity.virtual_runtime = floor_;
    entity.initialized = entity.queued = true;
    entity.ticket = ++sequence_;
    ++count_;
    queued_weight_ += entity.weight;
    if (kind_ == Kind::Cfs) return fair_.insert(entity);
    return fifo_[kind_ == Kind::Feedback ? entity.level : 0].push_back(entity);
  }

  // Eligibility is evaluated under queue exclusion, never by locking Process.
  template <class Eligible> Entity *take(Time now, Eligible eligible)
  {
    if (!valid()) return nullptr;
    boost(now);
    Entity *entity = nullptr;
    if (kind_ == Kind::Cfs)
    {
      for (auto *node = fair_.first(); node; node = decltype(fair_)::next(node))
        if (eligible(*node)) { entity = node; break; }
      if (entity) fair_.erase(*entity);
    }
    else
    {
      for (unsigned level = 0; level < 3 && !entity; ++level)
        for (auto *node = fifo_[level].first(); node; node = node->next)
          if (eligible(*node)) { entity = node; fifo_[level].erase(*node); break; }
    }
    if (entity) { entity->queued = false; --count_; queued_weight_ -= entity->weight; }
    return entity;
  }

  bool start(Entity &entity, Time now)
  {
    if (!valid() || !entity.weight || entity.weight > 4096 || entity.running || entity.queued || entity.linked) return false;
    if (!can_add(entity)) return false;
    boost(now);
    refresh(entity, now);
    entity.running = true;
    entity.started = entity.budget_started = now;
    running_.push_back(entity);
    ++running_count_;
    running_weight_ += entity.weight;
    entity.slice = kind_ == Kind::Cfs ? fair_slice(entity) : quantum(entity.level);
    return true;
  }

  void stop(Entity &entity, Time now)
  {
    if (!entity.running) return;
    boost(now);
    refresh(entity, now);
    auto elapsed = now - entity.started;
    entity.runtime = add(entity.runtime, elapsed);
    entity.virtual_runtime = add(entity.virtual_runtime, scaled(elapsed, entity.weight, entity.fractional));
    entity.fractional = ((elapsed % entity.weight) * 1024 + entity.fractional) % entity.weight;
    entity.used = add(entity.used, now - entity.budget_started);
    running_.erase(entity);
    --running_count_;
    running_weight_ -= entity.weight;
    entity.running = false;
    if (kind_ == Kind::Feedback)
    {
      while (entity.level < 2 && entity.used >= quantum(entity.level))
      {
        entity.used -= quantum(entity.level);
        ++entity.level;
      }
      if (entity.level == 2 && entity.used >= quantum(2)) entity.used %= quantum(2);
    }
    if (!count_ && !running_count_ && entity.virtual_runtime > floor_) floor_ = entity.virtual_runtime;
    update_floor(now, &entity);
  }

  bool tick(Entity &entity, Time now)
  {
    return tick(entity, now, [](const Entity &) { return true; });
  }

  template <class Eligible> bool tick(Entity &entity, Time now, Eligible eligible)
  {
    if (!entity.running || !unit_) return false;
    boost(now);
    refresh(entity, now);
    if (kind_ == Kind::RoundRobin) return now - entity.started >= entity.slice;
    Entity *waiting = nullptr;
    if (kind_ == Kind::Cfs)
    {
      for (auto *node = fair_.first(); node; node = decltype(fair_)::next(node))
        if (eligible(*node)) { waiting = node; break; }
    }
    else
    {
      for (unsigned level = 0; level < 3 && !waiting; ++level)
        for (auto *node = fifo_[level].first(); node; node = node->next)
          if (eligible(*node)) { waiting = node; break; }
    }
    if (!waiting) return false;
    if (kind_ == Kind::Feedback)
      return waiting->level < entity.level ||
             add(entity.used, now - entity.budget_started) >= quantum(entity.level);
    // Recompute when runnable load changes after dispatch (e.g. a wakeup).
    entity.slice = fair_slice(entity);
    auto elapsed = now - entity.started;
    if (elapsed >= entity.slice) return true;
    return elapsed >= unit_ &&
           add(entity.virtual_runtime, scaled(elapsed, entity.weight, entity.fractional)) >
               add(waiting->virtual_runtime, unit_);
  }

private:
  bool can_add(const Entity &entity) const
  {
    // Also bounds the remainder product in fair_slice, even at huge clocks.
    return runnable_weight() <= ~Time(0) / 4096 - entity.weight;
  }
  Time fair_slice(const Entity &entity) const
  {
    auto contenders = count_ + running_count_;
    Time period = contenders > ~Time(0) / unit_ ? ~Time(0) : contenders * unit_;
    if (period < 4 * unit_) period = 4 * unit_;
    auto total = runnable_weight();
    Time share = (period / total) * entity.weight +
                 ((period % total) * entity.weight) / total;
    return share < unit_ ? unit_ : share;
  }
  struct Earlier
  {
    bool operator()(const Entity &a, const Entity &b) const
    {
      return a.virtual_runtime < b.virtual_runtime ||
             (a.virtual_runtime == b.virtual_runtime &&
              (a.ticket < b.ticket || (a.ticket == b.ticket &&
               reinterpret_cast<unsigned long>(&a) < reinterpret_cast<unsigned long>(&b))));
    }
  };
  static Time add(Time a, Time b) { return b > ~Time(0) - a ? ~Time(0) : a + b; }
  static Time scaled(Time elapsed, unsigned weight, Time fractional)
  {
    auto whole = elapsed / weight;
    if (whole > ~Time(0) / 1024) return ~Time(0);
    return add(whole * 1024, ((elapsed % weight) * 1024 + fractional) / weight);
  }
  Time quantum(unsigned level) const { return unit_ << (kind_ == Kind::Feedback ? level : 0); }
  void refresh(Entity &entity, Time now)
  {
    // Sleepers and dequeued-but-not-started entities are not in either list
    // during a change. Their next activation joins the new policy's timeline.
    if (entity.policy_generation != generation_)
    {
      entity.policy_generation = generation_;
      entity.virtual_runtime = floor_;
      entity.fractional = entity.used = 0;
      entity.level = 0;
      entity.epoch = epoch_;
    }
    if (kind_ == Kind::Feedback && entity.epoch != epoch_)
    {
      entity.level = 0;
      entity.used = 0;
      entity.epoch = epoch_;
      if (entity.running) entity.budget_started = now;
    }
  }
  void boost(Time now)
  {
    if (kind_ != Kind::Feedback || now - last_boost_ < 20 * unit_) return;
    last_boost_ = now;
    ++epoch_;
    for (auto *node = running_.first(); node; node = node->next) refresh(*node, now);
    for (unsigned level = 0; level < 3; ++level)
    {
      // Level zero is also reset without changing its FIFO order.
      auto *node = fifo_[level].first();
      while (node)
      {
        auto *next = node->next;
        refresh(*node, now);
        if (level) { fifo_[level].erase(*node); fifo_[0].push_back(*node); }
        node = next;
      }
    }
  }
  void update_floor(Time now, const Entity *pending = nullptr)
  {
    if (kind_ != Kind::Cfs) return;
    auto *first = fair_.first();
    bool found = first != nullptr;
    Time minimum = first ? first->virtual_runtime : 0;
    if (pending && (!found || pending->virtual_runtime < minimum))
    {
      minimum = pending->virtual_runtime;
      found = true;
    }
    // At most one running entity per hart. Include running work so an SMP
    // dequeue cannot advance the floor beyond another hart's entitlement.
    for (auto *node = running_.first(); node; node = node->next)
    {
      auto value = add(node->virtual_runtime, scaled(now - node->started, node->weight, node->fractional));
      if (!found || value < minimum) { minimum = value; found = true; }
    }
    if (found && minimum > floor_) floor_ = minimum;
  }
  Kind kind_;
  Time generation_ = 1;
  Time queued_weight_ = 0, running_weight_ = 0;
  Time unit_, floor_ = 0, sequence_ = 0, epoch_ = 0, last_boost_ = 0;
  unsigned long count_ = 0, running_count_ = 0;
  support::RbTree<Entity, Earlier> fair_;
  support::IntrusiveList<Entity> fifo_[3];
  support::IntrusiveList<Entity> running_;
};
} // namespace xv::scheduling
