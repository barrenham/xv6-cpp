# `kernel/trap/clock/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::trap::TickClock::PauseFlow::interrupted`](flow.cppm.md#fn-8aee915edfe94088)
- [`xv::trap::TickClock::PauseFlow::interrupted_result`](flow.cppm.md#fn-651b7375836631c9)
- [`xv::trap::TickClock::PauseFlow::unlock`](flow.cppm.md#fn-e6ea99bf1de742a1)
- [`xv::trap::TickClock::PauseFlow::wait`](flow.cppm.md#fn-a352345220a3feef)
- [`xv::trap::TickClock::PauseFlow::waiting`](flow.cppm.md#fn-9a8a9c8543fd1ebf)

<a id="fn-8aee915edfe94088"></a>

## `xv::trap::TickClock::PauseFlow::interrupted`

```doxygen
@fn static inline bool xv::trap::TickClock::PauseFlow::interrupted(Context &) noexcept
@brief 检查当前进程的可中断等待状态。
@details 入口已持有时钟锁；每次睡眠前在同一锁下登记依赖，解锁调度后重新持锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-651b7375836631c9"></a>

## `xv::trap::TickClock::PauseFlow::interrupted_result`

```doxygen
@fn static inline void xv::trap::TickClock::PauseFlow::interrupted_result(Context &c) noexcept
@brief 记录被中断的负返回码。
@details 入口已持有时钟锁；每次睡眠前在同一锁下登记依赖，解锁调度后重新持锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e6ea99bf1de742a1"></a>

## `xv::trap::TickClock::PauseFlow::unlock`

```doxygen
@fn static inline void xv::trap::TickClock::PauseFlow::unlock(Context &c) noexcept
@brief 释放时钟锁。
@details 入口已持有时钟锁；每次睡眠前在同一锁下登记依赖，解锁调度后重新持锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a352345220a3feef"></a>

## `xv::trap::TickClock::PauseFlow::wait`

```doxygen
@fn static inline void xv::trap::TickClock::PauseFlow::wait(Context &c) noexcept
@brief 登记 tick 通道，解锁睡眠后重新持锁。
@details 入口已持有时钟锁；每次睡眠前在同一锁下登记依赖，解锁调度后重新持锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9a8a9c8543fd1ebf"></a>

## `xv::trap::TickClock::PauseFlow::waiting`

```doxygen
@fn static inline bool xv::trap::TickClock::PauseFlow::waiting(Context &c) noexcept
@brief 检查无符号 tick 差值是否尚未达到等待间隔。
@details 入口已持有时钟锁；每次睡眠前在同一锁下登记依赖，解锁调度后重新持锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
