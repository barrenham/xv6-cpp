module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:ethernet_interface_flows;
import :ethernet_interface;

namespace xv::net
{
namespace flow = xv::flow;

struct EthernetInterface::TransmitFlow
{
  struct Context
  {
    EthernetInterface &owner;
    Packet &packet;
    unsigned next_hop;
    int result = 0;
  };

  static inline bool online(Context &c) noexcept
  {
    return !c.owner.master() && c.owner.up() && c.owner.carrier();
  }

  static inline bool configured(Context &c) noexcept
  {
    return c.owner.address_count() != 0;
  }

  static inline bool fits(Context &c) noexcept
  {
    return c.packet.size() <= c.owner.mtu();
  }

  static inline bool valid_bootstrap(Context &c) noexcept
  {
    return c.packet && c.packet.size() >= 28 && fits(c);
  }

  static inline bool broadcast(Context &c) noexcept
  {
    return c.next_hop == 0xffffffff;
  }

  static inline void send_broadcast(Context &c) noexcept
  {
    constexpr unsigned char broadcast[6] = {255, 255, 255, 255, 255, 255};
    c.result = c.owner.link_.send_ip(c.packet, broadcast);
  }

  static inline void resolve_neighbor(Context &c) noexcept
  {
    c.result = c.owner.arp_.send(c.packet, c.next_hop);
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
  struct Configured;
  struct Size;
  struct Destination;
  struct Broadcast;
  struct Neighbor;
  struct Unreachable;
  struct TooLarge;
  using Graph = flow::graph<
      Online,
      flow::types<flow::node<Online, flow::noop,
                             flow::branch<flow::condition<&online>,
                                          flow::go<Configured>, flow::go<Unreachable>>>,
                  flow::node<Configured, flow::noop,
                             flow::branch<flow::condition<&configured>, flow::go<Size>,
                                          flow::go<Unreachable>>>,
                  flow::node<Size, flow::noop,
                             flow::branch<flow::condition<&fits>, flow::go<Destination>,
                                          flow::go<TooLarge>>>,
                  flow::node<Destination, flow::noop,
                             flow::branch<flow::condition<&broadcast>,
                                          flow::go<Broadcast>, flow::go<Neighbor>>>,
                  flow::node<Broadcast, flow::action<&send_broadcast>, flow::stop>,
                  flow::node<Neighbor, flow::action<&resolve_neighbor>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<TooLarge, flow::action<&too_large>, flow::stop>>>;
  struct BootstrapOnline;
  struct BootstrapSize;
  struct BootstrapSend;
  struct BootstrapUnreachable;
  struct BootstrapTooLarge;
  using BootstrapGraph = flow::graph<
      BootstrapOnline,
      flow::types<
          flow::node<BootstrapOnline, flow::noop,
                     flow::branch<flow::condition<&online>, flow::go<BootstrapSize>,
                                  flow::go<BootstrapUnreachable>>>,
          flow::node<
              BootstrapSize, flow::noop,
              flow::branch<flow::condition<&valid_bootstrap>, flow::go<BootstrapSend>,
                           flow::go<BootstrapTooLarge>>>,
          flow::node<BootstrapSend, flow::action<&send_broadcast>, flow::stop>,
          flow::node<BootstrapUnreachable, flow::action<&unreachable>, flow::stop>,
          flow::node<BootstrapTooLarge, flow::action<&too_large>, flow::stop>>>;
};

} // namespace xv::net
