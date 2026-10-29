#include "kernel/support/hash_table.h"
#include "kernel/support/intrusive_list.h"
#include "kernel/support/ring_buffer.h"
#include "kernel/support/bitmap.h"
#include <array>
#include <cstdlib>
#include <cstdio>
#include <deque>
#include <random>
#include <vector>

static void check(bool ok) { if (!ok) std::abort(); }
struct Node : xv::support::HashLinks<Node>, xv::support::ListLinks<Node> { unsigned id = 0; };
struct Key
{
  static unsigned key(const Node &node) { return node.id; }
  static unsigned long hash(unsigned) { return 0; } // Deliberate worst collision.
  static bool equal(unsigned a, unsigned b) { return a == b; }
};
int main()
{
  using namespace xv::support;
  check(hash_bytes("hello", 5) == 0xa430d84680aabd0bUL);
  IntrusiveHashTable<Node, unsigned, Key, 1> table;
  IntrusiveList<Node> list;
  std::vector<Node> nodes(2048);
  for (unsigned i = 0; i < nodes.size(); ++i)
  {
    nodes[i].id = i;
    check(table.insert(nodes[i]) && list.push_back(nodes[i]));
    check(!table.insert(nodes[i]) && !list.push_back(nodes[i]));
  }
  Node duplicate; duplicate.id = 17;
  check(!table.insert(duplicate) && !table.find(99999));
  for (unsigned i = 0; i < nodes.size(); i += 2)
  {
    check(table.find(i) == &nodes[i] && table.erase(nodes[i]) && !table.erase(nodes[i]));
    list.erase(nodes[i]);
  }
  for (unsigned i = 1; i < nodes.size(); i += 2)
  {
    check(list.first() == &nodes[i] && list.pop_front() == &nodes[i]);
    check(table.erase(nodes[i]));
  }
  check(list.empty() && !list.last() && !list.pop_front() && table.size() == 0);
  check(table.insert(duplicate) && table.erase(duplicate));

  RingBuffer<unsigned, 7> ring;
  std::deque<unsigned> reference;
  std::mt19937 random(0xc07a);
  for (unsigned i = 0; i < 100000; ++i)
  {
    if (random() & 1)
    {
      auto value = random();
      check(ring.push(value) == (reference.size() < 7));
      if (reference.size() < 7) reference.push_back(value);
    }
    else
    {
      unsigned value = 0;
      check(ring.pop(value) == !reference.empty());
      if (!reference.empty()) { check(value == reference.front()); reference.pop_front(); }
    }
    check(ring.size() == reference.size());
    check(reference.empty() ? !ring.front() : *ring.front() == reference.front());
  }
  ring.clear(); check(ring.empty());
  for (unsigned bits : {0U, 1U, 63U, 64U, 65U, 127U, 130U})
  {
    std::array<unsigned long, 3> words{};
    BitmapView bitmap(words.data(), bits);
    for (unsigned i = 0; i < bits; ++i) { check(bitmap.first_clear() == i); check(bitmap.set(i)); }
    check(bitmap.first_clear() == bits && !bitmap.set(bits) && !bitmap.clear(bits));
    for (unsigned i = 0; i < bits; ++i)
    {
      check(bitmap.test(i) && bitmap.clear(i) && !bitmap.test(i));
      check(bitmap.first_clear(i) == i && bitmap.first_clear(i + 1) == bits);
      check(bitmap.set(i));
    }
  }
  std::puts("support containers: collisions, lifetimes, wraparound and bitmap boundaries passed");
}
