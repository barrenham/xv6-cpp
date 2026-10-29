module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/support/placement_new.hpp"
module xv.net:tcp_transmit_flow;
import :tcp;
import :wire;

namespace xv::net
{
struct TcpProtocol::TransmitFlow
{
  struct Context
  {
    Ipv4Layer &ip;
    Endpoint local, peer;
    unsigned sequence, acknowledgement, flags, window;
    const void *data;
    unsigned length;
    unsigned mtu = 0, header_length = 0;
    Packet packet{};
    int result = 0;
  };

  struct SelectRoute
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.mtu = ctx.ip.mtu_for(ctx.peer.address, ctx.local.address);
      ctx.header_length = (ctx.flags & 2) ? 24 : 20;
    }
  };

  struct RouteUsable
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.mtu >= 68;
    }
  };

  struct ReportUnreachable
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.result = -ENETUNREACH;
    }
  };

  struct AllocateSegment
  {
    static inline void run(Context &ctx) noexcept
    {
      // Packet is move-constructible but has no assignment operator. Reconstruct
      // the initially empty owner in place; placement new does not allocate.
      ctx.packet.~Packet();
      new (&ctx.packet) Packet(ctx.ip.allocate(ctx.header_length + ctx.length));
    }
  };

  struct HasPacket
  {
    static inline bool test(Context &ctx) noexcept
    {
      return bool(ctx.packet);
    }
  };

  struct ReportNoBuffer
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.result = -ENOBUFS;
    }
  };

  struct EncodeFixedHeader
  {
    static inline void run(Context &ctx) noexcept
    {
      auto *segment = ctx.packet.data() + 20;
      wire::write16(segment, ctx.local.port);
      wire::write16(segment + 2, ctx.peer.port);
      wire::write32(segment + 4, ctx.sequence);
      wire::write32(segment + 8, ctx.acknowledgement);
      segment[12] = (ctx.header_length / 4) << 4;
      segment[13] = ctx.flags;
      wire::write16(segment + 14, ctx.window);
    }
  };

  struct HasSyn
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.flags & 2;
    }
  };

  struct EncodeMssOption
  {
    static inline void run(Context &ctx) noexcept
    {
      auto *segment = ctx.packet.data() + 20;
      segment[20] = 2;
      segment[21] = 4;
      wire::write16(segment + 22, ctx.mtu - 40);
    }
  };

  struct HasPayload
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.length != 0;
    }
  };

  struct CopyPayload
  {
    static inline void run(Context &ctx) noexcept
    {
      wire::copy(ctx.packet.data() + 20 + ctx.header_length, ctx.data, ctx.length);
    }
  };

  struct WriteChecksum
  {
    static inline void run(Context &ctx) noexcept
    {
      unsigned char pseudoheader[8];
      wire::write32(pseudoheader, ctx.local.address);
      wire::write32(pseudoheader + 4, ctx.peer.address);
      unsigned length = ctx.header_length + ctx.length;
      auto *segment = ctx.packet.data() + 20;
      wire::write16(segment + 16, ~wire::sum(segment, length,
                                             wire::sum(pseudoheader, 8, 6 + length)));
    }
  };

  struct SendPacket
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.result = ctx.ip.send(ctx.packet, ctx.peer.address, 6, ctx.local.address);
    }
  };

  struct Route;
  struct Unreachable;
  struct Allocate;
  struct NoBuffer;
  struct Header;
  struct Mss;
  struct Payload;
  struct Copy;
  struct Checksum;
  struct Send;

  using Graph = xv::flow::graph<
      Route,
      xv::flow::types<
          xv::flow::node<Route, SelectRoute,
                         xv::flow::branch<RouteUsable, xv::flow::go<Allocate>,
                                          xv::flow::go<Unreachable>>>,
          xv::flow::node<Unreachable, ReportUnreachable, xv::flow::stop>,
          xv::flow::node<Allocate, AllocateSegment,
                         xv::flow::branch<HasPacket, xv::flow::go<Header>,
                                          xv::flow::go<NoBuffer>>>,
          xv::flow::node<NoBuffer, ReportNoBuffer, xv::flow::stop>,
          xv::flow::node<
              Header, EncodeFixedHeader,
              xv::flow::branch<HasSyn, xv::flow::go<Mss>, xv::flow::go<Payload>>>,
          xv::flow::node<Mss, EncodeMssOption, xv::flow::go<Payload>>,
          xv::flow::node<
              Payload, xv::flow::noop,
              xv::flow::branch<HasPayload, xv::flow::go<Copy>, xv::flow::go<Checksum>>>,
          xv::flow::node<Copy, CopyPayload, xv::flow::go<Checksum>>,
          xv::flow::node<Checksum, WriteChecksum, xv::flow::go<Send>>,
          xv::flow::node<Send, SendPacket, xv::flow::stop>>>;
};
} // namespace xv::net
