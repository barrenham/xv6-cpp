#pragma once
namespace xv::echo
{
inline unsigned short read16(const unsigned char *p) { return (unsigned(p[0]) << 8) | p[1]; }
inline void write16(unsigned char *p, unsigned value) { p[0] = value >> 8; p[1] = value; }
inline unsigned short checksum(const unsigned char *p, unsigned length)
{
  unsigned sum = 0;
  while (length >= 2) { sum += read16(p); p += 2; length -= 2; }
  if (length) sum += unsigned(*p) << 8;
  while (sum >> 16) sum = (sum & 65535) + (sum >> 16);
  return static_cast<unsigned short>(~sum);
}
inline void request(unsigned char *p, unsigned length, unsigned sequence, unsigned long long stamp)
{
  p[0] = 8; p[1] = 0; p[2] = p[3] = p[4] = p[5] = 0;
  write16(p + 6, sequence);
  for (unsigned i = 8; i < length; ++i) p[i] = (i * 37 + sequence) & 255;
  if (length >= 16) for (unsigned i = 0; i < 8; ++i) p[8+i] = stamp >> ((7-i)*8);
  write16(p + 2, checksum(p, length));
}
inline bool matches(const unsigned char *reply, unsigned length, const unsigned char *sent,
                    unsigned sent_length, unsigned identifier)
{
  if (length != sent_length || length < 8 || reply[0] || reply[1] ||
      read16(reply + 4) != identifier || read16(reply + 6) != read16(sent + 6) ||
      checksum(reply, length)) return false;
  for (unsigned i = 8; i < length; ++i) if (reply[i] != sent[i]) return false;
  return true;
}
}
