#pragma once
#include "kernel/support/flow.h"
#include "kernel/net/filter/engine.h"

namespace xv::net::filter
{
namespace flow = xv::flow;

struct Engine::EvaluationFlow
{
  struct Frame
  {
    unsigned chain, next;
  };

  struct Context
  {
    Engine &owner;
    BuiltinChain entry_chain;
    const Metadata &packet;
    unsigned root = 0, current = 0, next = 0, depth = 0, step = 0;
    Frame stack[FW_MAX_CHAINS]{};
    Entry *entry = nullptr;
    int target = -1;
    bool valid = false, result = false;
  };

  static inline void select_root(Context &c) noexcept
  {
    switch (c.entry_chain)
    {
    case BuiltinChain::Input:
      c.root = InputChain;
      c.valid = true;
      break;
    case BuiltinChain::Output:
      c.root = OutputChain;
      c.valid = true;
      break;
    default:
      break;
    }
    c.current = c.root;
  }

  static inline bool root_valid(Context &c) noexcept
  {
    return c.valid;
  }

  static inline bool within_budget(Context &c) noexcept
  {
    return c.step++ < EvaluationStepLimit;
  }

  static inline bool more_rules(Context &c) noexcept
  {
    return c.next < c.owner.chains_[c.current].count;
  }

  static inline void fetch(Context &c) noexcept
  {
    c.entry = &c.owner.chains_[c.current].entries[c.next++];
  }

  static inline bool match(Context &c) noexcept
  {
    return c.owner.matches(c.entry->rule, c.packet);
  }

  static inline void count(Context &c) noexcept
  {
    c.entry->counter.hit(c.packet.total_length);
  }

  static inline bool terminal(Context &c) noexcept
  {
    return c.entry->rule.action == FW_ACCEPT || c.entry->rule.action == FW_DROP;
  }

  static inline void verdict(Context &c) noexcept
  {
    c.result = c.entry->rule.action == FW_ACCEPT;
  }

  static inline bool jump(Context &c) noexcept
  {
    return c.entry->rule.action == FW_JUMP;
  }

  static inline void lookup(Context &c) noexcept
  {
    c.target = c.owner.find(c.entry->rule.target);
  }

  static inline bool valid_target(Context &c) noexcept
  {
    return c.target >= int(BuiltinChainCount) && c.depth < FW_MAX_CHAINS;
  }

  static inline void push(Context &c) noexcept
  {
    c.stack[c.depth++] = {c.current, c.next};
    c.current = c.target;
    c.next = 0;
  }

  static inline bool has_caller(Context &c) noexcept
  {
    return c.depth != 0;
  }

  static inline void pop(Context &c) noexcept
  {
    auto caller = c.stack[--c.depth];
    c.current = caller.chain;
    c.next = caller.next;
  }

  static inline void policy(Context &c) noexcept
  {
    c.result = c.owner.apply_policy(c.root, c.packet.total_length);
  }
  struct Root;
  struct Step;
  struct Rules;
  struct Fetch;
  struct Count;
  struct Verdict;
  struct Jump;
  struct Lookup;
  struct Push;
  struct Unwind;
  struct Pop;
  struct Policy;
  using Graph = flow::graph<
      Root, flow::types<flow::node<Root, flow::action<&select_root>,
                                   flow::branch<flow::condition<&root_valid>,
                                                flow::go<Step>, flow::stop>>,
                        flow::node<Step, flow::noop,
                                   flow::branch<flow::condition<&within_budget>,
                                                flow::go<Rules>, flow::stop>>,
                        flow::node<Rules, flow::noop,
                                   flow::branch<flow::condition<&more_rules>,
                                                flow::go<Fetch>, flow::go<Unwind>>>,
                        flow::node<Fetch, flow::action<&fetch>,
                                   flow::branch<flow::condition<&match>,
                                                flow::go<Count>, flow::go<Step>>>,
                        flow::node<Count, flow::action<&count>,
                                   flow::branch<flow::condition<&terminal>,
                                                flow::go<Verdict>, flow::go<Jump>>>,
                        flow::node<Verdict, flow::action<&verdict>, flow::stop>,
                        flow::node<Jump, flow::noop,
                                   flow::branch<flow::condition<&jump>,
                                                flow::go<Lookup>, flow::go<Unwind>>>,
                        flow::node<Lookup, flow::action<&lookup>,
                                   flow::branch<flow::condition<&valid_target>,
                                                flow::go<Push>, flow::stop>>,
                        flow::node<Push, flow::action<&push>, flow::go<Step>>,
                        flow::node<Unwind, flow::noop,
                                   flow::branch<flow::condition<&has_caller>,
                                                flow::go<Pop>, flow::go<Policy>>>,
                        flow::node<Pop, flow::action<&pop>, flow::go<Step>>,
                        flow::node<Policy, flow::action<&policy>, flow::stop>>>;
};

struct Engine::CommandFlow
{
  struct Context
  {
    Engine &owner;
    unsigned operation;
    firewall_request &request;
    int chain = -1, result = 0;
  };

  static inline bool valid(Context &c) noexcept
  {
    return c.request.version == FW_VERSION && c.request.size == sizeof(c.request) &&
           !c.request.reserved[0] && !c.request.reserved[1] && !c.request.reserved[2];
  }

  template <unsigned Operation> static bool operation(Context &c) noexcept
  {
    return c.operation == Operation;
  }

  static inline bool valid_name(Context &c) noexcept
  {
    return name_valid(c.request.chain);
  }

  static inline void lookup(Context &c) noexcept
  {
    c.chain = c.owner.find(c.request.chain);
  }

  static inline bool found(Context &c) noexcept
  {
    return c.chain >= 0;
  }

  static inline void inspect(Context &c) noexcept
  {
    c.result = c.owner.inspect_chain(c.request);
  }

  static inline void create(Context &c) noexcept
  {
    c.result = c.owner.create_chain(c.request.chain);
  }

  static inline void rule(Context &c) noexcept
  {
    auto &chain = c.owner.chains_[c.chain];
    if (c.request.index >= chain.count)
    {
      return;
    }
    auto &entry = chain.entries[c.request.index];
    c.request.rule = entry.rule;
    c.request.packets = entry.counter.packets;
    c.request.bytes = entry.counter.bytes;
    c.result = 1;
  }

  static inline void append(Context &c) noexcept
  {
    c.result =
        c.owner.insert_rule(c.chain, c.owner.chains_[c.chain].count, c.request.rule);
  }

  static inline void insert(Context &c) noexcept
  {
    c.result = c.owner.insert_rule(c.chain, c.request.index, c.request.rule);
  }

  static inline void remove(Context &c) noexcept
  {
    c.result = c.owner.remove_rule(c.owner.chains_[c.chain], c.request.index);
  }

  static inline void flush(Context &c) noexcept
  {
    c.owner.chains_[c.chain].count = 0;
  }

  static inline void zero(Context &c) noexcept
  {
    auto &chain = c.owner.chains_[c.chain];
    chain.counter = {};
    for (unsigned i = 0; i < chain.count; ++i)
    {
      chain.entries[i].counter = {};
    }
  }

  static inline bool valid_policy(Context &c) noexcept
  {
    return unsigned(c.chain) < BuiltinChainCount &&
           (c.request.policy == FW_ACCEPT || c.request.policy == FW_DROP);
  }

  static inline void policy(Context &c) noexcept
  {
    c.owner.chains_[c.chain].policy = c.request.policy;
  }

  static inline void delete_chain(Context &c) noexcept
  {
    c.result = c.owner.remove_chain(c.chain);
  }

  static inline void invalid(Context &c) noexcept
  {
    c.result = xv::net::filter::Invalid;
  }

  static inline void missing(Context &c) noexcept
  {
    c.result = xv::net::filter::Missing;
  }
  struct Version;
  struct ChainInspection;
  struct Inspect;
  struct Name;
  struct Lookup;
  struct Create;
  struct Exists;
  struct IsRule;
  struct Rule;
  struct IsAppend;
  struct Append;
  struct IsInsert;
  struct Insert;
  struct IsDelete;
  struct Delete;
  struct IsFlush;
  struct Flush;
  struct IsZero;
  struct Zero;
  struct IsPolicy;
  struct Policy;
  struct IsDeleteChain;
  struct DeleteChain;
  struct SetPolicy;
  struct Invalid;
  struct Missing;
  using Graph = flow::graph<
      Version,
      flow::types<
          flow::node<Version, flow::noop,
                     flow::branch<flow::condition<&valid>, flow::go<ChainInspection>,
                                  flow::go<Invalid>>>,
          flow::node<ChainInspection, flow::noop,
                     flow::branch<flow::condition<&operation<FW_CHAIN>>,
                                  flow::go<Inspect>, flow::go<Name>>>,
          flow::node<Inspect, flow::action<&inspect>, flow::stop>,
          flow::node<Name, flow::noop,
                     flow::branch<flow::condition<&valid_name>, flow::go<Lookup>,
                                  flow::go<Invalid>>>,
          flow::node<Lookup, flow::action<&lookup>,
                     flow::branch<flow::condition<&operation<FW_NEW_CHAIN>>,
                                  flow::go<Create>, flow::go<Exists>>>,
          flow::node<Create, flow::action<&create>, flow::stop>,
          flow::node<Exists, flow::noop,
                     flow::branch<flow::condition<&found>, flow::go<IsRule>,
                                  flow::go<Missing>>>,
          flow::node<IsRule, flow::noop,
                     flow::branch<flow::condition<&operation<FW_RULE>>, flow::go<Rule>,
                                  flow::go<IsAppend>>>,
          flow::node<Rule, flow::action<&rule>, flow::stop>,
          flow::node<IsAppend, flow::noop,
                     flow::branch<flow::condition<&operation<FW_APPEND>>,
                                  flow::go<Append>, flow::go<IsInsert>>>,
          flow::node<Append, flow::action<&append>, flow::stop>,
          flow::node<IsInsert, flow::noop,
                     flow::branch<flow::condition<&operation<FW_INSERT>>,
                                  flow::go<Insert>, flow::go<IsDelete>>>,
          flow::node<Insert, flow::action<&insert>, flow::stop>,
          flow::node<IsDelete, flow::noop,
                     flow::branch<flow::condition<&operation<FW_DELETE>>,
                                  flow::go<Delete>, flow::go<IsFlush>>>,
          flow::node<Delete, flow::action<&remove>, flow::stop>,
          flow::node<IsFlush, flow::noop,
                     flow::branch<flow::condition<&operation<FW_FLUSH>>,
                                  flow::go<Flush>, flow::go<IsZero>>>,
          flow::node<Flush, flow::action<&flush>, flow::stop>,
          flow::node<IsZero, flow::noop,
                     flow::branch<flow::condition<&operation<FW_ZERO>>, flow::go<Zero>,
                                  flow::go<IsPolicy>>>,
          flow::node<Zero, flow::action<&zero>, flow::stop>,
          flow::node<IsPolicy, flow::noop,
                     flow::branch<flow::condition<&operation<FW_POLICY>>,
                                  flow::go<Policy>, flow::go<IsDeleteChain>>>,
          flow::node<Policy, flow::noop,
                     flow::branch<flow::condition<&valid_policy>, flow::go<SetPolicy>,
                                  flow::go<Invalid>>>,
          flow::node<IsDeleteChain, flow::noop,
                     flow::branch<flow::condition<&operation<FW_DELETE_CHAIN>>,
                                  flow::go<DeleteChain>, flow::go<Invalid>>>,
          flow::node<DeleteChain, flow::action<&delete_chain>, flow::stop>,
          flow::node<SetPolicy, flow::action<&policy>, flow::stop>,
          flow::node<Invalid, flow::action<&invalid>, flow::stop>,
          flow::node<Missing, flow::action<&missing>, flow::stop>>>;
};

inline bool Engine::accept(BuiltinChain entry_chain, const Metadata &packet)
{
  EvaluationFlow::Context ctx{*this, entry_chain, packet};
  (void)EvaluationFlow::Graph::run<0>(ctx);
  return ctx.result;
}

inline int Engine::execute(unsigned operation, firewall_request &request)
{
  CommandFlow::Context ctx{*this, operation, request};
  (void)CommandFlow::Graph::run<0>(ctx);
  return ctx.result;
}

} // namespace xv::net::filter
