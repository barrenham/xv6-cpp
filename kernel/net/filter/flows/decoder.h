#pragma once
#include "kernel/support/flow.h"
#include "kernel/net/filter/decoder.h"

namespace xv::net::filter
{
namespace flow = xv::flow;

struct MetadataDecodeFlow
{
  struct Context
  {
    const unsigned char *data;
    unsigned size, input, output;
    Metadata &metadata;
    unsigned header_length = 0, total_length = 0, payload_length = 0, tcp_header = 0;
    const unsigned char *payload = nullptr;
    bool result = false;
  };

  static inline bool minimum(Context &c) noexcept
  {
    return c.size >= 20 && c.data[0] >> 4 == 4;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.header_length = (c.data[0] & 15) * 4;
    c.total_length = read16(c.data + 2);
  }

  static inline bool layout(Context &c) noexcept
  {
    return c.header_length >= 20 && c.total_length >= c.header_length &&
           c.total_length <= c.size && !(read16(c.data + 6) & 0x3fff);
  }

  static inline void describe(Context &c) noexcept
  {
    c.metadata = {read32(c.data + 12),
                  read32(c.data + 16),
                  c.data[9],
                  0,
                  0,
                  c.input,
                  c.output,
                  c.total_length,
                  false};
    c.payload_length = c.total_length - c.header_length;
    c.payload = c.data + c.header_length;
  }

  static inline bool tcp(Context &c) noexcept
  {
    return c.metadata.protocol == 6;
  }

  static inline bool tcp_minimum(Context &c) noexcept
  {
    return c.payload_length >= 20;
  }

  static inline void inspect_tcp(Context &c) noexcept
  {
    c.tcp_header = (c.payload[12] >> 4) * 4;
  }

  static inline bool tcp_layout(Context &c) noexcept
  {
    return c.tcp_header >= 20 && c.tcp_header <= c.payload_length;
  }

  static inline bool udp(Context &c) noexcept
  {
    return c.metadata.protocol == 17;
  }

  static inline bool udp_layout(Context &c) noexcept
  {
    return c.payload_length >= 8 && read16(c.payload + 4) == c.payload_length;
  }

  static inline void ports(Context &c) noexcept
  {
    c.metadata.source_port = read16(c.payload);
    c.metadata.destination_port = read16(c.payload + 2);
    c.metadata.has_ports = true;
  }

  static inline void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Minimum;
  struct Inspect;
  struct Describe;
  struct TcpMinimum;
  struct TcpHeader;
  struct Udp;
  struct UdpHeader;
  struct Ports;
  struct Success;
  using Graph = flow::graph<
      Minimum,
      flow::types<
          flow::node<
              Minimum, flow::noop,
              flow::branch<flow::condition<&minimum>, flow::go<Inspect>, flow::stop>>,
          flow::node<
              Inspect, flow::action<&inspect>,
              flow::branch<flow::condition<&layout>, flow::go<Describe>, flow::stop>>,
          flow::node<
              Describe, flow::action<&describe>,
              flow::branch<flow::condition<&tcp>, flow::go<TcpMinimum>, flow::go<Udp>>>,
          flow::node<TcpMinimum, flow::noop,
                     flow::branch<flow::condition<&tcp_minimum>, flow::go<TcpHeader>,
                                  flow::stop>>,
          flow::node<
              TcpHeader, flow::action<&inspect_tcp>,
              flow::branch<flow::condition<&tcp_layout>, flow::go<Ports>, flow::stop>>,
          flow::node<Udp, flow::noop,
                     flow::branch<flow::condition<&udp>, flow::go<UdpHeader>,
                                  flow::go<Success>>>,
          flow::node<
              UdpHeader, flow::noop,
              flow::branch<flow::condition<&udp_layout>, flow::go<Ports>, flow::stop>>,
          flow::node<Ports, flow::action<&ports>, flow::go<Success>>,
          flow::node<Success, flow::action<&success>, flow::stop>>>;
};

inline bool decode(const unsigned char *data, unsigned size, unsigned input_interface,
                   unsigned output_interface, Metadata &metadata)
{
  MetadataDecodeFlow::Context ctx{data, size, input_interface, output_interface,
                                  metadata};
  (void)MetadataDecodeFlow::Graph::run<0>(ctx);
  return ctx.result;
}

} // namespace xv::net::filter
