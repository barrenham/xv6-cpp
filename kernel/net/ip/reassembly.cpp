module xv.net;
import :reassembly;
import :wire;
import :platform;

namespace xv::net
{
void Ipv4Reassembly::initialize()
{
  lock_.init("IPv4 reassembly");
}

Ipv4Reassembly::Assembly *Ipv4Reassembly::find_or_create(const Key &key,
                                                         const Packet &fragment,
                                                         unsigned long now)
{
  for (auto &assembly : assemblies_)
  {
    if (assembly.page && assembly.key.ingress == key.ingress &&
        assembly.key.source == key.source &&
        assembly.key.destination == key.destination &&
        assembly.key.identification == key.identification &&
        assembly.key.protocol == key.protocol &&
        assembly.context.tunnel_depth == fragment.context().tunnel_depth)
    {
      return &assembly;
    }
  }
  for (auto &assembly : assemblies_)
  {
    if (assembly.page)
    {
      continue;
    }
    auto *page = detail::allocate_page();
    if (!page)
    {
      return nullptr;
    }
    assembly.page = page;
    assembly.key = key;
    assembly.context = fragment.context();
    assembly.deadline = now + 15000;
    wire::copy(page, fragment.data(), 20);
    return &assembly;
  }
  return nullptr;
}

bool Ipv4Reassembly::insert(Assembly &assembly, const unsigned char *payload,
                            unsigned offset, unsigned length, bool last)
{
  unsigned end = offset + length;
  if (assembly.last && end > assembly.total)
  {
    return false;
  }
  for (unsigned i = offset; i < end; ++i)
  {
    if (assembly.present[i / 8] & (1U << (i % 8)))
    {
      return false;
    }
  }
  if (last)
  {
    if ((assembly.last && assembly.total != end) || assembly.received > end)
    {
      return false;
    }
    for (unsigned i = end; i < MaxPayload; ++i)
    {
      if (assembly.present[i / 8] & (1U << (i % 8)))
      {
        return false;
      }
    }
    assembly.total = end;
    assembly.last = true;
  }
  wire::copy(static_cast<unsigned char *>(assembly.page) + 20 + offset, payload,
             length);
  for (unsigned i = offset; i < end; ++i)
  {
    assembly.present[i / 8] |= 1U << (i % 8);
  }
  assembly.received += length;
  return true;
}

Packet Ipv4Reassembly::accept(const void *ingress, const Packet &fragment,
                              unsigned header_length, unsigned total_length,
                              unsigned long now)
{
  if (!fragment || header_length != 20 || total_length < header_length ||
      total_length > fragment.size())
  {
    return {};
  }
  const auto *header = fragment.data();
  unsigned flags = wire::read16(header + 6);
  unsigned offset = (flags & 0x1fff) * 8, length = total_length - header_length;
  if (!length || offset + length > MaxPayload || ((flags & 0x2000) && (length & 7)))
  {
    return {};
  }
  Key key{ingress, wire::read32(header + 12), wire::read32(header + 16),
          wire::read16(header + 4), header[9]};
  void *page;
  unsigned complete_length = 0;
  Packet::Context context;
  {
    xv::sync::ScopedLock guard(lock_);
    auto *assembly = find_or_create(key, fragment, now);
    if (!assembly)
    {
      return {};
    }
    bool valid =
        insert(*assembly, header + header_length, offset, length, !(flags & 0x2000));
    if (valid && !(assembly->last && assembly->received == assembly->total))
    {
      return {};
    }
    page = assembly->page;
    context = assembly->context;
    if (valid)
    {
      complete_length = 20 + assembly->total;
    }
    *assembly = {};
  }
  Packet complete(page, complete_length, context);
  if (!complete_length)
  {
    return {};
  }
  auto *ip = complete.data();
  wire::write16(ip + 2, complete_length);
  wire::write16(ip + 6, 0);
  wire::write16(ip + 10, 0);
  wire::write16(ip + 10, ~wire::sum(ip, 20));
  return complete;
}

void Ipv4Reassembly::tick(unsigned long now)
{
  for (auto &assembly : assemblies_)
  {
    void *expired = nullptr;
    {
      xv::sync::ScopedLock guard(lock_);
      if (assembly.page && assembly.deadline <= now)
      {
        expired = assembly.page;
        assembly = {};
      }
    }
    Packet discarded(expired, 0);
  }
}

void Ipv4Reassembly::clear()
{
  tick(~0UL);
}
} // namespace xv::net
