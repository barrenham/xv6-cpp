module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
module xv.net:udp_flows;
import :udp;

namespace xv::net
{
struct UdpProtocol::ReceiveFlow
{
  struct Context
  {
    UdpProtocol &owner;
    Packet &packet;
    const IpDatagram &datagram;
    const unsigned char *segment = nullptr;
    bool handled = false;
  };

  static inline bool has_header(Context &c) noexcept
  {
    return c.datagram.length >= 8;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.segment = c.packet.data() + c.datagram.offset;
  }

  static inline bool valid_header(Context &c) noexcept
  {
    return wire::read16(c.segment + 4) == c.datagram.length &&
           wire::read16(c.segment + 2);
  }

  static inline bool valid_checksum(Context &c) noexcept
  {
    return !wire::read16(c.segment + 6) ||
           wire::sum(c.segment, c.datagram.length,
                     wire::sum(c.packet.data() + 12, 8, 17 + c.datagram.length)) ==
               0xffff;
  }

  static inline void deliver(Context &c) noexcept
  {
    Endpoint source{c.datagram.source,
                    static_cast<unsigned short>(wire::read16(c.segment))};
    Endpoint destination{c.datagram.destination,
                         static_cast<unsigned short>(wire::read16(c.segment + 2))};
    c.handled = c.owner.sink_ &&
                c.owner.sink_->deliver(c.packet, source, destination,
                                       c.datagram.offset + 8, c.datagram.length - 8);
  }

  static inline bool handled(Context &c) noexcept
  {
    return c.handled;
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.owner.icmp_.unreachable(c.packet, c.datagram);
  }
  struct Length;
  struct Header;
  struct Checksum;
  struct Deliver;
  struct Unreachable;
  using Graph = xv::flow::graph<
      Length,
      xv::flow::types<
          xv::flow::node<Length, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&has_header>,
                                          xv::flow::go<Header>, xv::flow::stop>>,
          xv::flow::node<Header, xv::flow::action<&inspect>,
                         xv::flow::branch<xv::flow::condition<&valid_header>,
                                          xv::flow::go<Checksum>, xv::flow::stop>>,
          xv::flow::node<Checksum, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&valid_checksum>,
                                          xv::flow::go<Deliver>, xv::flow::stop>>,
          xv::flow::node<Deliver, xv::flow::action<&deliver>,
                         xv::flow::branch<xv::flow::condition<&handled>, xv::flow::stop,
                                          xv::flow::go<Unreachable>>>,
          xv::flow::node<Unreachable, xv::flow::action<&unreachable>, xv::flow::stop>>>;
};

struct UdpProtocol::SendFlow
{
  struct Context
  {
    UdpProtocol &owner;
    Endpoint source, destination;
    const void *data;
    unsigned length;
    Packet packet{};
    int result = 0;
  };

  static inline void select_source(Context &c) noexcept
  {
    c.source.address = c.owner.ip_.source_for(c.destination.address, c.source.address);
  }

  static inline bool has_source(Context &c) noexcept
  {
    return c.source.address != 0;
  }

  static inline bool fits(Context &c) noexcept
  {
    return c.length <= 4042;
  }

  static inline void allocate(Context &c) noexcept
  {
    c.packet.~Packet();
    new (&c.packet) Packet(c.owner.ip_.allocate(8 + c.length));
  }

  static inline bool allocated(Context &c) noexcept
  {
    return bool(c.packet);
  }

  static inline void encode(Context &c) noexcept
  {
    auto *p = c.packet.data() + 20;
    wire::write16(p, c.source.port);
    wire::write16(p + 2, c.destination.port);
    wire::write16(p + 4, 8 + c.length);
    wire::copy(p + 8, c.data, c.length);
  }

  static inline void checksum(Context &c) noexcept
  {
    unsigned char pseudo[8];
    wire::write32(pseudo, c.source.address);
    wire::write32(pseudo + 4, c.destination.address);
    auto *p = c.packet.data() + 20;
    auto checksum = static_cast<unsigned short>(
        ~wire::sum(p, 8 + c.length, wire::sum(pseudo, 8, 17 + 8 + c.length)));
    wire::write16(p + 6, checksum ? checksum : 0xffff);
  }

  static inline void send(Context &c) noexcept
  {
    c.result = c.owner.ip_.send(c.packet, c.destination.address, 17, c.source.address);
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }

  static inline void no_buffer(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }
  struct Source;
  struct Size;
  struct Allocate;
  struct Encode;
  struct Checksum;
  struct Send;
  struct Unreachable;
  struct TooLarge;
  struct NoBuffer;
  using Graph = xv::flow::graph<
      Source,
      xv::flow::types<
          xv::flow::node<
              Source, xv::flow::action<&select_source>,
              xv::flow::branch<xv::flow::condition<&has_source>, xv::flow::go<Size>,
                               xv::flow::go<Unreachable>>>,
          xv::flow::node<
              Size, xv::flow::noop,
              xv::flow::branch<xv::flow::condition<&fits>, xv::flow::go<Allocate>,
                               xv::flow::go<TooLarge>>>,
          xv::flow::node<
              Allocate, xv::flow::action<&allocate>,
              xv::flow::branch<xv::flow::condition<&allocated>, xv::flow::go<Encode>,
                               xv::flow::go<NoBuffer>>>,
          xv::flow::node<Encode, xv::flow::action<&encode>, xv::flow::go<Checksum>>,
          xv::flow::node<Checksum, xv::flow::action<&checksum>, xv::flow::go<Send>>,
          xv::flow::node<Send, xv::flow::action<&send>, xv::flow::stop>,
          xv::flow::node<Unreachable, xv::flow::action<&unreachable>, xv::flow::stop>,
          xv::flow::node<TooLarge, xv::flow::action<&too_large>, xv::flow::stop>,
          xv::flow::node<NoBuffer, xv::flow::action<&no_buffer>, xv::flow::stop>>>;
};
} // namespace xv::net
