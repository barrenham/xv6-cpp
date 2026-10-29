module;
#include "kernel/support/flow.h"

module xv.drivers;

import :protocol;
import :virtio;
import :platform;

namespace xv::drivers
{

namespace flow = xv::flow;

struct VirtioBlock::InitializeFlow
{
  struct Context
  {
    VirtioBlock &owner;
    unsigned status = 0;
  };

  static void validate(Context &c) noexcept
  {
    c.owner.lock_.init("virtio_disk");

    if (c.owner.registers_.read32(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
        c.owner.registers_.read32(VIRTIO_MMIO_VERSION) != 2 ||
        c.owner.registers_.read32(VIRTIO_MMIO_DEVICE_ID) != 2 ||
        c.owner.registers_.read32(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551)
    {
      detail::fail("could not find virtio disk");
    }
  }

  static void handshake(Context &c) noexcept
  {
    // reset device
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS, c.status);

    // set ACKNOWLEDGE c.status bit
    c.status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS, c.status);

    // set DRIVER c.status bit
    c.status |= VIRTIO_CONFIG_S_DRIVER;
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS, c.status);
  }

  static void features(Context &c) noexcept
  {
    // negotiate features
    unsigned long features = c.owner.registers_.read32(VIRTIO_MMIO_DEVICE_FEATURES);
    features &= ~(1 << VIRTIO_BLK_F_RO);
    features &= ~(1 << VIRTIO_BLK_F_SCSI);
    c.owner.can_flush_ = features & (1 << VIRTIO_BLK_F_FLUSH);
    features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
    features &= ~(1 << VIRTIO_BLK_F_MQ);
    features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
    features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
    features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
    c.owner.registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES, features);

    // tell device that feature negotiation is complete.
    c.status |= VIRTIO_CONFIG_S_FEATURES_OK;
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS, c.status);

    // re-read c.status to ensure FEATURES_OK is set.
    c.status = c.owner.registers_.read32(VIRTIO_MMIO_STATUS);
    if (!(c.status & VIRTIO_CONFIG_S_FEATURES_OK))
    {
      detail::fail("virtio disk FEATURES_OK unset");
    }
  }

  static void queue(Context &c) noexcept
  {
    // initialize queue 0.
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_SEL, 0);

    // ensure queue 0 is not in use.
    if (c.owner.registers_.read32(VIRTIO_MMIO_QUEUE_READY))
    {
      detail::fail("virtio disk should not be ready");
    }

    // check maximum queue size.
    unsigned max = c.owner.registers_.read32(VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (max == 0)
    {
      detail::fail("virtio disk has no queue 0");
    }
    if (max < QueueSize)
    {
      detail::fail("virtio disk max queue too short");
    }
  }

  static void allocate(Context &c) noexcept
  {
    // allocate and zero queue memory.
    c.owner.descriptors_ =
        static_cast<struct virtq_desc *>(detail::allocate_dma_page());
    c.owner.available_ = static_cast<struct virtq_avail *>(detail::allocate_dma_page());
    c.owner.used_ = static_cast<struct virtq_used *>(detail::allocate_dma_page());
    if (!c.owner.descriptors_ || !c.owner.available_ || !c.owner.used_)
    {
      detail::fail("virtio disk kalloc");
    }
    detail::zero_page(c.owner.descriptors_);
    detail::zero_page(c.owner.available_);
    detail::zero_page(const_cast<virtq_used *>(c.owner.used_));
  }

  static void publish(Context &c) noexcept
  {
    // set queue size.
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_NUM, QueueSize);

    // write physical addresses.
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_DESC_LOW,
                               (unsigned long)c.owner.descriptors_);
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_DESC_HIGH,
                               (unsigned long)c.owner.descriptors_ >> 32);
    c.owner.registers_.write32(VIRTIO_MMIO_DRIVER_DESC_LOW,
                               (unsigned long)c.owner.available_);
    c.owner.registers_.write32(VIRTIO_MMIO_DRIVER_DESC_HIGH,
                               (unsigned long)c.owner.available_ >> 32);
    c.owner.registers_.write32(VIRTIO_MMIO_DEVICE_DESC_LOW,
                               (unsigned long)c.owner.used_);
    c.owner.registers_.write32(VIRTIO_MMIO_DEVICE_DESC_HIGH,
                               (unsigned long)c.owner.used_ >> 32);

    // queue is ready.
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_READY, 0x1);
  }

  static void ready(Context &c) noexcept
  {
    // all QueueSize descriptors start out unused.
    for (int i = 0; i < QueueSize; i++)
    {
      c.owner.free_[i] = 1;
    }

    // tell device we're completely ready.
    c.status |= VIRTIO_CONFIG_S_DRIVER_OK;
    c.owner.registers_.write32(VIRTIO_MMIO_STATUS, c.status);
  }
  struct Validate;
  struct Handshake;
  struct Features;
  struct Queue;
  struct Allocate;
  struct Publish;
  struct Ready;
  using Graph = flow::graph<
      Validate,
      flow::types<flow::node<Validate, flow::action<&validate>, flow::go<Handshake>>,
                  flow::node<Handshake, flow::action<&handshake>, flow::go<Features>>,
                  flow::node<Features, flow::action<&features>, flow::go<Queue>>,
                  flow::node<Queue, flow::action<&queue>, flow::go<Allocate>>,
                  flow::node<Allocate, flow::action<&allocate>, flow::go<Publish>>,
                  flow::node<Publish, flow::action<&publish>, flow::go<Ready>>,
                  flow::node<Ready, flow::action<&ready>, flow::stop>>>;
};

struct VirtioBlock::ChainFlow
{
  struct Context
  {
    VirtioBlock &owner;
    int *indices;
    int index = 0, result = -1;
  };

  static void allocate(Context &c) noexcept
  {
    c.indices[c.index] = c.owner.allocate_descriptor();
  }

  static bool allocated(Context &c) noexcept
  {
    return c.indices[c.index] >= 0;
  }

  static void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static bool remaining(Context &c) noexcept
  {
    return c.index < 3;
  }

  static void rollback(Context &c) noexcept
  {
    for (int j = 0; j < c.index; ++j)
    {
      c.owner.free_descriptor(c.indices[j]);
    }
  }

  static void success(Context &c) noexcept
  {
    c.result = 0;
  }
  struct Allocate;
  struct Advance;
  struct Rollback;
  struct Success;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Advance>, flow::go<Rollback>>>,
                  flow::node<Advance, flow::action<&advance>,
                             flow::branch<flow::condition<&remaining>,
                                          flow::go<Allocate>, flow::go<Success>>>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct VirtioBlock::TransferFlow
{
  struct Context
  {
    VirtioBlock &owner;
    unsigned long sector;
    void *data;
    unsigned count;
    bool flushing, write;
    int (&indices)[3];
    bool ok = false;
  };

  static void allocate(Context &c) noexcept
  {
    c.ok = c.owner.allocate_chain(c.indices) == 0;
  }

  static bool ok(Context &c) noexcept
  {
    return c.ok;
  }

  static void wait_descriptors(Context &c) noexcept
  {
    detail::prepare_sleep(&c.owner.free_[0]);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }

  static void prepare(Context &c) noexcept
  {
    auto *header = &c.owner.operations_[c.indices[0]];
    if (c.flushing)
    {
      header->type = 4;
    }
    else if (c.write)
    {
      header->type = VIRTIO_BLK_T_OUT;
    }
    else
    {
      header->type = VIRTIO_BLK_T_IN;
    }
    header->reserved = 0;
    header->sector = c.sector;
    auto *descriptors = c.owner.descriptors_;
    descriptors[c.indices[0]].addr = reinterpret_cast<unsigned long>(header);
    descriptors[c.indices[0]].len = sizeof(virtio_blk_req);
    descriptors[c.indices[0]].flags = VRING_DESC_F_NEXT;
    descriptors[c.indices[0]].next = c.flushing ? c.indices[2] : c.indices[1];
    descriptors[c.indices[1]].addr = reinterpret_cast<unsigned long>(c.data);
    descriptors[c.indices[1]].len = c.count;
    if (c.write)
    {
      descriptors[c.indices[1]].flags = 0;
    }
    else
    {
      descriptors[c.indices[1]].flags = VRING_DESC_F_WRITE;
    }
    descriptors[c.indices[1]].flags |= VRING_DESC_F_NEXT;
    descriptors[c.indices[1]].next = c.indices[2];
    c.owner.requests_[c.indices[0]].status = 0xff;
    descriptors[c.indices[2]].addr =
        reinterpret_cast<unsigned long>(&c.owner.requests_[c.indices[0]].status);
    descriptors[c.indices[2]].len = 1;
    descriptors[c.indices[2]].flags = VRING_DESC_F_WRITE;
    descriptors[c.indices[2]].next = 0;
    c.owner.requests_[c.indices[0]].pending = true;
  }

  static void submit(Context &c) noexcept
  {
    c.owner.available_->ring[c.owner.available_->idx % QueueSize] = c.indices[0];
    detail::io_barrier();
    c.owner.available_->idx += 1;
    detail::io_barrier();
    c.owner.registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0);
  }

  static bool pending(Context &c) noexcept
  {
    return c.owner.requests_[c.indices[0]].pending;
  }

  static void wait_completion(Context &c) noexcept
  {
    detail::prepare_sleep(&c.owner.requests_[c.indices[0]]);
    c.owner.lock_.release();
    detail::sleep_process();
    c.owner.lock_.acquire();
  }

  static void reclaim(Context &c) noexcept
  {
    if (c.flushing)
    {
      c.owner.free_descriptor(c.indices[0]);
      c.owner.free_descriptor(c.indices[1]);
      c.owner.free_descriptor(c.indices[2]);
    }
    else
    {
      c.owner.free_chain(c.indices[0]);
    }
  }
  struct Allocate;
  struct WaitDescriptors;
  struct Prepare;
  struct Submit;
  struct WaitCompletion;
  struct Reclaim;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&ok>, flow::go<Prepare>,
                                          flow::go<WaitDescriptors>>>,
                  flow::node<WaitDescriptors, flow::action<&wait_descriptors>,
                             flow::go<Allocate>>,
                  flow::node<Prepare, flow::action<&prepare>, flow::go<Submit>>,
                  flow::node<Submit, flow::action<&submit>,
                             flow::branch<flow::condition<&pending>,
                                          flow::go<WaitCompletion>, flow::go<Reclaim>>>,
                  flow::node<WaitCompletion, flow::action<&wait_completion>,
                             flow::branch<flow::condition<&pending>,
                                          flow::go<WaitCompletion>, flow::go<Reclaim>>>,
                  flow::node<Reclaim, flow::action<&reclaim>, flow::stop>>>;
};

struct VirtioBlock::InterruptFlow
{
  struct Context
  {
    VirtioBlock &owner;
    int id = 0;
  };

  static void acknowledge(Context &c) noexcept
  {
    c.owner.registers_.write32(VIRTIO_MMIO_INTERRUPT_ACK,
                               c.owner.registers_.read32(VIRTIO_MMIO_INTERRUPT_STATUS) &
                                   0x3);
    detail::io_barrier();
  }

  static bool pending(Context &c) noexcept
  {
    return c.owner.used_index_ != c.owner.used_->idx;
  }

  static void read(Context &c) noexcept
  {
    detail::io_barrier();
    c.id = c.owner.used_->ring[c.owner.used_index_ % QueueSize].id;
    if (c.id < 0 || c.id >= QueueSize || !c.owner.requests_[c.id].pending)
    {
      detail::fail("virtio: invalid completion");
    }
    if (c.owner.requests_[c.id].status != 0)
    {
      detail::fail("virtio_disk_intr status");
    }
  }

  static void complete(Context &c) noexcept
  {
    c.owner.requests_[c.id].pending = false;
    detail::wake_processes(&c.owner.requests_[c.id]);
    c.owner.used_index_ += 1;
  }
  struct Acknowledge;
  struct Read;
  struct Complete;
  using Graph =
      flow::graph<Acknowledge,
                  flow::types<flow::node<Acknowledge, flow::action<&acknowledge>,
                                         flow::branch<flow::condition<&pending>,
                                                      flow::go<Read>, flow::stop>>,
                              flow::node<Read, flow::action<&read>, flow::go<Complete>>,
                              flow::node<Complete, flow::action<&complete>,
                                         flow::branch<flow::condition<&pending>,
                                                      flow::go<Read>, flow::stop>>>>;
};

void VirtioBlock::initialize()
{
  InitializeFlow::Context context{*this};
  (void)InitializeFlow::Graph::run<0>(context);
}

// find a free descriptor, mark it non-free, return its index.
int VirtioBlock::allocate_descriptor()
{
  for (int i = 0; i < QueueSize; i++)
  {
    if (free_[i])
    {
      free_[i] = 0;
      return i;
    }
  }
  return -1;
}

// mark a descriptor as free.
void VirtioBlock::free_descriptor(int i)
{
  if (i < 0 || i >= QueueSize)
  {
    detail::fail("free_desc 1");
  }
  if (free_[i])
  {
    detail::fail("free_desc 2");
  }
  descriptors_[i].addr = 0;
  descriptors_[i].len = 0;
  descriptors_[i].flags = 0;
  descriptors_[i].next = 0;
  free_[i] = 1;
  detail::wake_processes(&free_[0]);
}

// free a chain of descriptors.
void VirtioBlock::free_chain(int i)
{
  while (1)
  {
    int flag = descriptors_[i].flags;
    int nxt = descriptors_[i].next;
    free_descriptor(i);
    if (flag & VRING_DESC_F_NEXT)
    {
      i = nxt;
    }
    else
    {
      break;
    }
  }
}

// allocate three descriptors (they need not be contiguous).
// disk transfers always use three descriptors.
int VirtioBlock::allocate_chain(int *idx)
{
  ChainFlow::Context context{*this, idx};
  (void)ChainFlow::Graph::run<0>(context);
  return context.result;
}

void VirtioBlock::transfer(BlockOperation operation, unsigned long sector, void *data,
                           unsigned count)
{
  bool flushing = !data && count == 0;
  if (!flushing && (!data || count == 0 || count % SectorSize != 0))
  {
    detail::fail("virtio: invalid transfer");
  }
  const bool write = operation == BlockOperation::Write;
  lock_.acquire();
  int indices[3];
  TransferFlow::Context context{*this, sector, data, count, flushing, write, indices};
  (void)TransferFlow::Graph::run<0>(context);
  lock_.release();
}

void VirtioBlock::handle_interrupt()
{
  xv::sync::ScopedLock guard(lock_);
  InterruptFlow::Context context{*this};
  (void)InterruptFlow::Graph::run<0>(context);
}

bool VirtioBlock::present() const
{
  return registers_.read32(VIRTIO_MMIO_MAGIC_VALUE) == 0x74726976 &&
         registers_.read32(VIRTIO_MMIO_DEVICE_ID) == 2;
}

void VirtioBlock::flush()
{
  if (can_flush_)
  {
    transfer(BlockOperation::Write, 0, nullptr, 0);
  }
}
} // namespace xv::drivers
