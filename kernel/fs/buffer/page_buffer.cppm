export module xv.fs:buffer;
export namespace xv::fs
{
class PageBuffer final
{
public:
  constexpr PageBuffer() = default;
  ~PageBuffer();
  PageBuffer(const PageBuffer &) = delete;
  PageBuffer &operator=(const PageBuffer &) = delete;
  void take(PageBuffer &other);
  unsigned size() const { return size_; }
  bool append(const void *source, unsigned count);
  int read(unsigned offset, bool user, unsigned long destination, unsigned count) const;
  int write(unsigned offset, bool user, unsigned long source, unsigned count);
  void clear();
private:
  struct Page
  {
    Page *next;
    unsigned char bytes[4096 - sizeof(Page *)];
  };
  Page *page(unsigned index, bool create);
  const Page *page(unsigned index) const;
  Page *head_ = nullptr;
  Page *tail_ = nullptr;
  unsigned pages_ = 0;
  unsigned size_ = 0;
};
}
