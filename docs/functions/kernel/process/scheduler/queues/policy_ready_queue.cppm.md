# `kernel/process/scheduler/queues/policy_ready_queue.cppm`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/queues/policy_ready_queue.cppm)。

接口、生命周期与验证见 [设计说明](../../../../../scheduling.md)。

## 函数导航

- [`xv::process::PolicyReadyQueue::PolicyReadyQueue`](policy_ready_queue.cppm.md#fn-b060c2730b3bf673)
- [`xv::process::PolicyReadyQueue::enqueue`](policy_ready_queue.cpp.md#fn-4bdb7b543ce172ff)
- [`xv::process::PolicyReadyQueue::initialize`](policy_ready_queue.cpp.md#fn-d63119555b1dba60)
- [`xv::process::PolicyReadyQueue::name`](policy_ready_queue.cppm.md#fn-6d7d0caf3b93edb1)
- [`xv::process::PolicyReadyQueue::start`](policy_ready_queue.cpp.md#fn-983009faa418dbc6)
- [`xv::process::PolicyReadyQueue::stop`](policy_ready_queue.cpp.md#fn-0a559f1ae8548037)
- [`xv::process::PolicyReadyQueue::take`](policy_ready_queue.cpp.md#fn-be26df2afa027a04)
- [`xv::process::PolicyReadyQueue::tick`](policy_ready_queue.cpp.md#fn-6dfff5ba4787e7a8)

<a id="fn-b060c2730b3bf673"></a>

## `xv::process::PolicyReadyQueue::PolicyReadyQueue`

```doxygen
@fn constexpr explicit xv::process::PolicyReadyQueue::PolicyReadyQueue(xv::scheduling::Kind kind)
@brief 构造实际进程队列与纯策略模型的桥接对象，不分配实体。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-6d7d0caf3b93edb1"></a>

## `xv::process::PolicyReadyQueue::name`

```doxygen
@fn const char *xv::process::PolicyReadyQueue::name() const override
@brief 返回公平、轮转或 MLFQ 策略名称。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```
