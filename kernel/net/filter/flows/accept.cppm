module;
#include "kernel/support/flow.h"
#include "kernel/net/filter/engine.h"
#include "kernel/net/filter/decoder.h"
#include "kernel/net/errno.h"
module xv.net:filter_accept_flow;
import :filter;

namespace xv::net
{
namespace flow = xv::flow;

struct Firewall::AcceptFlow
{
  struct Context
  {
    Firewall &owner;
    FilterHook hook;
    const unsigned char *data;
    unsigned size, interface;
    filter::Engine::BuiltinChain chain = filter::Engine::BuiltinChain::Input;
    unsigned input = 0, output = 0;
    filter::Metadata packet{};
    bool result = false;
  };

  static inline bool input(Context &c) noexcept
  {
    return c.hook == FilterHook::Input;
  }

  static inline bool output(Context &c) noexcept
  {
    return c.hook == FilterHook::Output;
  }

  static inline void select_input(Context &c) noexcept
  {
    c.chain = filter::Engine::BuiltinChain::Input;
    c.input = c.interface;
  }

  static inline void select_output(Context &c) noexcept
  {
    c.chain = filter::Engine::BuiltinChain::Output;
    c.output = c.interface;
  }

  static inline bool decode(Context &c) noexcept
  {
    return filter::decode(c.data, c.size, c.input, c.output, c.packet);
  }

  static inline void evaluate(Context &c) noexcept
  {
    xv::sync::ScopedLock guard(c.owner.lock_);
    c.result = c.owner.engine_.accept(c.chain, c.packet);
  }
  struct Input;
  struct Output;
  struct SelectInput;
  struct SelectOutput;
  struct Decode;
  struct Evaluate;
  using Graph = flow::graph<
      Input,
      flow::types<
          flow::node<Input, flow::noop,
                     flow::branch<flow::condition<&input>, flow::go<SelectInput>,
                                  flow::go<Output>>>,
          flow::node<Output, flow::noop,
                     flow::branch<flow::condition<&output>, flow::go<SelectOutput>,
                                  flow::stop>>,
          flow::node<SelectInput, flow::action<&select_input>, flow::go<Decode>>,
          flow::node<SelectOutput, flow::action<&select_output>, flow::go<Decode>>,
          flow::node<
              Decode, flow::noop,
              flow::branch<flow::condition<&decode>, flow::go<Evaluate>, flow::stop>>,
          flow::node<Evaluate, flow::action<&evaluate>, flow::stop>>>;
};

} // namespace xv::net
