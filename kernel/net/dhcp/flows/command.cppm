module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:dhcp_command_flow;
import :dhcp;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct DhcpManager::CommandFlow
{
  struct Context
  {
    DhcpManager &owner;
    unsigned operation;
    netctl_request &request;
    NetworkInterface *interface = nullptr;
    DhcpManager::Client *client = nullptr;
    DhcpManager::Client stopping{};
    unsigned long now = 0;
    bool release = false, complete = false;
    int result = 0;
  };

  template <unsigned Operation> static bool operation(Context &c) noexcept
  {
    return c.operation == Operation;
  }

  static inline bool valid(Context &c) noexcept
  {
    return c.request.version == NETCTL_VERSION && c.request.size == sizeof(c.request);
  }

  static inline void lookup_interface(Context &c) noexcept
  {
    c.interface = c.owner.interfaces_.find(c.request.ifindex);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.interface != nullptr;
  }

  static inline void lookup_client(Context &c) noexcept
  {
    c.client = c.owner.find(*c.interface);
  }

  static inline void status(Context &c) noexcept
  {
    netctl_request out{};
    out.version = NETCTL_VERSION;
    out.size = sizeof(out);
    out.ifindex = c.interface->id();
    for (unsigned i = 0; i < sizeof(out.name) - 1 && c.interface->name()[i]; ++i)
    {
      out.name[i] = c.interface->name()[i];
    }
    if (c.client)
    {
      auto now = detail::milliseconds();
      auto &s = *c.client;
      out.flags = s.state;
      out.index = s.error;
      out.mtu = s.retries;
      out.address = s.lease.address;
      out.prefix = s.lease.prefix;
      out.gateway = s.lease.gateway;
      out.source = s.lease.dns;
      out.metric = s.lease.server;
      out.expires_ms = s.expires > now ? s.expires - now : 0;
      out.rx_packets = s.renewal > now ? s.renewal - now : 0;
      out.tx_packets = s.rebinding > now ? s.rebinding - now : 0;
    }
    c.request = out;
    c.result = 1;
    c.complete = true;
  }

  static inline void clock(Context &c) noexcept
  {
    c.now = detail::milliseconds();
  }

  static inline bool running(Context &c) noexcept
  {
    return c.client && c.client->state != DHCP_OFF && c.client->state != DHCP_FAILED;
  }

  static inline bool supported(Context &c) noexcept
  {
    return c.interface->mac() && c.interface->layer2() && c.interface->mtu() >= 604;
  }

  static inline bool ready(Context &c) noexcept
  {
    return !c.interface->master() && c.interface->up() && !c.interface->address_count();
  }

  static inline void find_slot(Context &c) noexcept
  {
    for (unsigned i = 0; i < 8 && !c.client; ++i)
    {
      if (!c.owner.clients_[i].interface)
      {
        c.client = &c.owner.clients_[i];
      }
    }
  }

  static inline bool has_slot(Context &c) noexcept
  {
    return c.client != nullptr;
  }

  static inline bool valid_mac(Context &c) noexcept
  {
    bool valid = false;
    for (auto byte : c.interface->mac()->bytes)
    {
      valid |= byte != 0;
    }
    return valid && !(c.interface->mac()->bytes[0] & 1);
  }

  static inline void start(Context &c) noexcept
  {
    c.client->interface = c.interface;
    c.client->mac = *c.interface->mac();
    c.client->error = 0;
    c.interface->set_dhcp_active(true);
    c.owner.select(*c.client, c.now);
    c.complete = true;
  }

  static inline bool renewable(Context &c) noexcept
  {
    return c.client &&
           (c.client->state == DHCP_BOUND || c.client->state == DHCP_RENEWING);
  }

  static inline void renew(Context &c) noexcept
  {
    if (c.client->state == DHCP_BOUND)
    {
      auto &s = *c.client;
      s.state = DHCP_RENEWING;
      s.xid = ++c.owner.counter_ ^ static_cast<unsigned>(c.now);
      ++s.serial;
      s.started = c.now;
      s.next = c.now;
      s.retries = 0;
    }
    c.complete = true;
  }

  static inline bool inactive(Context &c) noexcept
  {
    return !c.client || c.client->state == DHCP_OFF;
  }

  static inline void disable(Context &c) noexcept
  {
    c.interface->set_dhcp_active(false);
    c.complete = true;
  }

  static inline bool stopping(Context &c) noexcept
  {
    return c.client->state == DHCP_STOPPING;
  }

  static inline void mark_stopping(Context &c) noexcept
  {
    c.release = c.client->state == DHCP_BOUND || c.client->state == DHCP_RENEWING ||
                c.client->state == DHCP_REBINDING;
    c.client->state = DHCP_STOPPING;
    ++c.client->serial;
    c.stopping = *c.client;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
    c.complete = true;
  }

  static inline void no_device(Context &c) noexcept
  {
    c.result = -ENODEV;
    c.complete = true;
  }

  static inline void already(Context &c) noexcept
  {
    c.result = -EALREADY;
    c.complete = true;
  }

  static inline void unsupported(Context &c) noexcept
  {
    c.result = -EOPNOTSUPP;
    c.complete = true;
  }

  static inline void busy(Context &c) noexcept
  {
    c.result = -EBUSY;
    c.complete = true;
  }

  static inline void no_buffer(Context &c) noexcept
  {
    c.result = -ENOBUFS;
    c.complete = true;
  }

  static inline void not_connected(Context &c) noexcept
  {
    c.result = -ENOTCONN;
    c.complete = true;
  }
  struct Interface;
  struct LookupClient;
  struct Status;
  struct Clock;
  struct Running;
  struct Supported;
  struct Ready;
  struct FindSlot;
  struct Mac;
  struct Start;
  struct RenewOperation;
  struct Renewable;
  struct Renew;
  struct StopOperation;
  struct Inactive;
  struct Disable;
  struct Stopping;
  struct MarkStopping;
  struct Invalid;
  struct NoDevice;
  struct Already;
  struct Unsupported;
  struct Busy;
  struct NoBuffer;
  struct NotConnected;
  using ConfigurationGraph = flow::graph<
      Interface,
      flow::types<
          flow::node<Interface, flow::action<&lookup_interface>,
                     flow::branch<flow::condition<&found>, flow::go<LookupClient>,
                                  flow::go<NoDevice>>>,
          flow::node<LookupClient, flow::action<&lookup_client>,
                     flow::branch<flow::condition<&operation<NETCTL_DHCP_STATUS>>,
                                  flow::go<Status>, flow::go<Clock>>>,
          flow::node<Status, flow::action<&status>, flow::stop>,
          flow::node<Clock, flow::action<&clock>,
                     flow::branch<flow::condition<&operation<NETCTL_DHCP_START>>,
                                  flow::go<Running>, flow::go<RenewOperation>>>,
          flow::node<Running, flow::noop,
                     flow::branch<flow::condition<&running>, flow::go<Already>,
                                  flow::go<Supported>>>,
          flow::node<Supported, flow::noop,
                     flow::branch<flow::condition<&supported>, flow::go<Ready>,
                                  flow::go<Unsupported>>>,
          flow::node<Ready, flow::noop,
                     flow::branch<flow::condition<&ready>, flow::go<FindSlot>,
                                  flow::go<Busy>>>,
          flow::node<FindSlot, flow::action<&find_slot>,
                     flow::branch<flow::condition<&has_slot>, flow::go<Mac>,
                                  flow::go<NoBuffer>>>,
          flow::node<Mac, flow::noop,
                     flow::branch<flow::condition<&valid_mac>, flow::go<Start>,
                                  flow::go<Unsupported>>>,
          flow::node<Start, flow::action<&start>, flow::stop>,
          flow::node<RenewOperation, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_DHCP_RENEW>>,
                                  flow::go<Renewable>, flow::go<StopOperation>>>,
          flow::node<Renewable, flow::noop,
                     flow::branch<flow::condition<&renewable>, flow::go<Renew>,
                                  flow::go<NotConnected>>>,
          flow::node<Renew, flow::action<&renew>, flow::stop>,
          flow::node<StopOperation, flow::noop,
                     flow::branch<flow::condition<&operation<NETCTL_DHCP_STOP>>,
                                  flow::go<Inactive>, flow::go<Invalid>>>,
          flow::node<Inactive, flow::noop,
                     flow::branch<flow::condition<&inactive>, flow::go<Disable>,
                                  flow::go<Stopping>>>,
          flow::node<Disable, flow::action<&disable>, flow::stop>,
          flow::node<Stopping, flow::noop,
                     flow::branch<flow::condition<&stopping>, flow::go<Busy>,
                                  flow::go<MarkStopping>>>,
          flow::node<MarkStopping, flow::action<&mark_stopping>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<NoDevice, flow::action<&no_device>, flow::stop>,
          flow::node<Already, flow::action<&already>, flow::stop>,
          flow::node<Unsupported, flow::action<&unsupported>, flow::stop>,
          flow::node<Busy, flow::action<&busy>, flow::stop>,
          flow::node<NoBuffer, flow::action<&no_buffer>, flow::stop>,
          flow::node<NotConnected, flow::action<&not_connected>, flow::stop>>>;

  static inline void configure(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)ConfigurationGraph::run<0>(c);
  }

  static inline bool complete(Context &c) noexcept
  {
    return c.complete;
  }

  static inline bool release(Context &c) noexcept
  {
    return c.release;
  }

  static inline void send_release(Context &c) noexcept
  {
    c.owner.transmit(c.stopping, 7);
  }

  static inline void cleanup(Context &c) noexcept
  {
    ConfigurationGuard guard;
    auto *client = c.owner.find(*c.interface);
    if (client && client->serial == c.stopping.serial && client->state == DHCP_STOPPING)
    {
      c.owner.configuration_.remove(*c.interface);
      unsigned serial = client->serial + 1;
      *client = {};
      client->interface = c.interface;
      client->serial = serial;
      c.interface->set_dhcp_active(false);
    }
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.interface->invalidate();
  }
  struct Version;
  struct Configure;
  struct Release;
  struct SendRelease;
  struct Cleanup;
  struct Invalidate;
  struct BadVersion;
  using Graph = flow::graph<
      Version,
      flow::types<
          flow::node<Version, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<Configure>,
                                  flow::go<BadVersion>>>,
          flow::node<
              Configure, flow::action<&configure>,
              flow::branch<flow::condition<&complete>, flow::stop, flow::go<Release>>>,
          flow::node<Release, flow::noop,
                     flow::branch<flow::condition<&release>, flow::go<SendRelease>,
                                  flow::go<Cleanup>>>,
          flow::node<SendRelease, flow::action<&send_release>, flow::go<Cleanup>>,
          flow::node<Cleanup, flow::action<&cleanup>, flow::go<Invalidate>>,
          flow::node<Invalidate, flow::action<&invalidate>, flow::stop>,
          flow::node<BadVersion, flow::action<&invalid>, flow::stop>>>;
};

} // namespace xv::net
