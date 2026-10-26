module xv.net;
import :packet;
import :platform;
namespace xv::net
{
Packet::~Packet() { if (page_) detail::free_page(page_); }
Packet Packet::allocate(unsigned length)
{ return length <= 4096 ? Packet(detail::allocate_page(), length) : Packet(); }
}
