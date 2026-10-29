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
                   unsigned output_interface, Metadata &metadata);
} // namespace xv::net::filter

#include "kernel/net/filter/flows/decoder.h"
