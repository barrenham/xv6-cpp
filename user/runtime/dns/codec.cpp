#include "codec.h"
namespace
{
unsigned word(const unsigned char *p) { return (unsigned(p[0]) << 8) | p[1]; }
unsigned number(const unsigned char *p) { return (word(p) << 16) | word(p + 2); }
char lower(char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; }
bool character(unsigned c)
{ return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_'; }
bool equal(const char *a, const char *b)
{ while (*a && *a == *b) { ++a; ++b; } return *a == *b; }
void copy(char *out, const char *in) { while ((*out++ = *in++)) {} }
int name_at(const unsigned char *p, unsigned size, unsigned &offset, char *out)
{
  unsigned cursor = offset, end = 0, used = 0, hops = 0;
  for (;;)
  {
    if (cursor >= size || ++hops > 128) return DNS_MALFORMED;
    unsigned start = cursor, length = p[cursor++];
    if ((length & 0xc0) == 0xc0)
    {
      if (cursor >= size) return DNS_MALFORMED;
      unsigned target = ((length & 63) << 8) | p[cursor++];
      if (target >= start) return DNS_MALFORMED;
      if (!end) end = cursor;
      cursor = target;
      continue;
    }
    if (length & 0xc0) return DNS_MALFORMED;
    if (!length) { out[used] = 0; offset = end ? end : cursor; return 0; }
    if (cursor + length > size || used + (used ? 1 : 0) + length >= DNS_NAME_SIZE) return DNS_MALFORMED;
    if (used) out[used++] = '.';
    for (unsigned i = 0; i < length; ++i)
    {
      if (!character(p[cursor])) return DNS_MALFORMED;
      out[used++] = lower(p[cursor++]);
    }
  }
}
struct Record
{
  char owner[DNS_NAME_SIZE];
  unsigned type, klass, ttl, data, length;
};
int record_at(const unsigned char *p, unsigned size, unsigned &cursor, Record &r)
{
  if (name_at(p, size, cursor, r.owner) || cursor + 10 > size) return DNS_MALFORMED;
  r.type = word(p + cursor); r.klass = word(p + cursor + 2); r.ttl = number(p + cursor + 4);
  r.length = word(p + cursor + 8); r.data = cursor + 10;
  if (r.length > size - r.data) return DNS_MALFORMED;
  cursor = r.data + r.length;
  if (r.klass == 1 && r.type == 1 && r.length != 4) return DNS_MALFORMED;
  if (r.klass == 1 && r.type == 5)
  {
    char target[DNS_NAME_SIZE]; unsigned at = r.data;
    if (name_at(p, size, at, target) || at != cursor || !target[0]) return DNS_MALFORMED;
  }
  return 0;
}
}
extern "C" int dns_parse_ipv4(const char *text, unsigned *address)
{
  if (!text || !address) return DNS_INVALID;
  unsigned value = 0;
  for (unsigned part = 0; part < 4; ++part)
  {
    unsigned n = 0, digits = 0;
    while (*text >= '0' && *text <= '9')
    { if (++digits > 3) return DNS_INVALID; n = n * 10 + (*text++ - '0'); }
    if (!digits || n > 255) return DNS_INVALID;
    value = (value << 8) | n;
    if (part < 3) { if (*text++ != '.') return DNS_INVALID; }
    else if (*text) return DNS_INVALID;
  }
  *address = value; return 0;
}
namespace xv::dns
{
int normalize(const char *name, char *output)
{
  if (!name || !output) return DNS_INVALID;
  unsigned length = 0, label = 0;
  for (; name[length]; ++length)
  {
    if (length >= DNS_NAME_SIZE) return DNS_INVALID;
    char c = name[length];
    if (c == '.') { if (!label) return DNS_INVALID; label = 0; }
    else if (!character(c) || ++label > 63) return DNS_INVALID;
    output[length] = lower(c);
  }
  if (!length) return DNS_INVALID;
  if (!label) --length;
  if (length >= DNS_NAME_SIZE) return DNS_INVALID;
  output[length] = 0;
  return 0;
}
int make_query(const char *name, unsigned short id, unsigned char *p)
{
  char normalized[DNS_NAME_SIZE];
  if (normalize(name, normalized)) return DNS_INVALID;
  for (unsigned i = 0; i < 12; ++i) p[i] = 0;
  p[0] = id >> 8; p[1] = id; p[2] = 1; p[5] = 1;
  unsigned cursor = 12, start = 0;
  for (unsigned i = 0;; ++i)
    if (!normalized[i] || normalized[i] == '.')
    {
      p[cursor++] = i - start;
      for (unsigned j = start; j < i; ++j) p[cursor++] = normalized[j];
      if (!normalized[i]) break;
      start = i + 1;
    }
  p[cursor++] = 0; p[cursor++] = 0; p[cursor++] = 1; p[cursor++] = 0; p[cursor++] = 1;
  return int(cursor);
}
int parse_reply(const unsigned char *p, unsigned size, unsigned short id, const char *question, dns_result &result)
{
  if (size < 12 || word(p) != id || (word(p + 2) & 0xf840) != 0x8000 || word(p + 4) != 1) return DNS_MALFORMED;
  unsigned cursor = 12; char q[DNS_NAME_SIZE];
  if (name_at(p, size, cursor, q) || !equal(q, question) || cursor + 4 > size || word(p + cursor) != 1 || word(p + cursor + 2) != 1) return DNS_MALFORMED;
  cursor += 4;
  if (p[2] & 2) return truncated;
  unsigned answers = word(p + 6), total = answers + word(p + 8) + word(p + 10), first = cursor;
  for (unsigned i = 0; i < total; ++i)
  { Record r; if (record_at(p, size, cursor, r)) return DNS_MALFORMED; }
  if (cursor != size) return DNS_MALFORMED;
  unsigned rcode = p[3] & 15;
  if (rcode) return rcode == 3 ? DNS_NOT_FOUND : DNS_SERVER_ERROR;
  result.count = 0; result.ttl = ~0U; copy(result.canonical, question);
  for (unsigned depth = 0; depth < 9; ++depth)
  {
    cursor = first; char next[DNS_NAME_SIZE]{}; bool found_alias = false;
    for (unsigned i = 0; i < answers; ++i)
    {
      Record r; if (record_at(p, size, cursor, r)) return DNS_MALFORMED;
      if (r.klass != 1 || !equal(r.owner, result.canonical)) continue;
      if (r.type == 5)
      {
        char target[DNS_NAME_SIZE]; unsigned at = r.data;
        if (name_at(p, size, at, target)) return DNS_MALFORMED;
        if (found_alias && !equal(next, target)) return DNS_MALFORMED;
        copy(next, target); found_alias = true;
      }
      else if (r.type == 1)
      {
        unsigned ip = number(p + r.data); bool duplicate = false;
        for (unsigned j = 0; j < result.count; ++j) if (result.addresses[j] == ip) duplicate = true;
        if (!duplicate && result.count < DNS_MAX_ADDRESSES) result.addresses[result.count++] = ip;
      }
      if ((r.type == 1 || r.type == 5) && r.ttl < result.ttl) result.ttl = r.ttl;
    }
    if (found_alias && result.count) return DNS_MALFORMED;
    if (result.count) return 0;
    if (!found_alias) return depth ? alias : DNS_NO_ADDRESS;
    if (equal(next, result.canonical)) return DNS_MALFORMED;
    copy(result.canonical, next);
  }
  return DNS_LIMIT;
}
}
