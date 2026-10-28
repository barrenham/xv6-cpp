#pragma once

namespace xv::net::filter
{
struct Metadata
{
  unsigned source, destination, protocol;
  unsigned source_port, destination_port;
  unsigned input_interface, output_interface;
  unsigned total_length;
  bool has_ports;
};
} // namespace xv::net::filter
