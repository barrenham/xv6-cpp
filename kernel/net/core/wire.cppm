export module xv.net:wire;

namespace xv::net::wire
{
inline unsigned read16(const unsigned char *p)
{
  return (unsigned(p[0]) << 8) | p[1];
}

inline unsigned read32(const unsigned char *p)
{
  return (read16(p) << 16) | read16(p + 2);
}

inline void write16(unsigned char *p, unsigned n)
{
  p[0] = n >> 8;
  p[1] = n;
}

inline void write32(unsigned char *p, unsigned n)
{
  write16(p, n >> 16);
  write16(p + 2, n);
}

inline void copy(void *to, const void *from, unsigned n)
{
  auto *d = static_cast<unsigned char *>(to);
  auto *s = static_cast<const unsigned char *>(from);
  while (n--)
  {
    *d++ = *s++;
  }
}

inline void zero(void *to, unsigned n)
{
  auto *p = static_cast<unsigned char *>(to);
  while (n--)
  {
    *p++ = 0;
  }
}

inline bool equal(const void *left, const void *right, unsigned n)
{
  auto *a = static_cast<const unsigned char *>(left);
  auto *b = static_cast<const unsigned char *>(right);
  while (n--)
  {
    if (*a++ != *b++)
    {
      return false;
    }
  }
  return true;
}

inline unsigned sum(const unsigned char *data, unsigned length, unsigned value = 0)
{
  while (length >= 2)
  {
    value += read16(data);
    data += 2;
    length -= 2;
  }
  if (length)
  {
    value += unsigned(*data) << 8;
  }
  while (value >> 16)
  {
    value = (value & 0xffff) + (value >> 16);
  }
  return value;
}

inline bool before(unsigned left, unsigned right)
{
  return static_cast<int>(left - right) < 0;
}
} // namespace xv::net::wire
