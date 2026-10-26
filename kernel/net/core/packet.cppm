export module xv.net:packet;
export namespace xv::net
{
class Packet final
{
public:
  constexpr Packet() = default;
  Packet(void *page, unsigned length) : page_(page), length_(length) {}
  Packet(const Packet &) = delete;
  Packet &operator=(const Packet &) = delete;
  Packet(Packet &&other) : page_(other.release()), length_(other.length_), depth_(other.depth_) {}
  ~Packet();
  static Packet allocate(unsigned length);
  unsigned char *data() const { return static_cast<unsigned char *>(page_); }
  bool enter_tunnel() { if (depth_ == 4) return false; ++depth_; return true; }
  unsigned size() const { return length_; }
  bool prepend(unsigned count)
  {
    if (!page_ || length_ > 4096 || count > 4096-length_) return false;
    auto *p = data();
    for (unsigned i = length_; i; --i) p[i+count-1] = p[i-1];
    for (unsigned i = 0; i < count; ++i) p[i] = 0;
    length_ += count; return true;
  }
  bool remove_prefix(unsigned count)
  {
    if (!page_ || length_ > 4096 || count > length_) return false;
    auto *p = data(); length_ -= count;
    for (unsigned i = 0; i < length_; ++i) p[i] = p[i+count];
    return true;
  }
  bool resize(unsigned length)
  { if (!page_ || length > 4096) return false; length_ = length; return true; }
  explicit operator bool() const { return page_ != nullptr; }
  void *release() { void *page = page_; page_ = nullptr; return page; }
private:
  void *page_ = nullptr;
  unsigned length_ = 0;
  unsigned depth_ = 0;
};
}
