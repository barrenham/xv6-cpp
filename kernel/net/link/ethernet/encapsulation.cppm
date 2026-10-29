module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:ethernet_encapsulation_flow;
import :ethernet;

namespace xv::net
{
namespace flow = xv::flow;

struct EthernetLayer::EncapsulationFlow
{
  struct Context
  {
    EthernetLayer &owner;
    Packet &packet;
    const unsigned char *destination;
    int result = 0;
  };

  static inline bool prepend(Context &c) noexcept
  {
    return c.packet.prepend(14);
  }

  static inline void send(Context &c) noexcept
  {
    c.result = c.owner.send(c.packet, c.destination, 0x800);
  }

  static inline bool failed(Context &c) noexcept
  {
    return c.result < 0;
  }

  static inline void restore(Context &c) noexcept
  {
    c.packet.remove_prefix(14);
  }

  static inline void too_large(Context &c) noexcept
  {
    c.result = -EMSGSIZE;
  }
  struct Prepend;
  struct Send;
  struct Restore;
  struct TooLarge;
  using Graph = flow::graph<
      Prepend, flow::types<flow::node<Prepend, flow::noop,
                                      flow::branch<flow::condition<&prepend>,
                                                   flow::go<Send>, flow::go<TooLarge>>>,
                           flow::node<Send, flow::action<&send>,
                                      flow::branch<flow::condition<&failed>,
                                                   flow::go<Restore>, flow::stop>>,
                           flow::node<Restore, flow::action<&restore>, flow::stop>,
                           flow::node<TooLarge, flow::action<&too_large>, flow::stop>>>;
};

} // namespace xv::net
