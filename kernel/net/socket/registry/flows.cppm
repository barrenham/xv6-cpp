module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:registry_flows;
import :registry;
import :tcp_socket;
import :udp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct SocketRegistry::DeliveryFlow
{
  struct Context
  {
    SocketRegistry &owner;
    const TcpSegment &segment;
    unsigned index = 0;
    TcpSocket *socket = nullptr, *listener = nullptr, *child = nullptr;
    bool result = false;
  };

  static inline bool more(Context &c) noexcept
  {
    return c.index < Capacity;
  }

  static inline void inspect(Context &c) noexcept
  {
    auto *socket = c.owner.sockets_[c.index];
    c.socket =
        socket && socket->protocol() == 6 ? static_cast<TcpSocket *>(socket) : nullptr;
  }

  static inline bool tcp(Context &c) noexcept
  {
    return c.socket != nullptr;
  }

  static inline bool matches(Context &c) noexcept
  {
    return c.socket->matches(c.segment.destination, c.segment.source);
  }

  static inline void deliver(Context &c) noexcept
  {
    c.socket->incoming(c.segment.source, c.segment.destination, c.segment.data,
                       c.segment.length);
    c.result = true;
  }

  static inline void remember_listener(Context &c) noexcept
  {
    if (c.socket->listening(c.segment.destination))
    {
      c.listener = c.socket;
    }
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool listen_syn(Context &c) noexcept
  {
    return c.listener && (c.segment.data[13] & 0x17) == 2;
  }

  static inline void claim(Context &c) noexcept
  {
    c.result = true;
  }

  static inline bool backlog_space(Context &c) noexcept
  {
    return c.owner.child_count(*c.listener) < c.listener->backlog_;
  }

  static inline void allocate(Context &c) noexcept
  {
    c.child = c.owner.create_tcp();
  }

  static inline bool allocated(Context &c) noexcept
  {
    return c.child != nullptr;
  }

  static inline void passive(Context &c) noexcept
  {
    c.child->passive(*c.listener, c.segment.destination, c.segment.source,
                     c.segment.data, c.segment.length);
  }
  struct Scan;
  struct Inspect;
  struct Match;
  struct Deliver;
  struct Remember;
  struct Advance;
  struct ListenSyn;
  struct Claim;
  struct Allocate;
  struct Passive;
  using Graph = flow::graph<
      Scan,
      flow::types<
          flow::node<Scan, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<Inspect>,
                                  flow::go<ListenSyn>>>,
          flow::node<
              Inspect, flow::action<&inspect>,
              flow::branch<flow::condition<&tcp>, flow::go<Match>, flow::go<Advance>>>,
          flow::node<Match, flow::noop,
                     flow::branch<flow::condition<&matches>, flow::go<Deliver>,
                                  flow::go<Remember>>>,
          flow::node<Deliver, flow::action<&deliver>, flow::stop>,
          flow::node<Remember, flow::action<&remember_listener>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
          flow::node<
              ListenSyn, flow::noop,
              flow::branch<flow::condition<&listen_syn>, flow::go<Claim>, flow::stop>>,
          flow::node<Claim, flow::action<&claim>,
                     flow::branch<flow::condition<&backlog_space>, flow::go<Allocate>,
                                  flow::stop>>,
          flow::node<
              Allocate, flow::action<&allocate>,
              flow::branch<flow::condition<&allocated>, flow::go<Passive>, flow::stop>>,
          flow::node<Passive, flow::action<&passive>, flow::stop>>>;
};

struct InetSocket::BindFlow
{
  struct Context
  {
    InetSocket &owner;
    Endpoint local;
    unsigned destination = 0, source = 0;
    int result = 0;
  };

  static inline bool valid_bind(Context &c) noexcept
  {
    return !c.owner.local_.port &&
           (!c.local.address || c.owner.registry_.ip().local(c.local.address));
  }

  static inline void choose_port(Context &c) noexcept
  {
    if (!c.local.port)
    {
      c.local.port = c.owner.registry_.ephemeral(c.owner.protocol_);
    }
  }

  static inline bool available(Context &c) noexcept
  {
    return c.local.port && !c.owner.registry_.port_used(c.owner.protocol_, c.local.port,
                                                        &c.owner, c.local.address);
  }

  static inline void bind(Context &c) noexcept
  {
    c.owner.local_ = c.local;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -1;
  }

  static inline void used(Context &c) noexcept
  {
    c.result = -EADDRINUSE;
  }

  static inline void select_source(Context &c) noexcept
  {
    c.source = c.owner.registry_.ip().source_for(c.destination, c.owner.local_.address);
  }

  static inline bool routed(Context &c) noexcept
  {
    return c.source != 0;
  }

  static inline void auto_port(Context &c) noexcept
  {
    if (!c.owner.local_.port)
    {
      c.owner.local_.port = c.owner.registry_.ephemeral(c.owner.protocol_);
    }
  }

  static inline bool has_port(Context &c) noexcept
  {
    return c.owner.local_.port != 0;
  }

  static inline void assign_source(Context &c) noexcept
  {
    if (c.owner.protocol_ == 6)
    {
      c.owner.local_.address = c.source;
    }
  }

  static inline void unreachable(Context &c) noexcept
  {
    c.result = -ENETUNREACH;
  }
  struct Validate;
  struct Port;
  struct Bind;
  struct Invalid;
  struct Used;
  using Graph = flow::graph<
      Validate, flow::types<flow::node<Validate, flow::noop,
                                       flow::branch<flow::condition<&valid_bind>,
                                                    flow::go<Port>, flow::go<Invalid>>>,
                            flow::node<Port, flow::action<&choose_port>,
                                       flow::branch<flow::condition<&available>,
                                                    flow::go<Bind>, flow::go<Used>>>,
                            flow::node<Bind, flow::action<&bind>, flow::stop>,
                            flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                            flow::node<Used, flow::action<&used>, flow::stop>>>;
  struct Source;
  struct AutoPort;
  struct AssignSource;
  struct Unreachable;
  struct AutoUsed;
  using AutoGraph = flow::graph<
      Source,
      flow::types<flow::node<Source, flow::action<&select_source>,
                             flow::branch<flow::condition<&routed>, flow::go<AutoPort>,
                                          flow::go<Unreachable>>>,
                  flow::node<AutoPort, flow::action<&auto_port>,
                             flow::branch<flow::condition<&has_port>,
                                          flow::go<AssignSource>, flow::go<AutoUsed>>>,
                  flow::node<AssignSource, flow::action<&assign_source>, flow::stop>,
                  flow::node<Unreachable, flow::action<&unreachable>, flow::stop>,
                  flow::node<AutoUsed, flow::action<&used>, flow::stop>>>;
};

} // namespace xv::net
