# `kernel/process/scheduler/runtime/scheduler_runtime.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/runtime/scheduler_runtime.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:scheduler_runtime`, `:scheduler_context`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::SchedulerRuntime::RunFlow::dispatch`](scheduler_runtime.cpp.md#fn-6434d6a00274bd38)
- [`xv::process::SchedulerRuntime::RunFlow::idle`](scheduler_runtime.cpp.md#fn-041766ff57c94c2e)
- [`xv::process::SchedulerRuntime::RunFlow::notify`](scheduler_runtime.cpp.md#fn-fc92b7020df31561)
- [`xv::process::SchedulerRuntime::RunFlow::select`](scheduler_runtime.cpp.md#fn-9be27ba7ec981de3)
- [`xv::process::SchedulerRuntime::RunFlow::selected`](scheduler_runtime.cpp.md#fn-740e5d12027ca8f9)
- [`xv::process::SchedulerRuntime::make_runnable`](scheduler_runtime.cpp.md#fn-ef218500b96b3202)
- [`xv::process::SchedulerRuntime::run`](scheduler_runtime.cpp.md#fn-c56b6a8213328e00)
- [`xv::process::SchedulerRuntime::suspend`](scheduler_runtime.cpp.md#fn-ff847c806572406d)
- [`xv::process::SchedulerRuntime::timer_tick`](scheduler_runtime.cpp.md#fn-9e017e77aabcb69f)
- [`xv::process::SchedulerRuntime::yield`](scheduler_runtime.cpp.md#fn-1f791894fd7a2945)

<a id="fn-c56b6a8213328e00"></a>

## `xv::process::SchedulerRuntime::run`

```doxygen
@fn void xv::process::SchedulerRuntime::run()
@brief 在当前 hart 执行共用调度循环。
@details 为当前 hart 建立本地 RunFlow::Context，依次处理策略选取、进程派发、信号通知或空闲等待；就绪结构由所选策略共享，调用本身不会返回。
```

<a id="fn-ff847c806572406d"></a>

## `xv::process::SchedulerRuntime::suspend`

```doxygen
@fn void xv::process::SchedulerRuntime::suspend()
@brief 结束当前执行区间记账后挂起进程。
@details 调用方持进程锁；先调用策略 stop，再沿 SchedulerContext 的锁和中断检查切换回本核调度上下文。若 yield 已在入队前记账，stop 不会重复计费。
```

<a id="fn-1f791894fd7a2945"></a>

## `xv::process::SchedulerRuntime::yield`

```doxygen
@fn void xv::process::SchedulerRuntime::yield()
@brief 通过共享进程锁交接流程让出本核 CPU。
@details SchedulerContext 获取进程锁、将当前进程通过所选策略发布 Runnable，并挂起；该进程以后可在满足亲和性的其他 hart 恢复。
```

<a id="fn-ef218500b96b3202"></a>

## `xv::process::SchedulerRuntime::make_runnable`

```doxygen
@fn void xv::process::SchedulerRuntime::make_runnable(Process &process)
@brief 通过当前绑定策略发布唯一 Runnable 队列项。
@details 共享机制验证进程锁及合法状态转换；策略处理相应记账和队列插入。队列出队路径先释放队列锁，再等待进程锁。
```

<a id="fn-6434d6a00274bd38"></a>

## `xv::process::SchedulerRuntime::RunFlow::dispatch`

```doxygen
@fn static void xv::process::SchedulerRuntime::RunFlow::dispatch(Context &c) noexcept
@brief 将选中的稳定进程和当前队列交给共享切换机制。
@details 目标进程锁由 DispatchFlow 获取；在锁内启动策略记账，然后按原状态、TLB fencing 和 swtch 协议交接。
```

<a id="fn-041766ff57c94c2e"></a>

## `xv::process::SchedulerRuntime::RunFlow::idle`

```doxygen
@fn static void xv::process::SchedulerRuntime::RunFlow::idle(Context &) noexcept
@brief 无候选时按原中断与 WFI 规则等待。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fc92b7020df31561"></a>

## `xv::process::SchedulerRuntime::RunFlow::notify`

```doxygen
@fn static void xv::process::SchedulerRuntime::RunFlow::notify(Context &) noexcept
@brief 在出站进程锁释放后处理延迟信号通知。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9be27ba7ec981de3"></a>

## `xv::process::SchedulerRuntime::RunFlow::select`

```doxygen
@fn static void xv::process::SchedulerRuntime::RunFlow::select(Context &c) noexcept
@brief 开启中断处理窗口后，从所选策略队列领取本 hart 可运行的进程。
@details 选择只持队列锁，CPU 绑定不匹配的实体留在队列。选中后再由运行机制获取进程锁；未选中沿原空闲路径等待。
```

<a id="fn-740e5d12027ca8f9"></a>

## `xv::process::SchedulerRuntime::RunFlow::selected`

```doxygen
@fn static bool xv::process::SchedulerRuntime::RunFlow::selected(Context &c) noexcept
@brief 检查本轮是否取得候选进程。
@details 每轮打开再关闭中断后从原队列取候选；dispatch 释放进程锁后才处理通知，不再使用原候选指针；主循环迭代执行。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9e017e77aabcb69f"></a>

## `xv::process::SchedulerRuntime::timer_tick`

```doxygen
@fn bool xv::process::SchedulerRuntime::timer_tick()
@brief 询问所选策略是否应抢占当前 hart 的进程。
@details 本核中断关闭时调用；无当前进程返回 false，否则委托策略 tick。该调用不直接抢占其他 hart。
```
