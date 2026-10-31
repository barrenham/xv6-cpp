module xv.drivers:rtc;
import :device;
import :mmio;
import :platform;
import xv.sync;
namespace xv::drivers
{
class GoldfishRtc final : public Device
{
public:
  constexpr explicit GoldfishRtc(unsigned long base):registers_(base) {}
  const char *name() const override {return "Goldfish RTC";}
  void initialize() override {lock_.init("goldfish rtc");}
  unsigned long nanoseconds()
  {
    // Reading LOW latches HIGH globally in the device. Serialize the pair
    // across harts, and order MMIO so no competing reader can replace the latch.
    xv::sync::ScopedLock guard(lock_);
    detail::io_barrier();unsigned low=registers_.read32(0);
    detail::io_barrier();unsigned high=registers_.read32(4);
    detail::io_barrier();return (static_cast<unsigned long>(high)<<32)|low;
  }
  unsigned long seconds(){return nanoseconds()/1000000000UL;}
private:
  detail::MmioRegion registers_;
  xv::sync::SpinLock lock_;
};
}
