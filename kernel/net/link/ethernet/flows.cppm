module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:ethernet_flows;
import :ethernet;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct EthernetLayer::ReceiveFlow
{
  struct Context
  {
    EthernetLayer &owner;
    Packet &packet;
    unsigned index = 0;
  };

  static inline bool valid_size(Context &c) noexcept
  {
    return c.packet.size() >= 14 && c.packet.size() <= c.owner.mtu_ + 14;
  }

  static inline bool dispatched(Context &c) noexcept
  {
    return c.owner.dispatch_frame(c.packet);
  }

  static inline bool addressed(Context &c) noexcept
  {
    constexpr unsigned char broadcast[6] = {255, 255, 255, 255, 255, 255};
    return wire::equal(c.packet.data(), c.owner.mac_.bytes, 6) ||
           wire::equal(c.packet.data(), broadcast, 6);
  }

  static inline bool more_bindings(Context &c) noexcept
  {
    return c.index < sizeof(c.owner.bindings_) / sizeof(c.owner.bindings_[0]);
  }

  static inline bool matches(Context &c) noexcept
  {
    const auto &binding = c.owner.bindings_[c.index];
    return binding.protocol && binding.type == wire::read16(c.packet.data() + 12);
  }

  static inline void deliver(Context &c) noexcept
  {
    c.owner.bindings_[c.index].protocol->receive(c.packet);
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct Size;
  struct Bridge;
  struct Address;
  struct Scan;
  struct Match;
  struct Deliver;
  struct Advance;
  using Graph = flow::graph<
      Size, flow::types<flow::node<Size, flow::noop,
                                   flow::branch<flow::condition<&valid_size>,
                                                flow::go<Bridge>, flow::stop>>,
                        flow::node<Bridge, flow::noop,
                                   flow::branch<flow::condition<&dispatched>,
                                                flow::stop, flow::go<Address>>>,
                        flow::node<Address, flow::noop,
                                   flow::branch<flow::condition<&addressed>,
                                                flow::go<Scan>, flow::stop>>,
                        flow::node<Scan, flow::noop,
                                   flow::branch<flow::condition<&more_bindings>,
                                                flow::go<Match>, flow::stop>>,
                        flow::node<Match, flow::noop,
                                   flow::branch<flow::condition<&matches>,
                                                flow::go<Deliver>, flow::go<Advance>>>,
                        flow::node<Deliver, flow::action<&deliver>, flow::stop>,
                        flow::node<Advance, flow::action<&advance>, flow::go<Scan>>>>;
};

struct EthernetLayer::SendFlow
{
  struct Context
  {
    EthernetLayer &owner;
    Packet &packet;
    int result = 0;
  };

  static inline bool valid_size(Context &c) noexcept
  {
    return c.packet && c.packet.size() >= 14 && c.packet.size() <= c.owner.mtu_ + 14;
  }

  static inline bool online(Context &c) noexcept
  {
    return c.owner.online() && c.owner.carrier();
  }

  static inline bool accepted(Context &c) noexcept
  {
    return c.owner.device_->transmit(c.packet.data(), c.packet.size());
  }

  static inline void transfer(Context &c) noexcept
  {
    c.packet.release();
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }
  struct Size;
  struct Online;
  struct Device;
  struct Transfer;
  struct TooLarge;
  struct Unreachable;
  struct Busy;
  using Graph = flow::graph<
      Size,
      flow::types<flow::node<Size, flow::noop,
                             flow::branch<flow::condition<&valid_size>,
                                          flow::go<Online>, flow::go<TooLarge>>>,
                  flow::node<Online, flow::noop,
                             flow::branch<flow::condition<&online>, flow::go<Device>,
                                          flow::go<Unreachable>>>,
                  flow::node<Device, flow::noop,
                             flow::branch<flow::condition<&accepted>,
                                          flow::go<Transfer>, flow::go<Busy>>>,
                  flow::node<Transfer, flow::action<&transfer>, flow::stop>,
                  flow::node<TooLarge, flow::action<&too_large>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<Busy, flow::action<&busy>, flow::stop>>>;
};

} // namespace xv::net
