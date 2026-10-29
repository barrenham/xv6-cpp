# `kernel/net/ip/routing/lookup.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::RoutingTable::LookupFlow::advance`](lookup.cppm.md#fn-21fc550352ebc38e)
- [`xv::net::RoutingTable::LookupFlow::available_source`](lookup.cppm.md#fn-7316d78914df7625)
- [`xv::net::RoutingTable::LookupFlow::best`](lookup.cppm.md#fn-c93bc786bed5f4e2)
- [`xv::net::RoutingTable::LookupFlow::commit`](lookup.cppm.md#fn-cc90c5debab6b5d3)
- [`xv::net::RoutingTable::LookupFlow::consider`](lookup.cppm.md#fn-11d42ed4cfa55fd9)
- [`xv::net::RoutingTable::LookupFlow::gateway_connected`](lookup.cppm.md#fn-55bf00236c5f4788)
- [`xv::net::RoutingTable::LookupFlow::matches_path`](lookup.cppm.md#fn-b400b8fa2898b52d)
- [`xv::net::RoutingTable::LookupFlow::more`](lookup.cppm.md#fn-9ca6b3382ec63943)
- [`xv::net::RoutingTable::LookupFlow::reset`](lookup.cppm.md#fn-14255fa2e315986e)
- [`xv::net::RoutingTable::LookupFlow::select_source`](lookup.cppm.md#fn-f380eea2a0218bb6)
- [`xv::net::RoutingTable::LookupFlow::source`](lookup.cppm.md#fn-4502d77d80f9edb8)
- [`xv::net::RoutingTable::LookupFlow::source_owned`](lookup.cppm.md#fn-b783533ebb3b7a40)
- [`xv::net::RoutingTable::LookupFlow::valid_destination`](lookup.cppm.md#fn-c1d9aae284a9d0ee)

<a id="fn-21fc550352ebc38e"></a>

## `xv::net::RoutingTable::LookupFlow::advance`

```doxygen
@fn static inline void xv::net::RoutingTable::LookupFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7316d78914df7625"></a>

## `xv::net::RoutingTable::LookupFlow::available_source`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::available_source(Context &c) noexcept
@brief 检查候选路由能否提供源地址；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c93bc786bed5f4e2"></a>

## `xv::net::RoutingTable::LookupFlow::best`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::best(Context &c) noexcept
@brief 检查扫描是否找到了可用的最优路由；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cc90c5debab6b5d3"></a>

## `xv::net::RoutingTable::LookupFlow::commit`

```doxygen
@fn static inline void xv::net::RoutingTable::LookupFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-11d42ed4cfa55fd9"></a>

## `xv::net::RoutingTable::LookupFlow::consider`

```doxygen
@fn static inline void xv::net::RoutingTable::LookupFlow::consider(Context &c) noexcept
@brief 比较候选路由的前缀长度与 metric；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-55bf00236c5f4788"></a>

## `xv::net::RoutingTable::LookupFlow::gateway_connected`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::gateway_connected(Context &c) noexcept
@brief 检查候选路由网关是否仍与接口直连；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b400b8fa2898b52d"></a>

## `xv::net::RoutingTable::LookupFlow::matches_path`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::matches_path(Context &c) noexcept
@brief 检查候选路由的接口、目的和源地址限制；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9ca6b3382ec63943"></a>

## `xv::net::RoutingTable::LookupFlow::more`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-14255fa2e315986e"></a>

## `xv::net::RoutingTable::LookupFlow::reset`

```doxygen
@fn static inline void xv::net::RoutingTable::LookupFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f380eea2a0218bb6"></a>

## `xv::net::RoutingTable::LookupFlow::select_source`

```doxygen
@fn static inline void xv::net::RoutingTable::LookupFlow::select_source(Context &c) noexcept
@brief 为候选路由或传输发送选择源地址；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4502d77d80f9edb8"></a>

## `xv::net::RoutingTable::LookupFlow::source`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::source(Context &c) noexcept
@brief 为 ARP 请求读取可用源地址或检查源选择结果；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b783533ebb3b7a40"></a>

## `xv::net::RoutingTable::LookupFlow::source_owned`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::source_owned(Context &c) noexcept
@brief 检查候选路由的固定源地址是否仍被拥有；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1d9aae284a9d0ee"></a>

## `xv::net::RoutingTable::LookupFlow::valid_destination`

```doxygen
@fn static inline bool xv::net::RoutingTable::LookupFlow::valid_destination(Context &c) noexcept
@brief 检查目的地址是否允许进行路由查找；所属流程为 xv::net::RoutingTable::LookupFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
