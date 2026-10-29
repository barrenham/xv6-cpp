# `kernel/net/interface/registry/updates.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::InterfaceRegistry::SealFlow::commit`](updates.cppm.md#fn-65650291bd0b7a08)
- [`xv::net::InterfaceRegistry::SealFlow::conflict`](updates.cppm.md#fn-531f0e0bdb5fd4a0)
- [`xv::net::InterfaceRegistry::SealFlow::distinct`](updates.cppm.md#fn-ebfa4e853955975c)
- [`xv::net::InterfaceRegistry::SealFlow::first_address`](updates.cppm.md#fn-8807974fbe80c510)
- [`xv::net::InterfaceRegistry::SealFlow::more_addresses`](updates.cppm.md#fn-ac96c53be8a738e5)
- [`xv::net::InterfaceRegistry::SealFlow::more_interfaces`](updates.cppm.md#fn-bdef670faa934638)
- [`xv::net::InterfaceRegistry::SealFlow::more_others`](updates.cppm.md#fn-b967ef56e8530ef5)
- [`xv::net::InterfaceRegistry::SealFlow::next_address`](updates.cppm.md#fn-688087a8d33e311d)
- [`xv::net::InterfaceRegistry::SealFlow::next_interface`](updates.cppm.md#fn-19a6cad231c0fa3d)
- [`xv::net::InterfaceRegistry::SealFlow::next_other`](updates.cppm.md#fn-e879cbcf3c0d7304)
- [`xv::net::InterfaceRegistry::UpdateFlow::advance`](updates.cppm.md#fn-c1590a850532b282)
- [`xv::net::InterfaceRegistry::UpdateFlow::append`](updates.cppm.md#fn-b2ca1207fd13b610)
- [`xv::net::InterfaceRegistry::UpdateFlow::erase`](updates.cppm.md#fn-ea2bd86ededa63c1)
- [`xv::net::InterfaceRegistry::UpdateFlow::matches`](updates.cppm.md#fn-8d48d6a0c13ac191)
- [`xv::net::InterfaceRegistry::UpdateFlow::more`](updates.cppm.md#fn-01e2cfdd66b057dd)
- [`xv::net::InterfaceRegistry::UpdateFlow::nonnegative`](updates.cppm.md#fn-b0f75c8041bad21b)
- [`xv::net::InterfaceRegistry::UpdateFlow::publish`](updates.cppm.md#fn-28981a28213d1c9e)
- [`xv::net::InterfaceRegistry::UpdateFlow::sealed`](updates.cppm.md#fn-7886d770a5a86a76)
- [`xv::net::InterfaceRegistry::UpdateFlow::valid`](updates.cppm.md#fn-36f15505c3f366c0)
- [`xv::net::InterfaceRegistry::UpdateFlow::validate`](updates.cppm.md#fn-247aff6828488a90)

<a id="fn-65650291bd0b7a08"></a>

## `xv::net::InterfaceRegistry::SealFlow::commit`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::SealFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-531f0e0bdb5fd4a0"></a>

## `xv::net::InterfaceRegistry::SealFlow::conflict`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::SealFlow::conflict(Context &c) noexcept
@brief 记录地址冲突或检查封存时的地址重复；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ebfa4e853955975c"></a>

## `xv::net::InterfaceRegistry::SealFlow::distinct`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::SealFlow::distinct(Context &c) noexcept
@brief 检查当前扫描的两个接口是否不同；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8807974fbe80c510"></a>

## `xv::net::InterfaceRegistry::SealFlow::first_address`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::SealFlow::first_address(Context &c) noexcept
@brief 重置当前接口的地址扫描游标；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ac96c53be8a738e5"></a>

## `xv::net::InterfaceRegistry::SealFlow::more_addresses`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::SealFlow::more_addresses(Context &c) noexcept
@brief 检查封存扫描是否还有接口地址；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bdef670faa934638"></a>

## `xv::net::InterfaceRegistry::SealFlow::more_interfaces`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::SealFlow::more_interfaces(Context &c) noexcept
@brief 检查封存扫描是否还有接口；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b967ef56e8530ef5"></a>

## `xv::net::InterfaceRegistry::SealFlow::more_others`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::SealFlow::more_others(Context &c) noexcept
@brief 检查封存扫描是否还有对比接口；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-688087a8d33e311d"></a>

## `xv::net::InterfaceRegistry::SealFlow::next_address`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::SealFlow::next_address(Context &c) noexcept
@brief 推进封存检查中的地址游标；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-19a6cad231c0fa3d"></a>

## `xv::net::InterfaceRegistry::SealFlow::next_interface`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::SealFlow::next_interface(Context &c) noexcept
@brief 推进封存检查中的主接口游标；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e879cbcf3c0d7304"></a>

## `xv::net::InterfaceRegistry::SealFlow::next_other`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::SealFlow::next_other(Context &c) noexcept
@brief 推进封存检查中的对比接口游标；所属流程为 xv::net::InterfaceRegistry::SealFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1590a850532b282"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::advance`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::UpdateFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b2ca1207fd13b610"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::append`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::UpdateFlow::append(Context &c) noexcept
@brief 追加已验证的数据、路由或对象记录；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ea2bd86ededa63c1"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::erase`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::UpdateFlow::erase(Context &c) noexcept
@brief 删除选定记录并保持数组连续；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8d48d6a0c13ac191"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::matches`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::UpdateFlow::matches(Context &c) noexcept
@brief 检查对象的端点、接口或业务标识是否匹配；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-01e2cfdd66b057dd"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::more`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::UpdateFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b0f75c8041bad21b"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::nonnegative`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::UpdateFlow::nonnegative(Context &c) noexcept
@brief 检查前一步业务返回码是否非负；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-28981a28213d1c9e"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::publish`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::UpdateFlow::publish(Context &c) noexcept
@brief 封存并发布接口或调用接口注册表发布网桥；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7886d770a5a86a76"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::sealed`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::UpdateFlow::sealed(Context &c) noexcept
@brief 检查接口表是否已经启动封存；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-36f15505c3f366c0"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::valid`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::UpdateFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-247aff6828488a90"></a>

## `xv::net::InterfaceRegistry::UpdateFlow::validate`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::UpdateFlow::validate(Context &c) noexcept
@brief 执行本阶段对应对象的登记预检；所属流程为 xv::net::InterfaceRegistry::UpdateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
