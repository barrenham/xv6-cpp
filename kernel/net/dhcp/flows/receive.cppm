module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net:dhcp_receive_flow;
import :dhcp;
import :wire;
import :platform;

namespace xv::net
{
namespace flow = xv::flow;

struct DhcpManager::ReceiveFlow
{
  struct Context
  {
    DhcpManager &owner;
    NetworkInterface &interface;
    Packet &packet;
    const IpDatagram &datagram;
    const unsigned char *udp = nullptr;
    DhcpReply reply{};
    Client *client = nullptr;
    Client rejected{};
    DhcpLease lease{};
    unsigned long now = 0;
    bool result = false, changed = false, decline = false, renewing = false,
         rebinding = false;
    int installed = 0;
  };

  static inline bool has_header(Context &c) noexcept
  {
    return c.datagram.length >= 8;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.udp = c.packet.data() + c.datagram.offset;
  }

  static inline bool correct_ports(Context &c) noexcept
  {
    return wire::read16(c.udp) == 67 && wire::read16(c.udp + 2) == 68;
  }

  static inline bool enabled_client(Context &c) noexcept
  {
    ConfigurationGuard guard;
    auto *client = c.owner.find(c.interface);
    return client && client->state != DHCP_OFF && client->state != DHCP_STOPPING;
  }

  static inline void claim(Context &c) noexcept
  {
    c.result = true;
  }

  static inline bool valid_udp(Context &c) noexcept
  {
    return wire::read16(c.udp + 4) == c.datagram.length &&
           (!wire::read16(c.udp + 6) ||
            wire::sum(c.udp, c.datagram.length,
                      wire::sum(c.packet.data() + 12, 8, 17 + c.datagram.length)) ==
                0xffff);
  }

  static inline bool decode(Context &c) noexcept
  {
    return DhcpCodec::decode(c.udp + 8, c.datagram.length - 8, c.reply);
  }

  static inline void find_client(Context &c) noexcept
  {
    c.client = c.owner.find(c.interface);
  }

  static inline bool current(Context &c) noexcept
  {
    return c.client && c.interface.up() && !c.interface.master() &&
           c.interface.dhcp_active() && c.reply.xid == c.client->xid &&
           wire::equal(c.reply.mac.bytes, c.client->mac.bytes, 6);
  }

  static inline bool addressed(Context &c) noexcept
  {
    return c.datagram.destination == 0xffffffff ||
           c.datagram.destination == c.reply.address ||
           c.datagram.destination == c.client->lease.address;
  }

  static inline void clock(Context &c) noexcept
  {
    c.now = detail::milliseconds();
  }

  static inline bool offer(Context &c) noexcept
  {
    return c.reply.type == 2 && c.client->state == DHCP_SELECTING;
  }

  static inline bool valid_offer(Context &c) noexcept
  {
    unsigned address = c.reply.address;
    return address && (address >> 24) != 0 && (address >> 24) != 127 &&
           (address >> 28) < 14;
  }

  static inline void request_offer(Context &c) noexcept
  {
    auto &s = *c.client;
    s.lease.address = c.reply.address;
    s.lease.server = c.reply.server;
    s.state = DHCP_REQUESTING;
    ++s.serial;
    s.retries = 0;
    s.next = c.now;
  }

  static inline void classify_state(Context &c) noexcept
  {
    c.renewing = c.client->state == DHCP_RENEWING;
    c.rebinding = c.client->state == DHCP_REBINDING;
  }

  static inline bool expecting_reply(Context &c) noexcept
  {
    return c.client->state == DHCP_REQUESTING || c.renewing || c.rebinding;
  }

  static inline bool correct_server(Context &c) noexcept
  {
    return c.rebinding || c.reply.server == c.client->lease.server;
  }

  static inline bool nack(Context &c) noexcept
  {
    return c.reply.type == 6;
  }

  static inline void reject_lease(Context &c) noexcept
  {
    c.changed = c.owner.configuration_.remove(c.interface);
    c.client->error = ENETUNREACH;
    c.owner.select(*c.client, c.now, 1000);
  }

  static inline bool ack(Context &c) noexcept
  {
    return c.reply.type == 5;
  }

  static inline void normalize_address(Context &c) noexcept
  {
    if (!c.reply.address && (c.renewing || c.rebinding))
    {
      c.reply.address = c.client->lease.address;
    }
  }

  static inline bool same_address(Context &c) noexcept
  {
    return c.reply.address == c.client->lease.address;
  }

  static inline void inherit_options(Context &c) noexcept
  {
    if (c.renewing || c.rebinding)
    {
      if (!(c.reply.present & 4))
      {
        c.reply.mask = Ipv4Address{0, c.client->lease.prefix}.mask();
        c.reply.present |= 4;
      }
      if (!(c.reply.present & 8))
      {
        c.reply.gateway = c.client->lease.gateway;
      }
      if (!(c.reply.present & 16))
      {
        c.reply.dns = c.client->lease.dns;
      }
    }
  }

  static inline bool valid_lease(Context &c) noexcept
  {
    return DhcpCodec::lease(c.reply, c.lease);
  }

  static inline void install(Context &c) noexcept
  {
    c.installed = c.owner.configuration_.install(c.interface, c.lease, c.changed);
  }

  static inline bool installed(Context &c) noexcept
  {
    return c.installed >= 0;
  }

  static inline void rollback(Context &c) noexcept
  {
    c.client->error = -c.installed;
    if (c.installed == -EADDRINUSE)
    {
      c.rejected = *c.client;
      c.decline = true;
    }
    c.changed = c.owner.configuration_.remove(c.interface) || c.changed;
    c.owner.select(*c.client, c.now, 10000);
    if (c.decline)
    {
      c.rejected = *c.client;
      c.rejected.lease = c.lease;
    }
  }

  static inline void bind(Context &c) noexcept
  {
    auto &s = *c.client;
    s.lease = c.lease;
    s.state = DHCP_BOUND;
    ++s.serial;
    s.retries = 0;
    s.error = 0;
    if (c.lease.seconds == 0xffffffffU)
    {
      s.renewal = s.rebinding = s.expires = ~0UL;
    }
    else
    {
      unsigned long duration = static_cast<unsigned long>(c.lease.seconds) * 1000;
      unsigned long t1 = static_cast<unsigned long>(c.lease.renewal) * 1000,
                    t2 = static_cast<unsigned long>(c.lease.rebinding) * 1000;
      if (!t1 || t1 >= t2 || t2 >= duration)
      {
        t1 = duration / 2;
        t2 = duration * 7 / 8;
      }
      s.renewal = c.now + t1;
      s.rebinding = c.now + t2;
      s.expires = c.now + duration;
    }
  }
  struct Client;
  struct Addressed;
  struct Clock;
  struct OfferAddress;
  struct RequestOffer;
  struct State;
  struct Server;
  struct Nack;
  struct Reject;
  struct Ack;
  struct Normalize;
  struct Options;
  struct Install;
  struct Bind;
  struct Rollback;
  using ApplyGraph = flow::graph<
      Client,
      flow::types<flow::node<Client, flow::action<&find_client>,
                             flow::branch<flow::condition<&current>,
                                          flow::go<Addressed>, flow::stop>>,
                  flow::node<Addressed, flow::noop,
                             flow::branch<flow::condition<&addressed>, flow::go<Clock>,
                                          flow::stop>>,
                  flow::node<Clock, flow::action<&clock>,
                             flow::branch<flow::condition<&offer>,
                                          flow::go<OfferAddress>, flow::go<State>>>,
                  flow::node<OfferAddress, flow::noop,
                             flow::branch<flow::condition<&valid_offer>,
                                          flow::go<RequestOffer>, flow::stop>>,
                  flow::node<RequestOffer, flow::action<&request_offer>, flow::stop>,
                  flow::node<State, flow::action<&classify_state>,
                             flow::branch<flow::condition<&expecting_reply>,
                                          flow::go<Server>, flow::stop>>,
                  flow::node<Server, flow::noop,
                             flow::branch<flow::condition<&correct_server>,
                                          flow::go<Nack>, flow::stop>>,
                  flow::node<Nack, flow::noop,
                             flow::branch<flow::condition<&nack>, flow::go<Reject>,
                                          flow::go<Ack>>>,
                  flow::node<Reject, flow::action<&reject_lease>, flow::stop>,
                  flow::node<Ack, flow::noop,
                             flow::branch<flow::condition<&ack>, flow::go<Normalize>,
                                          flow::stop>>,
                  flow::node<Normalize, flow::action<&normalize_address>,
                             flow::branch<flow::condition<&same_address>,
                                          flow::go<Options>, flow::stop>>,
                  flow::node<Options, flow::action<&inherit_options>,
                             flow::branch<flow::condition<&valid_lease>,
                                          flow::go<Install>, flow::stop>>,
                  flow::node<Install, flow::action<&install>,
                             flow::branch<flow::condition<&installed>, flow::go<Bind>,
                                          flow::go<Rollback>>>,
                  flow::node<Bind, flow::action<&bind>, flow::stop>,
                  flow::node<Rollback, flow::action<&rollback>, flow::stop>>>;

  static inline void apply(Context &c) noexcept
  {
    ConfigurationGuard guard;
    (void)ApplyGraph::run<0>(c);
  }

  static inline bool changed(Context &c) noexcept
  {
    return c.changed;
  }

  static inline void invalidate(Context &c) noexcept
  {
    c.interface.invalidate();
  }

  static inline bool decline(Context &c) noexcept
  {
    return c.decline;
  }

  static inline void send_decline(Context &c) noexcept
  {
    c.owner.transmit(c.rejected, 4);
  }
  struct Length;
  struct Ports;
  struct Enabled;
  struct Claim;
  struct Decode;
  struct Apply;
  struct Invalidate;
  struct Decline;
  struct SendDecline;
  using Graph = flow::graph<
      Length,
      flow::types<flow::node<Length, flow::noop,
                             flow::branch<flow::condition<&has_header>, flow::go<Ports>,
                                          flow::stop>>,
                  flow::node<Ports, flow::action<&inspect>,
                             flow::branch<flow::condition<&correct_ports>,
                                          flow::go<Enabled>, flow::stop>>,
                  flow::node<Enabled, flow::noop,
                             flow::branch<flow::condition<&enabled_client>,
                                          flow::go<Claim>, flow::stop>>,
                  flow::node<Claim, flow::action<&claim>,
                             flow::branch<flow::condition<&valid_udp>, flow::go<Decode>,
                                          flow::stop>>,
                  flow::node<Decode, flow::noop,
                             flow::branch<flow::condition<&decode>, flow::go<Apply>,
                                          flow::stop>>,
                  flow::node<Apply, flow::action<&apply>,
                             flow::branch<flow::condition<&changed>,
                                          flow::go<Invalidate>, flow::go<Decline>>>,
                  flow::node<Invalidate, flow::action<&invalidate>, flow::go<Decline>>,
                  flow::node<Decline, flow::noop,
                             flow::branch<flow::condition<&decline>,
                                          flow::go<SendDecline>, flow::stop>>,
                  flow::node<SendDecline, flow::action<&send_decline>, flow::stop>>>;
};

} // namespace xv::net
