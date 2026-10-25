module;

//
// formatted console output -- printk, panic.
//

#include <stdarg.h>

#include "kernel/support/types.h"

module xv.print;

import xv.drivers;
import xv.sync;

namespace xv::print
{
namespace
{

constinit int panic_started = 0;  // printing a panic message
constinit int panic_finished = 0; // spinning forever at end of a panic

// lock to avoid interleaving concurrent printk's.
static constinit struct
{
  xv::sync::SpinLock lock;
} pr{};

static char digits[] = "0123456789abcdef";

static void printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if (sign && (sign = (xx < 0)))
  {
    x = -xx;
  }
  else
  {
    x = xx;
  }

  i = 0;
  do
  {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
  {
    buf[i++] = '-';
  }

  while (--i >= 0)
  {
    xv::drivers::console().put_sync(buf[i]);
  }
}

static void printptr(uint64 x)
{
  int i;
  xv::drivers::console().put_sync('0');
  xv::drivers::console().put_sync('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
  {
    xv::drivers::console().put_sync(digits[x >> (sizeof(uint64) * 8 - 4)]);
  }
}

} // namespace

// Print to the console.
int printk(const char *fmt, ...)
{
  va_list ap;
  int i, cx, c0, c1, c2;
  const char *s;

  if (!panicking())
  {
    pr.lock.acquire();
  }

  va_start(ap, fmt);
  for (i = 0; (cx = fmt[i] & 0xff) != 0; i++)
  {
    if (cx != '%')
    {
      xv::drivers::console().put_sync(cx);
      continue;
    }
    i++;
    c0 = fmt[i + 0] & 0xff;
    c1 = c2 = 0;
    if (c0)
    {
      c1 = fmt[i + 1] & 0xff;
    }
    if (c1)
    {
      c2 = fmt[i + 2] & 0xff;
    }
    if (c0 == 'd')
    {
      printint(va_arg(ap, int), 10, 1);
    }
    else if (c0 == 'l' && c1 == 'd')
    {
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    }
    else if (c0 == 'l' && c1 == 'l' && c2 == 'd')
    {
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    }
    else if (c0 == 'u')
    {
      printint(va_arg(ap, uint32), 10, 0);
    }
    else if (c0 == 'l' && c1 == 'u')
    {
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    }
    else if (c0 == 'l' && c1 == 'l' && c2 == 'u')
    {
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    }
    else if (c0 == 'x')
    {
      printint(va_arg(ap, uint32), 16, 0);
    }
    else if (c0 == 'l' && c1 == 'x')
    {
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    }
    else if (c0 == 'l' && c1 == 'l' && c2 == 'x')
    {
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    }
    else if (c0 == 'p')
    {
      printptr(va_arg(ap, uint64));
    }
    else if (c0 == 'c')
    {
      xv::drivers::console().put_sync(va_arg(ap, uint));
    }
    else if (c0 == 's')
    {
      if ((s = va_arg(ap, const char *)) == 0)
      {
        s = "(null)";
      }
      for (; *s; s++)
      {
        xv::drivers::console().put_sync(*s);
      }
    }
    else if (c0 == '%')
    {
      xv::drivers::console().put_sync('%');
    }
    else if (c0 == 0)
    {
      break;
    }
    else
    {
      // Print unknown % sequence to draw attention.
      xv::drivers::console().put_sync('%');
      xv::drivers::console().put_sync(c0);
    }
  }
  va_end(ap);

  if (!panicking())
  {
    pr.lock.release();
  }

  return 0;
}

void panic(const char *s)
{
  __atomic_store_n(&panic_started, 1, __ATOMIC_RELEASE);
  printk("panic: ");
  printk("%s\n", s);
  __atomic_store_n(&panic_finished, 1,
                   __ATOMIC_RELEASE); // freeze uart output from other CPUs
  for (;;)
    ;
}

bool panicking()
{
  return __atomic_load_n(&panic_started, __ATOMIC_ACQUIRE) != 0;
}

bool panicked()
{
  return __atomic_load_n(&panic_finished, __ATOMIC_ACQUIRE) != 0;
}

void initialize()
{
  pr.lock.init("pr");
}
} // namespace xv::print
