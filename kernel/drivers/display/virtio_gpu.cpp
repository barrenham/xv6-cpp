module;
#include "kernel/io/framebuffer.h"
#include "kernel/support/flow.h"
module xv.drivers;
import :gpu;
import :platform;

namespace xv::drivers
{
namespace
{
struct GpuHeader
{
  unsigned type, flags;
  unsigned long fence;
  unsigned context;
  unsigned char ring, pad[3];
};

struct GpuRect
{
  unsigned x, y, width, height;
};

struct GpuCreate
{
  GpuHeader header;
  unsigned resource, format, width, height;
};

struct GpuAttach
{
  GpuHeader header;
  unsigned resource, count;
};

struct GpuMemory
{
  unsigned long address;
  unsigned length, pad;
};

struct GpuScanout
{
  GpuHeader header;
  GpuRect rect;
  unsigned scanout, resource;
};

struct GpuTransfer
{
  GpuHeader header;
  GpuRect rect;
  unsigned long offset;
  unsigned resource, pad;
};

struct GpuFlush
{
  GpuHeader header;
  GpuRect rect;
  unsigned resource, pad;
};

struct GpuDisplay
{
  GpuHeader header;

  struct
  {
    GpuRect rect;
    unsigned enabled, flags;
  } modes[16];
};

static_assert(sizeof(GpuHeader) == 24 && sizeof(GpuMemory) == 16 &&
              sizeof(GpuDisplay) == 408);
constexpr unsigned GetDisplayInfo = 0x0100;
constexpr unsigned ResourceCreate2d = 0x0101;
constexpr unsigned SetScanout = 0x0103;
constexpr unsigned ResourceFlush = 0x0104;
constexpr unsigned TransferToHost2d = 0x0105;
constexpr unsigned ResourceAttachBacking = 0x0106;
constexpr unsigned Okay = 0x1100;
constexpr unsigned Display = 0x1101;
constexpr unsigned FenceFlag = 1;
constexpr unsigned ResourceId = 1;
constexpr unsigned Bgrx8888 = 2;
constexpr unsigned DriverPresent = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
constexpr unsigned FeaturesAccepted = DriverPresent | VIRTIO_CONFIG_S_FEATURES_OK;
constexpr unsigned DriverReady = FeaturesAccepted | VIRTIO_CONFIG_S_DRIVER_OK;
} // namespace

void VirtioGpu::packet(unsigned offset, const void *data, unsigned bytes)
{
  auto *source = static_cast<const unsigned char *>(data);
  while (bytes)
  {
    unsigned part = 4096 - offset % 4096;
    if (part > bytes)
    {
      part = bytes;
    }
    auto *target =
        static_cast<unsigned char *>(control_[3 + offset / 4096]) + offset % 4096;
    for (unsigned i = 0; i < part; ++i)
    {
      target[i] = source[i];
    }
    offset += part;
    source += part;
    bytes -= part;
  }
}

bool VirtioGpu::negotiate()
{
  auto &r = registers_;
  r.write32(VIRTIO_MMIO_STATUS, 0);
  if (r.read32(VIRTIO_MMIO_STATUS))
  {
    return false;
  }
  r.write32(VIRTIO_MMIO_STATUS, DriverPresent);
  r.write32(VIRTIO_MMIO_DEVICE_FEATURES_SEL, 1);
  if (!(r.read32(VIRTIO_MMIO_DEVICE_FEATURES) & 1))
  {
    return false;
  }
  r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
  r.write32(VIRTIO_MMIO_DRIVER_FEATURES, 0);
  r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 1);
  r.write32(VIRTIO_MMIO_DRIVER_FEATURES, 1);
  r.write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
  r.write32(VIRTIO_MMIO_STATUS, FeaturesAccepted);
  if (!(r.read32(VIRTIO_MMIO_STATUS) & VIRTIO_CONFIG_S_FEATURES_OK))
  {
    return false;
  }
  r.write32(VIRTIO_MMIO_QUEUE_SEL, 0);
  return !r.read32(VIRTIO_MMIO_QUEUE_READY) &&
         r.read32(VIRTIO_MMIO_QUEUE_NUM_MAX) >= GpuQueueEntries;
}

bool VirtioGpu::allocate()
{
  for (auto &page : control_)
  {
    page = detail::allocate_dma_page();
    if (!page)
    {
      return false;
    }
    detail::zero_page(page);
  }
  for (auto &page : frame_)
  {
    page = detail::allocate_dma_page();
    if (!page)
    {
      return false;
    }
    detail::zero_page(page);
  }
  auto &r = registers_;
  r.write32(VIRTIO_MMIO_QUEUE_NUM, GpuQueueEntries);
  auto address = [&](unsigned low, unsigned high, void *pointer)
  {
    auto value = reinterpret_cast<unsigned long>(pointer);
    r.write32(low, unsigned(value));
    r.write32(high, unsigned(value >> 32));
  };
  address(VIRTIO_MMIO_QUEUE_DESC_LOW, VIRTIO_MMIO_QUEUE_DESC_HIGH, control_[0]);
  address(VIRTIO_MMIO_DRIVER_DESC_LOW, VIRTIO_MMIO_DRIVER_DESC_HIGH, control_[1]);
  address(VIRTIO_MMIO_DEVICE_DESC_LOW, VIRTIO_MMIO_DEVICE_DESC_HIGH, control_[2]);
  detail::io_barrier();
  r.write32(VIRTIO_MMIO_QUEUE_READY, 1);
  r.write32(VIRTIO_MMIO_STATUS, DriverReady);
  return true;
}

bool VirtioGpu::submit(unsigned bytes, unsigned response_type, unsigned minimum)
{
  if (!bytes || bytes > RequestPages * 4096 || minimum > 4096)
  {
    return false;
  }
  auto *descriptors = static_cast<virtq_desc *>(control_[0]);
  auto *available = static_cast<GpuAvailableRing *>(control_[1]);
  auto *used = static_cast<volatile GpuUsedRing *>(control_[2]);
  auto *header = static_cast<GpuHeader *>(control_[3]);
  header->flags = FenceFlag;
  header->fence = ++fence_;
  header->context = 0;
  header->ring = 0;
  detail::zero_page(control_[ResponsePage]);
  unsigned count = (bytes + 4095) / 4096;
  for (unsigned i = 0; i < count; ++i)
  {
    unsigned length = bytes - i * 4096;
    if (length > 4096)
    {
      length = 4096;
    }
    descriptors[i] = {reinterpret_cast<unsigned long>(control_[3 + i]), length,
                      VRING_DESC_F_NEXT, static_cast<unsigned short>(i + 1)};
  }
  descriptors[count] = {reinterpret_cast<unsigned long>(control_[ResponsePage]), 4096,
                        VRING_DESC_F_WRITE, 0};
  available->ring[available->idx % GpuQueueEntries] = 0;
  detail::io_barrier();
  ++available->idx;
  detail::io_barrier();
  registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0);
  // Bounded polling also works during boot before IRQ routing and scheduling.
  // Runtime callers hold a sleep lock, never a spinlock or interrupt mask.
  auto begin = detail::terminal_milliseconds();
  while (used->idx == used_index_)
  {
    if (detail::terminal_milliseconds() - begin >= 500)
    {
      return false;
    }
    detail::io_barrier();
  }
  detail::io_barrier();
  const auto &entry = used->ring[used_index_ % GpuQueueEntries];
  if (static_cast<unsigned short>(used->idx - used_index_) != 1 || entry.id ||
      entry.len < minimum || entry.len > 4096)
  {
    return false;
  }
  auto *response = static_cast<GpuHeader *>(control_[ResponsePage]);
  if (response->type != response_type || !(response->flags & FenceFlag) ||
      response->fence != fence_)
  {
    return false;
  }
  ++used_index_;
  return true;
}

bool VirtioGpu::setup()
{
  GpuHeader info{GetDisplayInfo, 0, 0, 0, 0, {}};
  packet(0, &info, sizeof(info));
  if (!submit(sizeof(info), Display, sizeof(GpuDisplay)))
  {
    return false;
  }
  auto *display = static_cast<GpuDisplay *>(control_[ResponsePage]);
  for (scanout_ = 0; scanout_ < 16; ++scanout_)
  {
    if (display->modes[scanout_].enabled)
    {
      break;
    }
  }
  if (scanout_ == 16)
  {
    return false;
  }
  GpuCreate create{
      {ResourceCreate2d, 0, 0, 0, 0, {}}, ResourceId, Bgrx8888, Width, Height};
  packet(0, &create, sizeof(create));
  if (!submit(sizeof(create), Okay))
  {
    return false;
  }
  GpuAttach attach{{ResourceAttachBacking, 0, 0, 0, 0, {}}, ResourceId, FramePages};
  packet(0, &attach, sizeof(attach));
  for (unsigned i = 0; i < FramePages; ++i)
  {
    GpuMemory memory{reinterpret_cast<unsigned long>(frame_[i]), 4096, 0};
    packet(sizeof(attach) + i * sizeof(memory), &memory, sizeof(memory));
  }
  if (!submit(sizeof(attach) + FramePages * sizeof(GpuMemory), Okay))
  {
    return false;
  }
  GpuScanout scan{
      {SetScanout, 0, 0, 0, 0, {}}, {0, 0, Width, Height}, scanout_, ResourceId};
  packet(0, &scan, sizeof(scan));
  return submit(sizeof(scan), Okay) && present(0, 0, Width, Height);
}

bool VirtioGpu::present(unsigned x, unsigned y, unsigned width, unsigned height)
{
  GpuTransfer transfer{{TransferToHost2d, 0, 0, 0, 0, {}},
                       {x, y, width, height},
                       static_cast<unsigned long>(y) * Stride + x * 4,
                       1,
                       0};
  packet(0, &transfer, sizeof(transfer));
  if (!submit(sizeof(transfer), Okay))
  {
    return false;
  }
  GpuFlush flush{{ResourceFlush, 0, 0, 0, 0, {}}, {x, y, width, height}, 1, 0};
  packet(0, &flush, sizeof(flush));
  return submit(sizeof(flush), Okay);
}

void VirtioGpu::offline()
{
  ready_ = false;
  registers_.write32(VIRTIO_MMIO_STATUS, 0);
  detail::io_barrier();
  auto begin = detail::terminal_milliseconds();
  while (registers_.read32(VIRTIO_MMIO_STATUS))
  {
    if (detail::terminal_milliseconds() - begin >= 100)
    {
      return; // Keep DMA ownership if reset was not acknowledged.
    }
  }
  for (auto &page : frame_)
  {
    if (page)
    {
      detail::free_dma_page(page);
      page = nullptr;
    }
  }
  for (auto &page : control_)
  {
    if (page)
    {
      detail::free_dma_page(page);
      page = nullptr;
    }
  }
}

struct VirtioGpu::InitializeFlow
{
  struct Context
  {
    VirtioGpu &owner;
    bool okay = false;
  };

  static bool found(Context &c) noexcept
  {
    auto &r = c.owner.registers_;
    return r.read32(VIRTIO_MMIO_MAGIC_VALUE) == 0x74726976 &&
           r.read32(VIRTIO_MMIO_VERSION) == 2 && r.read32(VIRTIO_MMIO_DEVICE_ID) == 16;
  }

  static void negotiate(Context &c) noexcept
  {
    c.okay = c.owner.negotiate();
  }

  static void allocate(Context &c) noexcept
  {
    c.okay = c.owner.allocate();
  }

  static void setup(Context &c) noexcept
  {
    c.okay = c.owner.setup();
  }

  static bool okay(Context &c) noexcept
  {
    return c.okay;
  }

  static void fail(Context &c) noexcept
  {
    c.owner.offline();
  }

  static void ready(Context &c) noexcept
  {
    c.owner.ready_ = true;
  }
  struct Found;
  struct Negotiate;
  struct Allocate;
  struct Setup;
  struct Fail;
  struct Ready;
  using Graph = xv::flow::graph<
      Found,
      xv::flow::types<
          xv::flow::node<Found, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&found>,
                                          xv::flow::go<Negotiate>, xv::flow::stop>>,
          xv::flow::node<Negotiate, xv::flow::action<&negotiate>,
                         xv::flow::branch<xv::flow::condition<&okay>,
                                          xv::flow::go<Allocate>, xv::flow::go<Fail>>>,
          xv::flow::node<Allocate, xv::flow::action<&allocate>,
                         xv::flow::branch<xv::flow::condition<&okay>,
                                          xv::flow::go<Setup>, xv::flow::go<Fail>>>,
          xv::flow::node<Setup, xv::flow::action<&setup>,
                         xv::flow::branch<xv::flow::condition<&okay>,
                                          xv::flow::go<Ready>, xv::flow::go<Fail>>>,
          xv::flow::node<Fail, xv::flow::action<&fail>, xv::flow::stop>,
          xv::flow::node<Ready, xv::flow::action<&ready>, xv::flow::stop>>>;
};

void VirtioGpu::initialize()
{
  io_.init("virtio gpu");
  InitializeFlow::Context c{*this};
  (void)InitializeFlow::Graph::run<0>(c);
}

CharacterDevice *VirtioGpu::open(bool)
{
  xv::sync::ScopedLock guard(io_);
  return ready_ ? this : nullptr;
}

bool VirtioGpu::copy_pixels(const FramebufferUpdate &update, IoBuffer argument)
{
  const unsigned row_bytes = update.width * 4;
  for (unsigned row = 0; row < update.height; ++row)
  {
    unsigned offset = (update.y + row) * Stride + update.x * 4, done = 0;
    while (done < row_bytes)
    {
      unsigned part = 4096 - offset % 4096;
      if (part > row_bytes - done)
      {
        part = row_bytes - done;
      }
      auto *target =
          static_cast<unsigned char *>(frame_[offset / 4096]) + offset % 4096;
      if (detail::copy_in(target, argument.space == AddressSpace::User,
                          update.pixels +
                              static_cast<unsigned long>(row) * update.stride + done,
                          part) < 0)
      {
        return false;
      }
      offset += part;
      done += part;
    }
  }
  return true;
}

struct VirtioGpu::DrawFlow
{
  struct Context
  {
    VirtioGpu &owner;
    IoBuffer argument;
    FramebufferUpdate update{};
    int result = -1;
    bool okay = false;
  };

  static void read(Context &c) noexcept
  {
    c.okay = detail::copy_in(&c.update, c.argument.space == AddressSpace::User,
                             c.argument.address, sizeof(c.update)) == 0;
  }

  static bool valid(Context &c) noexcept
  {
    const auto &u = c.update;
    return c.okay && !u.reserved && u.width && u.height && u.x < Width &&
           u.y < Height && u.width <= Width - u.x && u.height <= Height - u.y &&
           u.stride >= u.width * 4 &&
           static_cast<unsigned long>(u.height - 1) * u.stride + u.width * 4 <=
               ~0UL - u.pixels;
  }

  static void copy(Context &c) noexcept
  {
    c.okay = c.owner.copy_pixels(c.update, c.argument);
  }

  static bool okay(Context &c) noexcept
  {
    return c.okay;
  }

  static void present(Context &c) noexcept
  {
    if (c.owner.present(c.update.x, c.update.y, c.update.width, c.update.height))
    {
      c.result = 0;
    }
    else
    {
      c.owner.offline();
    }
  }
  struct Read;
  struct Copy;
  struct Present;
  using Graph = xv::flow::graph<
      Read, xv::flow::types<
                xv::flow::node<Read, xv::flow::action<&read>,
                               xv::flow::branch<xv::flow::condition<&valid>,
                                                xv::flow::go<Copy>, xv::flow::stop>>,
                xv::flow::node<Copy, xv::flow::action<&copy>,
                               xv::flow::branch<xv::flow::condition<&okay>,
                                                xv::flow::go<Present>, xv::flow::stop>>,
                xv::flow::node<Present, xv::flow::action<&present>, xv::flow::stop>>>;
};

int VirtioGpu::ioctl(unsigned request, IoBuffer argument)
{
  xv::sync::ScopedLock guard(io_);
  if (!ready_)
  {
    return -1;
  }
  if (request == XV_FB_GET_INFO)
  {
    FramebufferInfo info{1, Width, Height, Stride, 4, 2};
    return detail::copy_out(argument.space == AddressSpace::User, argument.address,
                            &info, sizeof(info));
  }
  if (request != XV_FB_DRAW || detail::effective_uid() != 0)
  {
    return -1;
  }
  DrawFlow::Context c{*this, argument};
  (void)DrawFlow::Graph::run<0>(c);
  return c.result;
}

void VirtioGpu::handle_interrupt()
{
  unsigned status = registers_.read32(VIRTIO_MMIO_INTERRUPT_STATUS) & 3;
  if (status & 2)
  {
    registers_.write32(VIRTIO_MMIO_CONFIG + 4,
                       registers_.read32(VIRTIO_MMIO_CONFIG) & 1);
  }
  registers_.write32(VIRTIO_MMIO_INTERRUPT_ACK, status);
  detail::io_barrier();
}
} // namespace xv::drivers
