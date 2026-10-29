module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tunnel_protocol_flow;
import :tunnel;

namespace xv::net
{
namespace flow = xv::flow;

struct IpInIpProtocol::DispatchFlow
{
  struct AddContext
  {
    IpInIpProtocol &owner;
    IpInIpInterface &interface;
    unsigned index = 0;
    bool result = false;
  };

  struct ReceiveContext
  {
    IpInIpProtocol &owner;
    Packet &packet;
    const IpDatagram &datagram;
    unsigned index = 0;
  };

  static inline bool more_add(AddContext &c) noexcept
  {
    return c.index < 8;
  }

  static inline bool duplicate(AddContext &c) noexcept
  {
    return c.owner.tunnels_[c.index] == &c.interface;
  }

  static inline void advance_add(AddContext &c) noexcept
  {
    ++c.index;
  }

  static inline void reset(AddContext &c) noexcept
  {
    c.index = 0;
  }

  static inline bool empty(AddContext &c) noexcept
  {
    return c.owner.tunnels_[c.index] == nullptr;
  }

  static inline void install(AddContext &c) noexcept
  {
    c.owner.tunnels_[c.index] = &c.interface;
    c.result = true;
  }
  struct Duplicates;
  struct Duplicate;
  struct DuplicateAdvance;
  struct Reset;
  struct Slots;
  struct Empty;
  struct Install;
  struct SlotAdvance;
  using AddGraph = flow::graph<
      Duplicates,
      flow::types<
          flow::node<Duplicates, flow::noop,
                     flow::branch<flow::condition<&more_add>, flow::go<Duplicate>,
                                  flow::go<Reset>>>,
          flow::node<Duplicate, flow::noop,
                     flow::branch<flow::condition<&duplicate>, flow::stop,
                                  flow::go<DuplicateAdvance>>>,
          flow::node<DuplicateAdvance, flow::action<&advance_add>,
                     flow::go<Duplicates>>,
          flow::node<Reset, flow::action<&reset>, flow::go<Slots>>,
          flow::node<Slots, flow::noop,
                     flow::branch<flow::condition<&more_add>, flow::go<Empty>,
                                  flow::stop>>,
          flow::node<Empty, flow::noop,
                     flow::branch<flow::condition<&empty>, flow::go<Install>,
                                  flow::go<SlotAdvance>>>,
          flow::node<Install, flow::action<&install>, flow::stop>,
          flow::node<SlotAdvance, flow::action<&advance_add>, flow::go<Slots>>>>;

  static inline bool more_receive(ReceiveContext &c) noexcept
  {
    return c.index < 8;
  }

  static inline bool deliver(ReceiveContext &c) noexcept
  {
    auto *tunnel = c.owner.tunnels_[c.index];
    return tunnel && tunnel->receive_outer(c.packet, c.datagram);
  }

  static inline void advance_receive(ReceiveContext &c) noexcept
  {
    ++c.index;
  }
  struct ReceiveScan;
  struct Deliver;
  struct ReceiveAdvance;
  using ReceiveGraph = flow::graph<
      ReceiveScan,
      flow::types<flow::node<ReceiveScan, flow::noop,
                             flow::branch<flow::condition<&more_receive>,
                                          flow::go<Deliver>, flow::stop>>,
                  flow::node<Deliver, flow::noop,
                             flow::branch<flow::condition<&deliver>, flow::stop,
                                          flow::go<ReceiveAdvance>>>,
                  flow::node<ReceiveAdvance, flow::action<&advance_receive>,
                             flow::go<ReceiveScan>>>>;
};

} // namespace xv::net
