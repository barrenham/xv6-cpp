module;

//
// formatted console output -- printk, panic.
//

#include <stdarg.h>

#include "kernel/support/types.h"
#include "kernel/support/param.h"
#include "kernel/fs/format/layout.h"
#include "kernel/memory/layout/memlayout.h"
#include "kernel/arch/riscv/riscv.h"
#include "kernel/support/defs.h"

module xv.print;

import xv.drivers;
import xv.sync;

extern "C" {

volatile int panicking = 0; // printing a panic message
volatile int panicked = 0;  // spinning forever at end of a panic

// lock to avoid interleaving concurrent printk's.
static constinit struct {
  xv::sync::SpinLock lock;
} pr{};

static char digits[] = "0123456789abcdef";

static void
printint(long long xx, int base, int sign)
{
  char buf[20];
  int i;
  unsigned long long x;

  if (sign && (sign = (xx < 0)))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    xv::drivers::console().put_sync(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  xv::drivers::console().put_sync('0');
  xv::drivers::console().put_sync('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    xv::drivers::console().put_sync(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console.
int
printk(char *fmt, ...)
{
  va_list ap;
  int i, cx, c0, c1, c2;
  char *s;

  if (panicking == 0)
    pr.lock.acquire();

  va_start(ap, fmt);
  for (i = 0; (cx = fmt[i] & 0xff) != 0; i++) {
    if (cx != '%') {
      xv::drivers::console().put_sync(cx);
      continue;
    }
    i++;
    c0 = fmt[i + 0] & 0xff;
    c1 = c2 = 0;
    if (c0)
      c1 = fmt[i + 1] & 0xff;
    if (c1)
      c2 = fmt[i + 2] & 0xff;
    if (c0 == 'd') {
      printint(va_arg(ap, int), 10, 1);
    } else if (c0 == 'l' && c1 == 'd') {
      printint(va_arg(ap, uint64), 10, 1);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'd') {
      printint(va_arg(ap, uint64), 10, 1);
      i += 2;
    } else if (c0 == 'u') {
      printint(va_arg(ap, uint32), 10, 0);
    } else if (c0 == 'l' && c1 == 'u') {
      printint(va_arg(ap, uint64), 10, 0);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'u') {
      printint(va_arg(ap, uint64), 10, 0);
      i += 2;
    } else if (c0 == 'x') {
      printint(va_arg(ap, uint32), 16, 0);
    } else if (c0 == 'l' && c1 == 'x') {
      printint(va_arg(ap, uint64), 16, 0);
      i += 1;
    } else if (c0 == 'l' && c1 == 'l' && c2 == 'x') {
      printint(va_arg(ap, uint64), 16, 0);
      i += 2;
    } else if (c0 == 'p') {
      printptr(va_arg(ap, uint64));
    } else if (c0 == 'c') {
      xv::drivers::console().put_sync(va_arg(ap, uint));
    } else if (c0 == 's') {
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      for (; *s; s++)
        xv::drivers::console().put_sync(*s);
    } else if (c0 == '%') {
      xv::drivers::console().put_sync('%');
    } else if (c0 == 0) {
      break;
    } else {
      // Print unknown % sequence to draw attention.
      xv::drivers::console().put_sync('%');
      xv::drivers::console().put_sync(c0);
    }
  }
  va_end(ap);

  if (panicking == 0)
    pr.lock.release();

  return 0;
}

void
panic(char *s)
{
  panicking = 1;
  printk("panic: ");
  printk("%s\n", s);
  panicked = 1; // freeze uart output from other CPUs
  for (;;)
    ;
}

} // extern "C"

namespace xv::print
{
void initialize()
{
  pr.lock.init("pr");
}
} // namespace xv::print
