# `kernel/net/interface/address/remove.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::AddressConfiguration::RemoveLeaseFlow::advance_generation`](remove.cppm.md#fn-8b504f79cb52d1ec)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::any_change`](remove.cppm.md#fn-6b9439b4f061e4a9)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::changed`](remove.cppm.md#fn-c640fac6f2b26021)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::compact_addresses`](remove.cppm.md#fn-d4848611cad7ccf6)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::identify`](remove.cppm.md#fn-b06b2a86e230d755)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::registered`](remove.cppm.md#fn-8bcf5ab82b6b78d3)
- [`xv::net::AddressConfiguration::RemoveLeaseFlow::remove_routes`](remove.cppm.md#fn-a18e741fb35c29f2)

<a id="fn-8b504f79cb52d1ec"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::advance_generation`

```doxygen
@fn static inline void xv::net::AddressConfiguration::RemoveLeaseFlow::advance_generation(Context &c) noexcept
@brief 推进接口配置代次；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6b9439b4f061e4a9"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::any_change`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::RemoveLeaseFlow::any_change(Context &c) noexcept
@brief 检查本次事务是否改变地址或路由；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c640fac6f2b26021"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::changed`

```doxygen
@fn static inline void xv::net::AddressConfiguration::RemoveLeaseFlow::changed(Context &c) noexcept
@brief 记录或查询本次业务状态变化；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d4848611cad7ccf6"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::compact_addresses`

```doxygen
@fn static inline void xv::net::AddressConfiguration::RemoveLeaseFlow::compact_addresses(Context &c) noexcept
@brief 按地址归属压缩租约地址数组；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b06b2a86e230d755"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::identify`

```doxygen
@fn static inline void xv::net::AddressConfiguration::RemoveLeaseFlow::identify(Context &c) noexcept
@brief 保存本次接口标识或租约归属；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8bcf5ab82b6b78d3"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::registered`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::RemoveLeaseFlow::registered(Context &c) noexcept
@brief 按对象身份检查接口是否仍在注册表；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a18e741fb35c29f2"></a>

## `xv::net::AddressConfiguration::RemoveLeaseFlow::remove_routes`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::RemoveLeaseFlow::remove_routes(Context &c) noexcept
@brief 移除接口租约拥有的路由并报告变化；所属流程为 xv::net::AddressConfiguration::RemoveLeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
