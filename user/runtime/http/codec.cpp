#include "codec.h"
namespace
{
unsigned length(const char *s) { unsigned n = 0; while (s[n]) ++n; return n; }
char lower(char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
bool equal(const char *a, const char *b)
{ while (*a && lower(*a) == lower(*b)) { ++a; ++b; } return *a == *b; }
bool prefix(const char *a, const char *b)
{ while (*b) { if (lower(*a++) != *b++) return false; } return true; }
void copy(char *a, const char *b, unsigned n) { for (unsigned i = 0; i < n; ++i) a[i] = b[i]; a[n] = 0; }
bool digit(char c) { return c >= '0' && c <= '9'; }
bool token(char c)
{
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || digit(c)) return true;
  const char *extra = "!#$%&'*+-.^_`|~";
  while (*extra) if (*extra++ == c) return true;
  return false;
}
int uri_text(const char *s)
{
  for (unsigned i = 0; s[i]; ++i)
  {
    unsigned c = static_cast<unsigned char>(s[i]);
    if (c <= 32 || c >= 127 || c == '\\') return xv::http::InvalidUrl;
    if (c == '%')
    {
      for (unsigned j = 0; j < 2; ++j)
      { char h = lower(s[++i]); if (!digit(h) && !(h >= 'a' && h <= 'f')) return xv::http::InvalidUrl; }
    }
  }
  return 0;
}
int canonical_path(char *target)
{
  char *query = target;
  while (*query && *query != '?') ++query;
  unsigned end = unsigned(query - target), input = 0, output = 0;
  while (input < end)
  {
    if (input + 2 <= end && target[input] == '.' && target[input+1] == '/') { input += 2; continue; }
    if (input + 3 <= end && target[input] == '.' && target[input+1] == '.' && target[input+2] == '/') { input += 3; continue; }
    if (target[input] == '/' && input + 1 < end && target[input+1] == '.' && (input + 2 == end || target[input+2] == '/'))
    { input += 2; if (input == end) { target[output++] = '/'; break; } continue; }
    if (target[input] == '/' && input + 2 < end && target[input+1] == '.' && target[input+2] == '.' && (input + 3 == end || target[input+3] == '/'))
    {
      input += 3;
      if (output) { --output; while (output && target[output] != '/') --output; }
      if (input == end) { target[output++] = '/'; break; }
      continue;
    }
    target[output++] = target[input++];
    while (input < end && target[input] != '/') target[output++] = target[input++];
  }
  if (!output) target[output++] = '/';
  for (unsigned i = end;; ++i) { char c = target[i]; target[output++] = c; if (!c) break; }
  return 0;
}
}
namespace xv::http
{
int parse_url(const char *text, Url &url)
{
  if (!text) return InvalidUrl;
  unsigned total = length(text);
  if (!total || total > target_size + 270 || uri_text(text)) return InvalidUrl;
  if (!prefix(text, "http://")) return Unsupported;
  const char *host = text + 7, *end = host;
  while (*end && *end != ':' && *end != '/' && *end != '?' && *end != '#') ++end;
  unsigned host_length = unsigned(end - host);
  if (!host_length || host_length >= sizeof(url.host)) return InvalidUrl;
  unsigned label = 0;
  for (unsigned i = 0; i < host_length; ++i)
  {
    char c = lower(host[i]);
    if (c == '.') { if (!label) return InvalidUrl; label = 0; }
    else if (!((c >= 'a' && c <= 'z') || digit(c) || c == '-' || c == '_') || ++label > 63) return InvalidUrl;
    url.host[i] = c;
  }
  url.host[host_length] = 0; url.port = 80;
  if (*end == ':')
  {
    unsigned port = 0; ++end;
    if (!digit(*end)) return InvalidUrl;
    while (digit(*end)) { port = port * 10 + (*end++ - '0'); if (port > 65535) return InvalidUrl; }
    if (!port) return InvalidUrl;
    url.port = port;
  }
  if (*end && *end != '/' && *end != '?' && *end != '#') return InvalidUrl;
  unsigned n = 0;
  if (*end != '/') url.target[n++] = '/';
  while (*end && *end != '#')
  { if (n >= sizeof(url.target)-1) return InvalidUrl; url.target[n++] = *end++; }
  url.target[n] = 0;
  return canonical_path(url.target);
}
int resolve_location(const Url &base, const char *location, Url &url)
{
  if (!location || uri_text(location)) return InvalidUrl;
  if (prefix(location, "http://")) return parse_url(location, url);
  if (location[0] == '/' && location[1] == '/')
  {
    unsigned n = length(location);
    if (n + 6 >= target_size + 8) return InvalidUrl;
    char absolute[target_size+8]; copy(absolute, "http:", 5); copy(absolute+5, location, n);
    return parse_url(absolute, url);
  }
  for (unsigned i = 0; location[i] && location[i] != '/' && location[i] != '?' && location[i] != '#'; ++i)
    if (location[i] == ':') return Unsupported;
  if (&base == &url) return InvalidUrl;
  url = base;
  unsigned keep = 0;
  while (url.target[keep] && url.target[keep] != '?') ++keep;
  if (!*location || *location == '#') return 0;
  if (*location == '/') keep = 0;
  else if (*location != '?') { while (keep && url.target[keep-1] != '/') --keep; }
  unsigned out = keep;
  for (unsigned i = 0; location[i] && location[i] != '#'; ++i)
  { if (out >= sizeof(url.target)-1) return InvalidUrl; url.target[out++] = location[i]; }
  url.target[out] = 0;
  return canonical_path(url.target);
}
int HeaderParser::status_line(const char *line)
{
  value = {};
  if (length(line) < 12 || (!prefix(line, "http/1.1 ") && !prefix(line, "http/1.0 ")) ||
      !digit(line[9]) || !digit(line[10]) || !digit(line[11]) || (line[12] && line[12] != ' ')) return Protocol;
  value.status = unsigned(line[9]-'0')*100 + unsigned(line[10]-'0')*10 + unsigned(line[11]-'0');
  if (value.status < 100 || value.status > 599) return Protocol;
  for (unsigned i = 12; line[i]; ++i)
    if ((static_cast<unsigned char>(line[i]) < 32 && line[i] != '\t') || line[i] == 127) return Protocol;
  return 0;
}
int HeaderParser::field(const char *line, bool trailer)
{
  unsigned colon = 0;
  while (line[colon] && line[colon] != ':') { if (!token(line[colon])) return Protocol; ++colon; }
  if (!colon || line[colon] != ':' || colon >= 128) return Protocol;
  char name[128]; copy(name, line, colon);
  const char *start = line + colon + 1;
  while (*start == ' ' || *start == '\t') ++start;
  unsigned n = length(start);
  while (n && (start[n-1] == ' ' || start[n-1] == '\t')) --n;
  for (unsigned i = 0; i < n; ++i)
    if ((static_cast<unsigned char>(start[i]) < 32 && start[i] != '\t') || start[i] == 127) return Protocol;
  if (trailer)
    return equal(name, "content-length") || equal(name, "transfer-encoding") || equal(name, "host") ? Protocol : 0;
  if (equal(name, "content-length"))
  {
    if (!n) return Protocol;
    unsigned long long number = 0;
    for (unsigned i = 0; i < n; ++i)
    { if (!digit(start[i]) || number > (~0ULL - unsigned(start[i]-'0')) / 10) return Protocol; number = number*10 + unsigned(start[i]-'0'); }
    if (value.has_length && number != value.length) return Protocol;
    value.has_length = true; value.length = number;
  }
  else if (equal(name, "transfer-encoding"))
  {
    if (value.chunked || n != 7) return Protocol;
    for (unsigned i = 0; i < n; ++i) if (lower(start[i]) != "chunked"[i]) return Unsupported;
    value.chunked = true;
  }
  else if (equal(name, "location"))
  {
    if (value.has_location || n >= sizeof(value.location)) return Protocol;
    copy(value.location, start, n); value.has_location = true;
  }
  else if (equal(name, "content-encoding"))
  {
    if (n != 8) return Unsupported;
    for (unsigned i = 0; i < n; ++i) if (lower(start[i]) != "identity"[i]) return Unsupported;
  }
  return 0;
}
int HeaderParser::finish() const { return value.chunked && value.has_length ? Protocol : 0; }
int chunk_size(const char *line, unsigned long long &size)
{
  size = 0; unsigned n = 0;
  while (line[n] && line[n] != ';')
  {
    char c = lower(line[n]); unsigned d;
    if (digit(c)) d = unsigned(c-'0'); else if (c >= 'a' && c <= 'f') d = unsigned(c-'a')+10; else return Protocol;
    if (size > (~0ULL-d)/16) return Protocol;
    size = size*16+d; ++n;
  }
  if (!n) return Protocol;
  for (; line[n]; ++n) if (static_cast<unsigned char>(line[n]) < 32 || line[n] == 127) return Protocol;
  return 0;
}
}
