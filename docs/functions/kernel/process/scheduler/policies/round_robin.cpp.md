# `kernel/process/scheduler/policies/round_robin.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/round_robin.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:round_robin`, `:scheduler_context`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::RoundRobinScheduler::RunFlow::dispatch`](round_robin.cpp.md#fn-6434d6a00274bd38)
- [`xv::process::RoundRobinScheduler::RunFlow::idle`](round_robin.cpp.md#fn-041766ff57c94c2e)
- [`xv::process::RoundRobinScheduler::RunFlow::notify`](round_robin.cpp.md#fn-fc92b7020df31561)
- [`xv::process::RoundRobinScheduler::RunFlow::select`](round_robin.cpp.md#fn-9be27ba7ec981de3)
- [`xv::process::RoundRobinScheduler::RunFlow::selected`](round_robin.cpp.md#fn-740e5d12027ca8f9)
- [`xv::process::RoundRobinScheduler::make_runnable`](round_robin.cpp.md#fn-ef218500b96b3202)
- [`xv::process::RoundRobinScheduler::run`](round_robin.cpp.md#fn-c56b6a8213328e00)
- [`xv::process::RoundRobinScheduler::suspend`](round_robin.cpp.md#fn-ff847c806572406d)
- [`xv::process::RoundRobinScheduler::yield`](round_robin.cpp.md#fn-1f791894fd7a2945)

<a id="fn-c56b6a8213328e00"></a>

## `xv::process::RoundRobinScheduler::run`

```doxygen
@fn void xv::process::RoundRobinScheduler::run()
@brief 轮转调度策略：运行。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 RunFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-ff847c806572406d"></a>

## `xv::process::RoundRobinScheduler::suspend`

```doxygen
@fn void xv::process::RoundRobinScheduler::suspend()
@brief 轮转调度策略：在规定的锁状态下切回调度器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`SchedulerContext::suspend`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1f791894fd7a2945"></a>

## `xv::process::RoundRobinScheduler::yield`

```doxygen
@fn void xv::process::RoundRobinScheduler::yield()
@brief 轮转调度策略：把当前进程重新排队并主动让出 CPU。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`SchedulerContext::yield`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ef218500b96b3202"></a>

## `xv::process::RoundRobinScheduler::make_runnable`

```doxygen
@fn void xv::process::RoundRobinScheduler::make_runnable(Process &process)
@brief 轮转调度策略：把受锁保护的进程发布到就绪队列。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`SchedulerContext::make_runnable`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6434d6a00274bd38"></a>

## `xv::process::RoundRobinScheduler::RunFlow::dispatch`

```doxygen
@fn static void xv::process::RoundRobinScheduler::RunFlow::dispatch(Context &c) noexcept
@brief 执行候选的持锁上下文切换。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-041766ff57c94c2e"></a>

## `xv::process::RoundRobinScheduler::RunFlow::idle`

```doxygen
@fn static void xv::process::RoundRobinScheduler::RunFlow::idle(Context &) noexcept
@brief 无候选时按原中断与 WFI 规则等待。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fc92b7020df31561"></a>

## `xv::process::RoundRobinScheduler::RunFlow::notify`

```doxygen
@fn static void xv::process::RoundRobinScheduler::RunFlow::notify(Context &) noexcept
@brief 在出站进程锁释放后处理延迟信号通知。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9be27ba7ec981de3"></a>

## `xv::process::RoundRobinScheduler::RunFlow::select`

```doxygen
@fn static void xv::process::RoundRobinScheduler::RunFlow::select(Context &c) noexcept
@brief 开始调度轮次并从原就绪队列选择候选。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-740e5d12027ca8f9"></a>

## `xv::process::RoundRobinScheduler::RunFlow::selected`

```doxygen
@fn static bool xv::process::RoundRobinScheduler::RunFlow::selected(Context &c) noexcept
@brief 检查本轮是否取得候选进程。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```
