# `kernel/net/interface/tunnel/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::IpInIpInterface::ReceiveFlow::claim`](flows.cppm.md#fn-78deae80308eed77)
- [`xv::net::IpInIpInterface::ReceiveFlow::deliver`](flows.cppm.md#fn-58341be40473158f)
- [`xv::net::IpInIpInterface::ReceiveFlow::matches`](flows.cppm.md#fn-218cf2cb6c97c0ec)
- [`xv::net::IpInIpInterface::ReceiveFlow::may_decapsulate`](flows.cppm.md#fn-1f910b3107a97bc3)
- [`xv::net::IpInIpInterface::ReceiveFlow::remove_outer`](flows.cppm.md#fn-be285cfa4c0dfb55)
- [`xv::net::IpInIpInterface::ReceiveFlow::trim`](flows.cppm.md#fn-f69003b12408ea00)
- [`xv::net::IpInIpInterface::TransmitFlow::failed`](flows.cppm.md#fn-c7941635fed9c629)
- [`xv::net::IpInIpInterface::TransmitFlow::fits`](flows.cppm.md#fn-c0600e5799961f75)
- [`xv::net::IpInIpInterface::TransmitFlow::online`](flows.cppm.md#fn-8201e204d635e851)
- [`xv::net::IpInIpInterface::TransmitFlow::prepend`](flows.cppm.md#fn-fded8d7bfe22d529)
- [`xv::net::IpInIpInterface::TransmitFlow::restore`](flows.cppm.md#fn-5705cf4235d7fd91)
- [`xv::net::IpInIpInterface::TransmitFlow::send`](flows.cppm.md#fn-4eb3bd58f305b7e8)
- [`xv::net::IpInIpInterface::TransmitFlow::too_large`](flows.cppm.md#fn-f0a286da9ed12e64)
- [`xv::net::IpInIpInterface::TransmitFlow::unreachable`](flows.cppm.md#fn-ff2816c30bd659e4)

<a id="fn-78deae80308eed77"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::claim`

```doxygen
@fn static inline void xv::net::IpInIpInterface::ReceiveFlow::claim(Context &c) noexcept
@brief 记录本次报文或对象已由当前服务接管处理；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-58341be40473158f"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::deliver`

```doxygen
@fn static inline void xv::net::IpInIpInterface::ReceiveFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-218cf2cb6c97c0ec"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::matches`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::ReceiveFlow::matches(Context &c) noexcept
@brief 检查对象的端点、接口或业务标识是否匹配；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1f910b3107a97bc3"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::may_decapsulate`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::ReceiveFlow::may_decapsulate(Context &c) noexcept
@brief 检查内层长度并消耗隧道解封深度预算；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-be285cfa4c0dfb55"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::remove_outer`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::ReceiveFlow::remove_outer(Context &c) noexcept
@brief 移除已经检查过的外层 IP 前缀；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f69003b12408ea00"></a>

## `xv::net::IpInIpInterface::ReceiveFlow::trim`

```doxygen
@fn static inline void xv::net::IpInIpInterface::ReceiveFlow::trim(Context &c) noexcept
@brief 将隧道输入修整为数据报实际长度；所属流程为 xv::net::IpInIpInterface::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c7941635fed9c629"></a>

## `xv::net::IpInIpInterface::TransmitFlow::failed`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::TransmitFlow::failed(Context &c) noexcept
@brief 检查下层发送结果是否失败；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c0600e5799961f75"></a>

## `xv::net::IpInIpInterface::TransmitFlow::fits`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::TransmitFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8201e204d635e851"></a>

## `xv::net::IpInIpInterface::TransmitFlow::online`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::TransmitFlow::online(Context &c) noexcept
@brief 检查本阶段需要的链路和服务是否在线；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fded8d7bfe22d529"></a>

## `xv::net::IpInIpInterface::TransmitFlow::prepend`

```doxygen
@fn static inline bool xv::net::IpInIpInterface::TransmitFlow::prepend(Context &c) noexcept
@brief 为报文增加本层封装首部；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5705cf4235d7fd91"></a>

## `xv::net::IpInIpInterface::TransmitFlow::restore`

```doxygen
@fn static inline void xv::net::IpInIpInterface::TransmitFlow::restore(Context &c) noexcept
@brief 在发送失败时移除先前添加的封装首部；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4eb3bd58f305b7e8"></a>

## `xv::net::IpInIpInterface::TransmitFlow::send`

```doxygen
@fn static inline void xv::net::IpInIpInterface::TransmitFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f0a286da9ed12e64"></a>

## `xv::net::IpInIpInterface::TransmitFlow::too_large`

```doxygen
@fn static inline void xv::net::IpInIpInterface::TransmitFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ff2816c30bd659e4"></a>

## `xv::net::IpInIpInterface::TransmitFlow::unreachable`

```doxygen
@fn static inline void xv::net::IpInIpInterface::TransmitFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::IpInIpInterface::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
