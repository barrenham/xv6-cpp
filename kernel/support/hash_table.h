#pragma once

namespace xv::support
{
// Non-cryptographic integer avalanche and FNV-1a byte hashing.
constexpr unsigned long hash_integer(unsigned long value)
{
  static_assert(sizeof(unsigned long) == 8);
  value ^= value >> 30;
  value *= 0xbf58476d1ce4e5b9UL;
  value ^= value >> 27;
  value *= 0x94d049bb133111ebUL;
  return value ^ (value >> 31);
}
inline unsigned long hash_bytes(const void *bytes, unsigned long size)
{
  auto *data = static_cast<const unsigned char *>(bytes);
  unsigned long value = 14695981039346656037UL;
  for (unsigned long i = 0; i < size; ++i) value = (value ^ data[i]) * 1099511628211UL;
  return value;
}
template <class Node> struct HashLinks
{
  Node *hash_next = nullptr;
  bool hashed = false;
};

// Fixed buckets do not limit entry count. Expected O(1), worst O(N), no heap.
// Traits supplies key(node), hash(key), equal(a,b). Keys must remain immutable
// while linked. All operations/lifetimes require caller-provided exclusion.
template <class Node, class Key, class Traits, unsigned Buckets = 128>
class IntrusiveHashTable
{
public:
  static_assert(Buckets && !(Buckets & (Buckets - 1)));
  constexpr IntrusiveHashTable() = default;
  IntrusiveHashTable(const IntrusiveHashTable &) = delete;
  IntrusiveHashTable &operator=(const IntrusiveHashTable &) = delete;
  unsigned long size() const { return count_; }
  Node *find(const Key &key) const
  {
    for (auto *node = buckets_[index(key)]; node; node = node->hash_next)
      if (Traits::equal(Traits::key(*node), key)) return node;
    return nullptr;
  }
  bool insert(Node &node)
  {
    const auto key = Traits::key(node);
    if (node.hashed || find(key)) return false;
    auto &bucket = buckets_[index(key)];
    node.hash_next = bucket;
    bucket = &node;
    node.hashed = true;
    ++count_;
    return true;
  }
  bool erase(Node &node)
  {
    if (!node.hashed) return false;
    auto **link = &buckets_[index(Traits::key(node))];
    while (*link && *link != &node) link = &(*link)->hash_next;
    if (!*link) return false;
    *link = node.hash_next;
    node.hash_next = nullptr;
    node.hashed = false;
    --count_;
    return true;
  }
private:
  static unsigned index(const Key &key) { return Traits::hash(key) & (Buckets - 1); }
  Node *buckets_[Buckets]{};
  unsigned long count_ = 0;
};
} // namespace xv::support
