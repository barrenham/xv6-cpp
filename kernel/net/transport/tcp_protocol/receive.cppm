module;
#include "kernel/support/flow.h"
module xv.net:tcp_receive_flow;
import :tcp;
import :wire;

namespace xv::net
{
struct TcpProtocol::ReceiveFlow
{
  struct Context
  {
    TcpProtocol &protocol;
    TcpSegmentSink *sink;
    Packet &packet;
    const IpDatagram &datagram;
    const unsigned char *segment = nullptr;
    unsigned header_length = 0;
    Endpoint local{}, peer{};
    bool handled = false;
    unsigned reset_sequence = 0, reset_acknowledgement = 0, reset_flags = 0;
  };

  struct HasMinimumHeader
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.datagram.length >= 20;
    }
  };

  struct InspectHeader
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.segment = ctx.packet.data() + ctx.datagram.offset;
      ctx.header_length = (ctx.segment[12] >> 4) * 4;
    }
  };

  struct HeaderLayoutValid
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.header_length >= 20 && ctx.header_length <= ctx.datagram.length &&
             !(ctx.segment[12] & 15);
    }
  };

  struct ChecksumValid
  {
    static inline bool test(Context &ctx) noexcept
    {
      return wire::sum(ctx.segment, ctx.datagram.length,
                       wire::sum(ctx.packet.data() + 12, 8, 6 + ctx.datagram.length)) ==
             0xffff;
    }
  };

  struct PortsValid
  {
    static inline bool test(Context &ctx) noexcept
    {
      return wire::read16(ctx.segment) && wire::read16(ctx.segment + 2);
    }
  };

  struct DeliverValidatedSegment
  {
    static inline void run(Context &ctx) noexcept
    {
      ctx.peer = {ctx.datagram.source,
                  static_cast<unsigned short>(wire::read16(ctx.segment))};
      ctx.local = {ctx.datagram.destination,
                   static_cast<unsigned short>(wire::read16(ctx.segment + 2))};
      ctx.handled =
          ctx.sink && ctx.sink->deliver_segment(
                          {ctx.peer, ctx.local, ctx.segment, ctx.datagram.length});
    }
  };

  struct WasHandled
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.handled;
    }
  };

  struct IsReset
  {
    static inline bool test(Context &ctx) noexcept
    {
      return ctx.segment[13] & 4;
    }
  };

  struct PrepareResetResponse
  {
    static inline void run(Context &ctx) noexcept
    {
      unsigned flags = ctx.segment[13];
      unsigned sequence_length = ctx.datagram.length - ctx.header_length +
                                 ((flags & 2) ? 1 : 0) + ((flags & 1) ? 1 : 0);
      ctx.reset_sequence = (flags & 16) ? wire::read32(ctx.segment + 8) : 0;
      ctx.reset_acknowledgement =
          (flags & 16) ? 0 : wire::read32(ctx.segment + 4) + sequence_length;
      ctx.reset_flags = (flags & 16) ? 4 : 20;
    }
  };

  struct SendResetResponse
  {
    static inline void run(Context &ctx) noexcept
    {
      // The sink has returned (and released its own locks). Reset failures remain
      // best effort, just as in the original protocol implementation.
      ctx.protocol.transmit(ctx.local, ctx.peer, ctx.reset_sequence,
                            ctx.reset_acknowledgement, ctx.reset_flags, 0, nullptr, 0);
    }
  };

  // IDs describe workflow positions; actions and predicates do not choose targets.
  struct CheckLength;
  struct CheckLayout;
  struct CheckChecksum;
  struct CheckPorts;
  struct Deliver;
  struct CheckReset;
  struct PrepareReset;
  struct SendReset;

  using Graph = xv::flow::graph<
      CheckLength,
      xv::flow::types<
          xv::flow::node<CheckLength, xv::flow::noop,
                         xv::flow::branch<HasMinimumHeader, xv::flow::go<CheckLayout>,
                                          xv::flow::stop>>,
          xv::flow::node<CheckLayout, InspectHeader,
                         xv::flow::branch<HeaderLayoutValid,
                                          xv::flow::go<CheckChecksum>, xv::flow::stop>>,
          xv::flow::node<CheckChecksum, xv::flow::noop,
                         xv::flow::branch<ChecksumValid, xv::flow::go<CheckPorts>,
                                          xv::flow::stop>>,
          xv::flow::node<
              CheckPorts, xv::flow::noop,
              xv::flow::branch<PortsValid, xv::flow::go<Deliver>, xv::flow::stop>>,
          xv::flow::node<
              Deliver, DeliverValidatedSegment,
              xv::flow::branch<WasHandled, xv::flow::stop, xv::flow::go<CheckReset>>>,
          xv::flow::node<
              CheckReset, xv::flow::noop,
              xv::flow::branch<IsReset, xv::flow::stop, xv::flow::go<PrepareReset>>>,
          xv::flow::node<PrepareReset, PrepareResetResponse, xv::flow::go<SendReset>>,
          xv::flow::node<SendReset, SendResetResponse, xv::flow::stop>>>;
};
} // namespace xv::net
