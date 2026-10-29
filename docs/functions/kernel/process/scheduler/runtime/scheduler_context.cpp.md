# `kernel/process/scheduler/runtime/scheduler_context.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/runtime/scheduler_context.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:scheduler_context`, `xv.arch`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::SchedulerContext::DispatchFlow::enter`](scheduler_context.cpp.md#fn-2db240bad0144b08)
- [`xv::process::SchedulerContext::DispatchFlow::leave`](scheduler_context.cpp.md#fn-a291a904223611a7)
- [`xv::process::SchedulerContext::DispatchFlow::switch_context`](scheduler_context.cpp.md#fn-11074be1e7e203bb)
- [`xv::process::SchedulerContext::SuspendFlow::prepare`](scheduler_context.cpp.md#fn-a8fa96fc2c454a8e)
- [`xv::process::SchedulerContext::SuspendFlow::switch_context`](scheduler_context.cpp.md#fn-c5921855ce1d78b8)
- [`xv::process::SchedulerContext::YieldFlow::lock`](scheduler_context.cpp.md#fn-cab3e8017e65685d)
- [`xv::process::SchedulerContext::YieldFlow::publish`](scheduler_context.cpp.md#fn-56fc3c72e02d6051)
- [`xv::process::SchedulerContext::YieldFlow::suspend`](scheduler_context.cpp.md#fn-6a13358bcc56686f)
- [`xv::process::SchedulerContext::begin_round`](scheduler_context.cpp.md#fn-3168ca64e91ead29)
- [`xv::process::SchedulerContext::dispatch`](scheduler_context.cpp.md#fn-717e81da3f2d8fa2)
- [`xv::process::SchedulerContext::idle`](scheduler_context.cpp.md#fn-70ec78efb2fe5d09)
- [`xv::process::SchedulerContext::make_runnable`](scheduler_context.cpp.md#fn-28e041bc6bad922e)
- [`xv::process::SchedulerContext::prepare_cpu`](scheduler_context.cpp.md#fn-8eb37916f4622bf5)
- [`xv::process::SchedulerContext::suspend`](scheduler_context.cpp.md#fn-cba1163ffee29c4e)
- [`xv::process::SchedulerContext::yield`](scheduler_context.cpp.md#fn-5bdf0a1333169edf)

<a id="fn-8eb37916f4622bf5"></a>

## `xv::process::SchedulerContext::prepare_cpu`

```doxygen
@fn CpuState &xv::process::SchedulerContext::prepare_cpu()
@brief 调度与汇编切换边界：为调度循环建立当前 hart 的空闲状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current_cpu`。
@details 本函数可见的状态更新包括：`cpu->current = nullptr;`。局部临时变量与对象字段应按作用域区分。
@return 各分支实际返回：`*cpu`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3168ca64e91ead29"></a>

## `xv::process::SchedulerContext::begin_round`

```doxygen
@fn void xv::process::SchedulerContext::begin_round()
@brief 调度与汇编切换边界：建立一次调度循环的中断检查边界。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::intr_on` → `xv::arch::intr_off`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-70ec78efb2fe5d09"></a>

## `xv::process::SchedulerContext::idle`

```doxygen
@fn void xv::process::SchedulerContext::idle()
@brief 调度与汇编切换边界：在无就绪任务时允许中断并等待事件。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::intr_on` → `volatile`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-717e81da3f2d8fa2"></a>

## `xv::process::SchedulerContext::dispatch`

```doxygen
@fn void xv::process::SchedulerContext::dispatch(Process &process, CpuState &cpu, ReadyQueue &ready)
@brief 在进程锁内开始策略记账并完成一次上下文交接。
@details ready 是所选就绪队列。图内获取进程锁并验证 Runnable，调用 ready.start 后设置 Running/current；保持 swtch 两侧的 TLB fence，返回后清空 current 并释放进程锁。之后不能继续依赖可能已退出的进程对象。
```

<a id="fn-28e041bc6bad922e"></a>

## `xv::process::SchedulerContext::make_runnable`

```doxygen
@fn void xv::process::SchedulerContext::make_runnable(Process &process, ReadyQueue &ready)
@brief 调度与汇编切换边界：把受锁保护的进程发布到就绪队列。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param ready 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.lock.holding` → `xv::print::panic` → `ready.enqueue`。
@details 本函数可见的状态更新包括：`process.state = ProcessState::Runnable;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!process.lock.holding() || process.state == ProcessState::Unused || process.state == ProcessState::Zombie || process.state == ProcessState::Runnable`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-cba1163ffee29c4e"></a>

## `xv::process::SchedulerContext::suspend`

```doxygen
@fn void xv::process::SchedulerContext::suspend()
@brief 调度与汇编切换边界：在规定的锁状态下切回调度器。
@details 切换前验证只持当前进程锁、中断关闭且状态已不为 Running；保存中断恢复意图，经 swtch 转回当前 CPU 的调度 Context，恢复执行后还原该意图。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 SuspendFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5bdf0a1333169edf"></a>

## `xv::process::SchedulerContext::yield`

```doxygen
@fn void xv::process::SchedulerContext::yield(Scheduler &scheduler)
@brief 调度与汇编切换边界：把当前进程重新排队并主动让出 CPU。
@param scheduler 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 YieldFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2db240bad0144b08"></a>

## `xv::process::SchedulerContext::DispatchFlow::enter`

```doxygen
@fn static void xv::process::SchedulerContext::DispatchFlow::enter(Context &c) noexcept
@brief 取得进程锁、验证 Runnable 并开始所选策略的执行记账。
@details 在进程锁内调用队列 start；其短队列锁释放后再标记 Running 和 cpu.current，随后交给原 swtch 节点。
```

<a id="fn-a291a904223611a7"></a>

## `xv::process::SchedulerContext::DispatchFlow::leave`

```doxygen
@fn static void xv::process::SchedulerContext::DispatchFlow::leave(Context &c) noexcept
@brief 关闭调度帧的中断恢复标志、清除 CPU 当前进程并释放进程锁。
@details 进程锁从进入到离开调度切换始终持有；回收栈叶所需的两次 TLB fence 与 swtch 保持相邻。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-11074be1e7e203bb"></a>

## `xv::process::SchedulerContext::DispatchFlow::switch_context`

```doxygen
@fn static void xv::process::SchedulerContext::DispatchFlow::switch_context(Context &c) noexcept
@brief 在两次 TLB fence 之间切换到进程上下文并等待返回。
@details 进程锁从进入到离开调度切换始终持有；回收栈叶所需的两次 TLB fence 与 swtch 保持相邻。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a8fa96fc2c454a8e"></a>

## `xv::process::SchedulerContext::SuspendFlow::prepare`

```doxygen
@fn static void xv::process::SchedulerContext::SuspendFlow::prepare(Context &c) noexcept
@brief 验证调度前置条件并保存当前 CPU 的中断恢复标志。
@details 调用者已持进程锁；逐项核对锁、中断和进程状态，保存 intena，恢复后重新读取当前 CPU，支持跨核恢复。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c5921855ce1d78b8"></a>

## `xv::process::SchedulerContext::SuspendFlow::switch_context`

```doxygen
@fn static void xv::process::SchedulerContext::SuspendFlow::switch_context(Context &c) noexcept
@brief 切回当前 CPU 调度上下文并在恢复核上还原中断恢复标志。
@details 调用者已持进程锁；逐项核对锁、中断和进程状态，保存 intena，恢复后重新读取当前 CPU，支持跨核恢复。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cab3e8017e65685d"></a>

## `xv::process::SchedulerContext::YieldFlow::lock`

```doxygen
@fn static void xv::process::SchedulerContext::YieldFlow::lock(Context &c) noexcept
@brief 获取当前进程锁。
@details 持进程锁发布 Runnable 并挂入队列，持锁挂起，恢复后才解锁；执行器不依赖尾调用优化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-56fc3c72e02d6051"></a>

## `xv::process::SchedulerContext::YieldFlow::publish`

```doxygen
@fn static void xv::process::SchedulerContext::YieldFlow::publish(Context &c) noexcept
@brief 通过原调度器发布当前进程可运行。
@details 持进程锁发布 Runnable 并挂入队列，持锁挂起，恢复后才解锁；执行器不依赖尾调用优化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6a13358bcc56686f"></a>

## `xv::process::SchedulerContext::YieldFlow::suspend`

```doxygen
@fn static void xv::process::SchedulerContext::YieldFlow::suspend(Context &c) noexcept
@brief 持锁挂起并在恢复后释放进程锁。
@details 持进程锁发布 Runnable 并挂入队列，持锁挂起，恢复后才解锁；执行器不依赖尾调用优化。 下一节点由图策略声明，业务结果与控制信号分离。
```
