module xv.drivers:mmio;

namespace xv::drivers::detail
{
// A non-owning register window. The board supplies an already mapped address.
class MmioRegion final
{
public:

  constexpr explicit MmioRegion(unsigned long base) : base_(base)
  {
  }

  unsigned char read8(unsigned offset) const
  {
    return *reinterpret_cast<volatile unsigned char *>(base_ + offset);
  }

  void write8(unsigned offset, unsigned char value) const
  {
    *reinterpret_cast<volatile unsigned char *>(base_ + offset) = value;
  }

  unsigned read32(unsigned offset) const
  {
    return *reinterpret_cast<volatile unsigned *>(base_ + offset);
  }

  void write32(unsigned offset, unsigned value) const
  {
    *reinterpret_cast<volatile unsigned *>(base_ + offset) = value;
  }

private:

  unsigned long base_;
};

static_assert(sizeof(unsigned) == 4);
} // namespace xv::drivers::detail
