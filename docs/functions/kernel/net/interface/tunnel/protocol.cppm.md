# `kernel/net/interface/tunnel/protocol.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::IpInIpProtocol::DispatchFlow::advance_add`](protocol.cppm.md#fn-e1496a9c19688af7)
- [`xv::net::IpInIpProtocol::DispatchFlow::advance_receive`](protocol.cppm.md#fn-91d69621404fc0e7)
- [`xv::net::IpInIpProtocol::DispatchFlow::deliver`](protocol.cppm.md#fn-c3e509a0b4120ced)
- [`xv::net::IpInIpProtocol::DispatchFlow::duplicate`](protocol.cppm.md#fn-cdd56e3094a7e452)
- [`xv::net::IpInIpProtocol::DispatchFlow::empty`](protocol.cppm.md#fn-554c2f226eda09fc)
- [`xv::net::IpInIpProtocol::DispatchFlow::install`](protocol.cppm.md#fn-6582dab28a48271a)
- [`xv::net::IpInIpProtocol::DispatchFlow::more_add`](protocol.cppm.md#fn-6f44f72c81009716)
- [`xv::net::IpInIpProtocol::DispatchFlow::more_receive`](protocol.cppm.md#fn-2471663145f2b78e)
- [`xv::net::IpInIpProtocol::DispatchFlow::reset`](protocol.cppm.md#fn-b5e00350b26d3ce7)

<a id="fn-e1496a9c19688af7"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::advance_add`

```doxygen
@fn static inline void xv::net::IpInIpProtocol::DispatchFlow::advance_add(AddContext &c) noexcept
@brief 推进隧道登记扫描位置；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-91d69621404fc0e7"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::advance_receive`

```doxygen
@fn static inline void xv::net::IpInIpProtocol::DispatchFlow::advance_receive(ReceiveContext &c) noexcept
@brief 推进隧道接收分派游标；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c3e509a0b4120ced"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::deliver`

```doxygen
@fn static inline bool xv::net::IpInIpProtocol::DispatchFlow::deliver(ReceiveContext &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cdd56e3094a7e452"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::duplicate`

```doxygen
@fn static inline bool xv::net::IpInIpProtocol::DispatchFlow::duplicate(AddContext &c) noexcept
@brief 检查登记对象是否已经存在；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-554c2f226eda09fc"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::empty`

```doxygen
@fn static inline bool xv::net::IpInIpProtocol::DispatchFlow::empty(AddContext &c) noexcept
@brief 检查零长度请求或空闲表槽；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6582dab28a48271a"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::install`

```doxygen
@fn static inline void xv::net::IpInIpProtocol::DispatchFlow::install(AddContext &c) noexcept
@brief 安装已验证的租约配置或隧道登记记录；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6f44f72c81009716"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::more_add`

```doxygen
@fn static inline bool xv::net::IpInIpProtocol::DispatchFlow::more_add(AddContext &c) noexcept
@brief 检查隧道登记扫描是否还有候选槽；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2471663145f2b78e"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::more_receive`

```doxygen
@fn static inline bool xv::net::IpInIpProtocol::DispatchFlow::more_receive(ReceiveContext &c) noexcept
@brief 检查隧道接收分派是否还有候选入口；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b5e00350b26d3ce7"></a>

## `xv::net::IpInIpProtocol::DispatchFlow::reset`

```doxygen
@fn static inline void xv::net::IpInIpProtocol::DispatchFlow::reset(AddContext &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::IpInIpProtocol::DispatchFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
