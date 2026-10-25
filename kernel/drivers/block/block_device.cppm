export module xv.drivers:block;

export import :interrupt;

export namespace xv::drivers
{
enum class BlockOperation
{
  Read,
  Write
};

class BlockDevice : public InterruptDevice
{
public:

  static constexpr unsigned SectorSize = 512;
  // Synchronous process-context transfer. Data must remain DMA-accessible and
  // exclusively owned until return; count must be a positive sector multiple.
  // Completion state belongs to the driver, independently of filesystem buffers.
  virtual void transfer(BlockOperation operation, unsigned long sector, void *data,
                        unsigned count) = 0;

protected:

  constexpr BlockDevice() = default;
  ~BlockDevice() = default;
};
} // namespace xv::drivers
