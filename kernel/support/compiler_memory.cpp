import xv.runtime;

// Required by compiler-generated copies/initialization, never called by kernel
// module source. Keep these standard freestanding ABI symbols at the boundary.
extern "C"
{
  void *memset(void *destination, int value, unsigned long count)
  {
    return xv::runtime::fill(destination, value, count);
  }

  void *memmove(void *destination, const void *source, unsigned long count)
  {
    return xv::runtime::move(destination, source, count);
  }

  void *memcpy(void *destination, const void *source, unsigned long count)
  {
    return xv::runtime::move(destination, source, count);
  }

  int memcmp(const void *first, const void *second, unsigned long count)
  {
    return xv::runtime::compare_bytes(first, second, count);
  }
}
