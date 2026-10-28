module;
#include "kernel/net/errno.h"
#include "kernel/net/control.h"
module xv.net;
import :dhcp;
import :wire;
import :platform;

namespace xv::net
{
void DhcpManager::initialize()
{
  ip_.attach_bootstrap(*this);
}

DhcpManager::Client *DhcpManager::find(NetworkInterface &interface)
{
  for (auto &client : clients_)
  {
    if (client.interface == &interface)
    {
      return &client;
    }
  }
  return nullptr;
}

void DhcpManager::select(Client &client, unsigned long now, unsigned delay)
{
  client.state = DHCP_SELECTING;
  client.xid =
      ++counter_ ^ static_cast<unsigned>(now) ^ wire::read32(client.mac.bytes + 2);
  ++client.serial;
  client.retries = 0;
  client.started = now;
  client.next = now + delay;
  client.lease = {};
  client.renewal = client.rebinding = client.expires = 0;
}

int DhcpManager::execute(unsigned operation, netctl_request &request)
{
  if (request.version != NETCTL_VERSION || request.size != sizeof(request))
  {
    return -EINVAL;
  }
  NetworkInterface *interface;
  Client stopping{};
  bool release = false;
  {
    ConfigurationGuard guard;
    interface = interfaces_.find(request.ifindex);
    if (!interface)
    {
      return -ENODEV;
    }
    auto *client = find(*interface);
    if (operation == NETCTL_DHCP_STATUS)
    {
      netctl_request out{};
      out.version = NETCTL_VERSION;
      out.size = sizeof(out);
      out.ifindex = interface->id();
      for (unsigned i = 0; i < sizeof(out.name) - 1 && interface->name()[i]; ++i)
      {
        out.name[i] = interface->name()[i];
      }
      if (client)
      {
        auto now = detail::milliseconds();
        out.flags = client->state;
        out.index = client->error;
        out.mtu = client->retries;
        out.address = client->lease.address;
        out.prefix = client->lease.prefix;
        out.gateway = client->lease.gateway;
        out.source = client->lease.dns;
        out.metric = client->lease.server;
        out.expires_ms = client->expires > now ? client->expires - now : 0;
        out.rx_packets = client->renewal > now ? client->renewal - now : 0;
        out.tx_packets = client->rebinding > now ? client->rebinding - now : 0;
      }
      request = out;
      return 1;
    }
    auto now = detail::milliseconds();
    if (operation == NETCTL_DHCP_START)
    {
      if (client && client->state != DHCP_OFF && client->state != DHCP_FAILED)
      {
        return -EALREADY;
      }
      if (!interface->mac() || !interface->layer2() || interface->mtu() < 604)
      {
        return -EOPNOTSUPP;
      }
      if (interface->master() || !interface->up() || interface->address_count())
      {
        return -EBUSY;
      }
      for (unsigned i = 0; i < 8 && !client; ++i)
      {
        if (!clients_[i].interface)
        {
          client = &clients_[i];
        }
      }
      if (!client)
      {
        return -ENOBUFS;
      }
      bool valid = false;
      for (auto byte : interface->mac()->bytes)
      {
        valid |= byte != 0;
      }
      if (!valid || (interface->mac()->bytes[0] & 1))
      {
        return -EOPNOTSUPP;
      }
      client->interface = interface;
      client->mac = *interface->mac();
      client->error = 0;
      interface->set_dhcp_active(true);
      select(*client, now);
      return 0;
    }
    if (operation == NETCTL_DHCP_RENEW)
    {
      if (!client || (client->state != DHCP_BOUND && client->state != DHCP_RENEWING))
      {
        return -ENOTCONN;
      }
      if (client->state == DHCP_BOUND)
      {
        client->state = DHCP_RENEWING;
        client->xid = ++counter_ ^ static_cast<unsigned>(now);
        ++client->serial;
        client->started = now;
        client->next = now;
        client->retries = 0;
      }
      return 0;
    }
    if (operation != NETCTL_DHCP_STOP)
    {
      return -EINVAL;
    }
    if (!client || client->state == DHCP_OFF)
    {
      interface->set_dhcp_active(false);
      return 0;
    }
    if (client->state == DHCP_STOPPING)
    {
      return -EBUSY;
    }
    release = client->state == DHCP_BOUND || client->state == DHCP_RENEWING ||
              client->state == DHCP_REBINDING;
    client->state = DHCP_STOPPING;
    ++client->serial;
    stopping = *client;
  }
  if (release)
  {
    transmit(stopping, 7);
  }
  {
    ConfigurationGuard guard;
    auto *client = find(*interface);
    if (client && client->serial == stopping.serial && client->state == DHCP_STOPPING)
    {
      configuration_.remove(*interface);
      unsigned serial = client->serial + 1;
      *client = {};
      client->interface = interface;
      client->serial = serial;
      interface->set_dhcp_active(false);
    }
  }
  interface->invalidate();
  return 0;
}

int DhcpManager::transmit(const Client &c, unsigned type)
{
  auto packet = ip_.allocate(308);
  if (!packet)
  {
    return -ENOBUFS;
  }
  bool unicast = c.state == DHCP_RENEWING || type == 7;
  DhcpRequest request{};
  request.type = type ? type : (c.state == DHCP_SELECTING ? 1 : 3);
  request.xid = c.xid;
  request.mac = c.mac;
  request.broadcast = !unicast && c.state != DHCP_REBINDING;
  request.elapsed = (detail::milliseconds() - c.started) / 1000;
  if (unicast || c.state == DHCP_REBINDING)
  {
    request.address = c.lease.address;
  }
  if (c.state == DHCP_REQUESTING || type == 4)
  {
    request.requested = c.lease.address;
  }
  request.server = c.lease.server;
  auto *udp = packet.data() + 20;
  unsigned length = DhcpCodec::encode(udp + 8, 300, request);
  if (!length)
  {
    return -EINVAL;
  }
  wire::write16(udp, 68);
  wire::write16(udp + 2, 67);
  wire::write16(udp + 4, length + 8);
  unsigned source = request.address,
           destination = unicast ? c.lease.server : 0xffffffff;
  unsigned char pseudo[8];
  wire::write32(pseudo, source);
  wire::write32(pseudo + 4, destination);
  unsigned checksum = static_cast<unsigned short>(
      ~wire::sum(udp, length + 8, wire::sum(pseudo, 8, 17 + length + 8)));
  wire::write16(udp + 6, checksum ? checksum : 65535);
  {
    ConfigurationGuard guard;
    auto *current = find(*c.interface);
    if (!current || current->serial != c.serial || current->state != c.state ||
        !c.interface->dhcp_active())
    {
      return -EAGAIN;
    }
  }
  if (unicast)
  {
    return ip_.send(packet, destination, 17, source, c.interface);
  }
  return ip_.send_bootstrap(*c.interface, packet, source);
}

bool DhcpManager::receive_bootstrap(NetworkInterface &interface, Packet &packet,
                                    const IpDatagram &d)
{
  if (d.length < 8)
  {
    return false;
  }
  auto *udp = packet.data() + d.offset;
  if (wire::read16(udp) != 67 || wire::read16(udp + 2) != 68)
  {
    return false;
  }
  {
    ConfigurationGuard guard;
    auto *client = find(interface);
    if (!client || client->state == DHCP_OFF || client->state == DHCP_STOPPING)
    {
      return false;
    }
  }
  if (wire::read16(udp + 4) != d.length ||
      (wire::read16(udp + 6) &&
       wire::sum(udp, d.length, wire::sum(packet.data() + 12, 8, 17 + d.length)) !=
           0xffff))
  {
    return true;
  }
  DhcpReply reply;
  if (!DhcpCodec::decode(udp + 8, d.length - 8, reply))
  {
    return true;
  }
  bool changed = false, decline = false;
  Client rejected{};
  {
    ConfigurationGuard guard;
    auto *c = find(interface);
    if (!c || !interface.up() || interface.master() || !interface.dhcp_active() ||
        reply.xid != c->xid || !wire::equal(reply.mac.bytes, c->mac.bytes, 6))
    {
      return true;
    }
    if (d.destination != 0xffffffff && d.destination != reply.address &&
        d.destination != c->lease.address)
    {
      return true;
    }
    auto now = detail::milliseconds();
    if (reply.type == 2 && c->state == DHCP_SELECTING)
    {
      if (!reply.address || (reply.address >> 24) == 0 ||
          (reply.address >> 24) == 127 || (reply.address >> 28) >= 14)
      {
        return true;
      }
      c->lease.address = reply.address;
      c->lease.server = reply.server;
      c->state = DHCP_REQUESTING;
      ++c->serial;
      c->retries = 0;
      c->next = now;
      return true;
    }
    bool requesting = c->state == DHCP_REQUESTING, renewing = c->state == DHCP_RENEWING,
         rebinding = c->state == DHCP_REBINDING;
    if (!requesting && !renewing && !rebinding)
    {
      return true;
    }
    if (!rebinding && reply.server != c->lease.server)
    {
      return true;
    }
    if (reply.type == 6)
    {
      changed = configuration_.remove(interface);
      c->error = ENETUNREACH;
      select(*c, now, 1000);
    }
    else if (reply.type == 5)
    {
      if (!reply.address && (renewing || rebinding))
      {
        reply.address = c->lease.address;
      }
      if (reply.address != c->lease.address)
      {
        return true;
      }
      if (renewing || rebinding)
      {
        if (!(reply.present & 4))
        {
          reply.mask = Ipv4Address{0, c->lease.prefix}.mask();
          reply.present |= 4;
        }
        if (!(reply.present & 8))
        {
          reply.gateway = c->lease.gateway;
        }
        if (!(reply.present & 16))
        {
          reply.dns = c->lease.dns;
        }
      }
      DhcpLease lease;
      if (!DhcpCodec::lease(reply, lease))
      {
        return true;
      }
      int result = configuration_.install(interface, lease, changed);
      if (result < 0)
      {
        c->error = -result;
        if (result == -EADDRINUSE)
        {
          rejected = *c;
          decline = true;
        }
        changed = configuration_.remove(interface) || changed;
        select(*c, now, 10000);
        if (decline)
        {
          rejected = *c;
          rejected.lease = lease;
        }
      }
      else
      {
        c->lease = lease;
        c->state = DHCP_BOUND;
        ++c->serial;
        c->retries = 0;
        c->error = 0;
        if (lease.seconds == 0xffffffffU)
        {
          c->renewal = c->rebinding = c->expires = ~0UL;
        }
        else
        {
          unsigned long duration = static_cast<unsigned long>(lease.seconds) * 1000;
          unsigned long t1 = static_cast<unsigned long>(lease.renewal) * 1000,
                        t2 = static_cast<unsigned long>(lease.rebinding) * 1000;
          if (!t1 || t1 >= t2 || t2 >= duration)
          {
            t1 = duration / 2;
            t2 = duration * 7 / 8;
          }
          c->renewal = now + t1;
          c->rebinding = now + t2;
          c->expires = now + duration;
        }
      }
    }
  }
  if (changed)
  {
    interface.invalidate();
  }
  if (decline)
  {
    transmit(rejected, 4);
  }
  return true;
}

void DhcpManager::tick(unsigned long now)
{
  for (unsigned i = 0; i < 8; ++i)
  {
    Client snapshot{};
    bool send = false, changed = false;
    {
      ConfigurationGuard guard;
      auto &c = clients_[i];
      if (!c.interface || c.state == DHCP_OFF || c.state == DHCP_STOPPING)
      {
        continue;
      }
      if ((c.state == DHCP_BOUND || c.state == DHCP_RENEWING ||
           c.state == DHCP_REBINDING) &&
          now >= c.expires)
      {
        changed = configuration_.remove(*c.interface);
        c.error = ETIMEDOUT;
        select(c, now);
      }
      if (c.state == DHCP_FAILED && now >= c.next)
      {
        select(c, now);
      }
      if (c.state == DHCP_BOUND && c.renewal != ~0UL && now >= c.renewal)
      {
        c.state = DHCP_RENEWING;
        c.xid = ++counter_ ^ static_cast<unsigned>(now);
        ++c.serial;
        c.started = now;
        c.next = now;
        c.retries = 0;
      }
      if (c.state == DHCP_RENEWING && now >= c.rebinding)
      {
        c.state = DHCP_REBINDING;
        ++c.serial;
        c.next = now;
        c.retries = 0;
      }
      if (c.state != DHCP_BOUND && c.state != DHCP_FAILED && now >= c.next &&
          c.interface->up() && c.interface->carrier() && !c.interface->master())
      {
        if ((c.state == DHCP_SELECTING || c.state == DHCP_REQUESTING) && c.retries >= 6)
        {
          c.state = DHCP_FAILED;
          c.error = ETIMEDOUT;
          c.next = now + 60000;
        }
        else
        {
          unsigned long delay;
          if (c.state == DHCP_RENEWING || c.state == DHCP_REBINDING)
          {
            auto deadline = c.state == DHCP_RENEWING ? c.rebinding : c.expires;
            delay = (deadline - now) / 2;
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
            unsigned n = c.retries < 4 ? c.retries : 4;
            delay = (1000U << n) + (c.xid & 255);
          }
          ++c.retries;
          c.next = now + delay;
          send = true;
        }
      }
      snapshot = c;
    }
    if (changed)
    {
      snapshot.interface->invalidate();
    }
    if (send)
    {
      transmit(snapshot);
    }
  }
}
} // namespace xv::net
