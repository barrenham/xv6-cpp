export module xv.process:address_space;
import xv.vm_pool;
import xv.sync;

export namespace xv::process
{
using PageTable = unsigned long *;

// Process-owned address space. reset() is explicit because reaping holds the
// process lock and global process slots must have trivial destruction.
class AddressSpace final
{
public:

  constexpr AddressSpace() = default;
  AddressSpace(const AddressSpace &) = delete;
  AddressSpace &operator=(const AddressSpace &) = delete;

  PageTable page_table() const
  {
    return table_;
  }

  unsigned long size() const
  {
    return observed_size();
  }

  unsigned long observed_size() const
  {
    return __atomic_load_n(&observed_size_, __ATOMIC_ACQUIRE);
  }

  bool create(void *trap_frame, void *user_info, unsigned frame_slot = 0, bool describe = true);
  void enable_concurrency();
  bool operation_held() const;
  bool refresh_for_user() const;
  bool user_entry_allowed() const;
  void reset();
  // Borrowed supervisor-only register storage; the thread owns its physical page.
  bool map_thread_frame(unsigned slot, void *frame);
  bool unmap_thread_frame(unsigned slot, void *frame);
  void swap(AddressSpace &other);
  bool copy_to(AddressSpace &child) const;
  int resize(int bytes, bool eager = true, unsigned long *previous = nullptr);
  unsigned long page_entry(unsigned long address) const;
  int sample_access(unsigned long base, int pages, unsigned long output);
  bool grow_to(unsigned long end, int permissions, bool stack = false);
  bool protect_guard(unsigned long address);
  unsigned long translate(unsigned long address) const;
  xv::memory::FaultOutcome handle_fault(unsigned long address,
                                        xv::memory::FaultAccess access);

  const xv::memory::VirtualAddressPool &regions() const
  {
    return regions_;
  }

  // A shared, non-executable device aperture outside the ordinary heap.
  unsigned long map_device_pages(void *const *pages, unsigned count);
  bool unmap_device_pages();
  bool device_mapping(void *const *pages, unsigned count) const;

  unsigned long map_shared_pages(void *const *, unsigned count, unsigned permissions,
                                 bool private_copy = false, unsigned long hint = 0,
                                 unsigned maximum_permissions = 30,
                                 xv::memory::MappingBacking *backing = nullptr);
  void close_file_mappings();
  void refresh_file_mappings() const;
  // Caller owns the non-running process lock; never waits, allocates or does I/O.
  unsigned reclaim_inactive_mappings() const;
  int sync_mappings(unsigned long address, unsigned long bytes);
  int protect_mappings(unsigned long address, unsigned long bytes, unsigned permissions);
  int unmap_shared_pages(unsigned long address, unsigned long bytes);

  bool load_word(unsigned long address, unsigned &value) const;
  int copy_in(void *destination, unsigned long source, unsigned long bytes) const;
  int copy_out(unsigned long destination, const void *source,
               unsigned long bytes) const;
  int copy_string(char *destination, unsigned long source, unsigned long limit) const;
  bool prefault_mappings(unsigned long address, unsigned long bytes,
                         xv::memory::FaultAccess access) const;

private:

  class Access
  {
  public:

    explicit Access(const AddressSpace &owner);
    ~Access();

    Access(const Access &) = delete;
    Access &operator=(const Access &) = delete;

  private:

    const AddressSpace &owner_;
  };

  // Drop every nested VM acquisition around an owned backing callback.
  // Callers must snapshot references and revalidate generation after resuming.
  class Suspension
  {
  public:

    explicit Suspension(const AddressSpace &, bool keep_block = false);
    ~Suspension();
    Suspension(const Suspension &) = delete;
    Suspension &operator=(const Suspension &) = delete;

  private:

    const AddressSpace &owner_;
    unsigned depth_;
    bool blocked_;
  };

  bool invalidate_translations() const;
  bool prepare_write(unsigned long address, unsigned long bytes) const;
  mutable xv::sync::SpinLock operation_lock_;
  mutable unsigned user_blocks_ = 0;
  mutable bool operation_blocked_ = false;
  mutable unsigned operation_depth_ = 0;
  mutable unsigned long generation_ = 1;
  bool shared_execution_ = false;
  bool retiring_ = false;

  mutable unsigned long observed_reclamation_ = 0;
  unsigned reclaim_revoked_mappings() const;

  struct SharedMappings;
  SharedMappings *shared_ = nullptr;
  void reset_shared_pages();
  bool copy_shared_pages(AddressSpace &) const;
  xv::memory::FaultOutcome fault_mapping(unsigned long, xv::memory::FaultAccess) const;
  bool prepare_mappings(unsigned long address, unsigned long bytes,
                        xv::memory::FaultAccess access) const;

  struct CreateFlow;
  struct ResizeFlow;
  struct GrowFlow;
  struct CopyFlow;
  struct GuardFlow;
  struct DeviceMapFlow;
  unsigned device_pages_ = 0;
  mutable unsigned active_accesses_ = 0;
  unsigned long device_identity_ = 0;
  unsigned long map_device(void *const *pages, unsigned count,
                           const AddressSpace *inherited);

  PageTable table_ = nullptr;
  unsigned long size_ = 0;
  unsigned long observed_size_ = 0;
  unsigned long heap_base_ = 0;
  xv::memory::VirtualAddressPool regions_;
};

// VM mutation holds its operation lock; platform quiesces old user translations.
bool synchronize_address_space(const AddressSpace &);
void wake_address_space(const AddressSpace &);
void wait_address_space(const AddressSpace &);
} // namespace xv::process
