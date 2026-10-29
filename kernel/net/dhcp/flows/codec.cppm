module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:dhcp_codec_flow;
import :dhcp_message;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct DhcpCodec::DecodeFlow
{
  struct Context
  {
    const unsigned char *buffer;
    unsigned length;
    DhcpReply &reply;
    unsigned overload = 0, ignored = 0;
    bool result = false;
  };

  static inline void reset(Context &c) noexcept
  {
    c.reply = {};
  }

  static inline bool header(Context &c) noexcept
  {
    return c.length >= 240 && c.buffer[0] == 2 && c.buffer[1] == 1 &&
           c.buffer[2] == 6 && wire::read32(c.buffer + 236) == 0x63825363;
  }

  static inline void describe(Context &c) noexcept
  {
    c.reply.xid = wire::read32(c.buffer + 4);
    c.reply.address = wire::read32(c.buffer + 16);
    wire::copy(c.reply.mac.bytes, c.buffer + 28, 6);
  }

  static inline bool main_options(Context &c) noexcept
  {
    return DhcpCodec::options(c.buffer + 240, c.length - 240, c.reply, c.overload,
                              true);
  }

  static inline bool file_overload(Context &c) noexcept
  {
    return c.overload & 1;
  }

  static inline bool file_options(Context &c) noexcept
  {
    return DhcpCodec::options(c.buffer + 108, 128, c.reply, c.ignored, false);
  }

  static inline bool server_overload(Context &c) noexcept
  {
    return c.overload & 2;
  }

  static inline bool server_options(Context &c) noexcept
  {
    return DhcpCodec::options(c.buffer + 44, 64, c.reply, c.ignored, false);
  }

  static inline bool valid_reply(Context &c) noexcept
  {
    auto &r = c.reply;
    return (r.present & 3) == 3 && r.server && (r.server >> 24) != 127 &&
           (r.server >> 28) < 14 && (r.server >> 24) != 0 &&
           (r.type == 2 || r.type == 5 || r.type == 6);
  }

  static inline void success(Context &c) noexcept
  {
    c.result = true;
  }
  struct Header;
  struct Describe;
  struct File;
  struct FileOptions;
  struct Server;
  struct ServerOptions;
  struct Reply;
  struct Success;
  using Graph = flow::graph<
      Header,
      flow::types<flow::node<Header, flow::action<&reset>,
                             flow::branch<flow::condition<&header>, flow::go<Describe>,
                                          flow::stop>>,
                  flow::node<Describe, flow::action<&describe>,
                             flow::branch<flow::condition<&main_options>,
                                          flow::go<File>, flow::stop>>,
                  flow::node<File, flow::noop,
                             flow::branch<flow::condition<&file_overload>,
                                          flow::go<FileOptions>, flow::go<Server>>>,
                  flow::node<FileOptions, flow::noop,
                             flow::branch<flow::condition<&file_options>,
                                          flow::go<Server>, flow::stop>>,
                  flow::node<Server, flow::noop,
                             flow::branch<flow::condition<&server_overload>,
                                          flow::go<ServerOptions>, flow::go<Reply>>>,
                  flow::node<ServerOptions, flow::noop,
                             flow::branch<flow::condition<&server_options>,
                                          flow::go<Reply>, flow::stop>>,
                  flow::node<Reply, flow::noop,
                             flow::branch<flow::condition<&valid_reply>,
                                          flow::go<Success>, flow::stop>>,
                  flow::node<Success, flow::action<&success>, flow::stop>>>;
};

struct DhcpCodec::EncodeFlow
{
  struct Context
  {
    unsigned char *buffer;
    unsigned capacity;
    const DhcpRequest &request;
    unsigned next = 240, result = 0;
  };

  static inline bool capacity(Context &c) noexcept
  {
    return c.capacity >= 300;
  }

  static inline void header(Context &c) noexcept
  {
    auto *b = c.buffer;
    const auto &r = c.request;
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
  }

  static inline void identity(Context &c) noexcept
  {
    auto *b = c.buffer;
    auto &n = c.next;
    b[n++] = 53;
    b[n++] = 1;
    b[n++] = c.request.type;
    b[n++] = 61;
    b[n++] = 7;
    b[n++] = 1;
    wire::copy(b + n, c.request.mac.bytes, 6);
    n += 6;
  }

  static inline bool requested(Context &c) noexcept
  {
    return c.request.requested != 0;
  }

  static inline void requested_option(Context &c) noexcept
  {
    auto *b = c.buffer;
    auto &n = c.next;
    b[n++] = 50;
    b[n++] = 4;
    wire::write32(b + n, c.request.requested);
    n += 4;
  }

  static inline bool server(Context &c) noexcept
  {
    const auto &r = c.request;
    return r.server && ((r.type == 3 && !r.address) || r.type == 4 || r.type == 7);
  }

  static inline void server_option(Context &c) noexcept
  {
    auto *b = c.buffer;
    auto &n = c.next;
    b[n++] = 54;
    b[n++] = 4;
    wire::write32(b + n, c.request.server);
    n += 4;
  }

  static inline bool requests_configuration(Context &c) noexcept
  {
    return c.request.type == 1 || c.request.type == 3;
  }

  static inline void parameter_options(Context &c) noexcept
  {
    auto *b = c.buffer;
    auto &n = c.next;
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

  static inline void finish(Context &c) noexcept
  {
    c.buffer[c.next] = 255;
    c.result = 300;
  }
  struct Capacity;
  struct Header;
  struct Identity;
  struct RequestedOption;
  struct Server;
  struct ServerOption;
  struct Parameters;
  struct ParameterOptions;
  struct Finish;
  using Graph = flow::graph<
      Capacity,
      flow::types<
          flow::node<
              Capacity, flow::noop,
              flow::branch<flow::condition<&capacity>, flow::go<Header>, flow::stop>>,
          flow::node<Header, flow::action<&header>, flow::go<Identity>>,
          flow::node<Identity, flow::action<&identity>,
                     flow::branch<flow::condition<&requested>,
                                  flow::go<RequestedOption>, flow::go<Server>>>,
          flow::node<RequestedOption, flow::action<&requested_option>,
                     flow::go<Server>>,
          flow::node<Server, flow::noop,
                     flow::branch<flow::condition<&server>, flow::go<ServerOption>,
                                  flow::go<Parameters>>>,
          flow::node<ServerOption, flow::action<&server_option>, flow::go<Parameters>>,
          flow::node<Parameters, flow::noop,
                     flow::branch<flow::condition<&requests_configuration>,
                                  flow::go<ParameterOptions>, flow::go<Finish>>>,
          flow::node<ParameterOptions, flow::action<&parameter_options>,
                     flow::go<Finish>>,
          flow::node<Finish, flow::action<&finish>, flow::stop>>>;
};

struct DhcpCodec::LeaseFlow
{
  struct Context
  {
    const DhcpReply &reply;
    DhcpLease &lease;
    unsigned inverse = 0, prefix = 0;
    bool result = false;
  };

  static inline void inspect(Context &c) noexcept
  {
    c.inverse = ~c.reply.mask;
  }

  static inline bool valid_fields(Context &c) noexcept
  {
    const auto &r = c.reply;
    return (r.present & 36) == 36 && r.mask && !(c.inverse & (c.inverse + 1)) &&
           r.seconds && r.address && (r.address >> 24) != 0 &&
           (r.address >> 24) != 127 && (r.address >> 28) < 14;
  }

  static inline void prefix(Context &c) noexcept
  {
    for (unsigned mask = c.reply.mask; mask; mask <<= 1)
    {
      ++c.prefix;
    }
  }

  static inline bool host(Context &c) noexcept
  {
    unsigned host = c.reply.address & c.inverse;
    return c.prefix >= 31 || (host && host != c.inverse);
  }

  static inline bool gateway(Context &c) noexcept
  {
    const auto &r = c.reply;
    return !r.gateway ||
           ((r.gateway & r.mask) == (r.address & r.mask) && r.gateway != r.address &&
            (c.prefix >= 31 || r.gateway != (r.address | c.inverse)) &&
            (r.gateway >> 28) < 14 && (r.gateway >> 24) != 0);
  }

  static inline void commit(Context &c) noexcept
  {
    const auto &r = c.reply;
    c.lease = {r.address, c.prefix,  r.gateway, r.dns,
               r.server,  r.seconds, r.renewal, r.rebinding};
    c.result = true;
  }
  struct Fields;
  struct Prefix;
  struct Gateway;
  struct Commit;
  using Graph =
      flow::graph<Fields,
                  flow::types<flow::node<Fields, flow::action<&inspect>,
                                         flow::branch<flow::condition<&valid_fields>,
                                                      flow::go<Prefix>, flow::stop>>,
                              flow::node<Prefix, flow::action<&prefix>,
                                         flow::branch<flow::condition<&host>,
                                                      flow::go<Gateway>, flow::stop>>,
                              flow::node<Gateway, flow::noop,
                                         flow::branch<flow::condition<&gateway>,
                                                      flow::go<Commit>, flow::stop>>,
                              flow::node<Commit, flow::action<&commit>, flow::stop>>>;
};

} // namespace xv::net
