#include "examples/flow/minimal.h"

using namespace xv::flow;
#define CHECK(condition)                                                               \
  do                                                                                   \
  {                                                                                    \
    if (!(condition))                                                                  \
    {                                                                                  \
      return __LINE__;                                                                 \
    }                                                                                  \
  } while (false)

namespace
{
struct A;
struct B;
struct C;
struct D;

struct Counter
{
  unsigned count = 0;
  unsigned tests = 0;
};

struct Count
{
  static void run(Counter &ctx) noexcept
  {
    ++ctx.count;
  }
};

struct Million
{
  static bool test(Counter &ctx) noexcept
  {
    ++ctx.tests;
    return ctx.count < 1000000;
  }
};

struct Yes
{
  static bool test(Counter &ctx) noexcept
  {
    ++ctx.tests;
    return true;
  }
};

struct No
{
  static bool test(Counter &ctx) noexcept
  {
    ++ctx.tests;
    return false;
  }
};

using Composed = graph<
    A,
    types<node<A, Count,
               branch<Yes, branch<No, stop, call<B, C>>, branch<Yes, stop, return_>>>,
          node<B, Count, return_>, node<C, Count, stop>>>;

using Adapted = graph<
    A,
    types<node<A, action<&Count::run>, branch<condition<&Yes::test>, return_, stop>>>>;
using UnselectedCall =
    graph<A, types<node<A, Count, branch<Yes, return_, call<A, A>>>>>;

using Loop = graph<A, types<node<A, Count, branch<Million, go<A>, stop>>>>;
using Repeated = graph<A, types<node<A, Count, go<B>>, node<B, Count, stop>>>;
using RootReturn = graph<A, types<node<A, Count, return_>>>;
using Recursive = graph<A, types<node<A, Count, call<A, B>>, node<B, noop, stop>>>;
using Nested = graph<A, types<node<A, Count, call<B, D>>, node<B, Count, call<C, D>>,
                              node<C, Count, return_>, node<D, Count, return_>>>;
using NestedStop =
    graph<A, types<node<A, Count, call<B, D>>, node<B, Count, call<C, D>>,
                   node<C, Count, stop>, node<D, Count, return_>>>;

struct Inspector
{
  unsigned count = 0;

  template <class N> void node()
  {
    static_assert(Repeated::contains<typename N::id>);
    ++count;
  }
};
} // namespace

int main()
{
  flow_example::Context slow;
  CHECK(flow_example::Flow::run(slow) == outcome::stopped);
  CHECK(slow.business_result == 20 && slow.attempts == 3 && slow.common_visits == 2 &&
        slow.finished == 1);
  flow_example::Context fast;
  fast.fast = true;
  CHECK(flow_example::Flow::run(fast) == outcome::stopped);
  CHECK(fast.business_result == 10 && fast.common_visits == 2 && fast.finished == 1);
  flow_example::Context aborted;
  aborted.abort = true;
  CHECK(flow_example::Flow::run(aborted) == outcome::stopped);
  CHECK(aborted.common_visits == 1 && aborted.attempts == 0 && aborted.finished == 0);

  Counter ctx;
  CHECK(Adapted::run(ctx, step_budget{1}) == outcome::returned);
  CHECK(ctx.count == 1 && ctx.tests == 1);
  ctx = {};
  CHECK(UnselectedCall::run<0>(ctx) == outcome::returned && ctx.count == 1);
  ctx = {};
  CHECK(Composed::run(ctx) == outcome::stopped);
  CHECK(ctx.count == 3 && ctx.tests == 2);
  ctx = {};
  CHECK(Loop::run<0>(ctx) == outcome::stopped);
  CHECK(ctx.count == 1000000 && ctx.tests == 1000000);
  ctx = {};
  CHECK(Repeated::run<0>(ctx) == outcome::stopped && ctx.count == 2);
  ctx = {};
  CHECK(RootReturn::run<0>(ctx) == outcome::returned && ctx.count == 1);
  ctx = {};
  CHECK(Recursive::run<2>(ctx) == outcome::call_overflow && ctx.count == 3);
  ctx = {};
  CHECK(Recursive::run<0>(ctx) == outcome::call_overflow && ctx.count == 1);
  ctx = {};
  CHECK(Nested::run<2>(ctx) == outcome::returned && ctx.count == 5);
  ctx = {};
  CHECK(Nested::run<1>(ctx) == outcome::call_overflow && ctx.count == 2);
  ctx = {};
  CHECK(NestedStop::run<2>(ctx) == outcome::stopped && ctx.count == 3);
  ctx = {};
  CHECK(Loop::run(ctx, step_budget{0}) == outcome::step_limit && ctx.count == 0);
  CHECK(Loop::run(ctx, step_budget{17}) == outcome::step_limit && ctx.count == 17);
  ctx = {};
  CHECK(Repeated::run(ctx, step_budget{2}) == outcome::stopped && ctx.count == 2);
  ctx = {};
  CHECK(Repeated::run(ctx, step_budget{1}) == outcome::step_limit && ctx.count == 1);
  // Calls, return-only nodes and noops each consume a node budget unit.
  ctx = {};
  CHECK(Nested::run(ctx, step_budget{2}) == outcome::step_limit && ctx.count == 2);
  Inspector inspector;
  Repeated::visit(inspector);
  CHECK(inspector.count == 2 && Repeated::node_count == 2);
  return 0;
}
