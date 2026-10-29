module xv.drivers:virtio;

import :protocol;
import :block;
import :mmio;
import xv.sync;

namespace xv::drivers
{
class VirtioBlock final : public BlockDevice
{
public:

  constexpr VirtioBlock(unsigned long base, unsigned irq) : registers_(base), irq_(irq)
  {
  }

  const char *name() const override
  {
    return "VirtIO MMIO block";
  }

  unsigned irq() const override
  {
    return irq_;
  }

  void initialize() override;
  bool present() const;
  void flush() override;
  void transfer(BlockOperation operation, unsigned long sector, void *data,
                unsigned count) override;
  void handle_interrupt() override;

private:

  struct InitializeFlow;
  struct ChainFlow;
  struct TransferFlow;
  struct InterruptFlow;

  int allocate_descriptor();
  void free_descriptor(int index);
  void free_chain(int index);
  int allocate_chain(int *indices);

  detail::MmioRegion registers_;
  unsigned irq_;
  bool can_flush_ = false;
  virtq_desc *descriptors_ = nullptr;
  virtq_avail *available_ = nullptr;
  volatile virtq_used *used_ = nullptr;
  char free_[QueueSize]{};
  unsigned short used_index_ = 0;

  struct Request
  {
    volatile unsigned char status = 0;
    bool pending = false;
  } requests_[QueueSize]{};

  virtio_blk_req operations_[QueueSize]{};
  xv::sync::SpinLock lock_;
};
} // namespace xv::drivers
