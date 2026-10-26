# `kernel/process/scheduler/runtime/scheduler_context.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/runtime/scheduler_context.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:scheduler_context`, `xv.arch`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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
@fn void xv::process::SchedulerContext::dispatch(Process &process, CpuState &cpu)
@brief 调度与汇编切换边界：按输入事件或系统调用编号选择处理路径。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param cpu 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 取得目标进程锁并验证 Runnable，设置 Running 与 cpu.current，刷新地址翻译后 swtch 到进程内核栈；返回时再次刷新、清空当前进程并释放锁。之后不能继续依赖已退出对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.lock.acquire` → `xv::print::panic` → `xv::arch::sfence_vma` → `swtch` → `current_cpu` → `process.lock.release`。
@details 本函数可见的状态更新包括：`process.state = ProcessState::Running;`；`cpu.current = &process;`；`cpu.current = nullptr;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`process.state != ProcessState::Runnable`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`process.lock.acquire` → `process.lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `p->lock.holding` → `xv::print::panic` → `current_cpu` → `xv::arch::intr_get` → `swtch`。
@details 需要特别跟踪的条件：`!p->lock.holding()`；`current_cpu()->noff != 1`；`p->state == ProcessState::Running`；`xv::arch::intr_get()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5bdf0a1333169edf"></a>

## `xv::process::SchedulerContext::yield`

```doxygen
@fn void xv::process::SchedulerContext::yield(Scheduler &scheduler)
@brief 调度与汇编切换边界：把当前进程重新排队并主动让出 CPU。
@param scheduler 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `p->lock.acquire` → `scheduler.make_runnable` → `scheduler.suspend` → `p->lock.release`。
@note 源码中出现的锁操作：`p->lock.acquire` → `p->lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
