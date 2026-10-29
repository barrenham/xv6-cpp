#ifndef XV_SUPPORT_FLOW_H
#define XV_SUPPORT_FLOW_H

// Freestanding C++20. No includes, allocation, or runtime registration.
namespace xv::flow
{
template <class... T> struct types
{
};

template <class A, class B> inline constexpr bool same = false;
template <class A> inline constexpr bool same<A, A> = true;

template <class... Lists> struct concatenate;

template <> struct concatenate<>
{
  using type = types<>;
};

template <class... T> struct concatenate<types<T...>>
{
  using type = types<T...>;
};

template <class... A, class... B, class... Rest>
struct concatenate<types<A...>, types<B...>, Rest...>
    : concatenate<types<A..., B...>, Rest...>
{
};
template <class... Lists> using concat = typename concatenate<Lists...>::type;

template <class Target> struct go
{
  using target = Target;
};

template <class Entry, class Resume> struct call
{
  using entry = Entry;
  using resume = Resume;
};

struct return_
{
};

struct stop
{
};

template <class Predicate, class Yes, class No> struct branch
{
  using predicate = Predicate;
  using yes = Yes;
  using no = No;
};

struct noop
{
  template <class Context> static void run(Context &) noexcept
  {
  }
};

// Adapt named static functions without allocating a callable object. Preserve
// the callback's result and noexcept contract so graph diagnostics still apply.
template <auto Function> struct action
{
  template <class Context>
  static auto run(Context &ctx) noexcept(noexcept(Function(ctx)))
      -> decltype(Function(ctx))
  {
    return Function(ctx);
  }
};

template <auto Function> struct condition
{
  template <class Context>
  static auto test(Context &ctx) noexcept(noexcept(Function(ctx)))
      -> decltype(Function(ctx))
  {
    return Function(ctx);
  }
};

template <class Id, class Action, class Next> struct node
{
  using id = Id;
  using action = Action;
  using next = Next;
};

enum class outcome
{
  stopped,
  returned,
  step_limit,
  call_overflow
};

// The budget is copied into each invocation; it counts executed nodes.
struct unlimited
{
  bool take() noexcept
  {
    return true;
  }
};

struct step_budget
{
  unsigned remaining;

  bool take() noexcept
  {
    if (remaining == 0)
    {
      return false;
    }
    --remaining;
    return true;
  }
};

namespace detail
{
enum class control
{
  jump,
  invoke,
  leave,
  finish
};

struct transfer
{
  control kind;
  unsigned target = 0;
  unsigned resume = 0;
};

// Closed set of structural policies: unknown policies fail at instantiation.
template <class Policy> struct policy;

template <class Target> struct policy<go<Target>>
{
  static constexpr bool calls = false;
  using targets = types<Target>;

  template <class Graph, class Context> static transfer evaluate(Context &) noexcept
  {
    return {control::jump, Graph::template index<Target>()};
  }
};

template <class Entry, class Resume> struct policy<call<Entry, Resume>>
{
  static constexpr bool calls = true;
  using targets = types<Entry, Resume>;

  template <class Graph, class Context> static transfer evaluate(Context &) noexcept
  {
    return {control::invoke, Graph::template index<Entry>(),
            Graph::template index<Resume>()};
  }
};

template <> struct policy<return_>
{
  static constexpr bool calls = false;
  using targets = types<>;

  template <class Graph, class Context> static transfer evaluate(Context &) noexcept
  {
    return {control::leave};
  }
};

template <> struct policy<stop>
{
  static constexpr bool calls = false;
  using targets = types<>;

  template <class Graph, class Context> static transfer evaluate(Context &) noexcept
  {
    return {control::finish};
  }
};

template <class Predicate, class Yes, class No>
struct policy<branch<Predicate, Yes, No>>
{
  static constexpr bool calls = policy<Yes>::calls || policy<No>::calls;
  using targets = concat<typename policy<Yes>::targets, typename policy<No>::targets>;

  template <class Graph, class Context> static transfer evaluate(Context &ctx) noexcept
  {
    static_assert(same<decltype(Predicate::test(ctx)), bool>,
                  "flow: predicate must return bool");
    static_assert(noexcept(Predicate::test(ctx)), "flow: predicate must be noexcept");
    if (Predicate::test(ctx))
    {
      return policy<Yes>::template evaluate<Graph>(ctx);
    }
    return policy<No>::template evaluate<Graph>(ctx);
  }
};

template <class... Nodes> struct unique_ids;

template <> struct unique_ids<>
{
  static constexpr bool value = true;
};

template <class First, class... Rest> struct unique_ids<First, Rest...>
{
  static constexpr bool value =
      ((!same<typename First::id, typename Rest::id>) && ...) &&
      unique_ids<Rest...>::value;
};
} // namespace detail

// Instantiate a graph (e.g. sizeof(Graph)) to force all structural diagnostics.
template <class Entry, class Nodes> struct graph;

template <class Entry, class... Nodes> struct graph<Entry, types<Nodes...>>
{
  using entry = Entry;
  using nodes = types<Nodes...>;
  static constexpr unsigned node_count = sizeof...(Nodes);

  template <class Id>
  static constexpr bool contains = (same<Id, typename Nodes::id> || ...);

  template <class Id> static constexpr unsigned index() noexcept
  {
    static_assert(contains<Id>, "flow: target does not exist");
    unsigned current = 0;
    unsigned result = node_count;
    ((same<Id, typename Nodes::id> ? result = current : 0, ++current), ...);
    return result;
  }

private:

  static constexpr bool has_calls =
      (detail::policy<typename Nodes::next>::calls || ...);

  template <class... Ids> static constexpr bool valid_targets(types<Ids...>) noexcept
  {
    return (contains<Ids> && ...);
  }

  template <class N, class Context>
  static detail::transfer execute(Context &ctx) noexcept
  {
    static_assert(same<decltype(N::action::run(ctx)), void>,
                  "flow: action must return void; store business results in ctx");
    static_assert(noexcept(N::action::run(ctx)), "flow: action must be noexcept");
    N::action::run(ctx);
    return detail::policy<typename N::next>::template evaluate<graph>(ctx);
  }

  template <class N, class Context>
  static unsigned execute_direct(Context &ctx) noexcept
  {
    const auto next = execute<N>(ctx);
    if (next.kind == detail::control::finish)
    {
      return node_count;
    }
    if (next.kind == detail::control::leave)
    {
      return node_count + 1;
    }
    return next.target;
  }

public:

  static_assert(node_count != 0, "flow: empty graph");
  static_assert(detail::unique_ids<Nodes...>::value, "flow: duplicate node id");
  static_assert(contains<Entry>, "flow: invalid entry");
  static_assert(
      (valid_targets(typename detail::policy<typename Nodes::next>::targets{}) && ...),
      "flow: target does not exist");

  // Extraction has no execution overhead. N exposes id/action/next; transition
  // types expose target, entry/resume, predicate/yes/no for recursive inspection.
  template <class Visitor> static void visit(Visitor &visitor)
  {
    (visitor.template node<Nodes>(), ...);
  }

  // Each handler returns BEFORE the next dispatch. Even recursive calls use only
  // the explicit bounded continuation stack, never recursive C++ execution.
  template <unsigned MaxDepth = 16, class Context, class Budget = unlimited>
  [[nodiscard]] static outcome run(Context &ctx, Budget budget = {}) noexcept
  {
    static_assert(noexcept(budget.take()), "flow: budget must be noexcept");
    if constexpr (!has_calls)
    {
      // Without call policies a handler needs only a next-node index. The two
      // terminal indices preserve stop versus root return, with no return stack
      // or general control switch on this execution path.
      using handler = unsigned (*)(Context &) noexcept;
      static constexpr handler dispatch[] = {&execute_direct<Nodes, Context>...};
      unsigned pc = index<Entry>();
      for (;;)
      {
        if (!budget.take())
        {
          return outcome::step_limit;
        }
        pc = dispatch[pc](ctx);
        if (pc >= node_count)
        {
          return pc == node_count ? outcome::stopped : outcome::returned;
        }
      }
    }
    else
    {
      using handler = detail::transfer (*)(Context &) noexcept;
      static constexpr handler dispatch[] = {&execute<Nodes, Context>...};
      unsigned returns[MaxDepth == 0 ? 1 : MaxDepth];
      unsigned depth = 0;
      unsigned pc = index<Entry>();
      for (;;)
      {
        if (!budget.take())
        {
          return outcome::step_limit;
        }
        const auto next = dispatch[pc](ctx);
        switch (next.kind)
        {
        case detail::control::jump:
          pc = next.target;
          break;
        case detail::control::invoke:
          if (depth == MaxDepth)
          {
            return outcome::call_overflow;
          }
          returns[depth++] = next.resume;
          pc = next.target;
          break;
        case detail::control::leave:
          if (depth == 0)
          {
            return outcome::returned;
          }
          pc = returns[--depth];
          break;
        case detail::control::finish:
          return outcome::stopped;
        }
      }
    }
  }
};
} // namespace xv::flow
#endif
