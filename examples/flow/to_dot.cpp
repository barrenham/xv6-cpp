// Host-only illustration. The flow utility itself has no library dependency.
#include <stdio.h>
#include "examples/flow/minimal.h"

namespace
{
using namespace xv::flow;
using G = flow_example::Flow;

template <bool... Choices> struct route
{
  template <bool Choice> using append = route<Choices..., Choice>;

  static void print()
  {
    ((printf("%s/", Choices ? "true" : "false")), ...);
  }
};

template <class From, class Policy, class Route> struct edges;

template <class From, class To, class Route> struct edges<From, go<To>, Route>
{
  static void print()
  {
    printf("  n%u -> n%u [label=\"", G::index<From>(), G::index<To>());
    Route::print();
    puts("go\"];");
  }
};

template <class From, class Entry, class Resume, class Route>
struct edges<From, call<Entry, Resume>, Route>
{
  static void print()
  {
    printf("  n%u -> n%u [label=\"", G::index<From>(), G::index<Entry>());
    Route::print();
    puts("call\"];");
    printf("  n%u -> n%u [style=dashed,label=\"", G::index<From>(), G::index<Resume>());
    Route::print();
    puts("resume after return\"];");
  }
};

template <class From, class Route> struct edges<From, stop, Route>
{
  static void print()
  {
    printf("  n%u -> stop [label=\"", G::index<From>());
    Route::print();
    puts("stop\"];");
  }
};

template <class From, class Route> struct edges<From, return_, Route>
{
  static void print()
  {
    printf("  n%u -> caller [label=\"", G::index<From>());
    Route::print();
    puts("return\"];");
  }
};

template <class From, class Predicate, class Yes, class No, class Route>
struct edges<From, branch<Predicate, Yes, No>, Route>
{
  static void print()
  {
    edges<From, Yes, typename Route::template append<true>>::print();
    edges<From, No, typename Route::template append<false>>::print();
  }
};

struct Dot
{
  template <class N> void node()
  {
    printf("  n%u [label=\"node %u\"];\n", G::index<typename N::id>(),
           G::index<typename N::id>());
    edges<typename N::id, typename N::next, route<>>::print();
  }
};
} // namespace

int main()
{
  puts("digraph flow {\n  stop [shape=doublecircle];\n  caller [shape=oval];");
  printf("  entry [shape=point];\n  entry -> n%u;\n", G::index<G::entry>());
  Dot visitor;
  G::visit(visitor);
  puts("}");
}
