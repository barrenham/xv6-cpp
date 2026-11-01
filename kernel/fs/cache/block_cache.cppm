export module xv.fs:cache;

export import xv.sync;

export namespace xv::fs
{
inline constexpr unsigned BlockSize = 1024;

struct CacheStatistics
{
  unsigned long hits = 0, misses = 0, evictions = 0, lookup_steps = 0;
  unsigned long reads = 0, writes = 0, eviction_steps = 0;
  unsigned long locks = 0, contentions = 0, spins = 0, wait_ticks = 0;
};
class BlockCache;
class LruBlockCache;

class Buffer final
{
public:

  constexpr Buffer() = default;

  unsigned char *bytes()
  {
    return data;
  }

  const unsigned char *bytes() const
  {
    return data;
  }

  unsigned block_number() const
  {
    return blockno;
  }

  unsigned device() const
  {
    return dev;
  }

private:

  friend class LruBlockCache;
  friend class BlockCache;
  bool assigned_ = false;
  int valid = 0;
  unsigned dev = 0;
  unsigned blockno = 0;
  xv::sync::SleepLock lock;
  unsigned refcnt = 0;
  // Identity and links belong to bucket_; replacement alone changes bucket_.
  unsigned bucket_ = 0;
  Buffer *next = nullptr;
  // Atomic hints permit victim selection without locking every bucket.
  unsigned long last_use_ = 0;
  alignas(unsigned long) unsigned char data[BlockSize]{};
};

class BlockHandle final
{
public:

  constexpr BlockHandle() = default;

  BlockHandle(BlockCache &cache, Buffer &buffer) : cache_(&cache), buffer_(&buffer)
  {
  }

  ~BlockHandle();
  BlockHandle(const BlockHandle &) = delete;
  BlockHandle &operator=(const BlockHandle &) = delete;
  BlockHandle(BlockHandle &&other) noexcept;
  BlockHandle &operator=(BlockHandle &&other) noexcept;

  explicit operator bool() const
  {
    return buffer_ != nullptr;
  }

  Buffer *operator->() const
  {
    return buffer_;
  }

  Buffer &operator*() const
  {
    return *buffer_;
  }

  void reset();

private:

  BlockCache *cache_ = nullptr;
  Buffer *buffer_ = nullptr;
};

class BlockCache
{
public:

  BlockCache(const BlockCache &) = delete;
  BlockCache &operator=(const BlockCache &) = delete;
  virtual const char *name() const = 0;

  virtual CacheStatistics statistics() const
  {
    return {};
  }

  virtual void initialize() = 0;

  virtual void flush()
  {
  }

  virtual BlockHandle read(unsigned device, unsigned block) = 0;
  virtual void write(Buffer &buffer) = 0;
  virtual void release(Buffer &buffer) = 0;
  virtual void pin(Buffer &buffer) = 0;
  virtual void unpin(Buffer &buffer) = 0;

protected:

  constexpr BlockCache() = default;

  static void identify(Buffer &buffer, unsigned device, unsigned block)
  {
    buffer.dev = device;
    buffer.blockno = block;
  }

  ~BlockCache() = default;
};

inline BlockHandle::~BlockHandle()
{
  reset();
}

inline BlockHandle::BlockHandle(BlockHandle &&other) noexcept
    : cache_(other.cache_), buffer_(other.buffer_)
{
  other.cache_ = nullptr;
  other.buffer_ = nullptr;
}

inline BlockHandle &BlockHandle::operator=(BlockHandle &&other) noexcept
{
  if (this != &other)
  {
    reset();
    cache_ = other.cache_;
    buffer_ = other.buffer_;
    other.cache_ = nullptr;
    other.buffer_ = nullptr;
  }
  return *this;
}

inline void BlockHandle::reset()
{
  if (buffer_)
  {
    cache_->release(*buffer_);
  }
  cache_ = nullptr;
  buffer_ = nullptr;
}

BlockCache &cache();
void initialize_cache();
} // namespace xv::fs
