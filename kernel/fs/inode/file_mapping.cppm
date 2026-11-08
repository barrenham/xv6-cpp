export module xv.fs:mapping;
import :filesystem;
import xv.vm_types;

export namespace xv::fs
{
// Safe from allocation slow paths: only unleased, clean file frames; no RPC.
unsigned reclaim_file_pages(unsigned target);
unsigned long cached_file_pages();
unsigned reclaim_mapped_file_pages(unsigned target);
unsigned reclaim_file_pages_slow(unsigned target);
void start_file_cache_reclaimer();
void file_cache_reclaimer();

struct FileCacheInfo
{
  unsigned long pages, pending, idle_evictions, mapped_evictions, target;
};

FileCacheInfo file_cache_info();
// A lease on inode-backed physical pages. Call release outside process/spin locks:
// the last lease can write dirty data and release the backend inode reference.
class FileMapping final
{
public:

  static FileMapping *create(Inode *, unsigned offset, unsigned pages, int &error);
  FileMapping *retain();
  void release();
  xv::memory::MappingBacking *backing();
  int enable_write();
  int synchronize();
  bool page_available(unsigned page) const;
  unsigned long length() const;
  void *page(unsigned index);
  void *load_page(unsigned index, int &error);
  void *pin_page(unsigned index, int &error);
  void *resident_page(unsigned index) const;

  unsigned
  count() const {
    return pages_;
  }

private:

  friend struct MappedPages;
  void *load_page_impl(unsigned index, int &error, bool pin);
  FileMapping *next_ = nullptr;
  Inode *node_ = nullptr;
  unsigned offset_ = 0, pages_ = 0, references_ = 1;
  bool writing_ = false;
  void *entries_[256]{};
};
} // namespace xv::fs
