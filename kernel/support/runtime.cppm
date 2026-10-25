export module xv.runtime;

export namespace xv::runtime
{
void *fill(void *destination, int value, unsigned long count);
void *move(void *destination, const void *source, unsigned long count);
int compare_bytes(const void *first, const void *second, unsigned long count);
int compare_text(const char *first, const char *second, unsigned long count);
char *copy_fixed(char *destination, const char *source, int capacity);
char *copy_string(char *destination, const char *source, int capacity);
int length(const char *text);

template <typename T, unsigned long Count>
constexpr unsigned long array_size(const T (&)[Count])
{
  return Count;
}
} // namespace xv::runtime
