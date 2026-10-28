module xv.net;
import :packet;
import :platform;

namespace xv::net
{
Packet::~Packet()
{
  if (page_)
  {
    detail::free_page(page_);
  }
}

Packet Packet::allocate(unsigned length)
{
  return length <= Capacity ? Packet(detail::allocate_page(), length) : Packet();
}

Packet Packet::clone() const
{
  if (!page_ || length_ > Capacity)
  {
    return {};
  }
  auto copy = allocate(length_);
  if (copy)
  {
    copy.context_ = context_;
    for (unsigned i = 0; i < length_; ++i)
    {
      copy.data()[i] = data()[i];
    }
  }
  return copy;
}
} // namespace xv::net
