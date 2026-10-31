module;
#include "kernel/io/framebuffer.h"
#ifndef XV_DISPLAY_WIDTH
#define XV_DISPLAY_WIDTH 1280
#endif
#ifndef XV_DISPLAY_HEIGHT
#define XV_DISPLAY_HEIGHT 800
#endif
module xv.drivers:gpu;
import :character;
import :interrupt;
import :protocol;
import :mmio;
import xv.sync;

namespace xv::drivers
{
inline constexpr unsigned GpuQueueEntries = 16;

struct GpuAvailableRing
{
  unsigned short flags;
  unsigned short idx;
  unsigned short ring[GpuQueueEntries];
  unsigned short unused;
};

struct GpuUsedRing
{
  unsigned short flags;
  unsigned short idx;

  struct Entry
  {
    unsigned id;
    unsigned len;
  } ring[GpuQueueEntries];
};

class VirtioGpu final : public CharacterDevice, public InterruptDevice
{
public:

  constexpr VirtioGpu(unsigned long base, unsigned irq) : registers_(base), irq_(irq)
  {
  }

  const char *name() const override
  {
    return "VirtIO GPU 2D";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override;
  CharacterDevice *open(bool) override;

  int read(IoBuffer, int count) override
  {
    return count == 0 ? 0 : -1;
  }

  int write(IoBuffer, int count) override
  {
    return count == 0 ? 0 : -1;
  }

  void put_sync(int) override
  {
  }

  int ioctl(unsigned request, IoBuffer argument) override;
  void handle_interrupt() override;

private:

  struct InitializeFlow;
  struct DrawFlow;
  static constexpr unsigned Width = XV_DISPLAY_WIDTH;
  static constexpr unsigned Height = XV_DISPLAY_HEIGHT;
  static constexpr unsigned Stride = Width * 4;
  static_assert(Width >= 640 && Width <= 1920);
  static_assert(Height >= 480 && Height <= 1080);
  static constexpr unsigned FramePages = (Stride * Height + 4095) / 4096;
  static constexpr unsigned RequestPages = (32 + FramePages * 16 + 4095) / 4096;
  static constexpr unsigned ResponsePage = 3 + RequestPages;
  static constexpr unsigned ControlPages = ResponsePage + 1;
  static_assert(RequestPages + 1 <= GpuQueueEntries);
  static_assert(sizeof(virtq_desc) * GpuQueueEntries <= 4096);
  static_assert(sizeof(GpuAvailableRing) <= 4096 && sizeof(GpuUsedRing) <= 4096);
  detail::MmioRegion registers_;
  unsigned irq_;
  xv::sync::SleepLock io_;
  // DMA pages: descriptors, available ring, used ring, request pages, response.
  void *control_[ControlPages]{};
  void *frame_[FramePages]{};
  unsigned short used_index_ = 0;
  unsigned scanout_ = 0;
  unsigned long fence_ = 0;
  bool ready_ = false;
  bool negotiate();
  bool allocate();
  bool setup();
  bool submit(unsigned bytes, unsigned response_type, unsigned minimum = 24);
  void packet(unsigned offset, const void *data, unsigned bytes);
  bool present(unsigned x, unsigned y, unsigned width, unsigned height);
  void offline();
  bool copy_pixels(const FramebufferUpdate &, IoBuffer);
};
} // namespace xv::drivers
