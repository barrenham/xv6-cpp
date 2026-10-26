# `kernel/drivers/platform/kernel_platform.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/platform/kernel_platform.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`xv.arch`, `xv.sync`, `xv.print`, `xv.runtime`, `xv.process`, `:platform`, `xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::detail::allocate_dma_page`](kernel_platform.cpp.md#fn-3e2497f9a8bb8a76)
- [`xv::drivers::detail::claim_terminal`](kernel_platform.cpp.md#fn-eaf6516203ead07d)
- [`xv::drivers::detail::copy_in`](kernel_platform.cpp.md#fn-5c0e6bc6380432aa)
- [`xv::drivers::detail::copy_out`](kernel_platform.cpp.md#fn-1db6985a841bb5f6)
- [`xv::drivers::detail::detach_terminal`](kernel_platform.cpp.md#fn-3d096ebff7f651ec)
- [`xv::drivers::detail::dump_processes`](kernel_platform.cpp.md#fn-ce42a49f7c7724a2)
- [`xv::drivers::detail::fail`](kernel_platform.cpp.md#fn-bd791d74ae9d922d)
- [`xv::drivers::detail::free_dma_page`](kernel_platform.cpp.md#fn-2d8e5b711a2c2aec)
- [`xv::drivers::detail::free_terminal_page`](kernel_platform.cpp.md#fn-f60843cb4cde0d78)
- [`xv::drivers::detail::hangup_terminal`](kernel_platform.cpp.md#fn-c1af15eaa31d5e6f)
- [`xv::drivers::detail::io_barrier`](kernel_platform.cpp.md#fn-eec65ff52575da85)
- [`xv::drivers::detail::may_read_terminal`](kernel_platform.cpp.md#fn-39a74ec50ac9a282)
- [`xv::drivers::detail::panicked`](kernel_platform.cpp.md#fn-735b378b676359d6)
- [`xv::drivers::detail::panicking`](kernel_platform.cpp.md#fn-128e2bdfa3322a07)
- [`xv::drivers::detail::pop_interrupts_off`](kernel_platform.cpp.md#fn-7af9793254c5b352)
- [`xv::drivers::detail::post_terminal_interrupt`](kernel_platform.cpp.md#fn-5d2f829ce13fa9d7)
- [`xv::drivers::detail::prepare_interruptible_sleep`](kernel_platform.cpp.md#fn-1412ba43bdcc972f)
- [`xv::drivers::detail::prepare_sleep`](kernel_platform.cpp.md#fn-57dda9859ef34a3a)
- [`xv::drivers::detail::process_killed`](kernel_platform.cpp.md#fn-9ced86bffdf3c3f0)
- [`xv::drivers::detail::push_interrupts_off`](kernel_platform.cpp.md#fn-71423e5a4445cf2e)
- [`xv::drivers::detail::service_terminal_signals`](kernel_platform.cpp.md#fn-2151fe9eb501d5da)
- [`xv::drivers::detail::set_terminal_foreground`](kernel_platform.cpp.md#fn-d17648e30d4df93c)
- [`xv::drivers::detail::sleep_process`](kernel_platform.cpp.md#fn-e032c04f840cb18d)
- [`xv::drivers::detail::terminal_foreground`](kernel_platform.cpp.md#fn-52f3583ddfb3543d)
- [`xv::drivers::detail::terminal_milliseconds`](kernel_platform.cpp.md#fn-bd48ea235f0c5241)
- [`xv::drivers::detail::wake_processes`](kernel_platform.cpp.md#fn-63de6592afe7b624)
- [`xv::drivers::detail::zero_page`](kernel_platform.cpp.md#fn-20570dfe40006a2d)

<a id="fn-1412ba43bdcc972f"></a>

## `xv::drivers::detail::prepare_interruptible_sleep`

```doxygen
@fn void xv::drivers::detail::prepare_interruptible_sleep(void *channel)
@brief 设备对象、MMIO、设备中断与终端数据流：登记可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_interruptible_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-57dda9859ef34a3a"></a>

## `xv::drivers::detail::prepare_sleep`

```doxygen
@fn void xv::drivers::detail::prepare_sleep(void *channel)
@brief 设备对象、MMIO、设备中断与终端数据流：登记不可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e032c04f840cb18d"></a>

## `xv::drivers::detail::sleep_process`

```doxygen
@fn void xv::drivers::detail::sleep_process()
@brief 设备对象、MMIO、设备中断与终端数据流：执行当前进程已登记的睡眠。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-63de6592afe7b624"></a>

## `xv::drivers::detail::wake_processes`

```doxygen
@fn void xv::drivers::detail::wake_processes(void *channel)
@brief 设备对象、MMIO、设备中断与终端数据流：通知该对象通道上的进程等待者。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::wakeup`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-eaf6516203ead07d"></a>

## `xv::drivers::detail::claim_terminal`

```doxygen
@fn int xv::drivers::detail::claim_terminal(void *terminal)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::process::claim_terminal(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::claim_terminal`。
@return 各分支实际返回：`xv::process::claim_terminal(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-52f3583ddfb3543d"></a>

## `xv::drivers::detail::terminal_foreground`

```doxygen
@fn int xv::drivers::detail::terminal_foreground(void *terminal)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::process::terminal_foreground(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::terminal_foreground`。
@return 各分支实际返回：`xv::process::terminal_foreground(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d17648e30d4df93c"></a>

## `xv::drivers::detail::set_terminal_foreground`

```doxygen
@fn int xv::drivers::detail::set_terminal_foreground(void *terminal, int group)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::process::set_terminal_foreground(terminal, group)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param group 本次操作使用的 进程组 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::set_terminal_foreground`。
@return 各分支实际返回：`xv::process::set_terminal_foreground(terminal, group)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-39a74ec50ac9a282"></a>

## `xv::drivers::detail::may_read_terminal`

```doxygen
@fn bool xv::drivers::detail::may_read_terminal(void *terminal)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::process::may_read_terminal(terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::may_read_terminal`。
@return 各分支实际返回：`xv::process::may_read_terminal(terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5d2f829ce13fa9d7"></a>

## `xv::drivers::detail::post_terminal_interrupt`

```doxygen
@fn void xv::drivers::detail::post_terminal_interrupt(void *terminal, bool suspend)
@brief 设备对象、MMIO、设备中断与终端数据流：登记终端中断。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param suspend 本次操作使用的 suspend 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::post_terminal_interrupt`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9ced86bffdf3c3f0"></a>

## `xv::drivers::detail::process_killed`

```doxygen
@fn bool xv::drivers::detail::process_killed()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::process::current()->is_killed()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `is_killed`。
@return 各分支实际返回：`xv::process::current()->is_killed()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c1af15eaa31d5e6f"></a>

## `xv::drivers::detail::hangup_terminal`

```doxygen
@fn void xv::drivers::detail::hangup_terminal(void *terminal)
@brief 设备对象、MMIO、设备中断与终端数据流：hangup终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::post_terminal_interrupt` → `xv::process::service_signal_notifications`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3d096ebff7f651ec"></a>

## `xv::drivers::detail::detach_terminal`

```doxygen
@fn void xv::drivers::detail::detach_terminal(void *terminal)
@brief 设备对象、MMIO、设备中断与终端数据流：解除关联终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::detach_terminal`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2151fe9eb501d5da"></a>

## `xv::drivers::detail::service_terminal_signals`

```doxygen
@fn void xv::drivers::detail::service_terminal_signals()
@brief 设备对象、MMIO、设备中断与终端数据流：处理终端signals。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::service_signal_notifications`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f60843cb4cde0d78"></a>

## `xv::drivers::detail::free_terminal_page`

```doxygen
@fn void xv::drivers::detail::free_terminal_page(void *page)
@brief 设备对象、MMIO、设备中断与终端数据流：释放终端页。
@param page 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `deallocate`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ce42a49f7c7724a2"></a>

## `xv::drivers::detail::dump_processes`

```doxygen
@fn void xv::drivers::detail::dump_processes()
@brief 设备对象、MMIO、设备中断与终端数据流：dumpprocesses。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::dump`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bd48ea235f0c5241"></a>

## `xv::drivers::detail::terminal_milliseconds`

```doxygen
@fn unsigned long xv::drivers::detail::terminal_milliseconds()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::arch::r_time() / 10000`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::r_time`。
@return 各分支实际返回：`xv::arch::r_time() / 10000`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5c0e6bc6380432aa"></a>

## `xv::drivers::detail::copy_in`

```doxygen
@fn int xv::drivers::detail::copy_in(void *destination, bool user, unsigned long source, unsigned count)
@brief 设备对象、MMIO、设备中断与终端数据流：从指定地址空间复制数据到内核缓冲。
@param destination 接收结果的缓冲或目标对象。
@param user 指明地址是否属于用户地址空间。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::copy_in`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`xv::process::copy_in(destination, user, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1db6985a841bb5f6"></a>

## `xv::drivers::detail::copy_out`

```doxygen
@fn int xv::drivers::detail::copy_out(bool user, unsigned long destination, void *source, unsigned count)
@brief 设备对象、MMIO、设备中断与终端数据流：把内核缓冲复制到指定地址空间。
@param user 指明地址是否属于用户地址空间。
@param destination 接收结果的缓冲或目标对象。
@param source 输入数据位置或源对象。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::copy_out`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`xv::process::copy_out(user, destination, source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-71423e5a4445cf2e"></a>

## `xv::drivers::detail::push_interrupts_off`

```doxygen
@fn void xv::drivers::detail::push_interrupts_off()
@brief 设备对象、MMIO、设备中断与终端数据流：压入interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::sync::push_interrupts_off`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-7af9793254c5b352"></a>

## `xv::drivers::detail::pop_interrupts_off`

```doxygen
@fn void xv::drivers::detail::pop_interrupts_off()
@brief 设备对象、MMIO、设备中断与终端数据流：弹出interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::sync::pop_interrupts_off`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-128e2bdfa3322a07"></a>

## `xv::drivers::detail::panicking`

```doxygen
@fn bool xv::drivers::detail::panicking()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::print::panicking()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panicking`。
@return 各分支实际返回：`xv::print::panicking()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-735b378b676359d6"></a>

## `xv::drivers::detail::panicked`

```doxygen
@fn bool xv::drivers::detail::panicked()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::print::panicked()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panicked`。
@return 各分支实际返回：`xv::print::panicked()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-eec65ff52575da85"></a>

## `xv::drivers::detail::io_barrier`

```doxygen
@fn void xv::drivers::detail::io_barrier()
@brief 设备对象、MMIO、设备中断与终端数据流：设备 I/O屏障。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::io_fence`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3e2497f9a8bb8a76"></a>

## `xv::drivers::detail::allocate_dma_page`

```doxygen
@fn void *xv::drivers::detail::allocate_dma_page()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `xv::memory::allocator().allocate()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `allocate`。
@return 各分支实际返回：`xv::memory::allocator().allocate()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-20570dfe40006a2d"></a>

## `xv::drivers::detail::zero_page`

```doxygen
@fn void xv::drivers::detail::zero_page(void *page)
@brief 设备对象、MMIO、设备中断与终端数据流：清零页。
@param page 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::runtime::fill`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bd791d74ae9d922d"></a>

## `xv::drivers::detail::fail`

```doxygen
@fn void xv::drivers::detail::fail(const char *message)
@brief 设备对象、MMIO、设备中断与终端数据流：报告内核不变量错误并停止正常执行。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```

<a id="fn-2d8e5b711a2c2aec"></a>

## `xv::drivers::detail::free_dma_page`

```doxygen
@fn void xv::drivers::detail::free_dma_page(void *page)
@brief 把已经结束硬件所有权的 DMA 页归还物理分配器。
@param page 完整且独占的分配页，不得仍在 RX/TX 环中被设备使用。
```
