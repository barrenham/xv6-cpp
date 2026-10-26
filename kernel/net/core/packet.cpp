module xv.net;
import :packet;
import :platform;
namespace xv::net
{
Packet::~Packet() { if (page_) detail::free_page(page_); }
Packet Packet::allocate(unsigned length)
{ return length <= 4096 ? Packet(detail::allocate_page(), length) : Packet(); }
}

namespace xv::net
{
Packet Packet::clone() const
{
  if (!page_ || length_ > 4096) return {};
  auto copy = allocate(length_);
  if (copy) for (unsigned i = 0; i < length_; ++i) copy.data()[i] = data()[i];
  return copy;
}
}
