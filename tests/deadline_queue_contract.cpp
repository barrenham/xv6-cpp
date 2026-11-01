#include "kernel/support/deadline_queue.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <random>

struct Item
{
  xv::time::Deadline timer;
  unsigned deliveries = 0;
};

int main()
{
  xv::time::DeadlineQueue queue;
  std::array<Item, 1024> items;
  std::mt19937 random(20261101);
  auto notify = [](void *owner) { ++static_cast<Item *>(owner)->deliveries; };
  for (unsigned iteration = 0; iteration < 100000; ++iteration)
  {
    auto &item = items[random() % items.size()];
    if (random() % 3)
    {
      queue.reset(item.timer, random() % 2000, notify, &item);
    }
    else
    {
      queue.cancel(item.timer);
    }
    unsigned long next = ~0UL;
    for (const auto &candidate : items)
    {
      if (candidate.timer.armed && candidate.timer.at < next)
      {
        next = candidate.timer.at;
      }
    }
    assert(queue.next() == next);
    if (iteration % 100 == 0)
    {
      unsigned expected = 0;
      for (const auto &candidate : items)
      {
        if (candidate.timer.armed && candidate.timer.at <= 1000)
        {
          ++expected;
        }
      }
      if (expected > 7)
      {
        expected = 7;
      }
      assert(queue.expire(1000, 7) == expected);
    }
  }
  for (auto &item : items)
  {
    queue.cancel(item.timer);
  }
  assert(queue.next() == ~0UL);
  for (auto &item : items)
  {
    queue.reset(item.timer, 42, notify, &item);
  }
  assert(queue.expire(41) == 0);
  unsigned total = 0;
  while (auto count = queue.expire(42))
  {
    total += count;
  }
  assert(total == items.size() && queue.next() == ~0UL);
  assert(queue.expire(100) == 0);
  std::puts("deadline queue: randomized reset/cancel/order/budget/one-shot passed");
}
