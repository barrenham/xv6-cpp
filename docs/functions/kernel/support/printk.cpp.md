# `kernel/support/printk.cpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/printk.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.print`。

直接模块依赖：`xv.drivers`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::print::initialize`](printk.cpp.md#fn-8b793d2d14471e29)
- [`xv::print::panic`](printk.cpp.md#fn-f516e133f69f2bd6)
- [`xv::print::panicked`](printk.cpp.md#fn-98ce96df281386b6)
- [`xv::print::panicking`](printk.cpp.md#fn-4b43183d43eaac56)
- [`xv::print::printk`](printk.cpp.md#fn-87cc2e541b7ec985)
- [`xv::print::printint`](printk.cpp.md#fn-a15fb561d2af0a99)
- [`xv::print::printptr`](printk.cpp.md#fn-8af32894fdb1b77b)

<a id="fn-8b793d2d14471e29"></a>

## `xv::print::initialize`

```doxygen
@fn void xv::print::initialize()
@brief freestanding 运行时、字节操作和诊断输出：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pr.lock.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-87cc2e541b7ec985"></a>

## `xv::print::printk`

```doxygen
@fn int xv::print::printk(const char *fmt, ...)
@brief freestanding 运行时、字节操作和诊断输出：按格式输出内核诊断文本。
@param fmt 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`panicking` → `pr.lock.acquire` → `va_start` → `xv::drivers::console` → `put_sync` → `printint` → `va_arg` → `printptr` → `va_end` → `pr.lock.release`。
@details 需要特别跟踪的条件：`!panicking()`；`cx != '%'`；`c0`；`c1`；`c0 == 'd'`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`pr.lock.acquire` → `pr.lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f516e133f69f2bd6"></a>

## `xv::print::panic`

```doxygen
@fn void xv::print::panic(const char *s)
@brief freestanding 运行时、字节操作和诊断输出：报告不可恢复错误并进入 panic 路径。
@param s 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n` → `printk`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```

<a id="fn-4b43183d43eaac56"></a>

## `xv::print::panicking`

```doxygen
@fn bool xv::print::panicking()
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `__atomic_load_n(&panic_started, __ATOMIC_ACQUIRE) != 0`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&panic_started, __ATOMIC_ACQUIRE) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-98ce96df281386b6"></a>

## `xv::print::panicked`

```doxygen
@fn bool xv::print::panicked()
@brief freestanding 运行时、字节操作和诊断输出：读取或计算 `__atomic_load_n(&panic_finished, __ATOMIC_ACQUIRE) != 0`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&panic_finished, __ATOMIC_ACQUIRE) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a15fb561d2af0a99"></a>

## `xv::print::printint`

```doxygen
@fn static void xv::print::printint(long long xx, int base, int sign)
@brief freestanding 运行时、字节操作和诊断输出：printint。
@param xx 本次操作使用的 xx 值。
@param base 本次操作使用的 base 值。
@param sign 本次操作使用的 sign 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::drivers::console` → `put_sync`。
@details 本函数可见的状态更新包括：`buf[i++] = digits[x % base];`；`buf[i++] = '-';`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`sign && (sign = (xx < 0))`；`(x /= base) != 0`；`sign`；`--i >= 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8af32894fdb1b77b"></a>

## `xv::print::printptr`

```doxygen
@fn static void xv::print::printptr(uint64 x)
@brief freestanding 运行时、字节操作和诊断输出：printptr。
@param x 本次操作使用的 x 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::drivers::console` → `put_sync`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
