# `kernel/fs/pipe/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::Pipe::ReadFlow::again`](flow.cppm.md#fn-93ebf0971346b4f8)
- [`xv::fs::Pipe::ReadFlow::interrupted`](flow.cppm.md#fn-bd40805a74c2f7ce)
- [`xv::fs::Pipe::ReadFlow::nonblocking`](flow.cppm.md#fn-4e23c11d5d2a1b56)
- [`xv::fs::Pipe::ReadFlow::notify_writer`](flow.cppm.md#fn-169afbee3f665408)
- [`xv::fs::Pipe::ReadFlow::transfer`](flow.cppm.md#fn-1695fbe7fe4e418b)
- [`xv::fs::Pipe::ReadFlow::unlock`](flow.cppm.md#fn-1ecdccb052936c25)
- [`xv::fs::Pipe::ReadFlow::wait`](flow.cppm.md#fn-0e7353785a28e1bf)
- [`xv::fs::Pipe::ReadFlow::waiting`](flow.cppm.md#fn-7520b2526991241e)
- [`xv::fs::Pipe::WriteFlow::aborted`](flow.cppm.md#fn-109388dc46e7b144)
- [`xv::fs::Pipe::WriteFlow::again`](flow.cppm.md#fn-cde357d4b6c1edbf)
- [`xv::fs::Pipe::WriteFlow::atomic_wait`](flow.cppm.md#fn-9425751ff1424acb)
- [`xv::fs::Pipe::WriteFlow::needs_wait`](flow.cppm.md#fn-8a40215ae5b44d0b)
- [`xv::fs::Pipe::WriteFlow::nonblocking`](flow.cppm.md#fn-24295aa0d85a497a)
- [`xv::fs::Pipe::WriteFlow::notify_reader`](flow.cppm.md#fn-0459af1644e754c5)
- [`xv::fs::Pipe::WriteFlow::transfer`](flow.cppm.md#fn-846a8bfd001ce3ec)
- [`xv::fs::Pipe::WriteFlow::unavailable`](flow.cppm.md#fn-5385dd3017f1e08a)
- [`xv::fs::Pipe::WriteFlow::unlock`](flow.cppm.md#fn-5f48c9fe75d1ed25)
- [`xv::fs::Pipe::WriteFlow::wait`](flow.cppm.md#fn-e8c3f6a1ed42c01e)

<a id="fn-93ebf0971346b4f8"></a>

## `xv::fs::Pipe::ReadFlow::again`

```doxygen
@fn static void xv::fs::Pipe::ReadFlow::again(Context &c) noexcept
@brief 记录暂时无数据的负 EAGAIN。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-bd40805a74c2f7ce"></a>

## `xv::fs::Pipe::ReadFlow::interrupted`

```doxygen
@fn static bool xv::fs::Pipe::ReadFlow::interrupted(Context &) noexcept
@brief 检查当前进程是否被终止。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-4e23c11d5d2a1b56"></a>

## `xv::fs::Pipe::ReadFlow::nonblocking`

```doxygen
@fn static bool xv::fs::Pipe::ReadFlow::nonblocking(Context &c) noexcept
@brief 检查调用者是否禁止睡眠。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-169afbee3f665408"></a>

## `xv::fs::Pipe::ReadFlow::notify_writer`

```doxygen
@fn static void xv::fs::Pipe::ReadFlow::notify_writer(Context &c) noexcept
@brief 通知写侧空间可能已经可用。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1695fbe7fe4e418b"></a>

## `xv::fs::Pipe::ReadFlow::transfer`

```doxygen
@fn static void xv::fs::Pipe::ReadFlow::transfer(Context &c) noexcept
@brief 直接读取可用字节，保留首字节复制失败和部分进展约定。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1ecdccb052936c25"></a>

## `xv::fs::Pipe::ReadFlow::unlock`

```doxygen
@fn static void xv::fs::Pipe::ReadFlow::unlock(Context &c) noexcept
@brief 释放管道锁。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0e7353785a28e1bf"></a>

## `xv::fs::Pipe::ReadFlow::wait`

```doxygen
@fn static void xv::fs::Pipe::ReadFlow::wait(Context &c) noexcept
@brief 登记读侧等待通道，解锁睡眠后重新持锁。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7520b2526991241e"></a>

## `xv::fs::Pipe::ReadFlow::waiting`

```doxygen
@fn static bool xv::fs::Pipe::ReadFlow::waiting(Context &c) noexcept
@brief 检查队列为空且写端仍然打开。
@details 入口持管道锁；写端关闭且队列为空时返回 EOF。失败复制不消费当前字节，等待过程在重新检查条件前恢复管道锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-109388dc46e7b144"></a>

## `xv::fs::Pipe::WriteFlow::aborted`

```doxygen
@fn static bool xv::fs::Pipe::WriteFlow::aborted(Context &c) noexcept
@brief 检查当前结果是否要求直接解锁返回。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-cde357d4b6c1edbf"></a>

## `xv::fs::Pipe::WriteFlow::again`

```doxygen
@fn static void xv::fs::Pipe::WriteFlow::again(Context &c) noexcept
@brief 记录没有整体空间的负 EAGAIN。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9425751ff1424acb"></a>

## `xv::fs::Pipe::WriteFlow::atomic_wait`

```doxygen
@fn static bool xv::fs::Pipe::WriteFlow::atomic_wait(Context &c) noexcept
@brief 检查容量以内的写请求是否缺少整体空间。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-8a40215ae5b44d0b"></a>

## `xv::fs::Pipe::WriteFlow::needs_wait`

```doxygen
@fn static bool xv::fs::Pipe::WriteFlow::needs_wait(Context &c) noexcept
@brief 检查满队列是否要求再次等待。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-24295aa0d85a497a"></a>

## `xv::fs::Pipe::WriteFlow::nonblocking`

```doxygen
@fn static bool xv::fs::Pipe::WriteFlow::nonblocking(Context &c) noexcept
@brief 检查调用者是否禁止睡眠。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0459af1644e754c5"></a>

## `xv::fs::Pipe::WriteFlow::notify_reader`

```doxygen
@fn static void xv::fs::Pipe::WriteFlow::notify_reader(Context &c) noexcept
@brief 通知读侧已有数据或复制阶段结束。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-846a8bfd001ce3ec"></a>

## `xv::fs::Pipe::WriteFlow::transfer`

```doxygen
@fn static void xv::fs::Pipe::WriteFlow::transfer(Context &c) noexcept
@brief 直接复制一批字节，遇满队列、复制失败或终止时保存结果与等待状态。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5385dd3017f1e08a"></a>

## `xv::fs::Pipe::WriteFlow::unavailable`

```doxygen
@fn static bool xv::fs::Pipe::WriteFlow::unavailable(Context &c) noexcept
@brief 按原顺序检查读端关闭或进程终止。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5f48c9fe75d1ed25"></a>

## `xv::fs::Pipe::WriteFlow::unlock`

```doxygen
@fn static void xv::fs::Pipe::WriteFlow::unlock(Context &c) noexcept
@brief 释放管道锁。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e8c3f6a1ed42c01e"></a>

## `xv::fs::Pipe::WriteFlow::wait`

```doxygen
@fn static void xv::fs::Pipe::WriteFlow::wait(Context &c) noexcept
@brief 登记写侧等待通道，解锁睡眠后重新持锁。
@details 入口已持有管道锁；小请求先保证整体空间，大请求允许部分完成。复制逐字节检查仍保留直接循环；睡眠遵循锁下登记、解锁后调度。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
