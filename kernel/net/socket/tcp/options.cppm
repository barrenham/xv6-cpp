module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:tcp_option_flow;
import :tcp_socket;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct TcpMssFlow
{
  struct Context
  {
    const unsigned char *data;
    unsigned length, header = 0, index = 20, mss = 536;
    bool found = false;
    unsigned result = 0;
  };

  static inline void inspect(Context &c) noexcept
  {
    c.header = (c.data[12] >> 4) * 4;
  }

  static inline bool valid_header(Context &c) noexcept
  {
    return c.header >= 20 && c.header <= c.length;
  }

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.header;
  }

  static inline bool end(Context &c) noexcept
  {
    return c.data[c.index] == 0;
  }

  static inline bool padding(Context &c) noexcept
  {
    return c.data[c.index] == 1;
  }

  static inline void skip_padding(Context &c) noexcept
  {
    ++c.index;
  }

  static inline bool valid_length(Context &c) noexcept
  {
    return c.index + 1 < c.header && c.data[c.index + 1] >= 2 &&
           c.index + c.data[c.index + 1] <= c.header;
  }

  static inline bool mss(Context &c) noexcept
  {
    return c.data[c.index] == 2;
  }

  static inline bool unique_mss(Context &c) noexcept
  {
    return !c.found && c.data[c.index + 1] == 4;
  }

  static inline void read_mss(Context &c) noexcept
  {
    c.mss = wire::read16(c.data + c.index + 2);
  }

  static inline bool nonzero(Context &c) noexcept
  {
    return c.mss != 0;
  }

  static inline void remember(Context &c) noexcept
  {
    c.found = true;
  }

  static inline void advance(Context &c) noexcept
  {
    c.index += c.data[c.index + 1];
  }

  static inline void complete(Context &c) noexcept
  {
    c.result = c.mss < 1460 ? c.mss : 1460;
  }
  struct Header;
  struct Scan;
  struct End;
  struct Padding;
  struct SkipPadding;
  struct Length;
  struct Mss;
  struct Unique;
  struct ReadMss;
  struct Remember;
  struct Advance;
  struct Complete;
  using Graph = flow::graph<
      Header,
      flow::types<
          flow::node<
              Header, flow::action<&inspect>,
              flow::branch<flow::condition<&valid_header>, flow::go<Scan>, flow::stop>>,
          flow::node<
              Scan, flow::noop,
              flow::branch<flow::condition<&more>, flow::go<End>, flow::go<Complete>>>,
          flow::node<End, flow::noop,
                     flow::branch<flow::condition<&end>, flow::go<Complete>,
                                  flow::go<Padding>>>,
          flow::node<Padding, flow::noop,
                     flow::branch<flow::condition<&padding>, flow::go<SkipPadding>,
                                  flow::go<Length>>>,
          flow::node<SkipPadding, flow::action<&skip_padding>, flow::go<Scan>>,
          flow::node<
              Length, flow::noop,
              flow::branch<flow::condition<&valid_length>, flow::go<Mss>, flow::stop>>,
          flow::node<
              Mss, flow::noop,
              flow::branch<flow::condition<&mss>, flow::go<Unique>, flow::go<Advance>>>,
          flow::node<Unique, flow::noop,
                     flow::branch<flow::condition<&unique_mss>, flow::go<ReadMss>,
                                  flow::stop>>,
          flow::node<
              ReadMss, flow::action<&read_mss>,
              flow::branch<flow::condition<&nonzero>, flow::go<Remember>, flow::stop>>,
          flow::node<Remember, flow::action<&remember>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
          flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

} // namespace xv::net
