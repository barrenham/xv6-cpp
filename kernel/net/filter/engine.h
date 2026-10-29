#pragma once
#include "kernel/net/filter/control.h"
#include "kernel/net/filter/metadata.h"

// Pure, allocation-free policy engine. Its owner supplies serialization.
namespace xv::net::filter
{
enum Error : int
{
  Invalid = -22,
  Missing = -2,
  Exists = -17,
  Full = -28,
  Busy = -16
};

inline bool equal(const char *left, const char *right)
{
  for (unsigned i = 0; i < 16; ++i)
  {
    if (left[i] != right[i])
    {
      return false;
    }
    if (!left[i])
    {
      return true;
    }
  }
  return false;
}

inline bool name_valid(const char *name)
{
  if (!name[0])
  {
    return false;
  }
  for (unsigned i = 0; i < 16; ++i)
  {
    char character = name[i];
    if (!character)
    {
      return true;
    }
    if (!((character >= 'a' && character <= 'z') ||
          (character >= 'A' && character <= 'Z') ||
          (character >= '0' && character <= '9') || character == '_' ||
          character == '-'))
    {
      return false;
    }
  }
  return false;
}

inline unsigned mask(unsigned prefix)
{
  return prefix ? (~0U << (32 - prefix)) : 0;
}

struct Counter
{
  unsigned long long packets = 0, bytes = 0;

  void hit(unsigned length)
  {
    ++packets;
    bytes += length;
  }
};

class Engine final
{
public:

  enum class BuiltinChain
  {
    Input,
    Output
  };

  constexpr Engine()
  {
    const char *names[] = {"INPUT", "OUTPUT"};
    for (unsigned chain = 0; chain < BuiltinChainCount; ++chain)
    {
      for (unsigned i = 0; names[chain][i]; ++i)
      {
        chains_[chain].name[i] = names[chain][i];
      }
    }
  }

  int execute(unsigned operation, firewall_request &request);

  bool accept(BuiltinChain entry_chain, const Metadata &packet);

private:

  struct EvaluationFlow;
  struct CommandFlow;

  static constexpr unsigned InputChain = 0;
  static constexpr unsigned OutputChain = 1;
  static constexpr unsigned BuiltinChainCount = 2;
  static constexpr unsigned EvaluationStepLimit = 1024;

  struct Entry
  {
    firewall_rule rule{};
    Counter counter{};
  };

  struct Chain
  {
    char name[16]{};
    unsigned count = 0, policy = FW_ACCEPT;
    Counter counter{};
    Entry entries[FW_MAX_RULES]{};
  };

  int find(const char *name) const
  {
    for (unsigned i = 0; i < FW_MAX_CHAINS; ++i)
    {
      if (chains_[i].name[0] && equal(name, chains_[i].name))
      {
        return i;
      }
    }
    return -1;
  }

  int inspect_chain(firewall_request &request) const
  {
    unsigned ordinal = 0;
    for (const auto &chain : chains_)
    {
      if (!chain.name[0] || ordinal++ != request.index)
      {
        continue;
      }
      for (unsigned i = 0; i < 16; ++i)
      {
        request.chain[i] = chain.name[i];
      }
      request.policy = chain.policy;
      request.count = chain.count;
      request.packets = chain.counter.packets;
      request.bytes = chain.counter.bytes;
      return 1;
    }
    return 0;
  }

  int create_chain(const char *name)
  {
    if (find(name) >= 0 || equal(name, "ACCEPT") || equal(name, "DROP") ||
        equal(name, "RETURN") || equal(name, "FORWARD"))
    {
      return Exists;
    }
    for (unsigned i = BuiltinChainCount; i < FW_MAX_CHAINS; ++i)
    {
      auto &chain = chains_[i];
      if (chain.name[0])
      {
        continue;
      }
      for (unsigned n = 0; n < 16; ++n)
      {
        chain.name[n] = name[n];
      }
      chain.policy = FW_RETURN;
      return 0;
    }
    return Full;
  }

  int remove_chain(unsigned owner)
  {
    auto &chain = chains_[owner];
    if (owner < BuiltinChainCount || chain.count)
    {
      return Busy;
    }
    for (const auto &other : chains_)
    {
      for (unsigned i = 0; i < other.count; ++i)
      {
        if (other.entries[i].rule.action == FW_JUMP &&
            equal(other.entries[i].rule.target, chain.name))
        {
          return Busy;
        }
      }
    }
    chain = {};
    return 0;
  }

  int insert_rule(unsigned owner, unsigned index, const firewall_rule &rule)
  {
    auto &chain = chains_[owner];
    if (index > chain.count || !valid(rule, owner))
    {
      return Invalid;
    }
    if (chain.count == FW_MAX_RULES)
    {
      return Full;
    }
    for (unsigned i = chain.count; i > index; --i)
    {
      chain.entries[i] = chain.entries[i - 1];
    }
    chain.entries[index] = {rule, {}};
    ++chain.count;
    return 0;
  }

  static int remove_rule(Chain &chain, unsigned index)
  {
    if (index >= chain.count)
    {
      return Missing;
    }
    for (unsigned i = index + 1; i < chain.count; ++i)
    {
      chain.entries[i - 1] = chain.entries[i];
    }
    chain.entries[--chain.count] = {};
    return 0;
  }

  bool apply_policy(unsigned root, unsigned length)
  {
    auto &chain = chains_[root];
    chain.counter.hit(length);
    return chain.policy == FW_ACCEPT;
  }

  bool reaches(unsigned from, unsigned to) const
  {
    unsigned pending = 1U << from, visited = 0;
    while (pending)
    {
      unsigned current = 0;
      while (!(pending & (1U << current)))
      {
        ++current;
      }
      pending &= ~(1U << current);
      if (current == to)
      {
        return true;
      }
      if (visited & (1U << current))
      {
        continue;
      }
      visited |= 1U << current;
      for (unsigned i = 0; i < chains_[current].count; ++i)
      {
        const auto &rule = chains_[current].entries[i].rule;
        if (rule.action != FW_JUMP)
        {
          continue;
        }
        int target = find(rule.target);
        if (target >= 0)
        {
          pending |= (1U << target) & ~visited;
        }
      }
    }
    return false;
  }

  bool valid(const firewall_rule &rule, unsigned owner) const
  {
    if ((owner == InputChain && rule.output) || (owner == OutputChain && rule.input))
    {
      return false;
    }
    if (rule.source_prefix > 32 || rule.destination_prefix > 32 || rule.protocol > 255)
    {
      return false;
    }
    if ((rule.source & ~mask(rule.source_prefix)) ||
        (rule.destination & ~mask(rule.destination_prefix)))
    {
      return false;
    }
    if (rule.sport_low > rule.sport_high || rule.dport_low > rule.dport_high ||
        rule.sport_high > 65535 || rule.dport_high > 65535)
    {
      return false;
    }
    if ((rule.sport_high || rule.dport_high) && rule.protocol != 6 &&
        rule.protocol != 17)
    {
      return false;
    }
    if (rule.action < FW_ACCEPT || rule.action > FW_JUMP)
    {
      return false;
    }
    if (rule.action != FW_JUMP)
    {
      return !rule.target[0];
    }
    int target = find(rule.target);
    return target >= int(BuiltinChainCount) && !reaches(target, owner);
  }

  static bool matches(const firewall_rule &rule, const Metadata &packet)
  {
    return (packet.source & mask(rule.source_prefix)) == rule.source &&
           (packet.destination & mask(rule.destination_prefix)) == rule.destination &&
           (!rule.protocol || rule.protocol == packet.protocol) &&
           (!rule.input || rule.input == packet.input_interface) &&
           (!rule.output || rule.output == packet.output_interface) &&
           (!rule.sport_high ||
            (packet.has_ports && packet.source_port >= rule.sport_low &&
             packet.source_port <= rule.sport_high)) &&
           (!rule.dport_high ||
            (packet.has_ports && packet.destination_port >= rule.dport_low &&
             packet.destination_port <= rule.dport_high));
  }

  Chain chains_[FW_MAX_CHAINS]{};
};
} // namespace xv::net::filter

#include "kernel/net/filter/flows/engine.h"
