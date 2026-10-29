module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
module xv.net:icmp_reply_flow;
import :icmp;

namespace xv::net
{
struct IcmpProtocol::ReplyFlow
{
  struct Context
  {
    IcmpProtocol &owner;
    Packet &request;
    const IpDatagram &datagram;
    Packet reply{};
    unsigned quoted = 0;
  };

  static inline bool valid_echo(Context &c) noexcept
  {
    if (c.datagram.length < 8)
    {
      return false;
    }
    auto *p = c.request.data() + c.datagram.offset;
    return wire::sum(p, c.datagram.length) == 0xffff && p[0] == 8 && !p[1] &&
           c.owner.ip_.local(c.datagram.destination);
  }

  static inline bool local(Context &c) noexcept
  {
    return c.owner.ip_.local(c.datagram.destination);
  }

  static inline void allocate_echo(Context &c) noexcept
  {
    c.reply.~Packet();
    new (&c.reply) Packet(c.owner.ip_.allocate(c.datagram.length));
  }

  static inline void allocate_unreachable(Context &c) noexcept
  {
    c.quoted = c.datagram.offset + (c.datagram.length < 8 ? c.datagram.length : 8);
    c.reply.~Packet();
    new (&c.reply) Packet(c.owner.ip_.allocate(8 + c.quoted));
  }

  static inline bool allocated(Context &c) noexcept
  {
    return bool(c.reply);
  }

  static inline void encode_echo(Context &c) noexcept
  {
    auto *out = c.reply.data() + 20;
    wire::copy(out, c.request.data() + c.datagram.offset, c.datagram.length);
    out[0] = 0;
    wire::write16(out + 2, 0);
    wire::write16(out + 2, ~wire::sum(out, c.datagram.length));
  }

  static inline void encode_unreachable(Context &c) noexcept
  {
    auto *p = c.reply.data() + 20;
    p[0] = 3;
    p[1] = 3;
    wire::copy(p + 8, c.request.data(), c.quoted);
    wire::write16(p + 2, ~wire::sum(p, 8 + c.quoted));
  }

  static inline void send(Context &c) noexcept
  {
    c.owner.ip_.send(c.reply, c.datagram.source, 1, c.datagram.destination);
  }
  struct Validate;
  struct Allocate;
  struct Encode;
  struct Send;
  using SendNode = xv::flow::node<Send, xv::flow::action<&send>, xv::flow::stop>;
  using EchoGraph = xv::flow::graph<
      Validate,
      xv::flow::types<
          xv::flow::node<Validate, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&valid_echo>,
                                          xv::flow::go<Allocate>, xv::flow::stop>>,
          xv::flow::node<Allocate, xv::flow::action<&allocate_echo>,
                         xv::flow::branch<xv::flow::condition<&allocated>,
                                          xv::flow::go<Encode>, xv::flow::stop>>,
          xv::flow::node<Encode, xv::flow::action<&encode_echo>, xv::flow::go<Send>>,
          SendNode>>;
  using UnreachableGraph = xv::flow::graph<
      Validate,
      xv::flow::types<
          xv::flow::node<Validate, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&local>,
                                          xv::flow::go<Allocate>, xv::flow::stop>>,
          xv::flow::node<Allocate, xv::flow::action<&allocate_unreachable>,
                         xv::flow::branch<xv::flow::condition<&allocated>,
                                          xv::flow::go<Encode>, xv::flow::stop>>,
          xv::flow::node<Encode, xv::flow::action<&encode_unreachable>,
                         xv::flow::go<Send>>,
          SendNode>>;
};
} // namespace xv::net
