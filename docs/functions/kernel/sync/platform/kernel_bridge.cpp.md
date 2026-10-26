# `kernel/sync/platform/kernel_bridge.cpp`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/platform/kernel_bridge.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync`。

直接模块依赖：`xv.print`, `xv.process`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::detail::current_cpu`](kernel_bridge.cpp.md#fn-d83d20ff2ee4207e)
- [`xv::sync::detail::current_pid`](kernel_bridge.cpp.md#fn-c7137ed24713e0b5)
- [`xv::sync::detail::fail`](kernel_bridge.cpp.md#fn-d2c91545c0707296)
- [`xv::sync::detail::interrupted`](kernel_bridge.cpp.md#fn-34244a221cfe746d)
- [`xv::sync::detail::pop_interrupts_off`](kernel_bridge.cpp.md#fn-4b2a882b5e9104c1)
- [`xv::sync::detail::prepare_interruptible_sleep`](kernel_bridge.cpp.md#fn-f37ac8a65527566f)
- [`xv::sync::detail::prepare_sleep`](kernel_bridge.cpp.md#fn-78b07436e081d612)
- [`xv::sync::detail::push_interrupts_off`](kernel_bridge.cpp.md#fn-a6d34f68b527071c)
- [`xv::sync::detail::sleep_process`](kernel_bridge.cpp.md#fn-42fc3192bb67c511)
- [`xv::sync::detail::wake_processes`](kernel_bridge.cpp.md#fn-fed502f24b271bac)

<a id="fn-a6d34f68b527071c"></a>

## `xv::sync::detail::push_interrupts_off`

```doxygen
@fn void xv::sync::detail::push_interrupts_off()
@brief 锁所有权、屏蔽中断与对象等待登记：压入interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::sync::push_interrupts_off`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4b2a882b5e9104c1"></a>

## `xv::sync::detail::pop_interrupts_off`

```doxygen
@fn void xv::sync::detail::pop_interrupts_off()
@brief 锁所有权、屏蔽中断与对象等待登记：弹出interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::sync::pop_interrupts_off`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d83d20ff2ee4207e"></a>

## `xv::sync::detail::current_cpu`

```doxygen
@fn void *xv::sync::detail::current_cpu()
@brief 锁所有权、屏蔽中断与对象等待登记：取得当前 hart 的 CPU 状态对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current_cpu`。
@return 各分支实际返回：`xv::process::current_cpu()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c7137ed24713e0b5"></a>

## `xv::sync::detail::current_pid`

```doxygen
@fn int xv::sync::detail::current_pid()
@brief 锁所有权、屏蔽中断与对象等待登记：读取或计算 `xv::process::current()->id()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `id`。
@return 各分支实际返回：`xv::process::current()->id()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-34244a221cfe746d"></a>

## `xv::sync::detail::interrupted`

```doxygen
@fn bool xv::sync::detail::interrupted()
@brief 锁所有权、屏蔽中断与对象等待登记：读取或计算 `xv::process::current()->is_killed()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `is_killed`。
@return 各分支实际返回：`xv::process::current()->is_killed()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f37ac8a65527566f"></a>

## `xv::sync::detail::prepare_interruptible_sleep`

```doxygen
@fn void xv::sync::detail::prepare_interruptible_sleep(void *channel)
@brief 锁所有权、屏蔽中断与对象等待登记：登记可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_interruptible_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-78b07436e081d612"></a>

## `xv::sync::detail::prepare_sleep`

```doxygen
@fn void xv::sync::detail::prepare_sleep(void *channel)
@brief 锁所有权、屏蔽中断与对象等待登记：登记不可中断的睡眠意图。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::prepare_sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-42fc3192bb67c511"></a>

## `xv::sync::detail::sleep_process`

```doxygen
@fn void xv::sync::detail::sleep_process()
@brief 锁所有权、屏蔽中断与对象等待登记：执行当前进程已登记的睡眠。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::sleep`。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-fed502f24b271bac"></a>

## `xv::sync::detail::wake_processes`

```doxygen
@fn void xv::sync::detail::wake_processes(void *channel)
@brief 锁所有权、屏蔽中断与对象等待登记：通知该对象通道上的进程等待者。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::wakeup`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d2c91545c0707296"></a>

## `xv::sync::detail::fail`

```doxygen
@fn void xv::sync::detail::fail(const char *message)
@brief 锁所有权、屏蔽中断与对象等待登记：报告内核不变量错误并停止正常执行。
@param message 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic`。
@return 不返回正常调用点；后续执行流由异常终止、调度或特权级切换接管。
```
