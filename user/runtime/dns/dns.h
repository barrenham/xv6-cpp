#pragma once
#define DNS_MAX_ADDRESSES 8
#define DNS_NAME_SIZE 254
#define DNS_INVALID -1
#define DNS_NO_SERVER -2
#define DNS_TIMEOUT -3
#define DNS_NOT_FOUND -4
#define DNS_NO_ADDRESS -5
#define DNS_SERVER_ERROR -6
#define DNS_MALFORMED -7
#define DNS_NETWORK -8
#define DNS_LIMIT -9
#define DNS_MEMORY -10
struct dns_options
{
  unsigned server;
  unsigned short port;
  unsigned timeout_ms;
  unsigned attempts;
  unsigned total_timeout_ms;
};
struct dns_result
{
  unsigned addresses[DNS_MAX_ADDRESSES];
  unsigned count;
  unsigned ttl;
  unsigned server;
  char canonical[DNS_NAME_SIZE];
};
extern "C" int dns_lookup(const char *name, dns_result *result, const dns_options *options = nullptr);
extern "C" const char *dns_strerror(int status);
extern "C" int dns_parse_ipv4(const char *text, unsigned *address);
