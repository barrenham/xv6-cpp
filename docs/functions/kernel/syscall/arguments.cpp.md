# `kernel/syscall/arguments.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/arguments.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:arguments`, `xv.process`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::argument_address`](arguments.cpp.md#fn-c4a98f4f697b16cb)
- [`xv::syscall::argument_integer`](arguments.cpp.md#fn-6e984073bc69b0e8)
- [`xv::syscall::argument_string`](arguments.cpp.md#fn-3b44f8e5224d3f08)
- [`xv::syscall::fetch_address`](arguments.cpp.md#fn-212d775f8e97566b)
- [`xv::syscall::fetch_string`](arguments.cpp.md#fn-0b7f374ee7496d9c)
- [`xv::syscall::argument_raw`](arguments.cpp.md#fn-652d4f59369a5ecd)

<a id="fn-212d775f8e97566b"></a>

## `xv::syscall::fetch_address`

```doxygen
@fn int xv::syscall::fetch_address(uint64 addr, uint64 *ip)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：从用户地址空间读取一个地址值。
@param addr 本次操作使用的 addr 值。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `p->memory` → `size` → `copy_in`。
@details 需要特别跟踪的条件：`addr >= p->memory().size() || addr + sizeof(uint64) > p->memory().size()`；`p->memory().copy_in(ip, addr, sizeof(*ip)) != 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0b7f374ee7496d9c"></a>

## `xv::syscall::fetch_string`

```doxygen
@fn int xv::syscall::fetch_string(uint64 addr, char *buf, int max)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：从用户地址空间读取受长度限制的字符串。
@param addr 本次操作使用的 addr 值。
@param buf 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param max 本次操作使用的 最大 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `p->memory` → `copy_string`。
@details 需要特别跟踪的条件：`p->memory().copy_string(buf, addr, max) < 0`；`buf[length]`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`length`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-652d4f59369a5ecd"></a>

## `xv::syscall::argument_raw`

```doxygen
@fn static uint64 xv::syscall::argument_raw(int n)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：从当前用户寄存器读取一个原始系统调用参数。
@param n 本次处理的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `p->registers` → `xv::print::panic`。
@return 各分支实际返回：`p->registers()->a0`；`p->registers()->a1`；`p->registers()->a2`；`p->registers()->a3`；`p->registers()->a4`；`p->registers()->a5`；`-1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6e984073bc69b0e8"></a>

## `xv::syscall::argument_integer`

```doxygen
@fn void xv::syscall::argument_integer(int n, int *ip)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：读取系统调用的整型参数。
@param n 本次处理的数量上限。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_raw`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c4a98f4f697b16cb"></a>

## `xv::syscall::argument_address`

```doxygen
@fn void xv::syscall::argument_address(int n, uint64 *ip)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：读取系统调用的用户地址参数。
@param n 本次处理的数量上限。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_raw`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3b44f8e5224d3f08"></a>

## `xv::syscall::argument_string`

```doxygen
@fn int xv::syscall::argument_string(int n, char *buf, int max)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：复制并验证系统调用字符串参数。
@param n 本次处理的数量上限。
@param buf 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param max 本次操作使用的 最大 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `fetch_string`。
@return 各分支实际返回：`fetch_string(addr, buf, max)`。应结合上面的条件检查区分正常值、空值和错误值。
```
