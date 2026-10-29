# `kernel/trap/controller/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::trap::TrapController::EntryFlow::bad_action`](flow.cppm.md#fn-44cfb1f46b93066f)
- [`xv::trap::TrapController::EntryFlow::bad_interrupts`](flow.cppm.md#fn-03068e68eab973bc)
- [`xv::trap::TrapController::EntryFlow::bad_kernel_origin`](flow.cppm.md#fn-56b5b9a3d7ee0f12)
- [`xv::trap::TrapController::EntryFlow::bad_user_origin`](flow.cppm.md#fn-8dd1e37a881ab53a)
- [`xv::trap::TrapController::EntryFlow::dispatch_kernel`](flow.cppm.md#fn-9aec3cc07018da30)
- [`xv::trap::TrapController::EntryFlow::dispatch_user`](flow.cppm.md#fn-80f87fd9218ed0f6)
- [`xv::trap::TrapController::EntryFlow::exit`](flow.cppm.md#fn-bb2b7a25d2d76597)
- [`xv::trap::TrapController::EntryFlow::has_process`](flow.cppm.md#fn-b09989a369a5bba5)
- [`xv::trap::TrapController::EntryFlow::interrupts_enabled`](flow.cppm.md#fn-dd5d9e4228406d28)
- [`xv::trap::TrapController::EntryFlow::kernel_vector`](flow.cppm.md#fn-c0cd0f91c9649bbe)
- [`xv::trap::TrapController::EntryFlow::no_process`](flow.cppm.md#fn-9b8e84542b543a2b)
- [`xv::trap::TrapController::EntryFlow::prepare`](flow.cppm.md#fn-5f344b3dca055f40)
- [`xv::trap::TrapController::EntryFlow::reject_user`](flow.cppm.md#fn-fd9ba07dcd31732e)
- [`xv::trap::TrapController::EntryFlow::restore`](flow.cppm.md#fn-5563185d6c6a4966)
- [`xv::trap::TrapController::EntryFlow::resumable`](flow.cppm.md#fn-d58ed4e15b0a982e)
- [`xv::trap::TrapController::EntryFlow::signals`](flow.cppm.md#fn-6b2213b091b083ba)
- [`xv::trap::TrapController::EntryFlow::supervisor`](flow.cppm.md#fn-9f7f1e358cfab6cc)
- [`xv::trap::TrapController::EntryFlow::terminating`](flow.cppm.md#fn-34608644189cd45e)
- [`xv::trap::TrapController::EntryFlow::unhandled`](flow.cppm.md#fn-f94bb82d3e60f48f)
- [`xv::trap::TrapController::EntryFlow::yield`](flow.cppm.md#fn-63757a3f1d4a5980)
- [`xv::trap::TrapController::EntryFlow::yield_process`](flow.cppm.md#fn-d4f837076e7c7bb4)
- [`xv::trap::TrapController::EntryFlow::yielding`](flow.cppm.md#fn-05d3c49d4dd65b9a)

<a id="fn-44cfb1f46b93066f"></a>

## `xv::trap::TrapController::EntryFlow::bad_action`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::bad_action(Context &c) noexcept
@brief 报告不允许继续的内核陷阱结果。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-03068e68eab973bc"></a>

## `xv::trap::TrapController::EntryFlow::bad_interrupts`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::bad_interrupts(Context &c) noexcept
@brief 报告内核入口中断状态错误。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-56b5b9a3d7ee0f12"></a>

## `xv::trap::TrapController::EntryFlow::bad_kernel_origin`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::bad_kernel_origin(Context &c) noexcept
@brief 报告内核入口来源模式错误。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-8dd1e37a881ab53a"></a>

## `xv::trap::TrapController::EntryFlow::bad_user_origin`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::bad_user_origin(Context &c) noexcept
@brief 报告用户入口来源模式错误。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9aec3cc07018da30"></a>

## `xv::trap::TrapController::EntryFlow::dispatch_kernel`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::dispatch_kernel(Context &c) noexcept
@brief 分派内核陷阱事件。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-80f87fd9218ed0f6"></a>

## `xv::trap::TrapController::EntryFlow::dispatch_user`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::dispatch_user(Context &c) noexcept
@brief 保存用户程序计数器并分派事件。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-bb2b7a25d2d76597"></a>

## `xv::trap::TrapController::EntryFlow::exit`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::exit(Context &) noexcept
@brief 退出当前进程。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-b09989a369a5bba5"></a>

## `xv::trap::TrapController::EntryFlow::has_process`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::has_process(Context &) noexcept
@brief 检查当前是否存在进程。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-dd5d9e4228406d28"></a>

## `xv::trap::TrapController::EntryFlow::interrupts_enabled`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::interrupts_enabled(Context &c) noexcept
@brief 检查保存事件是否意外打开中断。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-c0cd0f91c9649bbe"></a>

## `xv::trap::TrapController::EntryFlow::kernel_vector`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::kernel_vector(Context &) noexcept
@brief 安装内核陷阱向量。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9b8e84542b543a2b"></a>

## `xv::trap::TrapController::EntryFlow::no_process`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::no_process(Context &c) noexcept
@brief 报告用户入口没有当前进程。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5f344b3dca055f40"></a>

## `xv::trap::TrapController::EntryFlow::prepare`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::prepare(Context &c) noexcept
@brief 准备返回用户态并保存用户页表令牌。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-fd9ba07dcd31732e"></a>

## `xv::trap::TrapController::EntryFlow::reject_user`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::reject_user(Context &c) noexcept
@brief 报告用户故障、标记终止并退出进程。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5563185d6c6a4966"></a>

## `xv::trap::TrapController::EntryFlow::restore`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::restore(Context &c) noexcept
@brief 恢复入口保存的内核程序计数器和状态。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d58ed4e15b0a982e"></a>

## `xv::trap::TrapController::EntryFlow::resumable`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::resumable(Context &c) noexcept
@brief 检查内核分派结果是否允许继续或调度。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-6b2213b091b083ba"></a>

## `xv::trap::TrapController::EntryFlow::signals`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::signals(Context &) noexcept
@brief 向当前进程递送待处理信号。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9f7f1e358cfab6cc"></a>

## `xv::trap::TrapController::EntryFlow::supervisor`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::supervisor(Context &c) noexcept
@brief 检查保存事件是否来自 supervisor。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-34608644189cd45e"></a>

## `xv::trap::TrapController::EntryFlow::terminating`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::terminating(Context &c) noexcept
@brief 检查陷阱处理要求终止或当前进程已被终止。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f94bb82d3e60f48f"></a>

## `xv::trap::TrapController::EntryFlow::unhandled`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::unhandled(Context &c) noexcept
@brief 检查分派结果是否无人处理。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-63757a3f1d4a5980"></a>

## `xv::trap::TrapController::EntryFlow::yield`

```doxygen
@fn static inline void xv::trap::TrapController::EntryFlow::yield(Context &) noexcept
@brief 让出处理器。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d4f837076e7c7bb4"></a>

## `xv::trap::TrapController::EntryFlow::yield_process`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::yield_process(Context &c) noexcept
@brief 检查是否请求调度且确实存在当前进程。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-05d3c49d4dd65b9a"></a>

## `xv::trap::TrapController::EntryFlow::yielding`

```doxygen
@fn static inline bool xv::trap::TrapController::EntryFlow::yielding(Context &c) noexcept
@brief 检查陷阱处理是否请求让出处理器。
@details 入口保存一次 TrapEvent；用户和内核入口使用独立图。信号递送、调度和寄存器恢复次序保持原协议。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
