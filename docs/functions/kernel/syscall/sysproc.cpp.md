# `kernel/syscall/sysproc.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysproc.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`xv.memory`, `xv.arch`, `xv.runtime`, `:arguments`, `:handlers`, `xv.trap`, `xv.process`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::sys_cpupin`](handlers.cppm.md#fn-e260733f13c74103)
- [`xv::syscall::sys_exit`](sysproc.cpp.md#fn-407b853e794ab99c)
- [`xv::syscall::sys_fork`](sysproc.cpp.md#fn-c9d853f3df63566a)
- [`xv::syscall::sys_getpgrp`](sysproc.cpp.md#fn-2edb2a42a8a878d9)
- [`xv::syscall::sys_getpid`](sysproc.cpp.md#fn-77147847a5efd0c1)
- [`xv::syscall::sys_getsid`](sysproc.cpp.md#fn-5321341cbd693195)
- [`xv::syscall::sys_kill`](sysproc.cpp.md#fn-323c866c33cd7f90)
- [`xv::syscall::sys_pause`](sysproc.cpp.md#fn-2e8395597b7be064)
- [`xv::syscall::sys_pgaccess`](sysproc.cpp.md#fn-d0463f44687962b6)
- [`xv::syscall::sys_sbrk`](sysproc.cpp.md#fn-e24ad0a860144ff2)
- [`xv::syscall::sys_setpgid`](sysproc.cpp.md#fn-9ebd01f65216ef04)
- [`xv::syscall::sys_setsid`](sysproc.cpp.md#fn-cd3291a03e4e4fd4)
- [`xv::syscall::sys_sigaction`](sysproc.cpp.md#fn-661c45c2c27dca21)
- [`xv::syscall::sys_sigdisposition`](sysproc.cpp.md#fn-fe4a961d03b34ad1)
- [`xv::syscall::sys_sigpending`](sysproc.cpp.md#fn-65a6c1417c187dda)
- [`xv::syscall::sys_sigprocmask`](sysproc.cpp.md#fn-950b274504f46ead)
- [`xv::syscall::sys_sigreturn`](sysproc.cpp.md#fn-1964b15046da773a)
- [`xv::syscall::sys_sigsend`](sysproc.cpp.md#fn-166824fa4bf09569)
- [`xv::syscall::sys_sigsuspend`](sysproc.cpp.md#fn-c136e1cfe8a4c0d7)
- [`xv::syscall::sys_uptime`](sysproc.cpp.md#fn-d5a85217c5ec8031)
- [`xv::syscall::sys_vmprint`](sysproc.cpp.md#fn-b43c13f418afd200)
- [`xv::syscall::sys_wait`](sysproc.cpp.md#fn-0e651ab6af812b15)
- [`xv::syscall::sys_waitpid`](sysproc.cpp.md#fn-47570f7534c0492e)

<a id="fn-c9d853f3df63566a"></a>

## `xv::syscall::sys_fork`

```doxygen
@fn uint64 xv::syscall::sys_fork(void)
@brief 系统调用入口：fork；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::fork_process`。
@return 各分支实际返回：`xv::process::fork_process()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-407b853e794ab99c"></a>

## `xv::syscall::sys_exit`

```doxygen
@fn uint64 xv::syscall::sys_exit(void)
@brief 系统调用入口：退出；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::exit_process`。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-0e651ab6af812b15"></a>

## `xv::syscall::sys_wait`

```doxygen
@fn uint64 xv::syscall::sys_wait(void)
@brief 系统调用入口：等待；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `xv::process::wait`。
@return 各分支实际返回：`xv::process::wait(p)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-47570f7534c0492e"></a>

## `xv::syscall::sys_waitpid`

```doxygen
@fn uint64 xv::syscall::sys_waitpid(void)
@brief 系统调用入口：waitpid；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argument_address` → `xv::process::wait_for_child`。
@return 各分支实际返回：`xv::process::wait_for_child(pid, address, options)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-323c866c33cd7f90"></a>

## `xv::syscall::sys_kill`

```doxygen
@fn uint64 xv::syscall::sys_kill(void)
@brief 系统调用入口：终止请求；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::kill`。
@return 各分支实际返回：`xv::process::kill(pid)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-77147847a5efd0c1"></a>

## `xv::syscall::sys_getpid`

```doxygen
@fn uint64 xv::syscall::sys_getpid(void)
@brief 系统调用入口：getpid；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `id`。
@return 各分支实际返回：`xv::process::current()->id()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-e24ad0a860144ff2"></a>

## `xv::syscall::sys_sbrk`

```doxygen
@fn uint64 xv::syscall::sys_sbrk(void)
@brief 系统调用入口：sbrk；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::current` → `memory` → `size` → `resize`。
@details 需要特别跟踪的条件：`xv::process::current()->memory().resize(n, t == SBRK_EAGER || n < 0) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`addr`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-2e8395597b7be064"></a>

## `xv::syscall::sys_pause`

```doxygen
@fn uint64 xv::syscall::sys_pause(void)
@brief 系统调用入口：等待；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::trap::clock` → `pause`。
@return 各分支实际返回：`xv::trap::clock().pause(count)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-d5a85217c5ec8031"></a>

## `xv::syscall::sys_uptime`

```doxygen
@fn uint64 xv::syscall::sys_uptime(void)
@brief 系统调用入口：运行 tick 数；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::trap::clock` → `uptime`。
@return 各分支实际返回：`xv::trap::clock().uptime()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-2edb2a42a8a878d9"></a>

## `xv::syscall::sys_getpgrp`

```doxygen
@fn uint64 xv::syscall::sys_getpgrp(void)
@brief 系统调用入口：getpgrp；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::get_process_group`。
@return 各分支实际返回：`xv::process::get_process_group()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-9ebd01f65216ef04"></a>

## `xv::syscall::sys_setpgid`

```doxygen
@fn uint64 xv::syscall::sys_setpgid(void)
@brief 系统调用入口：setpgid；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::set_process_group`。
@return 各分支实际返回：`xv::process::set_process_group(pid, group)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-5321341cbd693195"></a>

## `xv::syscall::sys_getsid`

```doxygen
@fn uint64 xv::syscall::sys_getsid(void)
@brief 系统调用入口：getsid；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::get_session`。
@return 各分支实际返回：`xv::process::get_session(pid)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-cd3291a03e4e4fd4"></a>

## `xv::syscall::sys_setsid`

```doxygen
@fn uint64 xv::syscall::sys_setsid(void)
@brief 系统调用入口：setsid；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::create_session`。
@return 各分支实际返回：`xv::process::create_session()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-661c45c2c27dca21"></a>

## `xv::syscall::sys_sigaction`

```doxygen
@fn uint64 xv::syscall::sys_sigaction()
@brief 系统调用入口：sigaction；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argument_address` → `xv::process::current` → `signal_action`。
@return 各分支实际返回：`xv::process::current()->signal_action(signal, action, old, restorer)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-950b274504f46ead"></a>

## `xv::syscall::sys_sigprocmask`

```doxygen
@fn uint64 xv::syscall::sys_sigprocmask()
@brief 系统调用入口：sigprocmask；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `argument_address` → `xv::process::current` → `signal_mask`。
@return 各分支实际返回：`xv::process::current()->signal_mask(how, set, old)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-65a6c1417c187dda"></a>

## `xv::syscall::sys_sigpending`

```doxygen
@fn uint64 xv::syscall::sys_sigpending()
@brief 系统调用入口：sigpending；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `xv::process::current` → `signal_pending`。
@return 各分支实际返回：`xv::process::current()->signal_pending(set)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-c136e1cfe8a4c0d7"></a>

## `xv::syscall::sys_sigsuspend`

```doxygen
@fn uint64 xv::syscall::sys_sigsuspend()
@brief 系统调用入口：sigsuspend；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `xv::process::current` → `signal_suspend`。
@return 各分支实际返回：`xv::process::current()->signal_suspend(set)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-1964b15046da773a"></a>

## `xv::syscall::sys_sigreturn`

```doxygen
@fn uint64 xv::syscall::sys_sigreturn()
@brief 系统调用入口：sigreturn；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `signal_return`。
@return 各分支实际返回：`xv::process::current()->signal_return()`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-166824fa4bf09569"></a>

## `xv::syscall::sys_sigsend`

```doxygen
@fn uint64 xv::syscall::sys_sigsend(void)
@brief 系统调用入口：sigsend；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::send_signal`。
@return 各分支实际返回：`xv::process::send_signal(target, signal)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-fe4a961d03b34ad1"></a>

## `xv::syscall::sys_sigdisposition`

```doxygen
@fn uint64 xv::syscall::sys_sigdisposition(void)
@brief 系统调用入口：sigdisposition；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_integer` → `xv::process::set_signal_disposition`。
@return 各分支实际返回：`xv::process::set_signal_disposition(signal, action)`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-b43c13f418afd200"></a>

## `xv::syscall::sys_vmprint`

```doxygen
@fn uint64 xv::syscall::sys_vmprint(void)
@brief 系统调用入口：vmprint；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `xv::memory::print_page_table` → `p->memory` → `page_table`。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-d0463f44687962b6"></a>

## `xv::syscall::sys_pgaccess`

```doxygen
@fn uint64 xv::syscall::sys_pgaccess(void)
@brief 系统调用入口：pgaccess；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `argument_address` → `argument_integer` → `PGROUNDDOWN` → `xv::runtime::fill` → `xv::memory::walk_entry` → `p->memory` → `page_table` → `PTE_LEAF` → `xv::memory::demote_user_pages` → `xv::memory::copy_to_user` → `size`。
@details 需要特别跟踪的条件：`npages <= 0 || npages > MAXPAGES || base >= MAXVA`；`(uint64)npages > (MAXVA - base) / PGSIZE`；`pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U) || !PTE_LEAF(*pte)`；`out >= MAXVA || (uint64)nbytes > MAXVA - out`；`pte == 0 || (*pte & (PTE_V | PTE_U | PTE_W)) != (PTE_V | PTE_U | PTE_W)`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```
