# `kernel/process/wait/wait_channels.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/wait_channels.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:wait_channels`。

直接模块依赖：`:process`, `:scheduler`, `:poll_entry`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::WaitChannels::WaitChannels`](wait_channels.cppm.md#fn-9ca4c3d1c7326296)
- [`xv::process::WaitChannels::arm_timer`](poll_wait.cpp.md#fn-3352c9d1a67a7dac)
- [`xv::process::WaitChannels::cancel`](../wait-channels.md#wait-waitchannels-cancel)
- [`xv::process::WaitChannels::expire_timers`](poll_wait.cpp.md#fn-de20066532420bb1)
- [`xv::process::WaitChannels::index`](../wait-channels.md#wait-waitchannels-index)
- [`xv::process::WaitChannels::initialize`](../wait-channels.md#wait-waitchannels-initialize)
- [`xv::process::WaitChannels::prepare`](../wait-channels.md#wait-waitchannels-prepare)
- [`xv::process::WaitChannels::remove_timer`](poll_wait.cpp.md#fn-e197f48a1ef8cac7)
- [`xv::process::WaitChannels::subscribe`](poll_wait.cpp.md#fn-1194a4f1b3678f15)
- [`xv::process::WaitChannels::unlink`](../wait-channels.md#wait-waitchannels-unlink)
- [`xv::process::WaitChannels::unsubscribe`](poll_wait.cpp.md#fn-417e58d7ce31f1b6)
- [`xv::process::WaitChannels::wake`](../wait-channels.md#wait-waitchannels-wake)
- [`xv::process::wait_channels`](../lifecycle/service.cpp.md#fn-00fe184141e83bc3)

<a id="fn-9ca4c3d1c7326296"></a>

## `xv::process::WaitChannels::WaitChannels`

```doxygen
@fn constexpr explicit xv::process::WaitChannels::WaitChannels(Scheduler &scheduler)
@brief 构造对象等待通道表，建立本重载要求的成员初值和借用关系。
@param scheduler 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`scheduler_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
