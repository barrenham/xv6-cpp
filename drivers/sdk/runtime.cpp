#include "drivers/sdk/host.h"

extern "C"
{
  void *
  memset(void *destination, int value, unsigned long count) {
    auto *out = static_cast<unsigned char *>(destination);
    for (unsigned long i = 0; i < count; ++i)
    {
      out[i] = value;
    }
    return destination;
  }

  void *
  memmove(void *destination, const void *source, unsigned long count) {
    auto *out = static_cast<unsigned char *>(destination);
    const auto *in = static_cast<const unsigned char *>(source);
    if (out < in)
    {
      for (unsigned long i = 0; i < count; ++i)
      {
        out[i] = in[i];
      }
    }
    else
    {
      while (count)
      {
        --count;
        out[count] = in[count];
      }
    }
    return destination;
  }

  void *
  memcpy(void *destination, const void *source, unsigned long count) {
    return memmove(destination, source, count);
  }

  [[noreturn]]
  void
  __cxa_pure_virtual() {
    xv_driver_host()->panic("driver: pure virtual call");
    __builtin_unreachable();
  }
}
