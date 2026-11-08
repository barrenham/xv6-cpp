export module xv.userfs.shared;
export import xv.userfs.storage;

export namespace userfs
{
// Generic shared heap for independent processes. Not used by FS request
// workers, which share an ordinary process heap. Mappings/capabilities must be
// prepared before fork; growth commits physical pages lazily.
class SharedStorage final : public MemoryStorage
{
public:

  SharedStorage();
  ~SharedStorage();
  SharedStorage(const SharedStorage &) = delete;
  SharedStorage &operator=(const SharedStorage &) = delete;

  bool
  ready() const {
    return state_ != nullptr;
  }

  void *allocate(unsigned bytes) override;
  void release(void *) override;
  void invalidate() override;

private:

  struct State;
  struct Block;
  State *state_ = nullptr;
  int queue_[2]{-1, -1};
  void *mappings_[16]{};
  void lock();
  void unlock();
  bool grow(unsigned);
  int borrow(unsigned);
  bool give(unsigned, int);
  void insert(Block *);
};
} // namespace userfs
