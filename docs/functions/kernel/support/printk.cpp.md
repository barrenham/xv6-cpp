# `kernel/support/printk.cpp`

开发职责：freestanding 运行时、字节操作和诊断输出。

源码：[打开实际文件](../../../../kernel/support/printk.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.print`。

直接模块依赖：`xv.drivers`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::print::FormatFlow::Integer::run`](printk.cpp.md#fn-e77ca6ab707d2c68)
- [`xv::print::FormatFlow::Match::test`](printk.cpp.md#fn-a35a5e3c6ff8001b)
- [`xv::print::FormatFlow::advance`](printk.cpp.md#fn-7cfb0771e8661ecc)
- [`xv::print::FormatFlow::character`](printk.cpp.md#fn-dcf1e827e0d7965c)
- [`xv::print::FormatFlow::decode`](printk.cpp.md#fn-833618c786d9fd04)
- [`xv::print::FormatFlow::end`](printk.cpp.md#fn-f90f6bbe10f996f1)
- [`xv::print::FormatFlow::literals`](printk.cpp.md#fn-aaf5209e6f6fc560)
- [`xv::print::FormatFlow::more`](printk.cpp.md#fn-1aa2e43ec25fc726)
- [`xv::print::FormatFlow::percent`](printk.cpp.md#fn-1ad4e04cb85954cd)
- [`xv::print::FormatFlow::pointer`](printk.cpp.md#fn-a6b040179a3a6eef)
- [`xv::print::FormatFlow::string`](printk.cpp.md#fn-58cbe91201d89954)
- [`xv::print::FormatFlow::unknown`](printk.cpp.md#fn-ea48ac9616fc7bcb)
- [`xv::print::initialize`](printk.cpp.md#fn-8b793d2d14471e29)
- [`xv::print::panic`](printk.cpp.md#fn-f516e133f69f2bd6)
- [`xv::print::panicked`](printk.cpp.md#fn-98ce96df281386b6)
- [`xv::print::panicking`](printk.cpp.md#fn-4b43183d43eaac56)
- [`xv::print::printint`](printk.cpp.md#fn-a15fb561d2af0a99)
- [`xv::print::printk`](printk.cpp.md#fn-87cc2e541b7ec985)
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
@note 源码中出现的锁操作：`pr.lock.acquire` → `pr.lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 编排由 FormatFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
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

<a id="fn-7cfb0771e8661ecc"></a>

## `xv::print::FormatFlow::advance`

```doxygen
@fn static void xv::print::FormatFlow::advance(Context &c) noexcept
@brief 推进到当前格式指令之后。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-dcf1e827e0d7965c"></a>

## `xv::print::FormatFlow::character`

```doxygen
@fn static void xv::print::FormatFlow::character(Context &c) noexcept
@brief 读取并输出字符参数。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-833618c786d9fd04"></a>

## `xv::print::FormatFlow::decode`

```doxygen
@fn static void xv::print::FormatFlow::decode(Context &c) noexcept
@brief 读取当前格式字符及最多两个安全前瞻字符。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f90f6bbe10f996f1"></a>

## `xv::print::FormatFlow::end`

```doxygen
@fn static bool xv::print::FormatFlow::end(Context &c) noexcept
@brief 检查当前百分号后是否为终止零。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-aaf5209e6f6fc560"></a>

## `xv::print::FormatFlow::literals`

```doxygen
@fn static void xv::print::FormatFlow::literals(Context &c) noexcept
@brief 直接输出连续普通字符直到百分号或终止零。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1aa2e43ec25fc726"></a>

## `xv::print::FormatFlow::more`

```doxygen
@fn static bool xv::print::FormatFlow::more(Context &c) noexcept
@brief 检查格式串是否仍有待解码指令。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1ad4e04cb85954cd"></a>

## `xv::print::FormatFlow::percent`

```doxygen
@fn static void xv::print::FormatFlow::percent(Context &) noexcept
@brief 输出一个百分号。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a6b040179a3a6eef"></a>

## `xv::print::FormatFlow::pointer`

```doxygen
@fn static void xv::print::FormatFlow::pointer(Context &c) noexcept
@brief 读取指针参数并输出固定宽度十六进制。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-58cbe91201d89954"></a>

## `xv::print::FormatFlow::string`

```doxygen
@fn static void xv::print::FormatFlow::string(Context &c) noexcept
@brief 读取字符串参数，以 null 文本替代空指针并直接循环输出。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-ea48ac9616fc7bcb"></a>

## `xv::print::FormatFlow::unknown`

```doxygen
@fn static void xv::print::FormatFlow::unknown(Context &c) noexcept
@brief 输出百分号和未知格式字符。
@details va_list 由 printk 在当前调用帧中启动并结束，Context 仅引用游标；普通字符和字符串保持直接循环输出，格式分支顺序保持原实现。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a35a5e3c6ff8001b"></a>

## `xv::print::FormatFlow::Match::test`

```doxygen
@fn static bool xv::print::FormatFlow::Match::test(Context &c) noexcept
@brief 检查解码字符是否匹配当前模板格式。
@details 模板字符声明一种格式匹配；只检查模板实际要求的前瞻位置。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e77ca6ab707d2c68"></a>

## `xv::print::FormatFlow::Integer::run`

```doxygen
@fn static void xv::print::FormatFlow::Integer::run(Context &c) noexcept
@brief 读取整数参数，调用原整数输出原语并推进额外格式字符。
@details 模板声明 va_arg 实际读取类型、进制、符号策略和额外消耗的格式长度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
