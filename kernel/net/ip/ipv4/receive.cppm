module;
#include "kernel/support/flow.h"
module xv.net:ipv4_receive_flow;
import :ipv4;
import :platform;

namespace xv::net
{
struct Ipv4Layer::ReceiveFlow
{
  struct Context
  {
    Ipv4Layer &owner;
    NetworkInterface &interface;
    Packet &packet;
    const unsigned char *header = nullptr;
    unsigned header_length = 0, total_length = 0, source = 0, destination = 0;
    bool local_destination = false, bootstrap_candidate = false, handled = false;
    IpDatagram datagram{};
  };

  static inline bool eligible(Context &c) noexcept
  {
    return c.interface.up() &&
           c.owner.interfaces_.find(c.interface.id()) == &c.interface && c.packet &&
           c.packet.size() >= 20;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.header = c.packet.data();
    c.header_length = (c.header[0] & 15) * 4;
    c.total_length = wire::read16(c.header + 2);
    c.source = wire::read32(c.header + 12);
    c.destination = wire::read32(c.header + 16);
  }

  static inline bool valid_header(Context &c) noexcept
  {
    return (c.header[0] >> 4) == 4 && c.header_length >= 20 &&
           c.total_length >= c.header_length && c.total_length <= c.packet.size() &&
           c.header[8] && !(wire::read16(c.header + 6) & 0x8000) &&
           wire::sum(c.header, c.header_length) == 0xffff && c.source &&
           (c.source >> 28) < 14;
  }

  static inline bool fragmented(Context &c) noexcept
  {
    return wire::read16(c.header + 6) & 0x3fff;
  }

  static inline bool accepts_fragment(Context &c) noexcept
  {
    return c.interface.accepts_destination(c.destination, c.owner.interfaces_);
  }

  static inline void reassemble(Context &c) noexcept
  {
    auto complete = c.owner.reassembly_.accept(&c.interface, c.packet, c.header_length,
                                               c.total_length, detail::milliseconds());
    if (complete)
    {
      // Re-enter after accept has released its lock, exactly once per completed
      // datagram. Fragment depth is bounded by the wire format, not a graph loop.
      c.owner.receive(c.interface, complete);
    }
  }

  static inline void classify_destination(Context &c) noexcept
  {
    c.local_destination =
        c.interface.accepts_destination(c.destination, c.owner.interfaces_);
    c.bootstrap_candidate = c.owner.bootstrap_ && c.interface.dhcp_active() &&
                            c.header[9] == 17 &&
                            c.total_length - c.header_length >= 8 &&
                            wire::read16(c.header + c.header_length) == 67 &&
                            wire::read16(c.header + c.header_length + 2) == 68;
  }

  static inline bool accepted_destination(Context &c) noexcept
  {
    return c.local_destination || c.bootstrap_candidate;
  }

  static inline bool input_allowed(Context &c) noexcept
  {
    return !c.owner.filter_ ||
           c.owner.filter_->accept(FilterHook::Input, c.header, c.total_length,
                                   c.interface.id());
  }

  static inline void describe_datagram(Context &c) noexcept
  {
    c.datagram = {c.source,
                  c.destination,
                  c.header[9],
                  c.header_length,
                  c.total_length - c.header_length,
                  &c.interface};
  }

  static inline void offer_bootstrap(Context &c) noexcept
  {
    c.handled =
        c.datagram.protocol == 17 && c.owner.bootstrap_ &&
        c.owner.bootstrap_->receive_bootstrap(c.interface, c.packet, c.datagram);
  }

  static inline bool was_handled(Context &c) noexcept
  {
    return c.handled;
  }

  static inline bool local(Context &c) noexcept
  {
    return c.local_destination;
  }

  static inline void dispatch(Context &c) noexcept
  {
    for (auto &binding : c.owner.handlers_)
    {
      if (binding.handler && binding.protocol == c.datagram.protocol)
      {
        binding.handler->receive(c.packet, c.datagram);
        return;
      }
    }
  }

  static inline void drop(Context &c) noexcept
  {
    c.interface.receive_drop();
  }

  struct Eligibility;
  struct Header;
  struct Fragment;
  struct FragmentDestination;
  struct Reassemble;
  struct Destination;
  struct Filter;
  struct Datagram;
  struct Bootstrap;
  struct Local;
  struct Dispatch;
  struct Drop;
  using Graph = xv::flow::graph<
      Eligibility,
      xv::flow::types<
          xv::flow::node<Eligibility, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&eligible>,
                                          xv::flow::go<Header>, xv::flow::go<Drop>>>,
          xv::flow::node<Header, xv::flow::action<&inspect>,
                         xv::flow::branch<xv::flow::condition<&valid_header>,
                                          xv::flow::go<Fragment>, xv::flow::go<Drop>>>,
          xv::flow::node<Fragment, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&fragmented>,
                                          xv::flow::go<FragmentDestination>,
                                          xv::flow::go<Destination>>>,
          xv::flow::node<
              FragmentDestination, xv::flow::noop,
              xv::flow::branch<xv::flow::condition<&accepts_fragment>,
                               xv::flow::go<Reassemble>, xv::flow::go<Drop>>>,
          xv::flow::node<Reassemble, xv::flow::action<&reassemble>, xv::flow::stop>,
          xv::flow::node<Destination, xv::flow::action<&classify_destination>,
                         xv::flow::branch<xv::flow::condition<&accepted_destination>,
                                          xv::flow::go<Filter>, xv::flow::go<Drop>>>,
          xv::flow::node<Filter, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&input_allowed>,
                                          xv::flow::go<Datagram>, xv::flow::go<Drop>>>,
          xv::flow::node<Datagram, xv::flow::action<&describe_datagram>,
                         xv::flow::go<Bootstrap>>,
          xv::flow::node<Bootstrap, xv::flow::action<&offer_bootstrap>,
                         xv::flow::branch<xv::flow::condition<&was_handled>,
                                          xv::flow::stop, xv::flow::go<Local>>>,
          xv::flow::node<Local, xv::flow::noop,
                         xv::flow::branch<xv::flow::condition<&local>,
                                          xv::flow::go<Dispatch>, xv::flow::go<Drop>>>,
          xv::flow::node<Dispatch, xv::flow::action<&dispatch>, xv::flow::stop>,
          xv::flow::node<Drop, xv::flow::action<&drop>, xv::flow::stop>>>;
};
} // namespace xv::net
