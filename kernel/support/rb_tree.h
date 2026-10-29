#pragma once

namespace xv::support
{
// Intrusive, allocation-free red-black tree. Nodes must remain at stable
// addresses until erased and may belong to only one tree per set of links.
template <class Node> struct RbLinks
{
  Node *parent = nullptr;
  Node *left = nullptr;
  Node *right = nullptr;
  bool red = false;
};

// Compare orders const Node references. Optional Node::update() recomputes a
// subtree aggregate from the node and its children (never from its color).
// Callers provide exclusion; erase requires a node belonging to this tree.
template <class Node, class Compare> class RbTree
{
public:
  constexpr RbTree() = default;
  RbTree(const RbTree &) = delete;
  RbTree &operator=(const RbTree &) = delete;

  void swap(RbTree &other)
  {
    auto *root = root_;
    root_ = other.root_;
    other.root_ = root;
  }
  Node *root() const { return root_; }
  Node *first() const { return minimum(root_); }
  Node *last() const
  {
    auto *node = root_;
    while (node && node->right) node = node->right;
    return node;
  }
  static Node *next(Node *node)
  {
    if (!node) return nullptr;
    if (node->right) return minimum(node->right);
    auto *parent = node->parent;
    while (parent && node == parent->right) { node = parent; parent = parent->parent; }
    return parent;
  }
  static Node *previous(Node *node)
  {
    if (!node) return nullptr;
    if (node->left)
    {
      node = node->left;
      while (node->right) node = node->right;
      return node;
    }
    auto *parent = node->parent;
    while (parent && node == parent->left) { node = parent; parent = parent->parent; }
    return parent;
  }
  Node *lower_bound(const Node &key) const
  {
    Node *node = root_, *found = nullptr;
    while (node)
    {
      if (Compare{}(*node, key)) node = node->right;
      else { found = node; node = node->left; }
    }
    return found;
  }
  Node *find(const Node &key) const
  {
    auto *node = lower_bound(key);
    return node && !Compare{}(key, *node) ? node : nullptr;
  }
  bool insert(Node &node)
  {
    Node *parent = nullptr;
    auto **link = &root_;
    while (*link)
    {
      parent = *link;
      if (Compare{}(node, *parent)) link = &parent->left;
      else if (Compare{}(*parent, node)) link = &parent->right;
      else return false;
    }
    node.parent = parent;
    node.left = node.right = nullptr;
    node.red = true;
    *link = &node;
    update_up(&node);
    insert_fix(&node);
    return true;
  }
  void erase(Node &node)
  {
    auto *removed = &node;
    bool was_red = removed->red;
    Node *child = nullptr, *parent = nullptr;
    if (!node.left || !node.right)
    {
      child = node.left ? node.left : node.right;
      parent = node.parent;
      replace(&node, child);
      update_up(parent);
    }
    else
    {
      removed = minimum(node.right);
      was_red = removed->red;
      child = removed->right;
      if (removed->parent == &node)
      {
        parent = removed;
        if (child) child->parent = removed;
      }
      else
      {
        parent = removed->parent;
        replace(removed, child);
        removed->right = node.right;
        removed->right->parent = removed;
      }
      replace(&node, removed);
      removed->left = node.left;
      removed->left->parent = removed;
      removed->red = node.red;
      update_up(parent);
    }
    if (!was_red) erase_fix(child, parent);
    node.parent = node.left = node.right = nullptr;
    node.red = false;
    update(&node);
  }

private:
  Node *root_ = nullptr;
  static bool red(Node *node) { return node && node->red; }
  static Node *minimum(Node *node)
  {
    while (node && node->left) node = node->left;
    return node;
  }
  static void update(Node *node)
  {
    if constexpr (requires { node->update(); }) { if (node) node->update(); }
  }
  static void update_up(Node *node)
  {
    while (node) { update(node); node = node->parent; }
  }
  void replace(Node *old, Node *node)
  {
    if (!old->parent) root_ = node;
    else if (old == old->parent->left) old->parent->left = node;
    else old->parent->right = node;
    if (node) node->parent = old->parent;
  }
  void rotate_left(Node *node)
  {
    auto *right = node->right;
    node->right = right->left;
    if (right->left) right->left->parent = node;
    replace(node, right);
    right->left = node;
    node->parent = right;
    update(node);
    update(right);
  }
  void rotate_right(Node *node)
  {
    auto *left = node->left;
    node->left = left->right;
    if (left->right) left->right->parent = node;
    replace(node, left);
    left->right = node;
    node->parent = left;
    update(node);
    update(left);
  }
  void insert_fix(Node *node)
  {
    while (red(node->parent))
    {
      auto *parent = node->parent;
      auto *grand = parent->parent;
      if (parent == grand->left)
      {
        auto *uncle = grand->right;
        if (red(uncle))
        {
          parent->red = uncle->red = false;
          grand->red = true;
          node = grand;
        }
        else
        {
          if (node == parent->right) { node = parent; rotate_left(node); }
          node->parent->red = false;
          node->parent->parent->red = true;
          rotate_right(node->parent->parent);
        }
      }
      else
      {
        auto *uncle = grand->left;
        if (red(uncle))
        {
          parent->red = uncle->red = false;
          grand->red = true;
          node = grand;
        }
        else
        {
          if (node == parent->left) { node = parent; rotate_right(node); }
          node->parent->red = false;
          node->parent->parent->red = true;
          rotate_left(node->parent->parent);
        }
      }
    }
    root_->red = false;
  }
  void erase_fix(Node *node, Node *parent)
  {
    while (node != root_ && !red(node))
    {
      if (node == parent->left)
      {
        auto *sibling = parent->right;
        if (red(sibling))
        {
          sibling->red = false; parent->red = true;
          rotate_left(parent); sibling = parent->right;
        }
        if (!sibling) { node = parent; parent = node->parent; continue; }
        if (!red(sibling->left) && !red(sibling->right))
        {
          sibling->red = true; node = parent; parent = node->parent;
        }
        else
        {
          if (!red(sibling->right))
          {
            sibling->left->red = false; sibling->red = true;
            rotate_right(sibling); sibling = parent->right;
          }
          sibling->red = parent->red; parent->red = false;
          if (sibling->right) sibling->right->red = false;
          rotate_left(parent); node = root_; parent = nullptr;
        }
      }
      else
      {
        auto *sibling = parent->left;
        if (red(sibling))
        {
          sibling->red = false; parent->red = true;
          rotate_right(parent); sibling = parent->left;
        }
        if (!sibling) { node = parent; parent = node->parent; continue; }
        if (!red(sibling->left) && !red(sibling->right))
        {
          sibling->red = true; node = parent; parent = node->parent;
        }
        else
        {
          if (!red(sibling->left))
          {
            sibling->right->red = false; sibling->red = true;
            rotate_left(sibling); sibling = parent->left;
          }
          sibling->red = parent->red; parent->red = false;
          if (sibling->left) sibling->left->red = false;
          rotate_right(parent); node = root_; parent = nullptr;
        }
      }
    }
    if (node) node->red = false;
  }
};
} // namespace xv::support
