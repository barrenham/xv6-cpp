export module xv.memory:address;

namespace xv::memory::detail
{
[[noreturn]] void fail(const char *message);
}

export namespace xv::memory
{
using MemorySize = unsigned long;
using PageIndex = unsigned long;
inline constexpr MemorySize PageSize = 4096;
static_assert(sizeof(MemorySize) == 8);

// An address value owns no storage. All integer/pointer conversions are explicit.
class PhysicalAddress final
{
public:

  using Value = unsigned long;

  constexpr PhysicalAddress() = default;

  explicit constexpr PhysicalAddress(Value value) : value_(value)
  {
  }

  static PhysicalAddress from_pointer(const void *pointer)
  {
    return PhysicalAddress(reinterpret_cast<Value>(pointer));
  }

  constexpr Value raw() const
  {
    return value_;
  }

  template <class T = void> T *pointer() const
  {
    return reinterpret_cast<T *>(value_);
  }

  constexpr bool is_null() const
  {
    return value_ == 0;
  }

  constexpr bool is_aligned(MemorySize alignment = PageSize) const
  {
    return alignment != 0 && value_ % alignment == 0;
  }

  constexpr PhysicalAddress align_down(MemorySize alignment = PageSize) const
  {
    if (alignment == 0)
    {
      detail::fail("physical address: zero alignment");
    }
    return PhysicalAddress(value_ - value_ % alignment);
  }

  constexpr PhysicalAddress align_up(MemorySize alignment = PageSize) const
  {
    if (alignment == 0)
    {
      detail::fail("physical address: zero alignment");
    }
    auto remainder = value_ % alignment;
    return remainder == 0 ? *this : advanced(alignment - remainder);
  }

  constexpr PhysicalAddress advanced(MemorySize bytes) const
  {
    if (bytes > ~Value(0) - value_)
    {
      detail::fail("physical address: overflow");
    }
    return PhysicalAddress(value_ + bytes);
  }

  constexpr MemorySize offset_from(PhysicalAddress origin) const
  {
    if (*this < origin)
    {
      detail::fail("physical address: negative offset");
    }
    return value_ - origin.value_;
  }

  // Absolute physical page number, independent of any managed RAM region.
  constexpr PageIndex page_number() const
  {
    return value_ / PageSize;
  }

  friend constexpr bool operator==(PhysicalAddress, PhysicalAddress) = default;

  friend constexpr bool operator<(PhysicalAddress left, PhysicalAddress right)
  {
    return left.value_ < right.value_;
  }

  friend constexpr bool operator<=(PhysicalAddress left, PhysicalAddress right)
  {
    return left.value_ <= right.value_;
  }

  friend constexpr bool operator>(PhysicalAddress left, PhysicalAddress right)
  {
    return right < left;
  }

  friend constexpr bool operator>=(PhysicalAddress left, PhysicalAddress right)
  {
    return right <= left;
  }

private:

  Value value_ = 0;
};

static_assert(sizeof(PhysicalAddress) == sizeof(PhysicalAddress::Value));
} // namespace xv::memory
