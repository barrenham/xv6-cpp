# `kernel/process/scheduler/queues/policy_ready_queue.cpp`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/queues/policy_ready_queue.cpp)。

接口、生命周期与验证见 [设计说明](../../../../../scheduling.md)。

## 函数导航

- [`xv::process::PolicyReadyQueue::enqueue`](policy_ready_queue.cpp.md#fn-4bdb7b543ce172ff)
- [`xv::process::PolicyReadyQueue::initialize`](policy_ready_queue.cpp.md#fn-d63119555b1dba60)
- [`xv::process::PolicyReadyQueue::start`](policy_ready_queue.cpp.md#fn-983009faa418dbc6)
- [`xv::process::PolicyReadyQueue::stop`](policy_ready_queue.cpp.md#fn-0a559f1ae8548037)
- [`xv::process::PolicyReadyQueue::take`](policy_ready_queue.cpp.md#fn-be26df2afa027a04)
- [`xv::process::PolicyReadyQueue::tick`](policy_ready_queue.cpp.md#fn-6dfff5ba4787e7a8)

<a id="fn-d63119555b1dba60"></a>

## `xv::process::PolicyReadyQueue::initialize`

```doxygen
@fn void xv::process::PolicyReadyQueue::initialize()
@brief 启动时验证并一次性绑定所选队列，初始化原有进程服务后 release 发布；无效或重复尝试返回 false。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-4bdb7b543ce172ff"></a>

## `xv::process::PolicyReadyQueue::enqueue`

```doxygen
@fn void xv::process::PolicyReadyQueue::enqueue(Process &process)
@brief 持进程锁和队列锁发布唯一调度实体；重复入队触发协议诊断。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-be26df2afa027a04"></a>

## `xv::process::PolicyReadyQueue::take`

```doxygen
@fn Process *xv::process::PolicyReadyQueue::take(CpuState &cpu)
@brief 仅持队列锁选择 CPU 绑定允许的进程，再归还借用描述符供运行机制取得进程锁。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-983009faa418dbc6"></a>

## `xv::process::PolicyReadyQueue::start`

```doxygen
@fn void xv::process::PolicyReadyQueue::start(Process &process)
@brief 持进程锁和队列锁开始本次实际执行区间记账。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-0a559f1ae8548037"></a>

## `xv::process::PolicyReadyQueue::stop`

```doxygen
@fn void xv::process::PolicyReadyQueue::stop(Process &process)
@brief 持进程锁和队列锁结束本次执行区间；已在 yield 入队时记账的实体不会重复计费。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```

<a id="fn-6dfff5ba4787e7a8"></a>

## `xv::process::PolicyReadyQueue::tick`

```doxygen
@fn bool xv::process::PolicyReadyQueue::tick(Process &process)
@brief 仅持队列锁判断当前执行实体是否需要抢占。
@details 遵守原进程锁、就绪队列锁、Runnable 生命周期和上下文交接约定；详见可选时钟调度设计。
```
