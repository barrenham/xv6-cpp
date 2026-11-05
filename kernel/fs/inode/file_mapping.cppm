export module xv.fs:mapping;
import :filesystem;
import xv.vm_types;

export namespace xv::fs
{
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
  void *page(unsigned index) const;

  unsigned
  count() const {
    return pages_;
  }

private:

  Inode *node_ = nullptr;
  unsigned offset_ = 0, pages_ = 0, references_ = 1;
  bool writing_ = false;
  void *entries_[256]{};
};
} // namespace xv::fs
