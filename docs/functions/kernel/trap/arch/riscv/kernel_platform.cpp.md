# `kernel/trap/arch/riscv/kernel_platform.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../../kernel/trap/arch/riscv/kernel_platform.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`xv.arch`, `xv.print`, `:platform`, `xv.process`, `xv.drivers`, `xv.syscall`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`kernel_stack_prepare`](kernel_platform.cpp.md#fn-5c3f11ead232f9df)
- [`xv::trap::detail::advance_user_pc`](kernel_platform.cpp.md#fn-b6c74a0b72429492)
- [`xv::trap::detail::arm_timer`](kernel_platform.cpp.md#fn-6d371a07cbcfe6cf)
- [`xv::trap::detail::deliver_signals`](kernel_platform.cpp.md#fn-4f041a4936f5176b)
- [`xv::trap::detail::dispatch_external_interrupt`](kernel_platform.cpp.md#fn-c6137e39ef8b7fbc)
- [`xv::trap::detail::dispatch_system_call`](kernel_platform.cpp.md#fn-4786b99366f6d39f)
- [`xv::trap::detail::enable_interrupts`](kernel_platform.cpp.md#fn-111982e6acd2b642)
- [`xv::trap::detail::enter_user`](kernel_platform.cpp.md#fn-c9c56524b5226794)
- [`xv::trap::detail::exit_current`](kernel_platform.cpp.md#fn-f003afe98a9ac7f7)
- [`xv::trap::detail::fatal`](kernel_platform.cpp.md#fn-c1ce930979d608fe)
- [`xv::trap::detail::hart_id`](kernel_platform.cpp.md#fn-64f75db4e245584e)
- [`xv::trap::detail::has_process`](kernel_platform.cpp.md#fn-7efdcba7acebcf8f)
- [`xv::trap::detail::install_kernel_vector`](kernel_platform.cpp.md#fn-13fcad96f6c4f2b3)
- [`xv::trap::detail::interrupted`](kernel_platform.cpp.md#fn-1841e13c7500b772)
- [`xv::trap::detail::killed`](kernel_platform.cpp.md#fn-d6ca1daf3715bdb9)
- [`xv::trap::detail::prepare_interruptible_sleep`](kernel_platform.cpp.md#fn-a4eda3906392ae77)
- [`xv::trap::detail::prepare_sleep`](kernel_platform.cpp.md#fn-b796ab17eb6c713b)
- [`xv::trap::detail::prepare_user_return`](kernel_platform.cpp.md#fn-1ed3457044a10998)
- [`xv::trap::detail::refill_stack_reserve`](kernel_platform.cpp.md#fn-8b71bba6470d2230)
- [`xv::trap::detail::report_user_fault`](kernel_platform.cpp.md#fn-f43a07ef20a5a6b5)
- [`xv::trap::detail::request_kill`](kernel_platform.cpp.md#fn-de1ad36dd5d6359e)
- [`xv::trap::detail::resolve_page_fault`](kernel_platform.cpp.md#fn-e5148c82fe11c46e)
- [`xv::trap::detail::restore_kernel`](kernel_platform.cpp.md#fn-e72bdb404644cf01)
- [`xv::trap::detail::save_user_pc`](kernel_platform.cpp.md#fn-72d8410c76641146)
- [`xv::trap::detail::service_timer_devices`](kernel_platform.cpp.md#fn-8bf053cbd79e3fd0)
- [`xv::trap::detail::sleep`](kernel_platform.cpp.md#fn-8655e02476dd6fd7)
- [`xv::trap::detail::snapshot`](kernel_platform.cpp.md#fn-99ff6f6b07035421)
- [`xv::trap::detail::take_stack_frame`](kernel_platform.cpp.md#fn-2d7062e5082e9be9)
- [`xv::trap::detail::time`](kernel_platform.cpp.md#fn-21bfc0d68188d860)
- [`xv::trap::detail::user_page_table_token`](kernel_platform.cpp.md#fn-fb9cf283bc7dc87e)
- [`xv::trap::detail::wakeup`](kernel_platform.cpp.md#fn-372407f34cc73123)
- [`xv::trap::detail::yield_current`](kernel_platform.cpp.md#fn-b99841e868c4b5b2)
- [`xv::trap::scheduler_stack_top`](kernel_platform.cpp.md#fn-ebe7c8cf89feb5d2)

<a id="fn-99ff6f6b07035421"></a>

## `xv::trap::detail::snapshot`

```doxygen
@fn TrapEvent xv::trap::detail::snapshot()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取架构寄存器组成陷阱事件快照。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`TrapCause` → `xv::arch::r_scause` → `xv::arch::r_sepc` → `xv::arch::r_sstatus` → `xv::arch::r_stval`。
@return 各分支实际返回：`{TrapCause(xv::arch::r_scause()), xv::arch::r_sepc(), xv::arch::r_sstatus(), xv::arch::r_stval()}`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-13fcad96f6c4f2b3"></a>

## `xv::trap::detail::install_kernel_vector`

```doxygen
@fn void xv::trap::detail::install_kernel_vector()
@brief 安装内核异常入口与当前 hart 的应急栈。
@details 先把 sscratch 设置为静态 16 KiB 应急栈顶，再设置 stvec；用户陷阱重新进入内核时恢复此约定，供尚未能在原栈保存现场的缺页使用。
```

<a id="fn-7efdcba7acebcf8f"></a>

## `xv::trap::detail::has_process`

```doxygen
@fn bool xv::trap::detail::has_process()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::process::current() != nullptr`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current`。
@return 各分支实际返回：`xv::process::current() != nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4f041a4936f5176b"></a>

## `xv::trap::detail::deliver_signals`

```doxygen
@fn void xv::trap::detail::deliver_signals()
@brief 陷阱快照、事件分派、设备服务及特权级返回：在安全返回边界执行默认动作或建立用户信号帧。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::deliver_signals`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1841e13c7500b772"></a>

## `xv::trap::detail::interrupted`

```doxygen
@fn bool xv::trap::detail::interrupted()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::process::current()->is_killed()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `is_killed`。
@return 各分支实际返回：`xv::process::current()->is_killed()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d6ca1daf3715bdb9"></a>

## `xv::trap::detail::killed`

```doxygen
@fn bool xv::trap::detail::killed()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::process::current()->termination_status() != 0`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `termination_status`。
@return 各分支实际返回：`xv::process::current()->termination_status() != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-de1ad36dd5d6359e"></a>

## `xv::trap::detail::request_kill`

```doxygen
@fn void xv::trap::detail::request_kill()
@brief 陷阱快照、事件分派、设备服务及特权级返回：请求终止请求。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `request_kill`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f003afe98a9ac7f7"></a>

## `xv::trap::detail::exit_current`

```doxygen
@fn void xv::trap::detail::exit_current(int status)
@brief 陷阱快照、事件分派、设备服务及特权级返回：退出当前。
@param status 退出状态或状态结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::exit_process`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b99841e868c4b5b2"></a>

## `xv::trap::detail::yield_current`

```doxygen
@fn void xv::trap::detail::yield_current()
@brief 陷阱快照、事件分派、设备服务及特权级返回：yield当前。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::scheduler` → `yield`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-72d8410c76641146"></a>

## `xv::trap::detail::save_user_pc`

```doxygen
@fn void xv::trap::detail::save_user_pc(Word pc)
@brief 陷阱快照、事件分派、设备服务及特权级返回：保存用户pc。
@param pc 本次操作使用的 pc 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `registers`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b6c74a0b72429492"></a>

## `xv::trap::detail::advance_user_pc`

```doxygen
@fn void xv::trap::detail::advance_user_pc(Word bytes)
@brief 陷阱快照、事件分派、设备服务及特权级返回：把用户返回 PC 推进到系统调用之后。
@param bytes 本次操作使用的 字节区域 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `registers`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-111982e6acd2b642"></a>

## `xv::trap::detail::enable_interrupts`

```doxygen
@fn void xv::trap::detail::enable_interrupts()
@brief 陷阱快照、事件分派、设备服务及特权级返回：开启interrupts。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::intr_on`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4786b99366f6d39f"></a>

## `xv::trap::detail::dispatch_system_call`

```doxygen
@fn void xv::trap::detail::dispatch_system_call()
@brief 陷阱快照、事件分派、设备服务及特权级返回：分派系统调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::syscall::dispatch`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c6137e39ef8b7fbc"></a>

## `xv::trap::detail::dispatch_external_interrupt`

```doxygen
@fn void xv::trap::detail::dispatch_external_interrupt()
@brief 陷阱快照、事件分派、设备服务及特权级返回：分派外部中断。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::drivers::handle_external_interrupt` → `xv::process::cpu_id` → `xv::process::service_signal_notifications`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e5148c82fe11c46e"></a>

## `xv::trap::detail::resolve_page_fault`

```doxygen
@fn bool xv::trap::detail::resolve_page_fault(Word address, MemoryAccess access)
@brief 把读写执行异常转成进程地址空间的结构化访问请求。
@details 转换 MemoryAccess 到 FaultAccess，仅 FaultOutcome 为 Resolved 时返回 true，其余由用户陷阱控制器报告并终止当前进程。
```

<a id="fn-f43a07ef20a5a6b5"></a>

## `xv::trap::detail::report_user_fault`

```doxygen
@fn void xv::trap::detail::report_user_fault(const TrapEvent &event)
@brief 陷阱快照、事件分派、设备服务及特权级返回：report用户缺页。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::printk` → `event.cause.raw` → `xv::process::current` → `id`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c1ce930979d608fe"></a>

## `xv::trap::detail::fatal`

```doxygen
@fn void xv::trap::detail::fatal(const char *message, const TrapEvent &event)
@brief 陷阱快照、事件分派、设备服务及特权级返回：fatal。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::printk` → `event.cause.raw` → `xv::print::panic`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1ed3457044a10998"></a>

## `xv::trap::detail::prepare_user_return`

```doxygen
@fn void xv::trap::detail::prepare_user_return()
@brief 在正常内核中设置用户返回现场和预留栈顶。
@details 关闭中断后安装 trampoline 返回上下文，kernel_sp 使用完整虚拟栈容量。XV_VM_SELF_TEST 非零的私有诊断内核会在首次返回前执行探针，正常构建恒为零，不执行测试代码。
```

<a id="fn-fb9cf283bc7dc87e"></a>

## `xv::trap::detail::user_page_table_token`

```doxygen
@fn Word xv::trap::detail::user_page_table_token()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `MAKE_SATP(xv::process::current()->memory().page_table())`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`MAKE_SATP` → `xv::process::current` → `memory` → `page_table`。
@return 各分支实际返回：`MAKE_SATP(xv::process::current()->memory().page_table())`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e72bdb404644cf01"></a>

## `xv::trap::detail::restore_kernel`

```doxygen
@fn void xv::trap::detail::restore_kernel(const TrapEvent &event)
@brief 陷阱快照、事件分派、设备服务及特权级返回：恢复被内核陷阱及调度打断的架构返回状态。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::w_sepc` → `xv::arch::w_sstatus`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c9c56524b5226794"></a>

## `xv::trap::detail::enter_user`

```doxygen
@fn void xv::trap::detail::enter_user(Word token)
@brief 陷阱快照、事件分派、设备服务及特权级返回：通过 trampoline 汇编进入用户页表和用户执行流。
@param token 本次操作使用的 token 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`void` → `__builtin_unreachable`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-64f75db4e245584e"></a>

## `xv::trap::detail::hart_id`

```doxygen
@fn int xv::trap::detail::hart_id()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::process::cpu_id()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::cpu_id`。
@return 各分支实际返回：`xv::process::cpu_id()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-21bfc0d68188d860"></a>

## `xv::trap::detail::time`

```doxygen
@fn Word xv::trap::detail::time()
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `xv::arch::r_time()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::r_time`。
@return 各分支实际返回：`xv::arch::r_time()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6d371a07cbcfe6cf"></a>

## `xv::trap::detail::arm_timer`

```doxygen
@fn void xv::trap::detail::arm_timer(Word deadline)
@brief 陷阱快照、事件分派、设备服务及特权级返回：设置下一次本 hart 的硬件定时截止值。
@param deadline 本次操作使用的 deadline 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::w_stimecmp`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a4eda3906392ae77"></a>

## `xv::trap::detail::prepare_interruptible_sleep`

```doxygen
@fn void xv::trap::detail::prepare_interruptible_sleep(void *channel)
@brief 陷阱快照、事件分派、设备服务及特权级返回：登记可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_interruptible_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b796ab17eb6c713b"></a>

## `xv::trap::detail::prepare_sleep`

```doxygen
@fn void xv::trap::detail::prepare_sleep(void *channel)
@brief 陷阱快照、事件分派、设备服务及特权级返回：登记不可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8655e02476dd6fd7"></a>

## `xv::trap::detail::sleep`

```doxygen
@fn void xv::trap::detail::sleep()
@brief 陷阱快照、事件分派、设备服务及特权级返回：在已登记的等待条件下挂起当前进程。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::sleep`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-372407f34cc73123"></a>

## `xv::trap::detail::wakeup`

```doxygen
@fn void xv::trap::detail::wakeup(void *channel)
@brief 陷阱快照、事件分派、设备服务及特权级返回：把通道唤醒请求交给进程等待服务。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::wakeup`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8bf053cbd79e3fd0"></a>

## `xv::trap::detail::service_timer_devices`

```doxygen
@fn void xv::trap::detail::service_timer_devices()
@brief 陷阱快照、事件分派、设备服务及特权级返回：处理定时器devices。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`hart_id` → `xv::process::poll_timer_tick` → `xv::arch::r_time` → `xv::drivers::terminal_timer_tick`。
@details 需要特别跟踪的条件：`hart_id() == 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 只在 hart 0 调用网络 timer_tick，处理 ARP 请求重试、IPv4 分片超时与回环队列、TCP 重传及关闭状态回收；不会在计时器中执行用户地址复制。
```

<a id="fn-5c3f11ead232f9df"></a>

## `kernel_stack_prepare`

```doxygen
@fn int kernel_stack_prepare(unsigned long stack, unsigned long cause, unsigned long fault)
@brief 在应急入口栈上分类并处理当前内核栈异常。
@details 从当前进程或 hart 调度器取得稳定栈归属。读写执行栈异常走无区域锁的 handle_stack_fault，仅 Resolved 才重试；权限、现存映射异常、保护区及应急帧耗尽明确 panic。普通入口只在 noff 为零时补池和尝试下方 32 KiB 余量，不在持锁时分配或调度。
```

<a id="fn-ebe7c8cf89feb5d2"></a>

## `xv::trap::scheduler_stack_top`

```doxygen
@fn unsigned long xv::trap::scheduler_stack_top()
@brief 准备本 hart 的应急页池和长期可增长调度栈。
@details 启动阶段中断关闭，先尽力预留 64 页应急帧，再从统一区域服务取得栈并记录底地址，返回完整容量顶端。重复初始化或栈创建失败 panic。
```

<a id="fn-8b71bba6470d2230"></a>

## `xv::trap::detail::refill_stack_reserve`

```doxygen
@fn void xv::trap::detail::refill_stack_reserve()
@brief 在未持自旋锁时补充当前 hart 的应急物理页。
@details noff 非零立即返回，只有安全上下文才调用物理分配器；池容量固定 64 页，OOM 停止补充而不改变已有页。调用时中断关闭，池由当前 hart 独占。
```

<a id="fn-2d7062e5082e9be9"></a>

## `xv::trap::detail::take_stack_frame`

```doxygen
@fn void *xv::trap::detail::take_stack_frame()
@brief 为实际栈缺页提供一个无需等待的物理页。
@details 无自旋锁上下文先尝试分配器，失败再取应急池；noff 非零时只从当前 hart 池取页，不递归取得分配器锁。池空返回 nullptr，调用者明确终止而不无限重试。
```
