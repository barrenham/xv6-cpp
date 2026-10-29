# `kernel/net/interface/loopback/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::LoopbackInterface::DrainFlow::advance`](flows.cppm.md#fn-6ae28f8d5c6f0cae)
- [`xv::net::LoopbackInterface::DrainFlow::deliver`](flows.cppm.md#fn-15509f7f9e1b99e5)
- [`xv::net::LoopbackInterface::DrainFlow::dequeue`](flows.cppm.md#fn-36c28f79fab76d40)
- [`xv::net::LoopbackInterface::DrainFlow::within_budget`](flows.cppm.md#fn-efb5845874a1008c)
- [`xv::net::LoopbackInterface::TransmitFlow::enqueue`](flows.cppm.md#fn-d910f0288f4cdb37)
- [`xv::net::LoopbackInterface::TransmitFlow::fits`](flows.cppm.md#fn-a0be2f48d903a2df)
- [`xv::net::LoopbackInterface::TransmitFlow::full`](flows.cppm.md#fn-d03cd64e30e39151)
- [`xv::net::LoopbackInterface::TransmitFlow::online`](flows.cppm.md#fn-e8389bf7dd479576)
- [`xv::net::LoopbackInterface::TransmitFlow::too_large`](flows.cppm.md#fn-28b79ba46990cbe1)
- [`xv::net::LoopbackInterface::TransmitFlow::unreachable`](flows.cppm.md#fn-ca2f2054946476c2)

<a id="fn-6ae28f8d5c6f0cae"></a>

## `xv::net::LoopbackInterface::DrainFlow::advance`

```doxygen
@fn static inline void xv::net::LoopbackInterface::DrainFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::LoopbackInterface::DrainFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-15509f7f9e1b99e5"></a>

## `xv::net::LoopbackInterface::DrainFlow::deliver`

```doxygen
@fn static inline void xv::net::LoopbackInterface::DrainFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::LoopbackInterface::DrainFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-36c28f79fab76d40"></a>

## `xv::net::LoopbackInterface::DrainFlow::dequeue`

```doxygen
@fn static inline bool xv::net::LoopbackInterface::DrainFlow::dequeue(Context &c) noexcept
@brief 在原锁下取出队列头部并交还报文所有权；所属流程为 xv::net::LoopbackInterface::DrainFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-efb5845874a1008c"></a>

## `xv::net::LoopbackInterface::DrainFlow::within_budget`

```doxygen
@fn static inline bool xv::net::LoopbackInterface::DrainFlow::within_budget(Context &c) noexcept
@brief 检查并消耗本解释或扫描迭代的工作预算；所属流程为 xv::net::LoopbackInterface::DrainFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d910f0288f4cdb37"></a>

## `xv::net::LoopbackInterface::TransmitFlow::enqueue`

```doxygen
@fn static inline bool xv::net::LoopbackInterface::TransmitFlow::enqueue(Context &c) noexcept
@brief 在原锁下将独占报文放入有界队列；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-a0be2f48d903a2df"></a>

## `xv::net::LoopbackInterface::TransmitFlow::fits`

```doxygen
@fn static inline bool xv::net::LoopbackInterface::TransmitFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d03cd64e30e39151"></a>

## `xv::net::LoopbackInterface::TransmitFlow::full`

```doxygen
@fn static inline void xv::net::LoopbackInterface::TransmitFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e8389bf7dd479576"></a>

## `xv::net::LoopbackInterface::TransmitFlow::online`

```doxygen
@fn static inline bool xv::net::LoopbackInterface::TransmitFlow::online(Context &c) noexcept
@brief 检查本阶段需要的链路和服务是否在线；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-28b79ba46990cbe1"></a>

## `xv::net::LoopbackInterface::TransmitFlow::too_large`

```doxygen
@fn static inline void xv::net::LoopbackInterface::TransmitFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ca2f2054946476c2"></a>

## `xv::net::LoopbackInterface::TransmitFlow::unreachable`

```doxygen
@fn static inline void xv::net::LoopbackInterface::TransmitFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::LoopbackInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
