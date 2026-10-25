export module xv.memory:kernel_paging;

export namespace xv::memory
{
// One shared kernel page table; activation is local to the calling hart.
class KernelPageTable final
{
public:

  constexpr KernelPageTable() = default;
  KernelPageTable(const KernelPageTable &) = delete;
  KernelPageTable &operator=(const KernelPageTable &) = delete;

  void initialize();
  void activate_hart() const;
  const void *root() const;
  // Internal process-stack service: slots are derived from physical page IDs.
  unsigned long map_process_stack(unsigned long slot, void *physical);
  void *unmap_process_stack(unsigned long address);

private:

  void *root_ = nullptr;
  unsigned long stack_slots_ = 0;
};

KernelPageTable &kernel_page_table();
} // namespace xv::memory
