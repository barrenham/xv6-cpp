module;
#include "kernel/support/types.h"
#include "kernel/support/byte_fill.h"

module xv.runtime;

namespace xv::runtime
{

void *fill(void *dst, int c, unsigned long n)
{
  return xv::support::byte_fill(dst, c, n);
}

int compare_bytes(const void *v1, const void *v2, unsigned long n)
{
  const uchar *s1, *s2;

  s1 = static_cast<const uchar *>(v1);
  s2 = static_cast<const uchar *>(v2);
  while (n-- > 0)
  {
    if (*s1 != *s2)
    {
      return *s1 - *s2;
    }
    s1++, s2++;
  }

  return 0;
}

void *move(void *dst, const void *src, unsigned long n)
{
  auto *d = static_cast<unsigned char *>(dst);
  auto *s = static_cast<const unsigned char *>(src);
  if (!n || d == s)
  {
    return dst;
  }
  // Word accesses are used only when both addresses have equal alignment.
  // Prefix/tail bytes stay inside the requested interval, including overlap.
  typedef unsigned long AliasWord __attribute__((__may_alias__));
  constexpr unsigned width = sizeof(AliasWord);
  auto destination = reinterpret_cast<unsigned long>(d);
  auto source = reinterpret_cast<unsigned long>(s);
  bool aligned = ((destination ^ source) & (width - 1)) == 0;
  if (destination > source && destination - source < n)
  {
    d += n;
    s += n;
    if (aligned)
    {
      while (n && (reinterpret_cast<unsigned long>(d) & (width - 1)))
      {
        *--d = *--s;
        --n;
      }
      while (n >= width)
      {
        d -= width;
        s -= width;
        *reinterpret_cast<AliasWord *>(d) = *reinterpret_cast<const AliasWord *>(s);
        n -= width;
      }
    }
    while (n)
    {
      *--d = *--s;
      --n;
    }
  }
  else
  {
    if (aligned)
    {
      while (n && (reinterpret_cast<unsigned long>(d) & (width - 1)))
      {
        *d++ = *s++;
        --n;
      }
      while (n >= width)
      {
        *reinterpret_cast<AliasWord *>(d) = *reinterpret_cast<const AliasWord *>(s);
        d += width;
        s += width;
        n -= width;
      }
    }
    while (n)
    {
      *d++ = *s++;
      --n;
    }
  }

  return dst;
}

int compare_text(const char *p, const char *q, unsigned long n)
{
  while (n > 0 && *p && *p == *q)
  {
    n--, p++, q++;
  }
  if (n == 0)
  {
    return 0;
  }
  return (uchar)*p - (uchar)*q;
}

char *copy_fixed(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while (n-- > 0 && (*s++ = *t++) != 0)
    ;
  while (n-- > 0)
  {
    *s++ = 0;
  }
  return os;
}

// Like copy_fixed but guaranteed to NUL-terminate.
char *copy_string(char *s, const char *t, int n)
{
  char *os;

  os = s;
  if (n <= 0)
  {
    return os;
  }
  while (--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

int length(const char *s)
{
  int n;

  for (n = 0; s[n]; n++)
    ;
  return n;
}

} // namespace xv::runtime
