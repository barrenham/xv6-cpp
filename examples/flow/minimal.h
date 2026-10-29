#ifndef XV_EXAMPLES_FLOW_MINIMAL_H
#define XV_EXAMPLES_FLOW_MINIMAL_H
#include "kernel/support/flow.h"

namespace flow_example
{
using namespace xv::flow;

struct Context
{
  bool fast = false;
  bool abort = false;
  unsigned attempts = 0;
  unsigned common_visits = 0;
  unsigned finished = 0;
  int business_result = 0;
};
struct Choose;
struct Fast;
struct Slow;
struct Merge;
struct Retry;
struct Again;
struct Finish;
struct Common;
struct CommonReturn;

struct FastPath
{
  static void run(Context &ctx) noexcept
  {
    ctx.business_result = 10;
  }
};

struct SlowPath
{
  static void run(Context &ctx) noexcept
  {
    ctx.business_result = 20;
  }
};

struct Attempt
{
  static void run(Context &ctx) noexcept
  {
    ++ctx.attempts;
  }
};

struct CountCommon
{
  static void run(Context &ctx) noexcept
  {
    ++ctx.common_visits;
  }
};

struct FinishWork
{
  static void run(Context &ctx) noexcept
  {
    ++ctx.finished;
  }
};

struct IsFast
{
  static bool test(Context &ctx) noexcept
  {
    return ctx.fast;
  }
};

struct Aborted
{
  static bool test(Context &ctx) noexcept
  {
    return ctx.abort;
  }
};

struct NeedsRetry
{
  static bool test(Context &ctx) noexcept
  {
    return ctx.attempts < 3;
  }
};

// Shared once in this graph, called from two sites with different continuations.
using CommonFlow =
    types<node<Common, CountCommon, branch<Aborted, stop, go<CommonReturn>>>,
          node<CommonReturn, noop, return_>>;
using MainFlow =
    types<node<Choose, noop, branch<IsFast, go<Fast>, go<Slow>>>,
          node<Fast, FastPath, go<Merge>>, node<Slow, SlowPath, go<Merge>>,
          node<Merge, noop, call<Common, Retry>>,
          node<Retry, Attempt, branch<NeedsRetry, go<Retry>, go<Again>>>,
          node<Again, noop, call<Common, Finish>>, node<Finish, FinishWork, stop>>;
using Flow = graph<Choose, concat<MainFlow, CommonFlow>>;
} // namespace flow_example
#endif
