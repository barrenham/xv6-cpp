# `kernel/net/ip/routing/transactions.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::RoutingTable::LeaseFlow::advance`](transactions.cppm.md#fn-269f4e137317f8c7)
- [`xv::net::RoutingTable::LeaseFlow::append`](transactions.cppm.md#fn-6b885b8999d9b867)
- [`xv::net::RoutingTable::LeaseFlow::capacity`](transactions.cppm.md#fn-18e2fddc7060f7f4)
- [`xv::net::RoutingTable::LeaseFlow::describe`](transactions.cppm.md#fn-c14c754bbef8e08e)
- [`xv::net::RoutingTable::LeaseFlow::matched`](transactions.cppm.md#fn-29bfe46da38b52ec)
- [`xv::net::RoutingTable::LeaseFlow::more`](transactions.cppm.md#fn-350a307026829166)
- [`xv::net::RoutingTable::LeaseFlow::remove_previous`](transactions.cppm.md#fn-885cf423cbd43467)
- [`xv::net::RoutingTable::LeaseFlow::same_count`](transactions.cppm.md#fn-e0743a2fb7252cbe)
- [`xv::net::RoutingTable::LeaseFlow::success`](transactions.cppm.md#fn-7913566b2a52b62c)
- [`xv::net::RoutingTable::LeaseFlow::valid_prefix`](transactions.cppm.md#fn-1293a856a0659b9d)
- [`xv::net::RoutingTable::MutationFlow::append`](transactions.cppm.md#fn-bad36e9afba9f2b0)
- [`xv::net::RoutingTable::MutationFlow::busy`](transactions.cppm.md#fn-a69d4ed80360ab38)
- [`xv::net::RoutingTable::MutationFlow::capacity`](transactions.cppm.md#fn-d12401270264926e)
- [`xv::net::RoutingTable::MutationFlow::describe`](transactions.cppm.md#fn-769ba61f72a09fb1)
- [`xv::net::RoutingTable::MutationFlow::erase`](transactions.cppm.md#fn-213e5afccfdf1bc9)
- [`xv::net::RoutingTable::MutationFlow::exists`](transactions.cppm.md#fn-25645875f43838f6)
- [`xv::net::RoutingTable::MutationFlow::found`](transactions.cppm.md#fn-cb961aa275ff6fcf)
- [`xv::net::RoutingTable::MutationFlow::full`](transactions.cppm.md#fn-3cf47bd38575ac15)
- [`xv::net::RoutingTable::MutationFlow::invalid`](transactions.cppm.md#fn-2c68ac2028b10903)
- [`xv::net::RoutingTable::MutationFlow::lookup`](transactions.cppm.md#fn-b9d1892d8a163e28)
- [`xv::net::RoutingTable::MutationFlow::managed`](transactions.cppm.md#fn-1875d5ccc278ae64)
- [`xv::net::RoutingTable::MutationFlow::missing`](transactions.cppm.md#fn-cd095da091a9ec80)
- [`xv::net::RoutingTable::MutationFlow::valid`](transactions.cppm.md#fn-575c57ce6f585ba8)
- [`xv::net::RoutingTable::MutationFlow::valid_prefix`](transactions.cppm.md#fn-8bdf6b6dfdc61955)
- [`xv::net::RoutingTable::ValidationFlow::attributes`](transactions.cppm.md#fn-edc1e51135697ef4)
- [`xv::net::RoutingTable::ValidationFlow::connectivity`](transactions.cppm.md#fn-b00a07d005ed257b)
- [`xv::net::RoutingTable::ValidationFlow::gateway`](transactions.cppm.md#fn-4ca2a9433a85ea5f)
- [`xv::net::RoutingTable::ValidationFlow::success`](transactions.cppm.md#fn-183597bea3f6b7e4)
- [`xv::net::RoutingTable::ValidationFlow::valid_gateway`](transactions.cppm.md#fn-09e4d35446dd024d)

<a id="fn-269f4e137317f8c7"></a>

## `xv::net::RoutingTable::LeaseFlow::advance`

```doxygen
@fn static inline void xv::net::RoutingTable::LeaseFlow::advance(MatchContext &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6b885b8999d9b867"></a>

## `xv::net::RoutingTable::LeaseFlow::append`

```doxygen
@fn static inline void xv::net::RoutingTable::LeaseFlow::append(Context &c) noexcept
@brief 追加已验证的数据、路由或对象记录；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-18e2fddc7060f7f4"></a>

## `xv::net::RoutingTable::LeaseFlow::capacity`

```doxygen
@fn static inline bool xv::net::RoutingTable::LeaseFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-29bfe46da38b52ec"></a>

## `xv::net::RoutingTable::LeaseFlow::matched`

```doxygen
@fn static inline bool xv::net::RoutingTable::LeaseFlow::matched(MatchContext &c) noexcept
@brief 检查当前目标路由是否在对应租约路由组中；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-350a307026829166"></a>

## `xv::net::RoutingTable::LeaseFlow::more`

```doxygen
@fn static inline bool xv::net::RoutingTable::LeaseFlow::more(MatchContext &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-885cf423cbd43467"></a>

## `xv::net::RoutingTable::LeaseFlow::remove_previous`

```doxygen
@fn static inline void xv::net::RoutingTable::LeaseFlow::remove_previous(Context &c) noexcept
@brief 移除同一接口旧租约的路由组；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e0743a2fb7252cbe"></a>

## `xv::net::RoutingTable::LeaseFlow::same_count`

```doxygen
@fn static inline bool xv::net::RoutingTable::LeaseFlow::same_count(MatchContext &c) noexcept
@brief 检查当前接口的租约路由数量是否与目标组一致；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7913566b2a52b62c"></a>

## `xv::net::RoutingTable::LeaseFlow::success`

```doxygen
@fn static inline void xv::net::RoutingTable::LeaseFlow::success(MatchContext &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c14c754bbef8e08e"></a>

## `xv::net::RoutingTable::LeaseFlow::describe`

```doxygen
@fn static inline void xv::net::RoutingTable::LeaseFlow::describe(State &c) noexcept
@brief 构造本阶段使用的具名值描述；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1293a856a0659b9d"></a>

## `xv::net::RoutingTable::LeaseFlow::valid_prefix`

```doxygen
@fn static inline bool xv::net::RoutingTable::LeaseFlow::valid_prefix(State &c) noexcept
@brief 检查配置前缀长度未超过 32 位；所属流程为 xv::net::RoutingTable::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bad36e9afba9f2b0"></a>

## `xv::net::RoutingTable::MutationFlow::append`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::append(Context &c) noexcept
@brief 追加已验证的数据、路由或对象记录；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a69d4ed80360ab38"></a>

## `xv::net::RoutingTable::MutationFlow::busy`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d12401270264926e"></a>

## `xv::net::RoutingTable::MutationFlow::capacity`

```doxygen
@fn static inline bool xv::net::RoutingTable::MutationFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-769ba61f72a09fb1"></a>

## `xv::net::RoutingTable::MutationFlow::describe`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::describe(Context &c) noexcept
@brief 构造本阶段使用的具名值描述；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-213e5afccfdf1bc9"></a>

## `xv::net::RoutingTable::MutationFlow::erase`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::erase(Context &c) noexcept
@brief 删除选定记录并保持数组连续；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-25645875f43838f6"></a>

## `xv::net::RoutingTable::MutationFlow::exists`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::exists(Context &c) noexcept
@brief 记录同名或同路径对象已存在的错误；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cb961aa275ff6fcf"></a>

## `xv::net::RoutingTable::MutationFlow::found`

```doxygen
@fn static inline bool xv::net::RoutingTable::MutationFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3cf47bd38575ac15"></a>

## `xv::net::RoutingTable::MutationFlow::full`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2c68ac2028b10903"></a>

## `xv::net::RoutingTable::MutationFlow::invalid`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b9d1892d8a163e28"></a>

## `xv::net::RoutingTable::MutationFlow::lookup`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::lookup(Context &c) noexcept
@brief 查找规则目标、记录或链内对象；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1875d5ccc278ae64"></a>

## `xv::net::RoutingTable::MutationFlow::managed`

```doxygen
@fn static inline bool xv::net::RoutingTable::MutationFlow::managed(Context &c) noexcept
@brief 检查路由是否由地址或租约配置自动管理；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cd095da091a9ec80"></a>

## `xv::net::RoutingTable::MutationFlow::missing`

```doxygen
@fn static inline void xv::net::RoutingTable::MutationFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-575c57ce6f585ba8"></a>

## `xv::net::RoutingTable::MutationFlow::valid`

```doxygen
@fn static inline bool xv::net::RoutingTable::MutationFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8bdf6b6dfdc61955"></a>

## `xv::net::RoutingTable::MutationFlow::valid_prefix`

```doxygen
@fn static inline bool xv::net::RoutingTable::MutationFlow::valid_prefix(Context &c) noexcept
@brief 检查配置前缀长度未超过 32 位；所属流程为 xv::net::RoutingTable::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-edc1e51135697ef4"></a>

## `xv::net::RoutingTable::ValidationFlow::attributes`

```doxygen
@fn static inline bool xv::net::RoutingTable::ValidationFlow::attributes(Context &c) noexcept
@brief 检查路由入口属性及接口归属；所属流程为 xv::net::RoutingTable::ValidationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b00a07d005ed257b"></a>

## `xv::net::RoutingTable::ValidationFlow::connectivity`

```doxygen
@fn static inline void xv::net::RoutingTable::ValidationFlow::connectivity(Context &c) noexcept
@brief 扫描接口地址以确认网关直连；所属流程为 xv::net::RoutingTable::ValidationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4ca2a9433a85ea5f"></a>

## `xv::net::RoutingTable::ValidationFlow::gateway`

```doxygen
@fn static inline bool xv::net::RoutingTable::ValidationFlow::gateway(Context &c) noexcept
@brief 检查租约网关的地址和子网约束；所属流程为 xv::net::RoutingTable::ValidationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-183597bea3f6b7e4"></a>

## `xv::net::RoutingTable::ValidationFlow::success`

```doxygen
@fn static inline void xv::net::RoutingTable::ValidationFlow::success(Context &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::RoutingTable::ValidationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-09e4d35446dd024d"></a>

## `xv::net::RoutingTable::ValidationFlow::valid_gateway`

```doxygen
@fn static inline bool xv::net::RoutingTable::ValidationFlow::valid_gateway(Context &c) noexcept
@brief 检查网关直连、主机地址与广播约束；所属流程为 xv::net::RoutingTable::ValidationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
