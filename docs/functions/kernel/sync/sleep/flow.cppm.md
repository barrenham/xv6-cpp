# `kernel/sync/sleep/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::sync::SleepLock::AcquireFlow::claim`](flow.cppm.md#fn-667c897e93e5c049)
- [`xv::sync::SleepLock::AcquireFlow::interrupted`](flow.cppm.md#fn-9c877438d77525ad)
- [`xv::sync::SleepLock::AcquireFlow::owned`](flow.cppm.md#fn-15418c5343f5ad96)
- [`xv::sync::SleepLock::AcquireFlow::unlock`](flow.cppm.md#fn-4d34ff6efe1f353f)
- [`xv::sync::SleepLock::AcquireFlow::wait`](flow.cppm.md#fn-9dbbdefa104d2dd7)

<a id="fn-667c897e93e5c049"></a>

## `xv::sync::SleepLock::AcquireFlow::claim`

```doxygen
@fn static inline void xv::sync::SleepLock::AcquireFlow::claim(Context &c) noexcept
@brief 标记锁已占用，保存当前进程身份并记录获取成功。
@details 入口已取得状态锁；等待时在锁下登记，释放状态锁后睡眠；结束时始终释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9c877438d77525ad"></a>

## `xv::sync::SleepLock::AcquireFlow::interrupted`

```doxygen
@fn static inline bool xv::sync::SleepLock::AcquireFlow::interrupted(Context &c) noexcept
@brief 仅对可中断请求检查当前中断状态。
@details 入口已取得状态锁；等待时在锁下登记，释放状态锁后睡眠；结束时始终释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-15418c5343f5ad96"></a>

## `xv::sync::SleepLock::AcquireFlow::owned`

```doxygen
@fn static inline bool xv::sync::SleepLock::AcquireFlow::owned(Context &c) noexcept
@brief 检查睡眠锁是否仍被持有。
@details 入口已取得状态锁；等待时在锁下登记，释放状态锁后睡眠；结束时始终释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-4d34ff6efe1f353f"></a>

## `xv::sync::SleepLock::AcquireFlow::unlock`

```doxygen
@fn static inline void xv::sync::SleepLock::AcquireFlow::unlock(Context &c) noexcept
@brief 释放睡眠锁的状态自旋锁。
@details 入口已取得状态锁；等待时在锁下登记，释放状态锁后睡眠；结束时始终释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9dbbdefa104d2dd7"></a>

## `xv::sync::SleepLock::AcquireFlow::wait`

```doxygen
@fn static inline void xv::sync::SleepLock::AcquireFlow::wait(Context &c) noexcept
@brief 拒绝递归获取并按可中断策略登记、解锁、睡眠和重新持锁。
@details 入口已取得状态锁；等待时在锁下登记，释放状态锁后睡眠；结束时始终释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
