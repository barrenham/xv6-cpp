#pragma once

// Placement construction only: no heap allocation and no hosted <new> header.
#if __STDC_HOSTED__
#include <new>
#else
inline void *operator new(__SIZE_TYPE__, void *storage) noexcept
{
  return storage;
}
#endif
