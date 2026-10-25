export module xv.memory:range;

export import :address;

export namespace xv::memory
{
// A half-open byte interval [begin, end). Page operations require aligned bounds.
class PhysicalMemoryRange final
{
public:

  constexpr PhysicalMemoryRange() = default;

  constexpr PhysicalMemoryRange(PhysicalAddress begin, PhysicalAddress end)
      : begin_(begin), end_(end)
  {
    if (end < begin)
    {
      detail::fail("physical range: reversed bounds");
    }
  }

  constexpr PhysicalAddress begin() const
  {
    return begin_;
  }

  constexpr PhysicalAddress end() const
  {
    return end_;
  }

  constexpr MemorySize size() const
  {
    return end_.offset_from(begin_);
  }

  constexpr bool empty() const
  {
    return begin_ == end_;
  }

  constexpr bool contains(PhysicalAddress address) const
  {
    return begin_ <= address && address < end_;
  }

  constexpr bool contains(PhysicalMemoryRange range) const
  {
    return begin_ <= range.begin_ && range.end_ <= end_;
  }

  constexpr bool contains_block(PhysicalAddress address, MemorySize bytes) const
  {
    return bytes != 0 && contains(address) && bytes <= end_.offset_from(address);
  }

  // Keep complete pages only; tiny intervals produce an empty aligned range.
  constexpr PhysicalMemoryRange page_aligned() const
  {
    auto last = end_.align_down();
    if (begin_ >= last)
    {
      return PhysicalMemoryRange(last, last);
    }
    return PhysicalMemoryRange(begin_.align_up(), last);
  }

  constexpr PageIndex page_count() const
  {
    if (!begin_.is_aligned() || !end_.is_aligned())
    {
      detail::fail("physical range: unaligned bounds");
    }
    return size() / PageSize;
  }

  constexpr PageIndex page_index(PhysicalAddress address) const
  {
    (void)page_count();
    if (!address.is_aligned() || !contains(address))
    {
      detail::fail("physical range: invalid page address");
    }
    return address.offset_from(begin_) / PageSize;
  }

  constexpr PhysicalAddress page_address(PageIndex index) const
  {
    if (index >= page_count())
    {
      detail::fail("physical range: invalid page index");
    }
    return begin_.advanced(index * PageSize);
  }

private:

  PhysicalAddress begin_;
  PhysicalAddress end_;
};
} // namespace xv::memory
