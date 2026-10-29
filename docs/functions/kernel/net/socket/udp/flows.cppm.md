# `kernel/net/socket/udp/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::UdpSocket::ReceiveFlow::again`](flows.cppm.md#fn-3290497496af95ea)
- [`xv::net::UdpSocket::ReceiveFlow::copy_out`](flows.cppm.md#fn-2acbeab81d57cf30)
- [`xv::net::UdpSocket::ReceiveFlow::dequeue`](flows.cppm.md#fn-ba82eae8863fd7cb)
- [`xv::net::UdpSocket::ReceiveFlow::interrupted`](flows.cppm.md#fn-2c234f391096f466)
- [`xv::net::UdpSocket::ReceiveFlow::interrupted_error`](flows.cppm.md#fn-2154b468e6d6b5c1)
- [`xv::net::UdpSocket::ReceiveFlow::nonblocking`](flows.cppm.md#fn-93f5cce5c8e0d641)
- [`xv::net::UdpSocket::ReceiveFlow::readable`](flows.cppm.md#fn-836f3630a61b00d3)
- [`xv::net::UdpSocket::ReceiveFlow::success`](flows.cppm.md#fn-d75ba59e54ea793d)
- [`xv::net::UdpSocket::ReceiveFlow::unlock`](flows.cppm.md#fn-d5f33c23489acf86)
- [`xv::net::UdpSocket::ReceiveFlow::wait`](flows.cppm.md#fn-27abfd498ecbb389)
- [`xv::net::UdpSocket::SendFlow::allocate`](flows.cppm.md#fn-70162e18a74ac9a0)
- [`xv::net::UdpSocket::SendFlow::allocated`](flows.cppm.md#fn-dfd5729e9a7ebb5a)
- [`xv::net::UdpSocket::SendFlow::bad_copy`](flows.cppm.md#fn-8fad03f5b17bdf7c)
- [`xv::net::UdpSocket::SendFlow::bind`](flows.cppm.md#fn-843112b108e1f577)
- [`xv::net::UdpSocket::SendFlow::bound`](flows.cppm.md#fn-7140c1a73b9e5b63)
- [`xv::net::UdpSocket::SendFlow::connected`](flows.cppm.md#fn-1bb201d71d2352fe)
- [`xv::net::UdpSocket::SendFlow::copy_in`](flows.cppm.md#fn-b37a7360bbabd459)
- [`xv::net::UdpSocket::SendFlow::fits`](flows.cppm.md#fn-253a36c367647fbb)
- [`xv::net::UdpSocket::SendFlow::no_buffer`](flows.cppm.md#fn-09e92607c43f061e)
- [`xv::net::UdpSocket::SendFlow::not_connected`](flows.cppm.md#fn-f5be8b79da8d63cc)
- [`xv::net::UdpSocket::SendFlow::select_peer`](flows.cppm.md#fn-41f72bfc57de3b30)
- [`xv::net::UdpSocket::SendFlow::send`](flows.cppm.md#fn-d3ee7725a0a26584)
- [`xv::net::UdpSocket::SendFlow::send_locked`](flows.cppm.md#fn-d6b45361b35cf692)
- [`xv::net::UdpSocket::SendFlow::too_large`](flows.cppm.md#fn-adc26b9582a75f75)

<a id="fn-3290497496af95ea"></a>

## `xv::net::UdpSocket::ReceiveFlow::again`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::again(Context &c) noexcept
@brief 记录非阻塞暂不可用的负 EAGAIN；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2acbeab81d57cf30"></a>

## `xv::net::UdpSocket::ReceiveFlow::copy_out`

```doxygen
@fn static inline bool xv::net::UdpSocket::ReceiveFlow::copy_out(Context &c) noexcept
@brief 将已取出的业务数据复制给用户；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ba82eae8863fd7cb"></a>

## `xv::net::UdpSocket::ReceiveFlow::dequeue`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::dequeue(Context &c) noexcept
@brief 在原锁下取出队列头部并交还报文所有权；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-2c234f391096f466"></a>

## `xv::net::UdpSocket::ReceiveFlow::interrupted`

```doxygen
@fn static inline bool xv::net::UdpSocket::ReceiveFlow::interrupted(Context &) noexcept
@brief 检查当前等待是否已经被中断；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2154b468e6d6b5c1"></a>

## `xv::net::UdpSocket::ReceiveFlow::interrupted_error`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::interrupted_error(Context &c) noexcept
@brief 记录中断或用户复制失败的负错误；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-93f5cce5c8e0d641"></a>

## `xv::net::UdpSocket::ReceiveFlow::nonblocking`

```doxygen
@fn static inline bool xv::net::UdpSocket::ReceiveFlow::nonblocking(Context &c) noexcept
@brief 检查本次应用调用是否为非阻塞；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-836f3630a61b00d3"></a>

## `xv::net::UdpSocket::ReceiveFlow::readable`

```doxygen
@fn static inline bool xv::net::UdpSocket::ReceiveFlow::readable(Context &c) noexcept
@brief 检查当前队列或接收流是否存在可读内容；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d75ba59e54ea793d"></a>

## `xv::net::UdpSocket::ReceiveFlow::success`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::success(Context &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d5f33c23489acf86"></a>

## `xv::net::UdpSocket::ReceiveFlow::unlock`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::unlock(Context &c) noexcept
@brief 释放本次 socket 调用仍持有的注册表锁；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-27abfd498ecbb389"></a>

## `xv::net::UdpSocket::ReceiveFlow::wait`

```doxygen
@fn static inline void xv::net::UdpSocket::ReceiveFlow::wait(Context &c) noexcept
@brief 沿原等待协议登记、解锁睡眠并重新持锁；所属流程为 xv::net::UdpSocket::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-70162e18a74ac9a0"></a>

## `xv::net::UdpSocket::SendFlow::allocate`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-dfd5729e9a7ebb5a"></a>

## `xv::net::UdpSocket::SendFlow::allocated`

```doxygen
@fn static inline bool xv::net::UdpSocket::SendFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8fad03f5b17bdf7c"></a>

## `xv::net::UdpSocket::SendFlow::bad_copy`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::bad_copy(Context &c) noexcept
@brief 记录用户复制失败的负错误；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-843112b108e1f577"></a>

## `xv::net::UdpSocket::SendFlow::bind`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::bind(Context &c) noexcept
@brief 完成绑定或进入租约已绑定状态；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7140c1a73b9e5b63"></a>

## `xv::net::UdpSocket::SendFlow::bound`

```doxygen
@fn static inline bool xv::net::UdpSocket::SendFlow::bound(Context &c) noexcept
@brief 检查绑定步骤的业务结果是否成功；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1bb201d71d2352fe"></a>

## `xv::net::UdpSocket::SendFlow::connected`

```doxygen
@fn static inline bool xv::net::UdpSocket::SendFlow::connected(Context &c) noexcept
@brief 检查当前 socket 是否允许本次连接操作；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b37a7360bbabd459"></a>

## `xv::net::UdpSocket::SendFlow::copy_in`

```doxygen
@fn static inline bool xv::net::UdpSocket::SendFlow::copy_in(Context &c) noexcept
@brief 从用户输入暂存数据并报告结果；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-253a36c367647fbb"></a>

## `xv::net::UdpSocket::SendFlow::fits`

```doxygen
@fn static inline bool xv::net::UdpSocket::SendFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-09e92607c43f061e"></a>

## `xv::net::UdpSocket::SendFlow::no_buffer`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::no_buffer(Context &c) noexcept
@brief 记录本次分配或容量不足的负错误；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f5be8b79da8d63cc"></a>

## `xv::net::UdpSocket::SendFlow::not_connected`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::not_connected(Context &c) noexcept
@brief 记录尚未连接的负 ENOTCONN；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-41f72bfc57de3b30"></a>

## `xv::net::UdpSocket::SendFlow::select_peer`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::select_peer(Context &c) noexcept
@brief 选择显式传入或已连接的 UDP 对端；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d3ee7725a0a26584"></a>

## `xv::net::UdpSocket::SendFlow::send`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d6b45361b35cf692"></a>

## `xv::net::UdpSocket::SendFlow::send_locked`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::send_locked(Context &c) noexcept
@brief 在 socket 注册表锁内执行 UDP 发送图；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-adc26b9582a75f75"></a>

## `xv::net::UdpSocket::SendFlow::too_large`

```doxygen
@fn static inline void xv::net::UdpSocket::SendFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::UdpSocket::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
