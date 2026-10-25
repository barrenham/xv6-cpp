export module xv.memory:user_paging;

export namespace xv::memory
{
using PageEntry = unsigned long;
using PageTable = PageEntry *;

void map_kernel_range(PageTable table, unsigned long virtual_address,
                      unsigned long physical_address, unsigned long bytes,
                      int permissions);
int map_user_pages(PageTable table, unsigned long address, unsigned long bytes,
                   unsigned long physical_address, int permissions);
PageTable create_user_table();
unsigned long allocate_user_pages(PageTable table, unsigned long old_size,
                                  unsigned long new_size, int permissions);
unsigned long deallocate_user_pages(PageTable table, unsigned long old_size,
                                    unsigned long new_size);
int clone_user_pages(PageTable source, PageTable destination, unsigned long size);
void free_user_table(PageTable table, unsigned long size);
void unmap_user_pages(PageTable table, unsigned long address, unsigned long pages,
                      int free_pages);
void protect_user_guard(PageTable table, unsigned long address);
PageEntry *walk_entry(PageTable table, unsigned long address, int allocate);
unsigned long translate_user_page(PageTable table, unsigned long address);
int copy_to_user(PageTable table, unsigned long address_space_size,
                 unsigned long destination, const char *source, unsigned long bytes);
int copy_from_user(PageTable table, unsigned long address_space_size, char *destination,
                   unsigned long source, unsigned long bytes);
int copy_user_string(PageTable table, unsigned long address_space_size,
                     char *destination, unsigned long source, unsigned long limit);
int is_user_mapped(PageTable table, unsigned long address);
unsigned long resolve_user_fault(PageTable table, unsigned long address_space_size,
                                 unsigned long address, int read);

#if defined(LAB_PGTBL) || defined(SOL_MMAP)
void print_page_table(PageTable table);
#endif

#ifdef LAB_PGTBL
int demote_user_pages(PageTable table, unsigned long address, unsigned long pages);
#endif
} // namespace xv::memory
