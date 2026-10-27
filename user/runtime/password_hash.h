#pragma once

namespace password_hash
{
inline void wipe(void *memory, unsigned size)
{
  auto *p = static_cast<volatile unsigned char *>(memory);
  while (size--)
  {
    *p++ = 0;
  }
}

class Sha256
{
  unsigned state_[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  unsigned char block_[64]{};
  unsigned long long size_ = 0;
  unsigned used_ = 0;

  static unsigned rotate(unsigned x, unsigned n)
  {
    return (x >> n) | (x << (32 - n));
  }

  void compress()
  {
    constexpr unsigned k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
        0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
        0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
        0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
        0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    unsigned w[64];
    for (unsigned i = 0; i < 16; ++i)
    {
      w[i] = (unsigned(block_[4 * i]) << 24) | (unsigned(block_[4 * i + 1]) << 16) |
             (unsigned(block_[4 * i + 2]) << 8) | block_[4 * i + 3];
    }
    for (unsigned i = 16; i < 64; ++i)
    {
      unsigned a = w[i - 15], b = w[i - 2];
      w[i] = w[i - 16] + (rotate(a, 7) ^ rotate(a, 18) ^ (a >> 3)) + w[i - 7] +
             (rotate(b, 17) ^ rotate(b, 19) ^ (b >> 10));
    }
    unsigned a = state_[0], b = state_[1], c = state_[2], d = state_[3], e = state_[4],
             f = state_[5], g = state_[6], h = state_[7];
    for (unsigned i = 0; i < 64; ++i)
    {
      unsigned first = h + (rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25)) +
                       ((e & f) ^ (~e & g)) + k[i] + w[i];
      unsigned second = (rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22)) +
                        ((a & b) ^ (a & c) ^ (b & c));
      h = g;
      g = f;
      f = e;
      e = d + first;
      d = c;
      c = b;
      b = a;
      a = first + second;
    }
    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
    state_[5] += f;
    state_[6] += g;
    state_[7] += h;
    wipe(w, sizeof(w));
  }

public:

  ~Sha256()
  {
    wipe(this, sizeof(*this));
  }

  void add(const void *data, unsigned size)
  {
    const auto *p = static_cast<const unsigned char *>(data);
    size_ += size;
    while (size--)
    {
      block_[used_++] = *p++;
      if (used_ == 64)
      {
        compress();
        used_ = 0;
      }
    }
  }

  void finish(unsigned char output[32])
  {
    unsigned long long bits = size_ * 8;
    unsigned char marker = 0x80, zero = 0;
    add(&marker, 1);
    while (used_ != 56)
    {
      add(&zero, 1);
    }
    unsigned char length[8];
    for (unsigned i = 0; i < 8; ++i)
    {
      length[7 - i] = bits >> (8 * i);
    }
    add(length, 8);
    for (unsigned i = 0; i < 32; ++i)
    {
      output[i] = state_[i / 4] >> (24 - 8 * (i % 4));
    }
  }
};

class Hmac
{
  Sha256 inner_, outer_;

public:

  Hmac(const void *key, unsigned size)
  {
    unsigned char block[64]{};
    if (size > 64)
    {
      Sha256 hash;
      hash.add(key, size);
      hash.finish(block);
    }
    else
    {
      const auto *p = static_cast<const unsigned char *>(key);
      for (unsigned i = 0; i < size; ++i)
      {
        block[i] = p[i];
      }
    }
    for (auto &b : block)
    {
      b ^= 0x36;
    }
    inner_.add(block, 64);
    for (auto &b : block)
    {
      b ^= 0x36 ^ 0x5c;
    }
    outer_.add(block, 64);
    wipe(block, sizeof(block));
  }

  void digest(const void *data, unsigned size, unsigned char output[32]) const
  {
    Sha256 inner = inner_, outer = outer_;
    unsigned char intermediate[32];
    inner.add(data, size);
    inner.finish(intermediate);
    outer.add(intermediate, 32);
    outer.finish(output);
    wipe(intermediate, 32);
  }
};

inline void derive(const char *password, unsigned length, const unsigned char salt[16],
                   unsigned iterations, unsigned char output[32])
{
  Hmac hmac(password, length);
  unsigned char block[20], value[32];
  for (unsigned i = 0; i < 16; ++i)
  {
    block[i] = salt[i];
  }
  block[16] = block[17] = block[18] = 0;
  block[19] = 1;
  hmac.digest(block, 20, value);
  for (unsigned i = 0; i < 32; ++i)
  {
    output[i] = value[i];
  }
  for (unsigned round = 1; round < iterations; ++round)
  {
    hmac.digest(value, 32, value);
    for (unsigned i = 0; i < 32; ++i)
    {
      output[i] ^= value[i];
    }
  }
  wipe(block, sizeof(block));
  wipe(value, sizeof(value));
}

inline void hex(const unsigned char *input, unsigned length, char *output)
{
  const char digits[] = "0123456789abcdef";
  for (unsigned i = 0; i < length; ++i)
  {
    output[i * 2] = digits[input[i] >> 4];
    output[i * 2 + 1] = digits[input[i] & 15];
  }
  output[length * 2] = 0;
}

inline bool unhex(const char *input, unsigned length, unsigned char *output)
{
  for (unsigned i = 0; i < length; ++i)
  {
    unsigned value = 0;
    for (unsigned j = 0; j < 2; ++j)
    {
      char c = input[i * 2 + j];
      if (!c)
      {
        return false;
      }
      unsigned n = c >= '0' && c <= '9'   ? c - '0'
                   : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                          : 16;
      if (n == 16)
      {
        return false;
      }
      value = value * 16 + n;
    }
    output[i] = value;
  }
  return true;
}
} // namespace password_hash
