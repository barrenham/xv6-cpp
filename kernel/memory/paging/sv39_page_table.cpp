module xv.memory;

import :sv39;
import :detail;

namespace xv::memory
{
namespace
{
constexpr MemorySize Valid = 1, Leaf = Sv39PageTable::Read | Sv39PageTable::Write |
                                       Sv39PageTable::Execute;

MemorySize *child_table(MemorySize entry)
{
  return PhysicalAddress((entry >> 10) << 12).pointer<MemorySize>();
}

MemorySize encode(PhysicalAddress address)
{
  return (address.raw() >> 12) << 10;
}

unsigned index_at(VirtualAddress va, unsigned level)
{
  return (va >> (12 + 9 * level)) & 511;
}
} // namespace

Sv39LeafLevel LargestPagePolicy::choose(VirtualAddress va, PhysicalAddress pa,
                                        MemorySize remaining) const
{
  for (int level = static_cast<int>(maximum_); level >= 0; --level)
  {
    auto size = Sv39PageTable::leaf_size(static_cast<Sv39LeafLevel>(level));
    if (remaining >= size && va % size == 0 && pa.is_aligned(size))
    {
      return static_cast<Sv39LeafLevel>(level);
    }
  }
  return Sv39LeafLevel::Page4KiB;
}

OwnedSv39PageTable::OwnedSv39PageTable(PhysicalPageAllocator &memory,
                                       const PageMappingPolicy &policy)
    : root_(memory), view_(root_.get(), memory, policy)
{
  if (root_)
  {
    detail::fill(root_.get(), 0, PageSize);
  }
}

MappingResult Sv39PageTable::map_leaf(VirtualAddress va, PhysicalAddress pa,
                                      unsigned target, MemorySize permissions)
{
  auto *table = root_;
  for (unsigned level = 2; level > target; --level)
  {
    auto &entry = table[index_at(va, level)];
    if (entry & Valid)
    {
      if (entry & Leaf)
      {
        return MappingResult::Collision;
      }
      table = child_table(entry);
    }
    else
    {
      OwnedPage candidate(memory_);
      if (!candidate)
      {
        return MappingResult::OutOfMemory;
      }
      detail::fill(candidate.get(), 0, PageSize);
      auto address = PhysicalAddress::from_pointer(candidate.get());
      if (!address.is_aligned())
      {
        return MappingResult::InvalidRange;
      }
      entry = encode(address) | Valid;
      table = static_cast<MemorySize *>(candidate.release());
    }
  }
  auto &entry = table[index_at(va, target)];
  if (entry & Valid)
  {
    return MappingResult::Collision;
  }
  entry = encode(pa) | permissions | Valid;
  return MappingResult::Ok;
}

MappingResult Sv39PageTable::map_range(VirtualAddress va, PhysicalAddress pa,
                                       MemorySize bytes, MemorySize permissions)
{
  if (!root_)
  {
    return MappingResult::OutOfMemory;
  }
  if (!bytes || va % PageSize || !pa.is_aligned() || bytes % PageSize ||
      va >= VirtualLimit || bytes > VirtualLimit - va || pa.raw() >= (1UL << 56) ||
      bytes > (1UL << 56) - pa.raw())
  {
    return MappingResult::InvalidRange;
  }
  constexpr MemorySize Allowed =
      Read | Write | Execute | User | Global | Accessed | Dirty;
  if ((permissions & ~Allowed) || !(permissions & (Read | Execute)) ||
      ((permissions & Write) && !(permissions & Read)))
  {
    return MappingResult::InvalidPermissions;
  }
  while (bytes)
  {
    auto level = policy_.choose(va, pa, bytes);
    if (static_cast<unsigned>(level) > 2)
    {
      return MappingResult::InvalidRange;
    }
    auto step = leaf_size(level);
    if (step > bytes || va % step || !pa.is_aligned(step))
    {
      return MappingResult::InvalidRange;
    }
    auto result = map_leaf(va, pa, static_cast<unsigned>(level), permissions);
    if (result != MappingResult::Ok)
    {
      return result;
    }
    va += step;
    pa = pa.advanced(step);
    bytes -= step;
  }
  return MappingResult::Ok;
}

bool Sv39PageTable::translate(VirtualAddress va, PageTranslation &result) const
{
  if (!root_ || va >= VirtualLimit)
  {
    return false;
  }
  const auto *table = root_;
  for (int level = 2; level >= 0; --level)
  {
    auto entry = table[index_at(va, level)];
    if (!(entry & Valid))
    {
      return false;
    }
    if (entry & Leaf)
    {
      auto physical = PhysicalAddress((entry >> 10) << 12);
      auto leaf = static_cast<Sv39LeafLevel>(level);
      auto bytes = leaf_size(leaf);
      if (!physical.is_aligned(bytes) || ((entry & Write) && !(entry & Read)))
      {
        return false;
      }
      result = {physical.advanced(va % bytes), entry & 0x3ff, leaf};
      return true;
    }
    if (level == 0)
    {
      return false;
    }
    table = child_table(entry);
  }
  return false;
}

void Sv39PageTable::count_table(const MemorySize *table, unsigned level,
                                PageTableStatistics &result)
{
  ++result.table_pages;
  for (unsigned i = 0; i < 512; ++i)
  {
    auto entry = __atomic_load_n(&table[i], __ATOMIC_ACQUIRE);
    if (!(entry & Valid))
    {
      continue;
    }
    if (entry & Leaf)
    {
      if (level == 2)
      {
        ++result.leaves_1g;
      }
      else if (level == 1)
      {
        ++result.leaves_2m;
      }
      else
      {
        ++result.leaves_4k;
      }
    }
    else if (level)
    {
      count_table(child_table(entry), level - 1, result);
    }
  }
}

PageTableStatistics Sv39PageTable::statistics() const
{
  PageTableStatistics result;
  if (root_)
  {
    count_table(root_, 2, result);
  }
  return result;
}

void Sv39PageTable::clear_table(MemorySize *table, unsigned level)
{
  for (unsigned i = 0; i < 512; ++i)
  {
    auto entry = table[i];
    if ((entry & Valid) && !(entry & Leaf) && level)
    {
      auto *child = child_table(entry);
      clear_table(child, level - 1);
      memory_.deallocate(child);
    }
    table[i] = 0;
  }
}

void Sv39PageTable::clear()
{
  if (root_)
  {
    clear_table(root_, 2);
  }
}
} // namespace xv::memory
