#pragma once
#include "http.h"
namespace xv::http
{
constexpr unsigned target_size = 1024;
constexpr unsigned line_size = 2048;
struct Url
{
  char host[254];
  char target[target_size];
  unsigned port;
};
int parse_url(const char *text, Url &url);
int resolve_location(const Url &base, const char *location, Url &url);
struct Head
{
  unsigned status;
  bool has_length, chunked, has_location;
  unsigned long long length;
  char location[target_size];
};
class HeaderParser
{
public:
  Head value{};
  int status_line(const char *line);
  int field(const char *line, bool trailer = false);
  int finish() const;
};
int chunk_size(const char *line, unsigned long long &size);
}
