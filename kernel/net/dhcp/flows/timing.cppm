module;
#include "kernel/support/flow.h"
#include "kernel/support/placement_new.hpp"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:dhcp_timing_flow;
import :dhcp;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct DhcpManager::TickFlow
{
  struct Context
  {
    DhcpManager &owner;
    unsigned long now;
    unsigned index = 0;
    Client snapshot{};
    bool send = false, changed = false;
  };

  static inline bool eligible(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return s.interface && s.state != DHCP_OFF && s.state != DHCP_STOPPING;
  }

  static inline bool expired(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return (s.state == DHCP_BOUND || s.state == DHCP_RENEWING ||
            s.state == DHCP_REBINDING) &&
           c.now >= s.expires;
  }

  static inline void expire(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    c.changed = c.owner.configuration_.remove(*s.interface);
    s.error = ETIMEDOUT;
    c.owner.select(s, c.now);
  }

  static inline bool retry_failed(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return s.state == DHCP_FAILED && c.now >= s.next;
  }

  static inline void select(Context &c) noexcept
  {
    c.owner.select(c.owner.clients_[c.index], c.now);
  }

  static inline bool renewal_due(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return s.state == DHCP_BOUND && s.renewal != ~0UL && c.now >= s.renewal;
  }

  static inline void renew(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    s.state = DHCP_RENEWING;
    s.xid = ++c.owner.counter_ ^ static_cast<unsigned>(c.now);
    ++s.serial;
    s.started = c.now;
    s.next = c.now;
    s.retries = 0;
  }

  static inline bool rebinding_due(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return s.state == DHCP_RENEWING && c.now >= s.rebinding;
  }

  static inline void rebind(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    s.state = DHCP_REBINDING;
    ++s.serial;
    s.next = c.now;
    s.retries = 0;
  }

  static inline bool transmission_due(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return s.state != DHCP_BOUND && s.state != DHCP_FAILED && c.now >= s.next &&
           s.interface->up() && s.interface->carrier() && !s.interface->master();
  }

  static inline bool exhausted(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    return (s.state == DHCP_SELECTING || s.state == DHCP_REQUESTING) && s.retries >= 6;
  }

  static inline void fail(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    s.state = DHCP_FAILED;
    s.error = ETIMEDOUT;
    s.next = c.now + 60000;
  }

  static inline void schedule(Context &c) noexcept
  {
    auto &s = c.owner.clients_[c.index];
    unsigned long delay;
    if (s.state == DHCP_RENEWING || s.state == DHCP_REBINDING)
    {
      auto deadline = s.state == DHCP_RENEWING ? s.rebinding : s.expires;
      delay = (deadline - c.now) / 2;
      if (delay < 250)
      {
        delay = 250;
      }
      if (delay > 60000)
      {
        delay = 60000;
      }
    }
    else
    {
      unsigned n = s.retries < 4 ? s.retries : 4;
      delay = (1000U << n) + (s.xid & 255);
    }
    ++s.retries;
    s.next = c.now + delay;
    c.send = true;
  }

  static inline void snapshot(Context &c) noexcept
  {
    c.snapshot = c.owner.clients_[c.index];
  }
  struct Eligible;
  struct Expiry;
  struct Expire;
  struct Failed;
  struct Select;
  struct Renewal;
  struct Renew;
  struct Rebinding;
  struct Rebind;
  struct Due;
  struct Retries;
  struct Fail;
  struct Schedule;
  struct Snapshot;
  using ClientGraph = flow::graph<
      Eligible,
      flow::types<flow::node<Eligible, flow::noop,
                             flow::branch<flow::condition<&eligible>, flow::go<Expiry>,
                                          flow::stop>>,
                  flow::node<Expiry, flow::noop,
                             flow::branch<flow::condition<&expired>, flow::go<Expire>,
                                          flow::go<Failed>>>,
                  flow::node<Expire, flow::action<&expire>, flow::go<Failed>>,
                  flow::node<Failed, flow::noop,
                             flow::branch<flow::condition<&retry_failed>,
                                          flow::go<Select>, flow::go<Renewal>>>,
                  flow::node<Select, flow::action<&select>, flow::go<Renewal>>,
                  flow::node<Renewal, flow::noop,
                             flow::branch<flow::condition<&renewal_due>,
                                          flow::go<Renew>, flow::go<Rebinding>>>,
                  flow::node<Renew, flow::action<&renew>, flow::go<Rebinding>>,
                  flow::node<Rebinding, flow::noop,
                             flow::branch<flow::condition<&rebinding_due>,
                                          flow::go<Rebind>, flow::go<Due>>>,
                  flow::node<Rebind, flow::action<&rebind>, flow::go<Due>>,
                  flow::node<Due, flow::noop,
                             flow::branch<flow::condition<&transmission_due>,
                                          flow::go<Retries>, flow::go<Snapshot>>>,
                  flow::node<Retries, flow::noop,
                             flow::branch<flow::condition<&exhausted>, flow::go<Fail>,
                                          flow::go<Schedule>>>,
                  flow::node<Fail, flow::action<&fail>, flow::go<Snapshot>>,
                  flow::node<Schedule, flow::action<&schedule>, flow::go<Snapshot>>,
                  flow::node<Snapshot, flow::action<&snapshot>, flow::stop>>>;

  static inline bool more(Context &c) noexcept
  {
    return c.index < 8;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.snapshot = {};
    c.send = c.changed = false;
    ConfigurationGuard guard;
    (void)ClientGraph::run<0>(c);
  }

  static inline bool changed(Context &c) noexcept
  {
    return c.changed;
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.snapshot.interface->invalidate();
  }

  static inline bool should_send(Context &c) noexcept
  {
    return c.send;
  }

  static inline void transmit(Context &c) noexcept
  {
    c.owner.transmit(c.snapshot);
  }

  static inline void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct Scan;
  struct Inspect;
  struct Invalidate;
  struct Sending;
  struct Transmit;
  struct Advance;
  using Graph = flow::graph<
      Scan,
      flow::types<flow::node<Scan, flow::noop,
                             flow::branch<flow::condition<&more>, flow::go<Inspect>,
                                          flow::stop>>,
                  flow::node<Inspect, flow::action<&inspect>,
                             flow::branch<flow::condition<&changed>,
                                          flow::go<Invalidate>, flow::go<Sending>>>,
                  flow::node<Invalidate, flow::action<&invalidate>, flow::go<Sending>>,
                  flow::node<Sending, flow::noop,
                             flow::branch<flow::condition<&should_send>,
                                          flow::go<Transmit>, flow::go<Advance>>>,
                  flow::node<Transmit, flow::action<&transmit>, flow::go<Advance>>,
                  flow::node<Advance, flow::action<&advance>, flow::go<Scan>>>>;
};

struct DhcpManager::TransmitFlow
{
  struct Context
  {
    DhcpManager &owner;
    const Client &snapshot;
    unsigned type;
    Packet packet{};
    DhcpRequest request{};
    unsigned char *udp = nullptr;
    bool unicast = false;
    unsigned length = 0, source = 0, destination = 0;
    int result = 0;
  };

  static inline void allocate(Context &c) noexcept
  {
    c.packet.~Packet();
    new (&c.packet) Packet(c.owner.ip_.allocate(308));
  }

  static inline bool allocated(Context &c) noexcept
  {
    return bool(c.packet);
  }

  static inline void describe(Context &c) noexcept
  {
    const auto &s = c.snapshot;
    c.unicast = s.state == DHCP_RENEWING || c.type == 7;
    auto &r = c.request;
    r.type = c.type ? c.type : (s.state == DHCP_SELECTING ? 1 : 3);
    r.xid = s.xid;
    r.mac = s.mac;
    r.broadcast = !c.unicast && s.state != DHCP_REBINDING;
    r.elapsed = (detail::milliseconds() - s.started) / 1000;
    if (c.unicast || s.state == DHCP_REBINDING)
    {
      r.address = s.lease.address;
    }
    if (s.state == DHCP_REQUESTING || c.type == 4)
    {
      r.requested = s.lease.address;
    }
    r.server = s.lease.server;
    c.udp = c.packet.data() + 20;
  }

  static inline void encode(Context &c) noexcept
  {
    c.length = DhcpCodec::encode(c.udp + 8, 300, c.request);
  }

  static inline bool encoded(Context &c) noexcept
  {
    return c.length != 0;
  }

  static inline void udp_header(Context &c) noexcept
  {
    wire::write16(c.udp, 68);
    wire::write16(c.udp + 2, 67);
    wire::write16(c.udp + 4, c.length + 8);
    c.source = c.request.address;
    c.destination = c.unicast ? c.snapshot.lease.server : 0xffffffff;
    unsigned char pseudo[8];
    wire::write32(pseudo, c.source);
    wire::write32(pseudo + 4, c.destination);
    unsigned checksum = static_cast<unsigned short>(
        ~wire::sum(c.udp, c.length + 8, wire::sum(pseudo, 8, 17 + c.length + 8)));
    wire::write16(c.udp + 6, checksum ? checksum : 65535);
  }

  static inline bool current(Context &c) noexcept
  {
    ConfigurationGuard guard;
    auto *current = c.owner.find(*c.snapshot.interface);
    return current && current->serial == c.snapshot.serial &&
           current->state == c.snapshot.state && c.snapshot.interface->dhcp_active();
  }

  static inline bool unicast(Context &c) noexcept
  {
    return c.unicast;
  }

  static inline void send(Context &c) noexcept
  {
    c.result =
        c.owner.ip_.send(c.packet, c.destination, 17, c.source, c.snapshot.interface);
  }

  static inline void bootstrap(Context &c) noexcept
  {
    c.result = c.owner.ip_.send_bootstrap(*c.snapshot.interface, c.packet, c.source);
  }

  static inline void no_buffer(Context &c) noexcept
  {
    c.result = -ENOBUFS;
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = -EINVAL;
  }

  static inline void stale(Context &c) noexcept
  {
    c.result = -EAGAIN;
  }
  struct Allocate;
  struct Request;
  struct Encode;
  struct Udp;
  struct Destination;
  struct Send;
  struct Bootstrap;
  struct NoBuffer;
  struct Invalid;
  struct Stale;
  using Graph = flow::graph<
      Allocate,
      flow::types<flow::node<Allocate, flow::action<&allocate>,
                             flow::branch<flow::condition<&allocated>,
                                          flow::go<Request>, flow::go<NoBuffer>>>,
                  flow::node<Request, flow::action<&describe>, flow::go<Encode>>,
                  flow::node<Encode, flow::action<&encode>,
                             flow::branch<flow::condition<&encoded>, flow::go<Udp>,
                                          flow::go<Invalid>>>,
                  flow::node<Udp, flow::action<&udp_header>,
                             flow::branch<flow::condition<&current>,
                                          flow::go<Destination>, flow::go<Stale>>>,
                  flow::node<Destination, flow::noop,
                             flow::branch<flow::condition<&unicast>, flow::go<Send>,
                                          flow::go<Bootstrap>>>,
                  flow::node<Send, flow::action<&send>, flow::stop>,
                  flow::node<Bootstrap, flow::action<&bootstrap>, flow::stop>,
                  flow::node<NoBuffer, flow::action<&no_buffer>, flow::stop>,
                  flow::node<Invalid, flow::action<&invalid>, flow::stop>,
                  flow::node<Stale, flow::action<&stale>, flow::stop>>>;
};

} // namespace xv::net
