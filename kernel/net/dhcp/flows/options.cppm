module;
#include "kernel/support/flow.h"
#include "kernel/net/errno.h"
module xv.net:dhcp_option_flow;
import :dhcp_message;
import :wire;

namespace xv::net
{
namespace flow = xv::flow;

struct DhcpCodec::OptionsFlow
{
  struct Context
  {
    const unsigned char *data;
    unsigned length;
    DhcpReply &reply;
    unsigned &overload;
    bool main;
    unsigned index = 0, code = 0, length_value = 0, bit = 0;
    bool result = false;
  };

  static inline bool more(Context &c) noexcept
  {
    return c.index < c.length;
  }

  static inline void code(Context &c) noexcept
  {
    c.code = c.data[c.index++];
  }

  static inline bool padding(Context &c) noexcept
  {
    return c.code == 0;
  }

  static inline bool end(Context &c) noexcept
  {
    return c.code == 255;
  }

  static inline bool has_length(Context &c) noexcept
  {
    return c.index != c.length;
  }

  static inline void length(Context &c) noexcept
  {
    c.length_value = c.data[c.index++];
  }

  static inline bool fits(Context &c) noexcept
  {
    return c.length_value <= c.length - c.index;
  }

  static inline bool valid_option(Context &c) noexcept
  {
    c.bit = 0;
    unsigned n = c.length_value;
    switch (c.code)
    {
    case 53:
      c.bit = 1;
      return n == 1;
    case 54:
      c.bit = 2;
      return n == 4;
    case 1:
      c.bit = 4;
      return n == 4;
    case 3:
      c.bit = 8;
      return n && !(n % 4);
    case 6:
      c.bit = 16;
      return n && !(n % 4);
    case 51:
      c.bit = 32;
      return n == 4;
    case 58:
      c.bit = 64;
      return n == 4;
    case 59:
      c.bit = 128;
      return n == 4;
    case 52:
      if (!c.main || n != 1 || c.overload || !c.data[c.index] || c.data[c.index] > 3)
      {
        return false;
      }
      c.overload = c.data[c.index];
      return true;
    default:
      return true;
    }
  }

  static inline bool unique(Context &c) noexcept
  {
    return !c.bit || !(c.reply.present & c.bit);
  }

  static inline void decode(Context &c) noexcept
  {
    auto &r = c.reply;
    r.present |= c.bit;
    auto *data = c.data + c.index;
    switch (c.code)
    {
    case 53:
      r.type = data[0];
      break;
    case 54:
      r.server = wire::read32(data);
      break;
    case 1:
      r.mask = wire::read32(data);
      break;
    case 3:
      r.gateway = wire::read32(data);
      break;
    case 6:
      r.dns = wire::read32(data);
      break;
    case 51:
      r.seconds = wire::read32(data);
      break;
    case 58:
      r.renewal = wire::read32(data);
      break;
    case 59:
      r.rebinding = wire::read32(data);
      break;
    default:
      break;
    }
  }

  static inline void advance(Context &c) noexcept
  {
    c.index += c.length_value;
  }

  static inline void complete(Context &c) noexcept
  {
    c.result = true;
  }
  struct Scan;
  struct Code;
  struct End;
  struct HasLength;
  struct Length;
  struct Validate;
  struct Unique;
  struct Decode;
  struct Advance;
  struct Complete;
  using Graph = flow::graph<
      Scan,
      flow::types<
          flow::node<Scan, flow::noop,
                     flow::branch<flow::condition<&more>, flow::go<Code>, flow::stop>>,
          flow::node<
              Code, flow::action<&code>,
              flow::branch<flow::condition<&padding>, flow::go<Scan>, flow::go<End>>>,
          flow::node<End, flow::noop,
                     flow::branch<flow::condition<&end>, flow::go<Complete>,
                                  flow::go<HasLength>>>,
          flow::node<
              HasLength, flow::noop,
              flow::branch<flow::condition<&has_length>, flow::go<Length>, flow::stop>>,
          flow::node<
              Length, flow::action<&length>,
              flow::branch<flow::condition<&fits>, flow::go<Validate>, flow::stop>>,
          flow::node<Validate, flow::noop,
                     flow::branch<flow::condition<&valid_option>, flow::go<Unique>,
                                  flow::stop>>,
          flow::node<
              Unique, flow::noop,
              flow::branch<flow::condition<&unique>, flow::go<Decode>, flow::stop>>,
          flow::node<Decode, flow::action<&decode>, flow::go<Advance>>,
          flow::node<Advance, flow::action<&advance>, flow::go<Scan>>,
          flow::node<Complete, flow::action<&complete>, flow::stop>>>;
};

} // namespace xv::net
