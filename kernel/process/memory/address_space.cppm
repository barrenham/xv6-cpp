export module xv.process:address_space;

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

  bool create(void *trap_frame, void *user_info);
  void reset();
  void swap(AddressSpace &other);
  bool copy_to(AddressSpace &child) const;
  int resize(int bytes, bool eager = true);
  bool grow_to(unsigned long end, int permissions);
  void protect_guard(unsigned long address);
  unsigned long translate(unsigned long address) const;
  unsigned long handle_fault(unsigned long address, bool read);
  int copy_in(void *destination, unsigned long source, unsigned long bytes) const;
  int copy_out(unsigned long destination, const void *source,
               unsigned long bytes) const;
  int copy_string(char *destination, unsigned long source, unsigned long limit) const;

private:

  PageTable table_ = nullptr;
  unsigned long size_ = 0;
};
} // namespace xv::process
