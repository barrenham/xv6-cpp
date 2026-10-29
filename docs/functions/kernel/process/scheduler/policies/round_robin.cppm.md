# `kernel/process/scheduler/policies/round_robin.cppm`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/round_robin.cppm)。

该策略由 SchedulerRuntime 调用；算法、记账与多核协议见 [调度设计](../../../../../scheduling.md)。

## 函数导航

- [`xv::process::RoundRobinPolicy::RoundRobinPolicy`](round_robin.cppm.md#fn-2256dc2599c02e20)
- [`xv::process::RoundRobinPolicy::name`](round_robin.cppm.md#fn-64832afc7577770e)

<a id="fn-2256dc2599c02e20"></a>

## `xv::process::RoundRobinPolicy::RoundRobinPolicy`

```doxygen
@fn constexpr xv::process::RoundRobinPolicy::RoundRobinPolicy()
@brief 构造原 FIFO 轮转策略入口，保留每次时钟请求让出的行为。
@details 节点、锁、亲和性筛选与上下文交接继续由底层队列适配器和共享运行机制管理。
```
<a id="fn-64832afc7577770e"></a>

## `xv::process::RoundRobinPolicy::name`

```doxygen
@fn const char *xv::process::RoundRobinPolicy::name() const override
@brief 返回轮转策略名称。
@details 节点、锁、亲和性筛选与上下文交接继续由底层队列适配器和共享运行机制管理。
```
