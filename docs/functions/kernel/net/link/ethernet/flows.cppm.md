# `kernel/net/link/ethernet/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::EthernetLayer::ReceiveFlow::addressed`](flows.cppm.md#fn-01ca62f869ff53c2)
- [`xv::net::EthernetLayer::ReceiveFlow::advance`](flows.cppm.md#fn-49aa93bb97c5d287)
- [`xv::net::EthernetLayer::ReceiveFlow::deliver`](flows.cppm.md#fn-7281dd570da9ff11)
- [`xv::net::EthernetLayer::ReceiveFlow::dispatched`](flows.cppm.md#fn-e78799538fb58719)
- [`xv::net::EthernetLayer::ReceiveFlow::matches`](flows.cppm.md#fn-3ac4ea8eb8cb2e10)
- [`xv::net::EthernetLayer::ReceiveFlow::more_bindings`](flows.cppm.md#fn-cacb31c4d8ba8974)
- [`xv::net::EthernetLayer::ReceiveFlow::valid_size`](flows.cppm.md#fn-8386e67c03e5952a)
- [`xv::net::EthernetLayer::SendFlow::accepted`](flows.cppm.md#fn-4197bf248543ddb9)
- [`xv::net::EthernetLayer::SendFlow::busy`](flows.cppm.md#fn-4ad03a194b51ab56)
- [`xv::net::EthernetLayer::SendFlow::online`](flows.cppm.md#fn-934cbc79fdeecebf)
- [`xv::net::EthernetLayer::SendFlow::too_large`](flows.cppm.md#fn-28c7c88a0a0fc5da)
- [`xv::net::EthernetLayer::SendFlow::transfer`](flows.cppm.md#fn-908e0cae42c07fbe)
- [`xv::net::EthernetLayer::SendFlow::unreachable`](flows.cppm.md#fn-43cc1a8ac5e31829)
- [`xv::net::EthernetLayer::SendFlow::valid_size`](flows.cppm.md#fn-c2e2799fa52811ef)

<a id="fn-01ca62f869ff53c2"></a>

## `xv::net::EthernetLayer::ReceiveFlow::addressed`

```doxygen
@fn static inline bool xv::net::EthernetLayer::ReceiveFlow::addressed(Context &c) noexcept
@brief 检查报文目的是否匹配本次客户端或租约；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-49aa93bb97c5d287"></a>

## `xv::net::EthernetLayer::ReceiveFlow::advance`

```doxygen
@fn static inline void xv::net::EthernetLayer::ReceiveFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7281dd570da9ff11"></a>

## `xv::net::EthernetLayer::ReceiveFlow::deliver`

```doxygen
@fn static inline void xv::net::EthernetLayer::ReceiveFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e78799538fb58719"></a>

## `xv::net::EthernetLayer::ReceiveFlow::dispatched`

```doxygen
@fn static inline bool xv::net::EthernetLayer::ReceiveFlow::dispatched(Context &c) noexcept
@brief 尝试先向网桥帧接收入口分派；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3ac4ea8eb8cb2e10"></a>

## `xv::net::EthernetLayer::ReceiveFlow::matches`

```doxygen
@fn static inline bool xv::net::EthernetLayer::ReceiveFlow::matches(Context &c) noexcept
@brief 检查对象的端点、接口或业务标识是否匹配；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cacb31c4d8ba8974"></a>

## `xv::net::EthernetLayer::ReceiveFlow::more_bindings`

```doxygen
@fn static inline bool xv::net::EthernetLayer::ReceiveFlow::more_bindings(Context &c) noexcept
@brief 检查以太类型分派是否还有处理器绑定；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8386e67c03e5952a"></a>

## `xv::net::EthernetLayer::ReceiveFlow::valid_size`

```doxygen
@fn static inline bool xv::net::EthernetLayer::ReceiveFlow::valid_size(Context &c) noexcept
@brief 检查当前帧有效性、最短长度与 MTU；所属流程为 xv::net::EthernetLayer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4197bf248543ddb9"></a>

## `xv::net::EthernetLayer::SendFlow::accepted`

```doxygen
@fn static inline bool xv::net::EthernetLayer::SendFlow::accepted(Context &c) noexcept
@brief 检查子连接或设备提交是否成功；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4ad03a194b51ab56"></a>

## `xv::net::EthernetLayer::SendFlow::busy`

```doxygen
@fn static inline void xv::net::EthernetLayer::SendFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-934cbc79fdeecebf"></a>

## `xv::net::EthernetLayer::SendFlow::online`

```doxygen
@fn static inline bool xv::net::EthernetLayer::SendFlow::online(Context &c) noexcept
@brief 检查本阶段需要的链路和服务是否在线；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-28c7c88a0a0fc5da"></a>

## `xv::net::EthernetLayer::SendFlow::too_large`

```doxygen
@fn static inline void xv::net::EthernetLayer::SendFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-908e0cae42c07fbe"></a>

## `xv::net::EthernetLayer::SendFlow::transfer`

```doxygen
@fn static inline void xv::net::EthernetLayer::SendFlow::transfer(Context &c) noexcept
@brief 在设备接收报文后交出独占页面；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-43cc1a8ac5e31829"></a>

## `xv::net::EthernetLayer::SendFlow::unreachable`

```doxygen
@fn static inline void xv::net::EthernetLayer::SendFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c2e2799fa52811ef"></a>

## `xv::net::EthernetLayer::SendFlow::valid_size`

```doxygen
@fn static inline bool xv::net::EthernetLayer::SendFlow::valid_size(Context &c) noexcept
@brief 检查当前帧有效性、最短长度与 MTU；所属流程为 xv::net::EthernetLayer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
