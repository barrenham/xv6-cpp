#include <stddef.h>
void *memset(void *destination, int value, size_t size)
{
  unsigned char *out = destination;
  for (size_t i = 0; i < size; ++i) out[i] = value;
  return destination;
}
void *memcpy(void *destination, const void *source, size_t size)
{
  unsigned char *out = destination;
  const unsigned char *in = source;
  for (size_t i = 0; i < size; ++i) out[i] = in[i];
  return destination;
}
void *memmove(void *destination, const void *source, size_t size)
{
  unsigned char *out = destination;
  const unsigned char *in = source;
  if (out < in) for (size_t i = 0; i < size; ++i) out[i] = in[i];
  else for (size_t i = size; i; --i) out[i - 1] = in[i - 1];
  return destination;
}
