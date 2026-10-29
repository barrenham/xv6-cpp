# `kernel/net/core/control/details.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::NetworkManager::InspectFlow::address`](details.cppm.md#fn-0f2b62439e5067fd)
- [`xv::net::NetworkManager::InspectFlow::commit`](details.cppm.md#fn-5aa458f560df8af6)
- [`xv::net::NetworkManager::InspectFlow::describe_address`](details.cppm.md#fn-329375e2dc71e795)
- [`xv::net::NetworkManager::InspectFlow::describe_link`](details.cppm.md#fn-14fd6f3e5c06bcc7)
- [`xv::net::NetworkManager::InspectFlow::describe_route`](details.cppm.md#fn-4340eaa2dc683c1a)
- [`xv::net::NetworkManager::InspectFlow::find_interface`](details.cppm.md#fn-9b292c1e39df32f3)
- [`xv::net::NetworkManager::InspectFlow::found`](details.cppm.md#fn-cdb7663676790ea1)
- [`xv::net::NetworkManager::InspectFlow::identify`](details.cppm.md#fn-833ca3330972ac37)
- [`xv::net::NetworkManager::InspectFlow::initialize`](details.cppm.md#fn-d823790a0a19474a)
- [`xv::net::NetworkManager::InspectFlow::invalid`](details.cppm.md#fn-33b8b7bfe7e434c8)
- [`xv::net::NetworkManager::InspectFlow::missing`](details.cppm.md#fn-5d87624de6aa09c7)
- [`xv::net::NetworkManager::InspectFlow::operation`](details.cppm.md#fn-7e07d04112b40113)
- [`xv::net::NetworkManager::InspectFlow::route`](details.cppm.md#fn-37065cef07d640b9)
- [`xv::net::NetworkManager::MutationFlow::add_address`](details.cppm.md#fn-a6672471cc99cec2)
- [`xv::net::NetworkManager::MutationFlow::add_route`](details.cppm.md#fn-507c4c11e33c4a9b)
- [`xv::net::NetworkManager::MutationFlow::invalid`](details.cppm.md#fn-63087f1b8a3249aa)
- [`xv::net::NetworkManager::MutationFlow::link`](details.cppm.md#fn-642b81cf64a14fcb)
- [`xv::net::NetworkManager::MutationFlow::loopback`](details.cppm.md#fn-a1e3490fec088053)
- [`xv::net::NetworkManager::MutationFlow::operation`](details.cppm.md#fn-6bcfa29e7e373dc1)
- [`xv::net::NetworkManager::MutationFlow::remove_address`](details.cppm.md#fn-2c4a94cd3009838f)
- [`xv::net::NetworkManager::MutationFlow::remove_route`](details.cppm.md#fn-c1992ecc59a37c86)
- [`xv::net::NetworkManager::MutationFlow::route_operation`](details.cppm.md#fn-e9c88d874b98cac1)
- [`xv::net::NetworkManager::MutationFlow::unsupported`](details.cppm.md#fn-0823ecaa5cffb5df)
- [`xv::net::NetworkManager::MutationFlow::valid_link_flags`](details.cppm.md#fn-83b7b34f3731ee15)
- [`xv::net::NetworkManager::MutationFlow::valid_route`](details.cppm.md#fn-4912bd060efbd82d)

<a id="fn-0f2b62439e5067fd"></a>

## `xv::net::NetworkManager::InspectFlow::address`

```doxygen
@fn static inline bool xv::net::NetworkManager::InspectFlow::address(Context &c) noexcept
@brief 检查地址查询索引是否存在；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5aa458f560df8af6"></a>

## `xv::net::NetworkManager::InspectFlow::commit`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-329375e2dc71e795"></a>

## `xv::net::NetworkManager::InspectFlow::describe_address`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::describe_address(Context &c) noexcept
@brief 将地址快照写入管理查询结果；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-14fd6f3e5c06bcc7"></a>

## `xv::net::NetworkManager::InspectFlow::describe_link`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::describe_link(Context &c) noexcept
@brief 将接口属性、地址数量和统计写入查询结果；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4340eaa2dc683c1a"></a>

## `xv::net::NetworkManager::InspectFlow::describe_route`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::describe_route(Context &c) noexcept
@brief 将路由值快照写入管理查询结果；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9b292c1e39df32f3"></a>

## `xv::net::NetworkManager::InspectFlow::find_interface`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::find_interface(Context &c) noexcept
@brief 按管理请求选择接口对象；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cdb7663676790ea1"></a>

## `xv::net::NetworkManager::InspectFlow::found`

```doxygen
@fn static inline bool xv::net::NetworkManager::InspectFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-833ca3330972ac37"></a>

## `xv::net::NetworkManager::InspectFlow::identify`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::identify(Context &c) noexcept
@brief 保存本次接口标识或租约归属；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d823790a0a19474a"></a>

## `xv::net::NetworkManager::InspectFlow::initialize`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::initialize(Context &c) noexcept
@brief 初始化当前业务阶段状态；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-33b8b7bfe7e434c8"></a>

## `xv::net::NetworkManager::InspectFlow::invalid`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5d87624de6aa09c7"></a>

## `xv::net::NetworkManager::InspectFlow::missing`

```doxygen
@fn static inline void xv::net::NetworkManager::InspectFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-37065cef07d640b9"></a>

## `xv::net::NetworkManager::InspectFlow::route`

```doxygen
@fn static inline bool xv::net::NetworkManager::InspectFlow::route(Context &c) noexcept
@brief 查询指定索引的路由值快照；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7e07d04112b40113"></a>

## `xv::net::NetworkManager::InspectFlow::operation`

```doxygen
@fn static bool xv::net::NetworkManager::InspectFlow::operation(Context &c) noexcept
@brief 检查命令编号是否等于模板指定编号；所属流程为 xv::net::NetworkManager::InspectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a6672471cc99cec2"></a>

## `xv::net::NetworkManager::MutationFlow::add_address`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::add_address(Context &c) noexcept
@brief 调用地址服务增加静态地址并保存返回码；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-507c4c11e33c4a9b"></a>

## `xv::net::NetworkManager::MutationFlow::add_route`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::add_route(Context &c) noexcept
@brief 调用路由表增加手工路由并保存返回码；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-63087f1b8a3249aa"></a>

## `xv::net::NetworkManager::MutationFlow::invalid`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-642b81cf64a14fcb"></a>

## `xv::net::NetworkManager::MutationFlow::link`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::link(Context &c) noexcept
@brief 按请求设置接口运行状态；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a1e3490fec088053"></a>

## `xv::net::NetworkManager::MutationFlow::loopback`

```doxygen
@fn static inline bool xv::net::NetworkManager::MutationFlow::loopback(Context &c) noexcept
@brief 检查目标接口是否为保留的回环接口；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2c4a94cd3009838f"></a>

## `xv::net::NetworkManager::MutationFlow::remove_address`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::remove_address(Context &c) noexcept
@brief 调用地址服务删除静态地址；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1992ecc59a37c86"></a>

## `xv::net::NetworkManager::MutationFlow::remove_route`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::remove_route(Context &c) noexcept
@brief 调用路由表删除手工路由；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e9c88d874b98cac1"></a>

## `xv::net::NetworkManager::MutationFlow::route_operation`

```doxygen
@fn static inline bool xv::net::NetworkManager::MutationFlow::route_operation(Context &c) noexcept
@brief 检查管理命令是否为路由增删；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0823ecaa5cffb5df"></a>

## `xv::net::NetworkManager::MutationFlow::unsupported`

```doxygen
@fn static inline void xv::net::NetworkManager::MutationFlow::unsupported(Context &c) noexcept
@brief 记录操作不支持的业务错误；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-83b7b34f3731ee15"></a>

## `xv::net::NetworkManager::MutationFlow::valid_link_flags`

```doxygen
@fn static inline bool xv::net::NetworkManager::MutationFlow::valid_link_flags(Context &c) noexcept
@brief 检查链路修改请求没有未知标志；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4912bd060efbd82d"></a>

## `xv::net::NetworkManager::MutationFlow::valid_route`

```doxygen
@fn static inline bool xv::net::NetworkManager::MutationFlow::valid_route(Context &c) noexcept
@brief 检查手工路由命令的前缀与保留标志；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6bcfa29e7e373dc1"></a>

## `xv::net::NetworkManager::MutationFlow::operation`

```doxygen
@fn static bool xv::net::NetworkManager::MutationFlow::operation(Context &c) noexcept
@brief 检查命令编号是否等于模板指定编号；所属流程为 xv::net::NetworkManager::MutationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
