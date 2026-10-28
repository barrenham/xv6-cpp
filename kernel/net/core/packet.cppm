export module xv.net:packet;

export namespace xv::net
{
class Packet final
{
public:

  static constexpr unsigned Capacity = 4096;
  static constexpr unsigned MaxTunnelDepth = 4;

  struct Context
  {
    unsigned tunnel_depth = 0;
  };

  constexpr Packet() = default;

  Packet(void *page, unsigned length) : page_(page), length_(length)
  {
  }

  Packet(void *page, unsigned length, Context context)
      : page_(page), length_(length), context_(context)
  {
  }

  Packet(const Packet &) = delete;
  Packet &operator=(const Packet &) = delete;

  Packet(Packet &&other)
      : page_(other.release()), length_(other.length_), context_(other.context_)
  {
  }

  ~Packet();
  static Packet allocate(unsigned length);
  Packet clone() const;

  unsigned char *data()
  {
    return static_cast<unsigned char *>(page_);
  }

  const unsigned char *data() const
  {
    return static_cast<const unsigned char *>(page_);
  }

  Context context() const
  {
    return context_;
  }

  bool enter_tunnel()
  {
    if (context_.tunnel_depth >= MaxTunnelDepth)
    {
      return false;
    }
    ++context_.tunnel_depth;
    return true;
  }

  unsigned size() const
  {
    return length_;
  }

  bool prepend(unsigned count)
  {
    if (!page_ || length_ > Capacity || count > Capacity - length_)
    {
      return false;
    }
    auto *bytes = data();
    for (unsigned i = length_; i; --i)
    {
      bytes[i + count - 1] = bytes[i - 1];
    }
    for (unsigned i = 0; i < count; ++i)
    {
      bytes[i] = 0;
    }
    length_ += count;
    return true;
  }

  bool remove_prefix(unsigned count)
  {
    if (!page_ || length_ > Capacity || count > length_)
    {
      return false;
    }
    auto *bytes = data();
    length_ -= count;
    for (unsigned i = 0; i < length_; ++i)
    {
      bytes[i] = bytes[i + count];
    }
    return true;
  }

  bool resize(unsigned length)
  {
    if (!page_ || length > Capacity)
    {
      return false;
    }
    length_ = length;
    return true;
  }

  explicit operator bool() const
  {
    return page_ != nullptr;
  }

  void *release()
  {
    void *page = page_;
    page_ = nullptr;
    return page;
  }

private:

  void *page_ = nullptr;
  unsigned length_ = 0;
  Context context_{};
};
} // namespace xv::net
