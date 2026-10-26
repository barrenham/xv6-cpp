# `kernel/process/lifecycle/service.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.runtime`, `:manager`, `:paged_store`, `:fifo_queue`, `:round_robin`, `:wait_channels`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::claim_terminal`](service.cpp.md#fn-4884f3bab775fd24)
- [`xv::process::copy_in`](service.cpp.md#fn-d6c3ce5695ec8c2b)
- [`xv::process::copy_out`](service.cpp.md#fn-393041135005f370)
- [`xv::process::create_initial_process`](service.cpp.md#fn-b7634e2bbca815d9)
- [`xv::process::create_session`](service.cpp.md#fn-88fb19488751f21e)
- [`xv::process::deliver_signals`](service.cpp.md#fn-86b6b47851577869)
- [`xv::process::detach_terminal`](service.cpp.md#fn-7ada75984ab49466)
- [`xv::process::dump`](service.cpp.md#fn-e42abc0ba076c8f0)
- [`xv::process::exit_process`](../service.md#service-exit-process)
- [`xv::process::fork_process`](../service.md#service-fork-process)
- [`xv::process::get_process_group`](service.cpp.md#fn-c482d8945fdaf0ab)
- [`xv::process::get_session`](service.cpp.md#fn-39121e4d5139c870)
- [`xv::process::initialize`](../service.md#service-initialize)
- [`xv::process::kill`](service.cpp.md#fn-42491b6e530470a7)
- [`xv::process::make_poll_wait`](service.cpp.md#fn-c10951196ee0a808)
- [`xv::process::may_read_terminal`](service.cpp.md#fn-a0836e0fefde5729)
- [`xv::process::poll_timer_tick`](service.cpp.md#fn-22490dbefd1fb8c0)
- [`xv::process::post_terminal_interrupt`](service.cpp.md#fn-b12e280ce26670d8)
- [`xv::process::scheduler`](service.cpp.md#fn-7abeaa33d65489ea)
- [`xv::process::send_signal`](service.cpp.md#fn-ac942543753e7e75)
- [`xv::process::service_signal_notifications`](service.cpp.md#fn-bdb81b83086795f2)
- [`xv::process::set_process_group`](service.cpp.md#fn-34091ebc76bb444c)
- [`xv::process::set_signal_disposition`](service.cpp.md#fn-bb3030848b31cf05)
- [`xv::process::set_terminal_foreground`](service.cpp.md#fn-2684c802094d44d8)
- [`xv::process::snapshot_after`](service.cpp.md#fn-4f4ab884257688de)
- [`xv::process::terminal_foreground`](service.cpp.md#fn-9d295666c0ed021d)
- [`xv::process::wait`](../service.md#service-wait)
- [`xv::process::wait_channels`](service.cpp.md#fn-00fe184141e83bc3)
- [`xv::process::wait_for_child`](../service.md#service-wait-for-child)
- [`xv::process::wakeup`](../service.md#service-wakeup)

<a id="fn-00fe184141e83bc3"></a>

## `xv::process::wait_channels`

```doxygen
@fn WaitChannels &xv::process::wait_channels()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `waits`，返回当前查询结果。
@return 各分支实际返回：`waits`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7abeaa33d65489ea"></a>

## `xv::process::scheduler`

```doxygen
@fn Scheduler &xv::process::scheduler()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `active_scheduler`，返回当前查询结果。
@return 各分支实际返回：`active_scheduler`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b7634e2bbca815d9"></a>

## `xv::process::create_initial_process`

```doxygen
@fn void xv::process::create_initial_process()
@brief 进程对象生命周期、组身份、调度、等待与信号：创建initial进程。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.create_initial_process`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-42491b6e530470a7"></a>

## `xv::process::kill`

```doxygen
@fn int xv::process::kill(int pid)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.kill(pid)`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.kill`。
@return 各分支实际返回：`process_manager.kill(pid)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ac942543753e7e75"></a>

## `xv::process::send_signal`

```doxygen
@fn int xv::process::send_signal(int target, int signal)
@brief 进程对象生命周期、组身份、调度、等待与信号：按 PID 或进程组选择目标并登记信号。
@param target 本次操作使用的 target 值。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.send_signal`。
@return 各分支实际返回：`process_manager.send_signal(target, signal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-86b6b47851577869"></a>

## `xv::process::deliver_signals`

```doxygen
@fn void xv::process::deliver_signals()
@brief 进程对象生命周期、组身份、调度、等待与信号：在安全返回边界执行默认动作或建立用户信号帧。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.deliver_signals`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bb3030848b31cf05"></a>

## `xv::process::set_signal_disposition`

```doxygen
@fn int xv::process::set_signal_disposition(int signal, int action)
@brief 进程对象生命周期、组身份、调度、等待与信号：安装兼容的默认或忽略信号处置。
@param signal 待查询、投递或处理的信号编号。
@param action 本次操作使用的 动作 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.set_signal_disposition`。
@return 各分支实际返回：`process_manager.set_signal_disposition(signal, action)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b12e280ce26670d8"></a>

## `xv::process::post_terminal_interrupt`

```doxygen
@fn void xv::process::post_terminal_interrupt(void *terminal, int signal)
@brief 进程对象生命周期、组身份、调度、等待与信号：登记终端中断。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.post_terminal_interrupt`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bdb81b83086795f2"></a>

## `xv::process::service_signal_notifications`

```doxygen
@fn void xv::process::service_signal_notifications()
@brief 进程对象生命周期、组身份、调度、等待与信号：处理信号notifications。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.service_signal_notifications`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-7ada75984ab49466"></a>

## `xv::process::detach_terminal`

```doxygen
@fn void xv::process::detach_terminal(void *terminal)
@brief 进程对象生命周期、组身份、调度、等待与信号：解除关联终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.detach_terminal`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4884f3bab775fd24"></a>

## `xv::process::claim_terminal`

```doxygen
@fn int xv::process::claim_terminal(void *terminal)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.claim_terminal(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.claim_terminal`。
@return 各分支实际返回：`process_manager.claim_terminal(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9d295666c0ed021d"></a>

## `xv::process::terminal_foreground`

```doxygen
@fn int xv::process::terminal_foreground(void *terminal)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.terminal_foreground(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.terminal_foreground`。
@return 各分支实际返回：`process_manager.terminal_foreground(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2684c802094d44d8"></a>

## `xv::process::set_terminal_foreground`

```doxygen
@fn int xv::process::set_terminal_foreground(void *terminal, int group)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.set_terminal_foreground(terminal, group)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param group 本次操作使用的 进程组 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.set_terminal_foreground`。
@return 各分支实际返回：`process_manager.set_terminal_foreground(terminal, group)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a0836e0fefde5729"></a>

## `xv::process::may_read_terminal`

```doxygen
@fn bool xv::process::may_read_terminal(void *terminal)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.may_read_terminal(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.may_read_terminal`。
@return 各分支实际返回：`process_manager.may_read_terminal(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c482d8945fdaf0ab"></a>

## `xv::process::get_process_group`

```doxygen
@fn int xv::process::get_process_group()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.get_process_group()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.get_process_group`。
@return 各分支实际返回：`process_manager.get_process_group()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-34091ebc76bb444c"></a>

## `xv::process::set_process_group`

```doxygen
@fn int xv::process::set_process_group(int pid, int group)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.set_process_group(pid, group)`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param group 本次操作使用的 进程组 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.set_process_group`。
@return 各分支实际返回：`process_manager.set_process_group(pid, group)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-39121e4d5139c870"></a>

## `xv::process::get_session`

```doxygen
@fn int xv::process::get_session(int pid)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.get_session(pid)`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.get_session`。
@return 各分支实际返回：`process_manager.get_session(pid)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-88fb19488751f21e"></a>

## `xv::process::create_session`

```doxygen
@fn int xv::process::create_session()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process_manager.create_session()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.create_session`。
@return 各分支实际返回：`process_manager.create_session()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c10951196ee0a808"></a>

## `xv::process::make_poll_wait`

```doxygen
@fn PollWait xv::process::make_poll_wait()
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `PollWait(*current(), waits)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PollWait` → `current`。
@return 各分支实际返回：`PollWait(*current(), waits)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-22490dbefd1fb8c0"></a>

## `xv::process::poll_timer_tick`

```doxygen
@fn void xv::process::poll_timer_tick(unsigned long milliseconds)
@brief 进程对象生命周期、组身份、调度、等待与信号：poll定时器tick。
@param milliseconds 以毫秒表示的时刻或截止值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`waits.expire_timers`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e42abc0ba076c8f0"></a>

## `xv::process::dump`

```doxygen
@fn void xv::process::dump()
@brief 进程对象生命周期、组身份、调度、等待与信号：dump。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.dump`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d6c3ce5695ec8c2b"></a>

## `xv::process::copy_in`

```doxygen
@fn int xv::process::copy_in(void *destination, bool user, unsigned long source, unsigned long count)
@brief 进程对象生命周期、组身份、调度、等待与信号：从指定地址空间复制数据到内核缓冲。
@param destination 接收结果的缓冲或目标对象。
@param user 指明地址是否属于用户地址空间。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `memory` → `copy_in` → `xv::runtime::move`。
@details 需要特别跟踪的条件：`user`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`current()->memory().copy_in(destination, source, count)`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-393041135005f370"></a>

## `xv::process::copy_out`

```doxygen
@fn int xv::process::copy_out(bool user, unsigned long destination, const void *source, unsigned long count)
@brief 进程对象生命周期、组身份、调度、等待与信号：把内核缓冲复制到指定地址空间。
@param user 指明地址是否属于用户地址空间。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `memory` → `copy_out` → `xv::runtime::move`。
@details 需要特别跟踪的条件：`user`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`current()->memory().copy_out(destination, source, count)`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-4f4ab884257688de"></a>

## `xv::process::snapshot_after`

```doxygen
@fn bool xv::process::snapshot_after(int after, ProcessInfo &result)
@brief 向调用者提供一条复制的进程观测记录。
@details 委托 ProcessManager，输出包含 PID、父 PID、组/会话、状态、名称与已发布地址空间大小，不暴露内部指针。
```
