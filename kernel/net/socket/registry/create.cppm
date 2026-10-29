module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
module xv.net:registry_create_flow;
import :registry;
import :tcp_socket;
import :udp_socket;

namespace xv::net
{
namespace flow = xv::flow;

struct SocketRegistry::CreationFlow
{
  static_assert(sizeof(TcpSocket) <= 4096 && sizeof(UdpSocket) <= 4096);

  struct Context
  {
    SocketRegistry &owner;
    int type, protocol;
    void *page = nullptr;
    TcpSocket *tcp = nullptr;
    UdpSocket *udp = nullptr;
    Socket *result = nullptr;
  };

  static inline bool valid(Context &c) noexcept
  {
    return (c.type == 1 || c.type == 2) &&
           (!c.protocol || c.protocol == (c.type == 1 ? 6 : 17));
  }

  static inline bool tcp(Context &c) noexcept
  {
    return c.type == 1;
  }

  static inline void allocate(Context &c) noexcept
  {
    c.page = detail::allocate_page();
  }

  static inline bool allocated(Context &c) noexcept
  {
    return c.page != nullptr;
  }

  static inline void construct_tcp(Context &c) noexcept
  {
    c.tcp = new (c.page) TcpSocket(c.owner);
  }

  static inline bool initialize_tcp(Context &c) noexcept
  {
    return c.tcp->initialize();
  }

  static inline bool register_tcp(Context &c) noexcept
  {
    return c.owner.add(*c.tcp);
  }

  static inline void commit_tcp(Context &c) noexcept
  {
    c.result = c.tcp;
  }

  static inline void destroy_tcp(Context &c) noexcept
  {
    c.tcp->destroy();
  }

  static inline void construct_udp(Context &c) noexcept
  {
    c.udp = new (c.page) UdpSocket(c.owner);
  }

  static inline bool register_udp(Context &c) noexcept
  {
    return c.owner.add(*c.udp);
  }

  static inline void commit_udp(Context &c) noexcept
  {
    c.result = c.udp;
  }

  static inline void destroy_udp(Context &c) noexcept
  {
    c.udp->destroy();
  }
  struct TcpAllocate;
  struct TcpConstruct;
  struct TcpInitialize;
  struct TcpRegister;
  struct TcpCommit;
  struct TcpDestroy;
  struct UdpAllocate;
  struct UdpConstruct;
  struct UdpRegister;
  struct UdpCommit;
  struct UdpDestroy;
  struct Choose;
  using TcpNodes = flow::types<
      flow::node<TcpAllocate, flow::action<&allocate>,
                 flow::branch<flow::condition<&allocated>, flow::go<TcpConstruct>,
                              flow::stop>>,
      flow::node<TcpConstruct, flow::action<&construct_tcp>, flow::go<TcpInitialize>>,
      flow::node<TcpInitialize, flow::noop,
                 flow::branch<flow::condition<&initialize_tcp>, flow::go<TcpRegister>,
                              flow::go<TcpDestroy>>>,
      flow::node<TcpRegister, flow::noop,
                 flow::branch<flow::condition<&register_tcp>, flow::go<TcpCommit>,
                              flow::go<TcpDestroy>>>,
      flow::node<TcpCommit, flow::action<&commit_tcp>, flow::stop>,
      flow::node<TcpDestroy, flow::action<&destroy_tcp>, flow::stop>>;
  using UdpNodes = flow::types<
      flow::node<UdpAllocate, flow::action<&allocate>,
                 flow::branch<flow::condition<&allocated>, flow::go<UdpConstruct>,
                              flow::stop>>,
      flow::node<UdpConstruct, flow::action<&construct_udp>, flow::go<UdpRegister>>,
      flow::node<UdpRegister, flow::noop,
                 flow::branch<flow::condition<&register_udp>, flow::go<UdpCommit>,
                              flow::go<UdpDestroy>>>,
      flow::node<UdpCommit, flow::action<&commit_udp>, flow::stop>,
      flow::node<UdpDestroy, flow::action<&destroy_udp>, flow::stop>>;
  using TcpGraph = flow::graph<TcpAllocate, TcpNodes>;
  using LockedGraph = flow::graph<
      Choose, flow::concat<flow::types<flow::node<Choose, flow::noop,
                                                  flow::branch<flow::condition<&tcp>,
                                                               flow::go<TcpAllocate>,
                                                               flow::go<UdpAllocate>>>>,
                           TcpNodes, UdpNodes>>;

  static inline void locked(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    (void)LockedGraph::run<0>(c);
  }
  struct Version;
  struct Locked;
  using Graph =
      flow::graph<Version,
                  flow::types<flow::node<Version, flow::noop,
                                         flow::branch<flow::condition<&valid>,
                                                      flow::go<Locked>, flow::stop>>,
                              flow::node<Locked, flow::action<&locked>, flow::stop>>>;
};

} // namespace xv::net
