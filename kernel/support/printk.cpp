module;

//
// formatted console output -- printk, panic.
//

#include <stdarg.h>
#include "kernel/support/flow.h"

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

// Referenced by Integer::run; clangd may parse without instantiating that template.
[[maybe_unused]] static void printint(long long xx, int base, int sign)
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

namespace flow = xv::flow;

struct FormatFlow
{
  struct Context
  {
    const char *format;
    va_list &arguments;
    int index = 0, c0 = 0, c1 = 0, c2 = 0;
  };

  static void literals(Context &c) noexcept
  {
    int ch;
    while ((ch = c.format[c.index] & 0xff) != 0 && ch != '%')
    {
      xv::drivers::console().put_sync(ch);
      ++c.index;
    }
  }

  static bool more(Context &c) noexcept
  {
    return c.format[c.index] != 0;
  }

  static void decode(Context &c) noexcept
  {
    ++c.index;
    c.c0 = c.format[c.index] & 0xff;
    c.c1 = c.c2 = 0;
    if (c.c0)
    {
      c.c1 = c.format[c.index + 1] & 0xff;
    }
    if (c.c1)
    {
      c.c2 = c.format[c.index + 2] & 0xff;
    }
  }

  template <char First, char Second = 0, char Third = 0> struct Match
  {
    static bool test(Context &c) noexcept
    {
      return c.c0 == First && (!Second || c.c1 == Second) && (!Third || c.c2 == Third);
    }
  };

  template <class Value, int Base, bool Signed, unsigned Extra = 0> struct Integer
  {
    static void run(Context &c) noexcept
    {
      printint(va_arg(c.arguments, Value), Base, Signed);
      c.index += Extra;
    }
  };

  static void pointer(Context &c) noexcept
  {
    printptr(va_arg(c.arguments, uint64));
  }

  static void character(Context &c) noexcept
  {
    xv::drivers::console().put_sync(va_arg(c.arguments, uint));
  }

  static void string(Context &c) noexcept
  {
    const char *text = va_arg(c.arguments, const char *);
    if (!text)
    {
      text = "(null)";
    }
    for (; *text; ++text)
    {
      xv::drivers::console().put_sync(*text);
    }
  }

  static void percent(Context &) noexcept
  {
    xv::drivers::console().put_sync('%');
  }

  static bool end(Context &c) noexcept
  {
    return c.c0 == 0;
  }

  static void unknown(Context &c) noexcept
  {
    xv::drivers::console().put_sync('%');
    xv::drivers::console().put_sync(c.c0);
  }

  static void advance(Context &c) noexcept
  {
    ++c.index;
  }
  struct Literal;
  struct Decode;
  struct Advance;
  struct End;
  struct Unknown;
  template <class Id> struct Emit;
  template <class Id, class Predicate, class Action, class Otherwise>
  using Case = flow::types<
      flow::node<Id, flow::noop,
                 flow::branch<Predicate, flow::go<Emit<Id>>, flow::go<Otherwise>>>,
      flow::node<Emit<Id>, Action, flow::go<Advance>>>;
  struct Signed;
  struct LongSigned;
  struct LongLongSigned;
  struct Unsigned;
  struct LongUnsigned;
  struct LongLongUnsigned;
  struct Hex;
  struct LongHex;
  struct LongLongHex;
  struct Pointer;
  struct Character;
  struct String;
  struct Percent;
  using Graph = flow::graph<
      Literal,
      flow::concat<
          flow::types<flow::node<Literal, flow::action<&literals>,
                                 flow::branch<flow::condition<&more>, flow::go<Decode>,
                                              flow::stop>>,
                      flow::node<Decode, flow::action<&decode>, flow::go<Signed>>,
                      flow::node<Advance, flow::action<&advance>, flow::go<Literal>>,
                      flow::node<End, flow::noop,
                                 flow::branch<flow::condition<&end>, flow::stop,
                                              flow::go<Unknown>>>,
                      flow::node<Unknown, flow::action<&unknown>, flow::go<Advance>>>,
          Case<Signed, Match<'d'>, Integer<int, 10, true>, LongSigned>,
          Case<LongSigned, Match<'l', 'd'>, Integer<uint64, 10, true, 1>,
               LongLongSigned>,
          Case<LongLongSigned, Match<'l', 'l', 'd'>, Integer<uint64, 10, true, 2>,
               Unsigned>,
          Case<Unsigned, Match<'u'>, Integer<uint32, 10, false>, LongUnsigned>,
          Case<LongUnsigned, Match<'l', 'u'>, Integer<uint64, 10, false, 1>,
               LongLongUnsigned>,
          Case<LongLongUnsigned, Match<'l', 'l', 'u'>, Integer<uint64, 10, false, 2>,
               Hex>,
          Case<Hex, Match<'x'>, Integer<uint32, 16, false>, LongHex>,
          Case<LongHex, Match<'l', 'x'>, Integer<uint64, 16, false, 1>, LongLongHex>,
          Case<LongLongHex, Match<'l', 'l', 'x'>, Integer<uint64, 16, false, 2>,
               Pointer>,
          Case<Pointer, Match<'p'>, flow::action<&pointer>, Character>,
          Case<Character, Match<'c'>, flow::action<&character>, String>,
          Case<String, Match<'s'>, flow::action<&string>, Percent>,
          Case<Percent, Match<'%'>, flow::action<&percent>, End>>>;
};

} // namespace

// Print to the console.
int printk(const char *fmt, ...)
{
  va_list ap;
  if (!panicking())
  {
    pr.lock.acquire();
  }
  va_start(ap, fmt);
  FormatFlow::Context context{fmt, ap};
  (void)FormatFlow::Graph::run<0>(context);
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
