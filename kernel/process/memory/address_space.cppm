export module xv.process:address_space;
import xv.vm_pool;

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
    return size_;
  }

  unsigned long observed_size() const
  {
    return __atomic_load_n(&observed_size_, __ATOMIC_ACQUIRE);
  }

  bool create(void *trap_frame, void *user_info);
  void reset();
  void swap(AddressSpace &other);
  bool copy_to(AddressSpace &child) const;
  int resize(int bytes, bool eager = true);
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
  int sync_mappings(unsigned long address, unsigned long bytes);
  int protect_mappings(unsigned long address, unsigned long bytes, unsigned permissions);
  int unmap_shared_pages(unsigned long address, unsigned long bytes);

  int copy_in(void *destination, unsigned long source, unsigned long bytes) const;
  int copy_out(unsigned long destination, const void *source,
               unsigned long bytes) const;
  int copy_string(char *destination, unsigned long source, unsigned long limit) const;
  bool prefault_mappings(unsigned long address, unsigned long bytes,
                         xv::memory::FaultAccess access) const;

private:

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
  unsigned long device_identity_ = 0;
  unsigned long map_device(void *const *pages, unsigned count,
                           const AddressSpace *inherited);

  PageTable table_ = nullptr;
  unsigned long size_ = 0;
  unsigned long observed_size_ = 0;
  unsigned long heap_base_ = 0;
  xv::memory::VirtualAddressPool regions_;
};
} // namespace xv::process
