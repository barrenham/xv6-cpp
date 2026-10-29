# `kernel/process/scheduler/policies/mlfq.cppm`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/mlfq.cppm)。

该策略由 SchedulerRuntime 调用；算法、记账与多核协议见 [调度设计](../../../../../scheduling.md)。

## 函数导航

- [`xv::process::MlfqPolicy::MlfqPolicy`](mlfq.cppm.md#fn-0b8bdace0aa89a80)

<a id="fn-0b8bdace0aa89a80"></a>

## `xv::process::MlfqPolicy::MlfqPolicy`

```doxygen
@fn constexpr xv::process::MlfqPolicy::MlfqPolicy()
@brief 构造固定多级反馈模式的策略入口，复用分层队列、累计额度与周期提升。
@details 节点、锁、亲和性筛选与上下文交接继续由底层队列适配器和共享运行机制管理。
```
