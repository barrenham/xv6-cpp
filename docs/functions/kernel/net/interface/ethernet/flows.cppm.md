# `kernel/net/interface/ethernet/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::EthernetInterface::TransmitFlow::broadcast`](flows.cppm.md#fn-666ecb79bf5e15b4)
- [`xv::net::EthernetInterface::TransmitFlow::configured`](flows.cppm.md#fn-bb81ad0fdda23c75)
- [`xv::net::EthernetInterface::TransmitFlow::fits`](flows.cppm.md#fn-aed4a3125a0abb16)
- [`xv::net::EthernetInterface::TransmitFlow::online`](flows.cppm.md#fn-150333c248c035e1)
- [`xv::net::EthernetInterface::TransmitFlow::resolve_neighbor`](flows.cppm.md#fn-5c1383648ed2e277)
- [`xv::net::EthernetInterface::TransmitFlow::send_broadcast`](flows.cppm.md#fn-33c35c239e94ae57)
- [`xv::net::EthernetInterface::TransmitFlow::too_large`](flows.cppm.md#fn-5168546352ec8a03)
- [`xv::net::EthernetInterface::TransmitFlow::unreachable`](flows.cppm.md#fn-e934ad32f178e915)
- [`xv::net::EthernetInterface::TransmitFlow::valid_bootstrap`](flows.cppm.md#fn-deb8b03118fd692a)

<a id="fn-666ecb79bf5e15b4"></a>

## `xv::net::EthernetInterface::TransmitFlow::broadcast`

```doxygen
@fn static inline bool xv::net::EthernetInterface::TransmitFlow::broadcast(Context &c) noexcept
@brief 检查下一跳是否为广播；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bb81ad0fdda23c75"></a>

## `xv::net::EthernetInterface::TransmitFlow::configured`

```doxygen
@fn static inline bool xv::net::EthernetInterface::TransmitFlow::configured(Context &c) noexcept
@brief 检查启动配置结果或执行锁内配置阶段；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aed4a3125a0abb16"></a>

## `xv::net::EthernetInterface::TransmitFlow::fits`

```doxygen
@fn static inline bool xv::net::EthernetInterface::TransmitFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-150333c248c035e1"></a>

## `xv::net::EthernetInterface::TransmitFlow::online`

```doxygen
@fn static inline bool xv::net::EthernetInterface::TransmitFlow::online(Context &c) noexcept
@brief 检查本阶段需要的链路和服务是否在线；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5c1383648ed2e277"></a>

## `xv::net::EthernetInterface::TransmitFlow::resolve_neighbor`

```doxygen
@fn static inline void xv::net::EthernetInterface::TransmitFlow::resolve_neighbor(Context &c) noexcept
@brief 通过 ARP 为下一跳解析并发送；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-33c35c239e94ae57"></a>

## `xv::net::EthernetInterface::TransmitFlow::send_broadcast`

```doxygen
@fn static inline void xv::net::EthernetInterface::TransmitFlow::send_broadcast(Context &c) noexcept
@brief 将报文提交给链路广播地址；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5168546352ec8a03"></a>

## `xv::net::EthernetInterface::TransmitFlow::too_large`

```doxygen
@fn static inline void xv::net::EthernetInterface::TransmitFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e934ad32f178e915"></a>

## `xv::net::EthernetInterface::TransmitFlow::unreachable`

```doxygen
@fn static inline void xv::net::EthernetInterface::TransmitFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-deb8b03118fd692a"></a>

## `xv::net::EthernetInterface::TransmitFlow::valid_bootstrap`

```doxygen
@fn static inline bool xv::net::EthernetInterface::TransmitFlow::valid_bootstrap(Context &c) noexcept
@brief 检查引导报文长度与接口 MTU；所属流程为 xv::net::EthernetInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
