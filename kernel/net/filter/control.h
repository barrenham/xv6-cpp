#pragma once
// Dedicated netctl operation range; fixed-width, pointer-free userspace ABI.
#define FW_VERSION 1U
#define FW_CHAIN 64
#define FW_RULE 65
#define FW_APPEND 66
#define FW_INSERT 67
#define FW_DELETE 68
#define FW_FLUSH 69
#define FW_POLICY 70
#define FW_NEW_CHAIN 71
#define FW_DELETE_CHAIN 72
#define FW_ZERO 73
#define FW_ACCEPT 1U
#define FW_DROP 2U
#define FW_RETURN 3U
#define FW_JUMP 4U
#define FW_MAX_CHAINS 10U
#define FW_MAX_RULES 32U

struct firewall_rule
{
  unsigned source, destination, source_prefix, destination_prefix;
  unsigned protocol, sport_low, sport_high, dport_low, dport_high;
  unsigned input, output, action;
  char target[16];
};

struct firewall_request
{
  unsigned version, size, index, policy, count, reserved[3];
  char chain[16];
  firewall_rule rule;
  unsigned long long packets, bytes;
};

static_assert(sizeof(firewall_rule) == 64);
static_assert(sizeof(firewall_request) == 128);
