# `kernel/process/scheduler/policies/fair.cppm`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/fair.cppm)。

该策略由 SchedulerRuntime 调用；算法、记账与多核协议见 [调度设计](../../../../../scheduling.md)。

## 函数导航

- [`xv::process::FairPolicy::FairPolicy`](fair.cppm.md#fn-d4692453ad52c9f1)

<a id="fn-d4692453ad52c9f1"></a>

## `xv::process::FairPolicy::FairPolicy`

```doxygen
@fn constexpr xv::process::FairPolicy::FairPolicy()
@brief 构造固定公平模式的策略入口，复用共享队列与运行时间记账。
@details 节点、锁、亲和性筛选与上下文交接继续由底层队列适配器和共享运行机制管理。
```
