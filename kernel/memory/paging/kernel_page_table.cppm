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

private:

  void *root_ = nullptr;
};

KernelPageTable &kernel_page_table();
} // namespace xv::memory
