# `kernel/net/link/ethernet/encapsulation.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::EthernetLayer::EncapsulationFlow::failed`](encapsulation.cppm.md#fn-345d7d11ac454653)
- [`xv::net::EthernetLayer::EncapsulationFlow::prepend`](encapsulation.cppm.md#fn-18d75ac29ef06425)
- [`xv::net::EthernetLayer::EncapsulationFlow::restore`](encapsulation.cppm.md#fn-ef8c7f458e537d24)
- [`xv::net::EthernetLayer::EncapsulationFlow::send`](encapsulation.cppm.md#fn-2bac4959ef68a0fe)
- [`xv::net::EthernetLayer::EncapsulationFlow::too_large`](encapsulation.cppm.md#fn-2d5efcfa50af5218)

<a id="fn-345d7d11ac454653"></a>

## `xv::net::EthernetLayer::EncapsulationFlow::failed`

```doxygen
@fn static inline bool xv::net::EthernetLayer::EncapsulationFlow::failed(Context &c) noexcept
@brief 检查下层发送结果是否失败；所属流程为 xv::net::EthernetLayer::EncapsulationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-18d75ac29ef06425"></a>

## `xv::net::EthernetLayer::EncapsulationFlow::prepend`

```doxygen
@fn static inline bool xv::net::EthernetLayer::EncapsulationFlow::prepend(Context &c) noexcept
@brief 为报文增加本层封装首部；所属流程为 xv::net::EthernetLayer::EncapsulationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ef8c7f458e537d24"></a>

## `xv::net::EthernetLayer::EncapsulationFlow::restore`

```doxygen
@fn static inline void xv::net::EthernetLayer::EncapsulationFlow::restore(Context &c) noexcept
@brief 在发送失败时移除先前添加的封装首部；所属流程为 xv::net::EthernetLayer::EncapsulationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2bac4959ef68a0fe"></a>

## `xv::net::EthernetLayer::EncapsulationFlow::send`

```doxygen
@fn static inline void xv::net::EthernetLayer::EncapsulationFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::EthernetLayer::EncapsulationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2d5efcfa50af5218"></a>

## `xv::net::EthernetLayer::EncapsulationFlow::too_large`

```doxygen
@fn static inline void xv::net::EthernetLayer::EncapsulationFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::EthernetLayer::EncapsulationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
