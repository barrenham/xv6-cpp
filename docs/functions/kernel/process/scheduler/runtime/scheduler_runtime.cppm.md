# `kernel/process/scheduler/runtime/scheduler_runtime.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/runtime/scheduler_runtime.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:scheduler_runtime`。

直接模块依赖：`:scheduler`, `:ready_queue`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::SchedulerRuntime::SchedulerRuntime`](scheduler_runtime.cppm.md#fn-ae05d5851108cba7)
- [`xv::process::SchedulerRuntime::make_runnable`](scheduler_runtime.cpp.md#fn-ef218500b96b3202)
- [`xv::process::SchedulerRuntime::name`](scheduler_runtime.cppm.md#fn-4381b51eb1479f72)
- [`xv::process::SchedulerRuntime::run`](scheduler_runtime.cpp.md#fn-c56b6a8213328e00)
- [`xv::process::SchedulerRuntime::select`](scheduler_runtime.cppm.md#fn-a02d912a5a59a0df)
- [`xv::process::SchedulerRuntime::suspend`](scheduler_runtime.cpp.md#fn-ff847c806572406d)
- [`xv::process::SchedulerRuntime::timer_tick`](scheduler_runtime.cpp.md#fn-9e017e77aabcb69f)
- [`xv::process::SchedulerRuntime::yield`](scheduler_runtime.cpp.md#fn-1f791894fd7a2945)

<a id="fn-ae05d5851108cba7"></a>

## `xv::process::SchedulerRuntime::SchedulerRuntime`

```doxygen
@fn constexpr explicit xv::process::SchedulerRuntime::SchedulerRuntime(ReadyQueue &ready)
@brief 借用所选就绪策略构造共用运行机制。
@details 策略对象的生命周期覆盖 Runtime；运行循环、挂起与时钟入口均委托该策略，启动服务负责一次性绑定，不允许运行中热切换。
```

<a id="fn-4381b51eb1479f72"></a>

## `xv::process::SchedulerRuntime::name`

```doxygen
@fn const char *xv::process::SchedulerRuntime::name() const override
@brief 返回当前所选策略的调度名称。
@details 委托所选就绪对象的 scheduler_name，可能为 fair、round-robin 或 mlfq；类本身不代表某一种策略。
```

<a id="fn-a02d912a5a59a0df"></a>

## `xv::process::SchedulerRuntime::select`

```doxygen
@fn void xv::process::SchedulerRuntime::select(ReadyQueue &ready)
@brief 在启动阶段绑定共用运行机制使用的策略。
@details 由进程服务初始化的 CAS 状态守卫调用；其他 hart 进入调度前完成发布，运行期间策略指针保持不变。
```
