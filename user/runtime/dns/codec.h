#pragma once
#include "dns.h"
namespace xv::dns
{
constexpr int truncated = 1;
constexpr int alias = 2;
int normalize(const char *name, char *output);
int make_query(const char *name, unsigned short id, unsigned char *packet);
int parse_reply(const unsigned char *packet, unsigned size, unsigned short id,
                const char *question, dns_result &result);
}
