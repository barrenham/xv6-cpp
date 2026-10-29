#pragma once

namespace xv::support
{
template <class Node> struct ListLinks
{
  Node *previous = nullptr, *next = nullptr;
  bool linked = false;
};

// Stable caller-owned nodes, caller-provided exclusion. erase requires a node
// belonging to this list; a set of links can participate in only one list.
template <class Node> class IntrusiveList
{
public:
  constexpr IntrusiveList() = default;
  IntrusiveList(const IntrusiveList &) = delete;
  IntrusiveList &operator=(const IntrusiveList &) = delete;
  Node *first() const { return head_; }
  Node *last() const { return tail_; }
  bool empty() const { return !head_; }
  bool push_back(Node &node)
  {
    if (node.linked) return false;
    node.previous = tail_;
    node.next = nullptr;
    if (tail_) tail_->next = &node;
    else head_ = &node;
    tail_ = &node;
    node.linked = true;
    return true;
  }
  void erase(Node &node)
  {
    if (node.previous) node.previous->next = node.next;
    else head_ = node.next;
    if (node.next) node.next->previous = node.previous;
    else tail_ = node.previous;
    node.previous = node.next = nullptr;
    node.linked = false;
  }
  Node *pop_front()
  {
    auto *node = head_;
    if (node) erase(*node);
    return node;
  }
private:
  Node *head_ = nullptr, *tail_ = nullptr;
};
} // namespace xv::support
