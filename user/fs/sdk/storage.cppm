export module xv.userfs.storage;

export namespace userfs
{
class MemoryStorage
{
public:

  virtual void *allocate(unsigned bytes) = 0;
  virtual void release(void *) = 0;

  // Exclusive owner only, after every worker has stopped following a failure.
  virtual void
  invalidate() {
  }

protected:

  ~MemoryStorage() = default;
};
} // namespace userfs
