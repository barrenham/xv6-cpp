#pragma once
#include "kernel/net/filter/metadata.h"

namespace xv::net::filter
{
inline unsigned read16(const unsigned char *data)
{
  return (unsigned(data[0]) << 8) | data[1];
}

inline unsigned read32(const unsigned char *data)
{
  return (read16(data) << 16) | read16(data + 2);
}

// Called with complete IPv4 datagrams, after reassembly on INPUT.
inline bool decode(const unsigned char *data, unsigned size, unsigned input_interface,
                   unsigned output_interface, Metadata &metadata)
{
  if (size < 20 || data[0] >> 4 != 4)
  {
    return false;
  }
  unsigned header_length = (data[0] & 15) * 4;
  unsigned total_length = read16(data + 2);
  if (header_length < 20 || total_length < header_length || total_length > size ||
      (read16(data + 6) & 0x3fff))
  {
    return false;
  }

  metadata = {read32(data + 12), read32(data + 16), data[9],      0,    0,
              input_interface,   output_interface,  total_length, false};
  unsigned payload_length = total_length - header_length;
  const auto *payload = data + header_length;
  if (metadata.protocol == 6)
  {
    if (payload_length < 20)
    {
      return false;
    }
    unsigned tcp_header_length = (payload[12] >> 4) * 4;
    if (tcp_header_length < 20 || tcp_header_length > payload_length)
    {
      return false;
    }
  }
  else if (metadata.protocol == 17)
  {
    if (payload_length < 8 || read16(payload + 4) != payload_length)
    {
      return false;
    }
  }
  else
  {
    return true;
  }

  metadata.source_port = read16(payload);
  metadata.destination_port = read16(payload + 2);
  metadata.has_ports = true;
  return true;
}
} // namespace xv::net::filter
