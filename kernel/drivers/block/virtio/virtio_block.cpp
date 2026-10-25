module xv.drivers;

import :protocol;
import :virtio;
import :platform;

namespace xv::drivers
{
void VirtioBlock::initialize()
{
  unsigned status = 0;

  lock_.init("virtio_disk");

  if (registers_.read32(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976 ||
      registers_.read32(VIRTIO_MMIO_VERSION) != 2 ||
      registers_.read32(VIRTIO_MMIO_DEVICE_ID) != 2 ||
      registers_.read32(VIRTIO_MMIO_VENDOR_ID) != 0x554d4551)
  {
    detail::fail("could not find virtio disk");
  }

  // reset device
  registers_.write32(VIRTIO_MMIO_STATUS, status);

  // set ACKNOWLEDGE status bit
  status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
  registers_.write32(VIRTIO_MMIO_STATUS, status);

  // set DRIVER status bit
  status |= VIRTIO_CONFIG_S_DRIVER;
  registers_.write32(VIRTIO_MMIO_STATUS, status);

  // negotiate features
  unsigned long features = registers_.read32(VIRTIO_MMIO_DEVICE_FEATURES);
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_SCSI);
  features &= ~(1 << VIRTIO_BLK_F_FLUSH);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_BLK_F_MQ);
  features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
  features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
  features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
  registers_.write32(VIRTIO_MMIO_DRIVER_FEATURES, features);

  // tell device that feature negotiation is complete.
  status |= VIRTIO_CONFIG_S_FEATURES_OK;
  registers_.write32(VIRTIO_MMIO_STATUS, status);

  // re-read status to ensure FEATURES_OK is set.
  status = registers_.read32(VIRTIO_MMIO_STATUS);
  if (!(status & VIRTIO_CONFIG_S_FEATURES_OK))
  {
    detail::fail("virtio disk FEATURES_OK unset");
  }

  // initialize queue 0.
  registers_.write32(VIRTIO_MMIO_QUEUE_SEL, 0);

  // ensure queue 0 is not in use.
  if (registers_.read32(VIRTIO_MMIO_QUEUE_READY))
  {
    detail::fail("virtio disk should not be ready");
  }

  // check maximum queue size.
  unsigned max = registers_.read32(VIRTIO_MMIO_QUEUE_NUM_MAX);
  if (max == 0)
  {
    detail::fail("virtio disk has no queue 0");
  }
  if (max < QueueSize)
  {
    detail::fail("virtio disk max queue too short");
  }

  // allocate and zero queue memory.
  descriptors_ = static_cast<struct virtq_desc *>(detail::allocate_dma_page());
  available_ = static_cast<struct virtq_avail *>(detail::allocate_dma_page());
  used_ = static_cast<struct virtq_used *>(detail::allocate_dma_page());
  if (!descriptors_ || !available_ || !used_)
  {
    detail::fail("virtio disk kalloc");
  }
  detail::zero_page(descriptors_);
  detail::zero_page(available_);
  detail::zero_page(const_cast<virtq_used *>(used_));

  // set queue size.
  registers_.write32(VIRTIO_MMIO_QUEUE_NUM, QueueSize);

  // write physical addresses.
  registers_.write32(VIRTIO_MMIO_QUEUE_DESC_LOW, (unsigned long)descriptors_);
  registers_.write32(VIRTIO_MMIO_QUEUE_DESC_HIGH, (unsigned long)descriptors_ >> 32);
  registers_.write32(VIRTIO_MMIO_DRIVER_DESC_LOW, (unsigned long)available_);
  registers_.write32(VIRTIO_MMIO_DRIVER_DESC_HIGH, (unsigned long)available_ >> 32);
  registers_.write32(VIRTIO_MMIO_DEVICE_DESC_LOW, (unsigned long)used_);
  registers_.write32(VIRTIO_MMIO_DEVICE_DESC_HIGH, (unsigned long)used_ >> 32);

  // queue is ready.
  registers_.write32(VIRTIO_MMIO_QUEUE_READY, 0x1);

  // all QueueSize descriptors start out unused.
  for (int i = 0; i < QueueSize; i++)
  {
    free_[i] = 1;
  }

  // tell device we're completely ready.
  status |= VIRTIO_CONFIG_S_DRIVER_OK;
  registers_.write32(VIRTIO_MMIO_STATUS, status);

  // The board router connects the device IRQ after global initialization.
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
  for (int i = 0; i < 3; i++)
  {
    idx[i] = allocate_descriptor();
    if (idx[i] < 0)
    {
      for (int j = 0; j < i; j++)
      {
        free_descriptor(idx[j]);
      }
      return -1;
    }
  }
  return 0;
}

void VirtioBlock::transfer(BlockOperation operation, unsigned long sector, void *data,
                           unsigned count)
{
  if (!data || count == 0 || count % SectorSize != 0)
  {
    detail::fail("virtio: invalid transfer");
  }
  const bool write = operation == BlockOperation::Write;

  lock_.acquire();

  // the spec's Section 5.2 says that block operations use
  // three descriptors: one for type/reserved/sector, one for the
  // data, one for a 1-byte status result.

  // allocate the three descriptors.
  int idx[3];
  while (1)
  {
    if (allocate_chain(idx) == 0)
    {
      break;
    }
    detail::prepare_sleep(&free_[0]);
    lock_.release();
    detail::sleep_process();
    lock_.acquire();
  }

  // format the three descriptors.
  // qemu's virtio-blk.c reads them.

  struct virtio_blk_req *buf0 = &operations_[idx[0]];

  if (write)
  {
    buf0->type = VIRTIO_BLK_T_OUT; // write the disk
  }
  else
  {
    buf0->type = VIRTIO_BLK_T_IN; // read the disk
  }
  buf0->reserved = 0;
  buf0->sector = sector;

  descriptors_[idx[0]].addr = (unsigned long)buf0;
  descriptors_[idx[0]].len = sizeof(struct virtio_blk_req);
  descriptors_[idx[0]].flags = VRING_DESC_F_NEXT;
  descriptors_[idx[0]].next = idx[1];

  descriptors_[idx[1]].addr = (unsigned long)data;
  descriptors_[idx[1]].len = count;
  if (write)
  {
    descriptors_[idx[1]].flags = 0; // device reads data
  }
  else
  {
    descriptors_[idx[1]].flags = VRING_DESC_F_WRITE; // device writes data
  }
  descriptors_[idx[1]].flags |= VRING_DESC_F_NEXT;
  descriptors_[idx[1]].next = idx[2];

  requests_[idx[0]].status = 0xff; // device writes 0 on success
  descriptors_[idx[2]].addr = (unsigned long)&requests_[idx[0]].status;
  descriptors_[idx[2]].len = 1;
  descriptors_[idx[2]].flags = VRING_DESC_F_WRITE; // device writes the status
  descriptors_[idx[2]].next = 0;

  // Track completion independently of the caller and its filesystem layout.
  requests_[idx[0]].pending = true;

  // tell the device the first index in our chain of descriptors.
  available_->ring[available_->idx % QueueSize] = idx[0];

  detail::io_barrier();

  // tell the device another avail ring entry is available.
  available_->idx += 1; // not % QueueSize ...

  detail::io_barrier();

  registers_.write32(VIRTIO_MMIO_QUEUE_NOTIFY, 0); // value is queue number

  // The interrupt handler clears pending after DMA completion.
  while (requests_[idx[0]].pending)
  {
    detail::prepare_sleep(&requests_[idx[0]]);
    lock_.release();
    detail::sleep_process();
    lock_.acquire();
  }

  free_chain(idx[0]);

  lock_.release();
}

void VirtioBlock::handle_interrupt()
{
  xv::sync::ScopedLock guard(lock_);

  // the device won't raise another interrupt until we tell it
  // we've seen this interrupt, which the following line does.
  // this may race with the device writing new entries to
  // the "used" ring, in which case we may process the new
  // completion entries in this interrupt, and have nothing to do
  // in the next interrupt, which is harmless.
  registers_.write32(VIRTIO_MMIO_INTERRUPT_ACK,
                     registers_.read32(VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3);

  detail::io_barrier();

  // the device increments used_->idx when it
  // adds an entry to the used ring.

  while (used_index_ != used_->idx)
  {
    detail::io_barrier();
    int id = used_->ring[used_index_ % QueueSize].id;

    if (id < 0 || id >= QueueSize || !requests_[id].pending)
    {
      detail::fail("virtio: invalid completion");
    }
    if (requests_[id].status != 0)
    {
      detail::fail("virtio_disk_intr status");
    }

    requests_[id].pending = false;
    detail::wake_processes(&requests_[id]);

    used_index_ += 1;
  }
}

} // namespace xv::drivers
