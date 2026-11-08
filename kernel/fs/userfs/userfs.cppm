module;
#include "kernel/fs/userfs/protocol.h"
module xv.fs:userfs;
import :filesystem;
import :file;

namespace xv::fs
{
class UserFileSystem final : public FileSystem
{
public:

  constexpr UserFileSystem() = default;

  const char *
  name() const override {
    return registration_.type;
  }

  bool
  read_only() const override {
    return registration_.flags & xv::userfs::ReadOnly;
  }

  bool
  snapshot_reads() const override {
    return registration_.flags & xv::userfs::Snapshots;
  }

  bool snapshot(Inode *, PageBuffer &) override;
  bool mappable() const override { return cache_flags_ & xv::userfs::CacheMetadata; }

  unsigned
  name_max() const override {
    return 255;
  }

  bool
  persistent() const override {
    return false;
  }

  void initialize_inodes();
  int provider_pid() const { return provider_pid_; }
  bool operation_held();
  static UserFileSystem *create(unsigned device, const xv::userfs::Registration &,
                                bool anonymous);
  void destroy();
  bool reserve(unsigned device);
  bool claim_mount();
  bool retired();

  bool
  anonymous() const {
    return anonymous_;
  }

  bool describe_provider(xv::userfs::ProviderInfo &);

  void attached(bool value);

  xv::io::IoObject &
  endpoint() {
    return endpoint_;
  }

  xv::drivers::CharacterDevice *
  open_device(Inode *node, bool nonblocking) override {
    if (node->device_major() == 5 &&
        !devices().get(node->device_major(), node->device_minor()))
    {
      return xv::drivers::open_named_pty(node->device_minor());
    }
    return open_registered_device(node, nonblocking);
  }

  xv::drivers::BlockDevice *
  open_block_device(Inode *node, bool writable) override {
    return open_registered_block(node, writable);
  }

  Inode *root() override;

  Inode *retain(Inode *) override;
  bool pin_cached_inode(Inode *) override;
  void release(Inode *) override;

  void
  lock(Inode *node) override {
    tree_.acquire();
    node->data_lock_.acquire();
  }

  void
  unlock(Inode *node) override {
    node->data_lock_.release();
    tree_.release();
  }

  void
  unlock_release(Inode *node) override {
    unlock(node);
    release(node);
  }

  void lock_data(Inode *) override;
  void unlock_data(Inode *) override;
  void lock_read(Inode *) override;
  void unlock_read(Inode *) override;

  void
  record_read_hit() override {
    __atomic_fetch_add(&concurrency_.read_hits, 1UL, __ATOMIC_RELAXED);
  }
  int read_data(Inode *, int, unsigned long, unsigned, unsigned) override;
  int write_data(Inode *, int, unsigned long, unsigned, unsigned) override;
  int write_cached_data(Inode *, int, unsigned long, unsigned, unsigned) override;

  bool
  publishes_cached_writes(Inode *node) const override {
    return __atomic_load_n(&node->metadata_valid_, __ATOMIC_ACQUIRE);
  }
  int resize(Inode *, unsigned) override;
  void stat(Inode *, Stat *) override;
  int status(Inode *, Stat *) override;
  int synchronize(Inode *) override;
  bool attributes(Inode *, unsigned, unsigned, unsigned) override;
  Inode *lookup_entry(Inode *, const char *, unsigned *) override;
  Inode *create_at(Inode *, const char *, short, short, short) override;
  int link_at(Inode *, const char *, Inode *) override;
  int unlink_at(Inode *, const char *) override;
  int rename_at(Inode *, const char *, Inode *, const char *) override;
  int next_directory(Inode *, unsigned long &, DirectoryRecord &) override;
  int read(Inode *, int, unsigned long, unsigned, unsigned) override;
  int write(Inode *, int, unsigned long, unsigned, unsigned) override;
  int control(Inode *, unsigned long, unsigned) override;
  bool busy() override;
  void shutdown() override;

private:

  class Endpoint final : public xv::io::IoObject, public xv::io::DeviceControl
  {
  public:

    constexpr explicit Endpoint(UserFileSystem &owner) : owner_(owner) {
    }

    xv::io::Capability *
    capability(xv::io::CapabilityId id) override {
      return id == xv::io::DeviceControl::Id
                 ? static_cast<xv::io::DeviceControl *>(this)
                 : nullptr;
    }

    int ioctl(unsigned, xv::io::Buffer) override;
    int read(xv::io::Buffer, int) override;
    int write(xv::io::Buffer, int) override;
    unsigned poll(xv::sync::PollTable *) override;
    void close() override;

  private:

    UserFileSystem &owner_;
  } endpoint_{*this};

  void begin(xv::userfs::Operation, unsigned node = 1);
  bool exchange();
  int bulk_copy(unsigned long, unsigned, bool user, bool writing, unsigned offset = 0);
  Inode *node(const xv::userfs::Attributes &);
  void apply(Inode *, const xv::userfs::Attributes &);
  void disconnect();
  xv::sync::SleepLock tree_, serial_;
  xv::sync::SpinLock state_, references_;
  bool connected_ = false, mounted_ = false, reserved_ = false;
  bool endpoint_open_ = false;
  bool pending_ = false, delivered_ = false, replied_ = false;
  unsigned device_ = 0, live_references_ = 0;
  unsigned long sequence_ = 0;

  struct NodePage
  {
    NodePage *next = nullptr;
    Inode nodes[(4096 - sizeof(NodePage *)) / sizeof(Inode)]{};
  };

  NodePage *node_pages_ = nullptr;
  Inode *nodes_[64]{};
  xv::userfs::Registration registration_;
  int provider_pid_ = 0;
  bool anonymous_ = false, mounting_ = false, completed_ = false;
  xv::userfs::Message request_, response_;
  xv::userfs::TransportStatistics statistics_;
  File *peer_ = nullptr, *bulk_ = nullptr;
  File *data_peers_[xv::userfs::TransportLanes - 1]{};
  struct DataSlot { int owner = 0; } data_slots_[xv::userfs::TransportLanes - 1];
  unsigned lanes_ = 1;
  xv::userfs::ConcurrencyStatistics concurrency_;
  unsigned long data_epoch_ = 0, request_epoch_ = 0;
  class Request;
  struct LookupCache;
  LookupCache *lookup_cache_ = nullptr;
  unsigned cache_flags_ = 0;
  bool cache_active();
  Inode *cached_lookup(Inode *, const char *);
  void cache_lookup(Inode *, const char *, Inode *);
  void invalidate_lookup();
  void cache_pressure();
  int read_impl(Inode *, int, unsigned long, unsigned, unsigned, bool);
  int write_impl(Inode *, int, unsigned long, unsigned, unsigned, bool,
                 bool publish = false);
  int write_fault_blocks(Request &, Inode *, int, unsigned long, unsigned, unsigned,
                         bool);
};
} // namespace xv::fs
