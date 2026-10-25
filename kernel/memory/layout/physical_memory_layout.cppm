export module xv.memory:layout;

export import :range;

export namespace xv::memory
{
// Copies of this descriptor own no RAM. The selected allocator owns its metadata.
class PhysicalMemoryLayout final
{
public:

  static constexpr unsigned MaxRanges = 32;

  bool configure(PhysicalMemoryRange ram, PhysicalAddress first_usable)
  {
    if (ram.empty() || !ram.begin().is_aligned() || !ram.end().is_aligned() ||
        first_usable < ram.begin() || first_usable >= ram.end())
    {
      return false;
    }
    auto begin = first_usable.align_up();
    if (begin >= ram.end())
    {
      return false;
    }
    ram_ = ram;
    metadata_ = {};
    usable_count_ = direct_count_ = 1;
    usable_[0] = PhysicalMemoryRange(begin, ram.end());
    direct_[0] = ram;
    return !usable_[0].empty();
  }

  bool reserve(PhysicalMemoryRange reservation, bool no_map = false)
  {
    if (reservation.empty() || reservation.end() <= ram_.begin() ||
        reservation.begin() >= ram_.end())
    {
      return true;
    }
    auto begin =
        reservation.begin() < ram_.begin() ? ram_.begin() : reservation.begin();
    auto end = reservation.end() > ram_.end() ? ram_.end() : reservation.end();
    PhysicalMemoryRange pages(begin.align_down(), end.align_up());
    if (!can_subtract(usable_, usable_count_, pages) ||
        (no_map && !can_subtract(direct_, direct_count_, pages)))
    {
      return false;
    }
    if (!subtract(usable_, usable_count_, pages))
    {
      return false;
    }
    return !no_map || subtract(direct_, direct_count_, pages);
  }

  bool reserve_metadata(MemorySize bytes)
  {
    if (!metadata_.empty() || bytes == 0 || bytes > ~MemorySize(0) - (PageSize - 1))
    {
      return false;
    }
    bytes = (bytes + PageSize - 1) / PageSize * PageSize;
    for (unsigned i = 0; i < usable_count_; ++i)
    {
      if (usable_[i].size() >= bytes)
      {
        auto begin = usable_[i].begin();
        PhysicalMemoryRange chosen(begin, begin.advanced(bytes));
        if (!reserve(chosen))
        {
          return false;
        }
        metadata_ = chosen;
        return true;
      }
    }
    return false;
  }

  const PhysicalMemoryRange &ram() const
  {
    return ram_;
  }

  const PhysicalMemoryRange &metadata() const
  {
    return metadata_;
  }

  unsigned usable_count() const
  {
    return usable_count_;
  }

  unsigned direct_count() const
  {
    return direct_count_;
  }

  const PhysicalMemoryRange &usable(unsigned index) const
  {
    if (index >= usable_count_)
    {
      detail::fail("memory layout: invalid usable index");
    }
    return usable_[index];
  }

  const PhysicalMemoryRange &direct(unsigned index) const
  {
    if (index >= direct_count_)
    {
      detail::fail("memory layout: invalid direct index");
    }
    return direct_[index];
  }

  PageIndex usable_pages() const
  {
    PageIndex count = 0;
    for (unsigned i = 0; i < usable_count_; ++i)
    {
      count += usable_[i].page_count();
    }
    return count;
  }

  bool contains_usable_block(PhysicalAddress address, MemorySize bytes) const
  {
    for (unsigned i = 0; i < usable_count_; ++i)
    {
      if (usable_[i].contains_block(address, bytes))
      {
        return true;
      }
    }
    return false;
  }

private:

  static bool can_subtract(const PhysicalMemoryRange *ranges, unsigned count,
                           PhysicalMemoryRange excluded)
  {
    if (count < MaxRanges)
    {
      return true;
    }
    for (unsigned i = 0; i < count; ++i)
    {
      if (excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end())
      {
        return false;
      }
    }
    return true;
  }

  // Callers preflight both arrays before subtracting any interval.
  static bool subtract(PhysicalMemoryRange *ranges, unsigned &count,
                       PhysicalMemoryRange excluded)
  {
    for (unsigned i = 0; i < count; ++i)
    {
      if (excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end() &&
          count == MaxRanges)
      {
        return false;
      }
    }
    for (unsigned i = 0; i < count;)
    {
      auto range = ranges[i];
      if (excluded.end() <= range.begin() || excluded.begin() >= range.end())
      {
        ++i;
        continue;
      }
      auto left = excluded.begin() > range.begin() ? excluded.begin() : range.begin();
      auto right = excluded.end() < range.end() ? excluded.end() : range.end();
      if (left == range.begin() && right == range.end())
      {
        for (unsigned j = i + 1; j < count; ++j)
        {
          ranges[j - 1] = ranges[j];
        }
        --count;
      }
      else if (left == range.begin())
      {
        ranges[i++] = PhysicalMemoryRange(right, range.end());
      }
      else if (right == range.end())
      {
        ranges[i++] = PhysicalMemoryRange(range.begin(), left);
      }
      else
      {
        for (unsigned j = count; j > i + 1; --j)
        {
          ranges[j] = ranges[j - 1];
        }
        ranges[i] = PhysicalMemoryRange(range.begin(), left);
        ranges[i + 1] = PhysicalMemoryRange(right, range.end());
        ++count;
        i += 2;
      }
    }
    return true;
  }

  PhysicalMemoryRange ram_;
  PhysicalMemoryRange metadata_;
  PhysicalMemoryRange usable_[MaxRanges]{};
  PhysicalMemoryRange direct_[MaxRanges]{};
  unsigned usable_count_ = 0;
  unsigned direct_count_ = 0;
};
} // namespace xv::memory
