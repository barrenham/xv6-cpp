#include "kernel/support/rb_tree.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <set>
#include <vector>

static void check(bool ok) { if (!ok) std::abort(); }
struct Node : xv::support::RbLinks<Node>
{
  unsigned key = 0, count = 1;
  void update() { count = 1 + (left ? left->count : 0) + (right ? right->count : 0); }
};
struct Less { bool operator()(const Node &a, const Node &b) const { return a.key < b.key; } };
using Tree = xv::support::RbTree<Node, Less>;

static unsigned validate(Node *node, Node *parent, long low, long high)
{
  if (!node) return 1;
  check(node->parent == parent && low < node->key && node->key < high);
  if (node->red) check((!node->left || !node->left->red) && (!node->right || !node->right->red));
  auto left = validate(node->left, node, low, node->key);
  auto right = validate(node->right, node, node->key, high);
  check(left == right);
  check(node->count == 1 + (node->left ? node->left->count : 0) + (node->right ? node->right->count : 0));
  return left + !node->red;
}
static void validate(Tree &tree, const std::set<unsigned> &expected)
{
  check(!tree.root() || !tree.root()->red);
  validate(tree.root(), nullptr, -1, 1000000);
  check((tree.root() ? tree.root()->count : 0) == expected.size());
  auto *node = tree.first();
  for (auto key : expected) { check(node && node->key == key); node = Tree::next(node); }
  check(!node);
  node = tree.last();
  for (auto it = expected.rbegin(); it != expected.rend(); ++it)
  { check(node && node->key == *it); node = Tree::previous(node); }
  check(!node);
}
int main()
{
  Tree tree;
  std::vector<Node> nodes(1024);
  std::set<unsigned> expected;
  for (unsigned i = 0; i < nodes.size(); ++i) nodes[i].key = i;
  // Ordered insertions and opposite deletion orders stress root/successor cases.
  for (unsigned pass = 0; pass < 4; ++pass)
  {
    for (unsigned i = 0; i < nodes.size(); ++i)
    {
      auto key = pass & 1 ? nodes.size() - 1 - i : i;
      check(tree.insert(nodes[key])); expected.insert(key); validate(tree, expected);
      check(!tree.insert(nodes[key])); validate(tree, expected);
    }
    for (unsigned i = 0; i < nodes.size(); ++i)
    {
      auto key = pass & 2 ? nodes.size() - 1 - i : i;
      tree.erase(nodes[key]); expected.erase(key); validate(tree, expected);
    }
  }
  std::mt19937 random(0x524254);
  for (unsigned i = 0; i < 30000; ++i)
  {
    unsigned key = random() % nodes.size();
    if (expected.erase(key)) tree.erase(nodes[key]);
    else { check(tree.insert(nodes[key])); expected.insert(key); }
    validate(tree, expected);
    Node probe; probe.key = random() % (nodes.size() + 1);
    auto it = expected.lower_bound(probe.key);
    auto *found = tree.lower_bound(probe);
    check(it == expected.end() ? !found : found && found->key == *it);
    check(bool(tree.find(probe)) == expected.contains(probe.key));
  }
  while (auto *node = tree.first()) { expected.erase(node->key); tree.erase(*node); validate(tree, expected); }
  std::puts("rb_tree_contract: ordering, colors, black heights, links and augmentation OK");
}
