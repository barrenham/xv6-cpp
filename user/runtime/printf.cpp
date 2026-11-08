#include "kernel/support/types.h"
#include "user/runtime/user.h"
#include <stdarg.h>

namespace
{
// One invocation owns its buffer: concurrent threads and signal handlers do
// not share mutable formatter state. Flush at return, including bare prompts.
class Output final
{
public:

  explicit Output(int fd) : fd_(fd) {
  }

  ~Output() {
    flush();
  }

  void
  put(char value) {
    if (failed_)
    {
      return;
    }
    bytes_[used_++] = value;
    if (used_ == sizeof(bytes_))
    {
      flush();
    }
  }

private:

  void
  flush() {
    unsigned offset = 0;
    while (offset < used_)
    {
      auto count = write(fd_, bytes_ + offset, used_ - offset);
      if (count < 0 && errno == EINTR)
      {
        continue;
      }
      if (count <= 0)
      {
        failed_ = true;
        break;
      }
      offset += static_cast<unsigned>(count);
    }
    used_ = 0;
  }

  int fd_;
  unsigned used_ = 0;
  bool failed_ = false;
  char bytes_[256];
};

void
field(Output &out, const char *text, unsigned length, unsigned width, bool left,
      bool zero = false) {
  unsigned padding = width > length ? width - length : 0;
  unsigned begin = 0;
  if (!left && zero && length && text[0] == '-')
  {
    out.put('-');
    begin = 1;
  }
  if (!left)
  {
    for (unsigned i = 0; i < padding; ++i)
    {
      out.put(zero ? '0' : ' ');
    }
  }
  for (unsigned i = begin; i < length; ++i)
  {
    out.put(text[i]);
  }
  if (left)
  {
    for (unsigned i = 0; i < padding; ++i)
    {
      out.put(' ');
    }
  }
}

void
number(Output &out, uint64 value, bool negative, unsigned base, unsigned width,
       bool left, bool zero) {
  constexpr char digits[] = "0123456789ABCDEF";
  char reversed[24], text[24];
  unsigned count = 0, length = 0;
  do
  {
    reversed[count++] = digits[value % base];
    value /= base;
  } while (value);
  if (negative)
  {
    text[length++] = '-';
  }
  while (count)
  {
    text[length++] = reversed[--count];
  }
  field(out, text, length, width, left, zero);
}
} // namespace

extern "C" void
xv_vprintf_styled(int fd, const char *style, const char *format, va_list arguments) {
  Output out(fd);
  if (style)
  {
    for (const char *p = style; *p; ++p)
    {
      out.put(*p);
    }
  }
  while (*format)
  {
    if (*format != '%')
    {
      out.put(*format++);
      continue;
    }
    ++format;
    bool left = false, zero = false;
    while (*format == '-' || *format == '0')
    {
      if (*format == '-')
      {
        left = true;
      }
      else
      {
        zero = true;
      }
      ++format;
    }
    unsigned width = 0;
    while (*format >= '0' && *format <= '9')
    {
      width = width * 10 + *format++ - '0';
      if (width > 1024)
      {
        width = 1024;
      }
    }
    unsigned longs = 0;
    while (longs < 2 && *format == 'l')
    {
      ++longs;
      ++format;
    }
    char type = *format;
    if (!type)
    {
      break;
    }
    ++format;
    if (type == 'd')
    {
      long long value = longs == 2   ? va_arg(arguments, long long)
                        : longs == 1 ? va_arg(arguments, long)
                                     : va_arg(arguments, int);
      bool negative = value < 0;
      uint64 magnitude = negative ? 0ULL - static_cast<uint64>(value) : value;
      number(out, magnitude, negative, 10, width, left, zero);
    }
    else if (type == 'u' || type == 'x')
    {
      uint64 value = longs == 2   ? va_arg(arguments, unsigned long long)
                     : longs == 1 ? va_arg(arguments, unsigned long)
                                  : va_arg(arguments, uint32);
      number(out, value, false, type == 'u' ? 10 : 16, width, left, zero);
    }
    else if (type == 'p')
    {
      uint64 value = va_arg(arguments, uint64);
      char text[18] = {'0', 'x'};
      constexpr char digits[] = "0123456789ABCDEF";
      for (unsigned i = 0; i < 16; ++i)
      {
        text[2 + i] = digits[(value >> (60 - 4 * i)) & 15];
      }
      field(out, text, sizeof(text), width, left);
    }
    else if (type == 's')
    {
      const char *text = va_arg(arguments, const char *);
      if (!text)
      {
        text = "(null)";
      }
      unsigned length = 0;
      while (text[length])
      {
        ++length;
      }
      field(out, text, length, width, left);
    }
    else if (type == 'c')
    {
      char character = va_arg(arguments, int);
      field(out, &character, 1, width, left);
    }
    else if (type == '%')
    {
      out.put('%');
    }
    else
    {
      out.put('%');
      out.put(type);
    }
  }
  if (style && *style)
  {
    for (const char *p = "\033[0m"; *p; ++p)
    {
      out.put(*p);
    }
  }
}

extern "C" void
vprintf(int fd, const char *format, va_list arguments) {
  xv_vprintf_styled(fd, nullptr, format, arguments);
}

extern "C" void
fprintf(int fd, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  vprintf(fd, format, arguments);
  va_end(arguments);
}

extern "C" void
printf(const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  vprintf(1, format, arguments);
  va_end(arguments);
}
