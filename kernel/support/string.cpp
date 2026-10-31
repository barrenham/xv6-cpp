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
  const char *s;
  char *d;

  if (n == 0)
  {
    return dst;
  }

  s = static_cast<const char *>(src);
  d = static_cast<char *>(dst);
  if (s < d && s + n > d)
  {
    s += n;
    d += n;
    while (n-- > 0)
    {
      *--d = *--s;
    }
  }
  else
  {
    while (n-- > 0)
    {
      *d++ = *s++;
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
