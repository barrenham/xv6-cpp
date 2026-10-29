# `kernel/syscall/sysproc.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysproc.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`xv.memory`, `xv.arch`, `xv.runtime`, `:arguments`, `:handlers`, `xv.trap`, `xv.process`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::GetGroupsFlow::capacity`](sysproc.cpp.md#fn-a3327362c7faeccb)
- [`xv::syscall::GetGroupsFlow::copy`](sysproc.cpp.md#fn-cbe3b521989823e9)
- [`xv::syscall::GetGroupsFlow::query`](sysproc.cpp.md#fn-2eeefab5a39ca6e6)
- [`xv::syscall::GetGroupsFlow::success`](sysproc.cpp.md#fn-c5bd897c28eae130)
- [`xv::syscall::PgAccessFlow::bounds`](sysproc.cpp.md#fn-2c551d7ba8d68336)
- [`xv::syscall::PgAccessFlow::clear`](sysproc.cpp.md#fn-0d8e3e8993ae2b97)
- [`xv::syscall::PgAccessFlow::copy_result`](sysproc.cpp.md#fn-7d398d1be53676ad)
- [`xv::syscall::PgAccessFlow::demote`](sysproc.cpp.md#fn-3666f2d7f1ee58af)
- [`xv::syscall::PgAccessFlow::output`](sysproc.cpp.md#fn-38d835c9c91fbbd4)
- [`xv::syscall::PgAccessFlow::prepare`](sysproc.cpp.md#fn-dc9a2bc8b631d3c9)
- [`xv::syscall::PgAccessFlow::sample`](sysproc.cpp.md#fn-0d919278228a518d)
- [`xv::syscall::PgAccessFlow::sources`](sysproc.cpp.md#fn-6b82e6da6fa10296)
- [`xv::syscall::SetGroupsFlow::apply`](sysproc.cpp.md#fn-ff45fa0513d870a2)
- [`xv::syscall::SetGroupsFlow::capacity`](sysproc.cpp.md#fn-dfef84e3c2fe7a82)
- [`xv::syscall::SetGroupsFlow::copy`](sysproc.cpp.md#fn-a635e73518ce6011)
- [`xv::syscall::sys_clockticks`](sysproc.cpp.md#fn-42d4b9f1d1ddbf8f)
- [`xv::syscall::sys_cpupin`](handlers.cppm.md#fn-e260733f13c74103)
- [`xv::syscall::sys_exit`](sysproc.cpp.md#fn-407b853e794ab99c)
- [`xv::syscall::sys_fork`](sysproc.cpp.md#fn-c9d853f3df63566a)
- [`xv::syscall::sys_getegid`](sysproc.cpp.md#fn-d68bd62d0fbe3db2)
- [`xv::syscall::sys_geteuid`](sysproc.cpp.md#fn-b909918b519ad4d2)
- [`xv::syscall::sys_getgid`](sysproc.cpp.md#fn-56110f90d793c01f)
- [`xv::syscall::sys_getgroups`](sysproc.cpp.md#fn-b06caf5a4800bafa)
- [`xv::syscall::sys_getpgrp`](sysproc.cpp.md#fn-2edb2a42a8a878d9)
- [`xv::syscall::sys_getpid`](sysproc.cpp.md#fn-77147847a5efd0c1)
- [`xv::syscall::sys_getsid`](sysproc.cpp.md#fn-5321341cbd693195)
- [`xv::syscall::sys_getuid`](sysproc.cpp.md#fn-0fb94103b49fba6f)
- [`xv::syscall::sys_kill`](sysproc.cpp.md#fn-323c866c33cd7f90)
- [`xv::syscall::sys_pause`](sysproc.cpp.md#fn-2e8395597b7be064)
- [`xv::syscall::sys_pgaccess`](sysproc.cpp.md#fn-d0463f44687962b6)
- [`xv::syscall::sys_sbrk`](sysproc.cpp.md#fn-e24ad0a860144ff2)
- [`xv::syscall::sys_setegid`](sysproc.cpp.md#fn-308f76c7fc068944)
- [`xv::syscall::sys_seteuid`](sysproc.cpp.md#fn-29e5718b681685e1)
- [`xv::syscall::sys_setgid`](sysproc.cpp.md#fn-f3114f2d2ce2d746)
- [`xv::syscall::sys_setgroups`](sysproc.cpp.md#fn-08391c17eedcd604)
- [`xv::syscall::sys_setpgid`](sysproc.cpp.md#fn-9ebd01f65216ef04)
- [`xv::syscall::sys_setsid`](sysproc.cpp.md#fn-cd3291a03e4e4fd4)
- [`xv::syscall::sys_setuid`](sysproc.cpp.md#fn-93715c26c01a3131)
- [`xv::syscall::sys_sigaction`](sysproc.cpp.md#fn-661c45c2c27dca21)
- [`xv::syscall::sys_sigdisposition`](sysproc.cpp.md#fn-fe4a961d03b34ad1)
- [`xv::syscall::sys_sigpending`](sysproc.cpp.md#fn-65a6c1417c187dda)
- [`xv::syscall::sys_sigprocmask`](sysproc.cpp.md#fn-950b274504f46ead)
- [`xv::syscall::sys_sigreturn`](sysproc.cpp.md#fn-1964b15046da773a)
- [`xv::syscall::sys_sigsend`](sysproc.cpp.md#fn-166824fa4bf09569)
- [`xv::syscall::sys_sigsuspend`](sysproc.cpp.md#fn-c136e1cfe8a4c0d7)
- [`xv::syscall::sys_umask`](sysproc.cpp.md#fn-898db4e88339d123)
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
@brief 验证访问位采样范围并通过 COW 安全地写回位图。
@details 输出必须已映射且为用户可写或 COW，不能是纯只读或懒空洞；采样前按需拆大页，copyout 使用区域权限并处理 COW。失败不清除输入访问位，成功后清位并刷新翻译。
@details 流程由 PgAccessFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```
<a id="fn-898db4e88339d123"></a>

## `xv::syscall::sys_umask`

```doxygen
@fn uint64 xv::syscall::sys_umask()
@brief 修改当前进程创建掩码。
@details 委托凭据对象设置低九位并返回旧掩码；不改变已有 inode，子进程在 fork 时继承。
```

<a id="fn-08391c17eedcd604"></a>

## `xv::syscall::sys_setgroups`

```doxygen
@fn uint64 xv::syscall::sys_setgroups()
@brief 从用户内存复制并替换附加组。
@details 校验数量 0..16，先安全 copy_in 到内核固定数组，再由凭据对象执行 root 检查与整体替换。坏指针或无权限返回 -1。
@details 流程由 SetGroupsFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-b06caf5a4800bafa"></a>

## `xv::syscall::sys_getgroups`

```doxygen
@fn uint64 xv::syscall::sys_getgroups()
@brief 读取附加组数量或列表。
@details count=0 返回当前组数；容量不足、负数或用户复制失败返回 -1。一次凭据快照保证数量与数组来自同一状态。
@details 流程由 GetGroupsFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-308f76c7fc068944"></a>

## `xv::syscall::sys_setegid`

```doxygen
@fn uint64 xv::syscall::sys_setegid()
@brief 修改当前进程有效 GID。
@details 整数参数由 Credentials::set_gid 验证，effective_only=true；真实和保存组保持，非 root 只能选真实或保存组。
```

<a id="fn-f3114f2d2ce2d746"></a>

## `xv::syscall::sys_setgid`

```doxygen
@fn uint64 xv::syscall::sys_setgid()
@brief 修改当前进程 GID。
@details root 同时改真实、有效和保存组；非 root 只能在真实/保存组之间改变有效组，附加组列表保持。
```

<a id="fn-29e5718b681685e1"></a>

## `xv::syscall::sys_seteuid`

```doxygen
@fn uint64 xv::syscall::sys_seteuid()
@brief 修改当前进程有效 UID。
@details 仅改变有效身份；root 或真实/保存 UID 可用，保存 root 身份允许显式恢复。完整放弃特权应使用 setuid。
```

<a id="fn-93715c26c01a3131"></a>

## `xv::syscall::sys_setuid`

```doxygen
@fn uint64 xv::syscall::sys_setuid()
@brief 修改当前进程 UID。
@details root 调用时同时改变真实、有效、保存 UID，实现不可逆降权；非 root 不可任意取得其他 UID。
```

<a id="fn-d68bd62d0fbe3db2"></a>

## `xv::syscall::sys_getegid`

```doxygen
@fn uint64 xv::syscall::sys_getegid()
@brief 取得当前有效 GID。
@details 返回凭据快照中的 effective_gid；与 getgid 的真实组含义不同。
```

<a id="fn-56110f90d793c01f"></a>

## `xv::syscall::sys_getgid`

```doxygen
@fn uint64 xv::syscall::sys_getgid()
@brief 取得当前真实 GID。
@details 返回凭据快照中的 gid，不查询用户态账户文件。
```

<a id="fn-b909918b519ad4d2"></a>

## `xv::syscall::sys_geteuid`

```doxygen
@fn uint64 xv::syscall::sys_geteuid()
@brief 取得当前有效 UID。
@details 返回凭据快照中的 effective_uid，文件访问和管理操作用它授权。
```

<a id="fn-0fb94103b49fba6f"></a>

## `xv::syscall::sys_getuid`

```doxygen
@fn uint64 xv::syscall::sys_getuid()
@brief 取得当前真实 UID。
@details 返回凭据快照中的 uid；账户名称由用户态 passwd 解析，不由内核管理。
```

<a id="fn-42d4b9f1d1ddbf8f"></a>

## `xv::syscall::sys_clockticks`

```doxygen
@fn uint64 xv::syscall::sys_clockticks()
@brief 读取 RISC-V 单调硬件时间计数。
@details 在内核上下文执行 rdtime，并原样返回 64 位计数；不是墙上时间，也不是随机数。用户态通过 clockticks 包装调用。
@return 硬件 time 计数值，单位取决于平台时基。
```

<a id="fn-2c551d7ba8d68336"></a>

## `xv::syscall::PgAccessFlow::bounds`

```doxygen
@fn static bool xv::syscall::PgAccessFlow::bounds(Context &c) noexcept
@brief 检查页数和地址上界并对齐采样起点。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0d8e3e8993ae2b97"></a>

## `xv::syscall::PgAccessFlow::clear`

```doxygen
@fn static void xv::syscall::PgAccessFlow::clear(Context &c) noexcept
@brief 复制成功后清除采样页访问位并刷新地址转换。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7d398d1be53676ad"></a>

## `xv::syscall::PgAccessFlow::copy_result`

```doxygen
@fn static bool xv::syscall::PgAccessFlow::copy_result(Context &c) noexcept
@brief 复制完整位图给用户并报告结果。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3666f2d7f1ee58af"></a>

## `xv::syscall::PgAccessFlow::demote`

```doxygen
@fn static bool xv::syscall::PgAccessFlow::demote(Context &c) noexcept
@brief 将采样范围内超页按原策略降级为单页映射。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-38d835c9c91fbbd4"></a>

## `xv::syscall::PgAccessFlow::output`

```doxygen
@fn static bool xv::syscall::PgAccessFlow::output(Context &c) noexcept
@brief 完整检查输出范围内用户可写或 COW 映射。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dc9a2bc8b631d3c9"></a>

## `xv::syscall::PgAccessFlow::prepare`

```doxygen
@fn static void xv::syscall::PgAccessFlow::prepare(Context &c) noexcept
@brief 计算位图字节数并清零实际使用部分。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0d919278228a518d"></a>

## `xv::syscall::PgAccessFlow::sample`

```doxygen
@fn static void xv::syscall::PgAccessFlow::sample(Context &c) noexcept
@brief 采集各页硬件访问位到本次位图。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6b82e6da6fa10296"></a>

## `xv::syscall::PgAccessFlow::sources`

```doxygen
@fn static bool xv::syscall::PgAccessFlow::sources(Context &c) noexcept
@brief 完整检查采样范围内所有用户叶映射。
@details 保持范围验证、输出映射验证、超页降级、采样、复制和最后清位的顺序；失败不清访问位，固定页扫描为直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a3327362c7faeccb"></a>

## `xv::syscall::GetGroupsFlow::capacity`

```doxygen
@fn static bool xv::syscall::GetGroupsFlow::capacity(Context &c) noexcept
@brief 检查非查询输出容量是否覆盖快照中的全部组。
@details 凭据快照在入口参数读取后取得；零数量直接查询，容量不足不触及输出，空组集合不复制用户内存。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cbe3b521989823e9"></a>

## `xv::syscall::GetGroupsFlow::copy`

```doxygen
@fn static bool xv::syscall::GetGroupsFlow::copy(Context &c) noexcept
@brief 仅非空组集合复制到用户地址并报告结果。
@details 凭据快照在入口参数读取后取得；零数量直接查询，容量不足不触及输出，空组集合不复制用户内存。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2eeefab5a39ca6e6"></a>

## `xv::syscall::GetGroupsFlow::query`

```doxygen
@fn static bool xv::syscall::GetGroupsFlow::query(Context &c) noexcept { return c.count == 0; }
@brief 检查是否只查询补充组数量。
@details 凭据快照在入口参数读取后取得；零数量直接查询，容量不足不触及输出，空组集合不复制用户内存。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c5bd897c28eae130"></a>

## `xv::syscall::GetGroupsFlow::success`

```doxygen
@fn static void xv::syscall::GetGroupsFlow::success(Context &c) noexcept { c.result = c.state.group_count; }
@brief 记录凭据快照中的补充组数量。
@details 凭据快照在入口参数读取后取得；零数量直接查询，容量不足不触及输出，空组集合不复制用户内存。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ff45fa0513d870a2"></a>

## `xv::syscall::SetGroupsFlow::apply`

```doxygen
@fn static void xv::syscall::SetGroupsFlow::apply(Context &c) noexcept
@brief 调用原凭据组提交策略并保存业务返回值。
@details 固定数组留在原调用帧且不清零；先验证数量和复制输入，后交由原凭据业务检查权限并提交，零数量跳过指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dfef84e3c2fe7a82"></a>

## `xv::syscall::SetGroupsFlow::capacity`

```doxygen
@fn static bool xv::syscall::SetGroupsFlow::capacity(Context &c) noexcept { return c.count >= 0 && c.count <= 16; }
@brief 检查补充组数量是否处于零到十六。
@details 固定数组留在原调用帧且不清零；先验证数量和复制输入，后交由原凭据业务检查权限并提交，零数量跳过指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a635e73518ce6011"></a>

## `xv::syscall::SetGroupsFlow::copy`

```doxygen
@fn static bool xv::syscall::SetGroupsFlow::copy(Context &c) noexcept
@brief 仅非零数量从用户复制组数组。
@details 固定数组留在原调用帧且不清零；先验证数量和复制输入，后交由原凭据业务检查权限并提交，零数量跳过指针。 下一节点由图策略声明，业务结果与控制信号分离。
```
