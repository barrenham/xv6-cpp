module xv.net;
import :dhcp_message;
import :wire;

namespace xv::net
{
unsigned DhcpCodec::encode(unsigned char *b, unsigned capacity, const DhcpRequest &r)
{
  if (capacity < 300)
  {
    return 0;
  }
  wire::zero(b, 300);
  b[0] = 1;
  b[1] = 1;
  b[2] = 6;
  wire::write32(b + 4, r.xid);
  wire::write16(b + 8, r.elapsed > 65535 ? 65535 : r.elapsed);
  wire::write16(b + 10, r.broadcast ? 0x8000 : 0);
  wire::write32(b + 12, r.address);
  wire::copy(b + 28, r.mac.bytes, 6);
  wire::write32(b + 236, 0x63825363);
  unsigned n = 240;
  b[n++] = 53;
  b[n++] = 1;
  b[n++] = r.type;
  b[n++] = 61;
  b[n++] = 7;
  b[n++] = 1;
  wire::copy(b + n, r.mac.bytes, 6);
  n += 6;
  if (r.requested)
  {
    b[n++] = 50;
    b[n++] = 4;
    wire::write32(b + n, r.requested);
    n += 4;
  }
  if (r.server && ((r.type == 3 && !r.address) || r.type == 4 || r.type == 7))
  {
    b[n++] = 54;
    b[n++] = 4;
    wire::write32(b + n, r.server);
    n += 4;
  }
  if (r.type == 1 || r.type == 3)
  {
    b[n++] = 55;
    b[n++] = 6;
    constexpr unsigned char requested[] = {1, 3, 6, 51, 58, 59};
    for (auto option : requested)
    {
      b[n++] = option;
    }
    b[n++] = 57;
    b[n++] = 2;
    wire::write16(b + n, 576);
    n += 2;
  }
  b[n] = 255;
  return 300;
}

bool DhcpCodec::options(const unsigned char *data, unsigned length, DhcpReply &r,
                        unsigned &overload, bool main)
{
  for (unsigned i = 0; i < length;)
  {
    unsigned code = data[i++];
    if (!code)
    {
      continue;
    }
    if (code == 255)
    {
      return true;
    }
    if (i == length)
    {
      return false;
    }
    unsigned n = data[i++];
    if (n > length - i)
    {
      return false;
    }
    unsigned bit = 0;
    switch (code)
    {
    case 53:
      bit = 1;
      if (n != 1)
      {
        return false;
      }
      break;
    case 54:
      bit = 2;
      if (n != 4)
      {
        return false;
      }
      break;
    case 1:
      bit = 4;
      if (n != 4)
      {
        return false;
      }
      break;
    case 3:
      bit = 8;
      if (!n || n % 4)
      {
        return false;
      }
      break;
    case 6:
      bit = 16;
      if (!n || n % 4)
      {
        return false;
      }
      break;
    case 51:
      bit = 32;
      if (n != 4)
      {
        return false;
      }
      break;
    case 58:
      bit = 64;
      if (n != 4)
      {
        return false;
      }
      break;
    case 59:
      bit = 128;
      if (n != 4)
      {
        return false;
      }
      break;
    case 52:
      if (!main || n != 1 || overload || !data[i] || data[i] > 3)
      {
        return false;
      }
      overload = data[i];
      break;
    default:
      break;
    }
    if (bit && (r.present & bit))
    {
      return false;
    }
    r.present |= bit;
    switch (code)
    {
    case 53:
      r.type = data[i];
      break;
    case 54:
      r.server = wire::read32(data + i);
      break;
    case 1:
      r.mask = wire::read32(data + i);
      break;
    case 3:
      r.gateway = wire::read32(data + i);
      break;
    case 6:
      r.dns = wire::read32(data + i);
      break;
    case 51:
      r.seconds = wire::read32(data + i);
      break;
    case 58:
      r.renewal = wire::read32(data + i);
      break;
    case 59:
      r.rebinding = wire::read32(data + i);
      break;
    default:
      break;
    }
    i += n;
  }
  return false;
}

bool DhcpCodec::decode(const unsigned char *b, unsigned length, DhcpReply &r)
{
  r = {};
  if (length < 240 || b[0] != 2 || b[1] != 1 || b[2] != 6 ||
      wire::read32(b + 236) != 0x63825363)
  {
    return false;
  }
  r.xid = wire::read32(b + 4);
  r.address = wire::read32(b + 16);
  wire::copy(r.mac.bytes, b + 28, 6);
  unsigned overload = 0;
  if (!options(b + 240, length - 240, r, overload, true))
  {
    return false;
  }
  unsigned ignored = 0;
  if ((overload & 1) && !options(b + 108, 128, r, ignored, false))
  {
    return false;
  }
  if ((overload & 2) && !options(b + 44, 64, r, ignored, false))
  {
    return false;
  }
  return (r.present & 3) == 3 && r.server && (r.server >> 24) != 127 &&
         (r.server >> 28) < 14 && (r.server >> 24) != 0 &&
         (r.type == 2 || r.type == 5 || r.type == 6);
}

bool DhcpCodec::lease(const DhcpReply &r, DhcpLease &result)
{
  unsigned inverse = ~r.mask;
  if ((r.present & 36) != 36 || !r.mask || (inverse & (inverse + 1)) || !r.seconds ||
      !r.address || (r.address >> 24) == 0 || (r.address >> 24) == 127 ||
      (r.address >> 28) >= 14)
  {
    return false;
  }
  unsigned prefix = 0;
  for (unsigned mask = r.mask; mask; mask <<= 1)
  {
    ++prefix;
  }
  if (prefix < 31 && (!(r.address & inverse) || (r.address & inverse) == inverse))
  {
    return false;
  }
  if (r.gateway &&
      ((r.gateway & r.mask) != (r.address & r.mask) || r.gateway == r.address ||
       (prefix < 31 && r.gateway == (r.address | inverse)) || (r.gateway >> 28) >= 14 ||
       (r.gateway >> 24) == 0))
  {
    return false;
  }
  result = {r.address, prefix,    r.gateway, r.dns,
            r.server,  r.seconds, r.renewal, r.rebinding};
  return true;
}
} // namespace xv::net
