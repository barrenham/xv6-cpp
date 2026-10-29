# `kernel/drivers/serial/flow.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::drivers::Uart16550::DrainFlow::empty`](flow.cppm.md#fn-da8336071fdfd5ad)
- [`xv::drivers::Uart16550::DrainFlow::hardware_drain`](flow.cppm.md#fn-06902f25862f6fae)
- [`xv::drivers::Uart16550::DrainFlow::pump`](flow.cppm.md#fn-739b766bd26a8c5d)
- [`xv::drivers::Uart16550::DrainFlow::unlock`](flow.cppm.md#fn-363867253e602dae)
- [`xv::drivers::Uart16550::DrainFlow::wait`](flow.cppm.md#fn-0174ba05fccbb49c)
- [`xv::drivers::Uart16550::DrainFlow::waiting`](flow.cppm.md#fn-7be368661ff68660)
- [`xv::drivers::Uart16550::TransmitFlow::acquire_writer`](flow.cppm.md#fn-e88c16fa57d7bad6)
- [`xv::drivers::Uart16550::TransmitFlow::again`](flow.cppm.md#fn-d72e2418be9e6c33)
- [`xv::drivers::Uart16550::TransmitFlow::enqueue`](flow.cppm.md#fn-72ae7433ea9297ea)
- [`xv::drivers::Uart16550::TransmitFlow::killed`](flow.cppm.md#fn-8e12372cacb26a68)
- [`xv::drivers::Uart16550::TransmitFlow::nonblocking`](flow.cppm.md#fn-5ae0cfbb2675b117)
- [`xv::drivers::Uart16550::TransmitFlow::start`](flow.cppm.md#fn-e645bbbb3af48e02)
- [`xv::drivers::Uart16550::TransmitFlow::unlock_state`](flow.cppm.md#fn-8ab5ec025b68e881)
- [`xv::drivers::Uart16550::TransmitFlow::unlock_writer`](flow.cppm.md#fn-1ce4bb3b111ef2ca)
- [`xv::drivers::Uart16550::TransmitFlow::wait`](flow.cppm.md#fn-9776f7400477c0e8)
- [`xv::drivers::Uart16550::TransmitFlow::waiting`](flow.cppm.md#fn-fed22cff6d6eda83)

<a id="fn-da8336071fdfd5ad"></a>

## `xv::drivers::Uart16550::DrainFlow::empty`

```doxygen
@fn static bool xv::drivers::Uart16550::DrainFlow::empty(Context &c) noexcept
@brief 检查泵送后队列是否已空。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-06902f25862f6fae"></a>

## `xv::drivers::Uart16550::DrainFlow::hardware_drain`

```doxygen
@fn static void xv::drivers::Uart16550::DrainFlow::hardware_drain(Context &c) noexcept
@brief 直接轮询硬件移位寄存器排空状态，中断时退出。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-739b766bd26a8c5d"></a>

## `xv::drivers::Uart16550::DrainFlow::pump`

```doxygen
@fn static void xv::drivers::Uart16550::DrainFlow::pump(Context &c) noexcept
@brief 尝试将队列字符写入硬件。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-363867253e602dae"></a>

## `xv::drivers::Uart16550::DrainFlow::unlock`

```doxygen
@fn static void xv::drivers::Uart16550::DrainFlow::unlock(Context &c) noexcept
@brief 释放发送状态锁。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0174ba05fccbb49c"></a>

## `xv::drivers::Uart16550::DrainFlow::wait`

```doxygen
@fn static void xv::drivers::Uart16550::DrainFlow::wait(Context &c) noexcept
@brief 登记发送通道，释放状态锁睡眠后重新取得状态锁。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7be368661ff68660"></a>

## `xv::drivers::Uart16550::DrainFlow::waiting`

```doxygen
@fn static bool xv::drivers::Uart16550::DrainFlow::waiting(Context &c) noexcept
@brief 检查队列仍非空且当前进程未被终止。
@details 调用者持有写者锁；等待期间暂时释放状态锁，硬件轮询前释放状态锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e88c16fa57d7bad6"></a>

## `xv::drivers::Uart16550::TransmitFlow::acquire_writer`

```doxygen
@fn static bool xv::drivers::Uart16550::TransmitFlow::acquire_writer(Context &c) noexcept
@brief 按阻塞策略取得写者锁，失败记录 EAGAIN 或保留中断错误。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d72e2418be9e6c33"></a>

## `xv::drivers::Uart16550::TransmitFlow::again`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::again(Context &c) noexcept
@brief 记录空间不足的负 EAGAIN。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-72ae7433ea9297ea"></a>

## `xv::drivers::Uart16550::TransmitFlow::enqueue`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::enqueue(Context &c) noexcept
@brief 直接循环复制整笔数据到队列，再泵送并记录完成字节数。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-8e12372cacb26a68"></a>

## `xv::drivers::Uart16550::TransmitFlow::killed`

```doxygen
@fn static bool xv::drivers::Uart16550::TransmitFlow::killed(Context &) noexcept
@brief 检查当前进程是否已被终止。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5ae0cfbb2675b117"></a>

## `xv::drivers::Uart16550::TransmitFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::Uart16550::TransmitFlow::nonblocking(Context &c) noexcept
@brief 检查调用者是否禁止睡眠。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-e645bbbb3af48e02"></a>

## `xv::drivers::Uart16550::TransmitFlow::start`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::start(Context &c) noexcept
@brief 取得状态锁并泵送既有发送队列。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-8ab5ec025b68e881"></a>

## `xv::drivers::Uart16550::TransmitFlow::unlock_state`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::unlock_state(Context &c) noexcept
@brief 释放发送状态锁。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1ce4bb3b111ef2ca"></a>

## `xv::drivers::Uart16550::TransmitFlow::unlock_writer`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::unlock_writer(Context &c) noexcept
@brief 释放写者睡眠锁。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9776f7400477c0e8"></a>

## `xv::drivers::Uart16550::TransmitFlow::wait`

```doxygen
@fn static void xv::drivers::Uart16550::TransmitFlow::wait(Context &c) noexcept
@brief 登记发送通道，释放状态锁后睡眠，重新加锁并泵送队列。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-fed22cff6d6eda83"></a>

## `xv::drivers::Uart16550::TransmitFlow::waiting`

```doxygen
@fn static bool xv::drivers::Uart16550::TransmitFlow::waiting(Context &c) noexcept
@brief 检查是否仍缺少本次整笔数据所需队列空间。
@details 写者睡眠锁串行化整次提交；状态自旋锁覆盖队列检查与写入，等待时释放，结束时先释放状态锁再释放写者锁。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
