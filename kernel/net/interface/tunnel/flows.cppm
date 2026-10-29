module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tunnel_flows;
import :tunnel;

namespace xv::net
{
namespace flow = xv::flow;

struct IpInIpInterface::TransmitFlow
{
  struct Context
  {
    IpInIpInterface &owner;
    Packet &packet;
    int result = 0;
  };

  static inline bool online(Context &c) noexcept
  {
    return c.owner.up() && c.owner.carrier() && c.owner.ip_;
  }

  static inline bool fits(Context &c) noexcept
  {
    return c.packet.size() <= c.owner.mtu() && c.packet.size() >= 20;
  }

  static inline bool prepend(Context &c) noexcept
  {
    return c.packet.prepend(20);
  }

  static inline void send(Context &c) noexcept
  {
    c.result = c.owner.ip_->send(c.packet, c.owner.outer_remote_, 4,
                                 c.owner.outer_local_, c.owner.underlay_);
  }

  static inline bool failed(Context &c) noexcept
  {
    return c.result < 0;
  }

  static inline void restore(Context &c) noexcept
  {
    c.packet.remove_prefix(20);
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }
  struct Online;
  struct Size;
  struct Prepend;
  struct Send;
  struct Restore;
  struct Unreachable;
  struct TooLarge;
  using Graph = flow::graph<
      Online,
      flow::types<flow::node<Online, flow::noop,
                             flow::branch<flow::condition<&online>, flow::go<Size>,
                                          flow::go<Unreachable>>>,
                  flow::node<Size, flow::noop,
                             flow::branch<flow::condition<&fits>, flow::go<Prepend>,
                                          flow::go<TooLarge>>>,
                  flow::node<Prepend, flow::noop,
                             flow::branch<flow::condition<&prepend>, flow::go<Send>,
                                          flow::go<TooLarge>>>,
                  flow::node<Send, flow::action<&send>,
                             flow::branch<flow::condition<&failed>, flow::go<Restore>,
                                          flow::stop>>,
                  flow::node<Restore, flow::action<&restore>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<TooLarge, flow::action<&too_large>, flow::stop>>>;
};

struct IpInIpInterface::ReceiveFlow
{
  struct Context
  {
    IpInIpInterface &owner;
    Packet &packet;
    const IpDatagram &datagram;
    bool result = false;
  };

  static inline bool matches(Context &c) noexcept
  {
    return c.owner.up() && c.owner.ip_ && c.datagram.interface == c.owner.underlay_ &&
           c.datagram.source == c.owner.outer_remote_ &&
           c.datagram.destination == c.owner.outer_local_;
  }

  static inline void claim(Context &c) noexcept
  {
    c.result = true;
  }

  static inline bool may_decapsulate(Context &c) noexcept
  {
    return c.datagram.length >= 20 && c.packet.enter_tunnel();
  }

  static inline void trim(Context &c) noexcept
  {
    c.packet.resize(c.datagram.offset + c.datagram.length);
  }

  static inline bool remove_outer(Context &c) noexcept
  {
    return c.packet.remove_prefix(c.datagram.offset);
  }

  static inline void deliver(Context &c) noexcept
  {
    c.owner.deliver(c.packet);
  }
  struct Match;
  struct Claim;
  struct Trim;
  struct Deliver;
  using Graph =
      flow::graph<Match,
                  flow::types<flow::node<Match, flow::noop,
                                         flow::branch<flow::condition<&matches>,
                                                      flow::go<Claim>, flow::stop>>,
                              flow::node<Claim, flow::action<&claim>,
                                         flow::branch<flow::condition<&may_decapsulate>,
                                                      flow::go<Trim>, flow::stop>>,
                              flow::node<Trim, flow::action<&trim>,
                                         flow::branch<flow::condition<&remove_outer>,
                                                      flow::go<Deliver>, flow::stop>>,
                              flow::node<Deliver, flow::action<&deliver>, flow::stop>>>;
};

} // namespace xv::net
