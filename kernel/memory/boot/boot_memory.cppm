export module xv.memory:boot;

export import :layout;

export namespace xv::memory
{
class BootMemoryInfo final
{
public:

  static constexpr unsigned MaxReservations = 16;

  struct Reservation
  {
    PhysicalMemoryRange range;
    bool no_map;
  };

  bool set_ram(PhysicalMemoryRange ram)
  {
    if (!ram_.empty() || ram.empty())
    {
      return false;
    }
    ram_ = ram;
    return true;
  }

  bool reserve(PhysicalMemoryRange range, bool no_map = false)
  {
    if (range.empty())
    {
      return true;
    }
    if (count_ == MaxReservations)
    {
      return false;
    }
    reserved_[count_++] = {range, no_map};
    return true;
  }

  PhysicalMemoryRange ram() const
  {
    return ram_;
  }

  unsigned reservation_count() const
  {
    return count_;
  }

  const Reservation &reservation(unsigned index) const
  {
    if (index >= count_)
    {
      detail::fail("boot memory: invalid reservation index");
    }
    return reserved_[index];
  }

private:

  PhysicalMemoryRange ram_;
  Reservation reserved_[MaxReservations]{};
  unsigned count_ = 0;
};

// A source describes hardware; it does not initialize an allocator or map RAM.
class BootMemorySource
{
public:

  virtual const char *name() const = 0;
  virtual bool discover(BootMemoryInfo &result) = 0;

protected:

  ~BootMemorySource() = default;
};

class FdtMemorySource final : public BootMemorySource
{
public:

  static constexpr MemorySize MaxBlobSize = 2 * 1024 * 1024;

  FdtMemorySource(const void *blob, MemorySize capacity)
      : blob_(static_cast<const unsigned char *>(blob)), capacity_(capacity)
  {
  }

  const char *name() const override
  {
    return "device-tree";
  }

  bool discover(BootMemoryInfo &result) override;

private:

  const unsigned char *blob_;
  MemorySize capacity_;
};
} // namespace xv::memory
