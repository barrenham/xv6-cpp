# `kernel/net/core/control/execute.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::NetworkManager::ExecuteFlow::bridge`](execute.cppm.md#fn-64c5b931ad4b98c2)
- [`xv::net::NetworkManager::ExecuteFlow::bridge_command`](execute.cppm.md#fn-890bfa31711ecf4a)
- [`xv::net::NetworkManager::ExecuteFlow::complete`](execute.cppm.md#fn-3f2efc1df7bce8f5)
- [`xv::net::NetworkManager::ExecuteFlow::configured`](execute.cppm.md#fn-cf03604b9bf51fbe)
- [`xv::net::NetworkManager::ExecuteFlow::dhcp`](execute.cppm.md#fn-b42dcedf830212b7)
- [`xv::net::NetworkManager::ExecuteFlow::dhcp_command`](execute.cppm.md#fn-513aa7c6399836f6)
- [`xv::net::NetworkManager::ExecuteFlow::find_interface`](execute.cppm.md#fn-5004a3d380254ed4)
- [`xv::net::NetworkManager::ExecuteFlow::found`](execute.cppm.md#fn-de5b1c668e8cc020)
- [`xv::net::NetworkManager::ExecuteFlow::inspect`](execute.cppm.md#fn-9d01536fa286c9f4)
- [`xv::net::NetworkManager::ExecuteFlow::inspect_neighbor`](execute.cppm.md#fn-2cf61e5cba982108)
- [`xv::net::NetworkManager::ExecuteFlow::inspection`](execute.cppm.md#fn-bfc6b02da954ed65)
- [`xv::net::NetworkManager::ExecuteFlow::invalid`](execute.cppm.md#fn-41dab7e3b4af2c74)
- [`xv::net::NetworkManager::ExecuteFlow::invalidate`](execute.cppm.md#fn-ddf2754e720c0c36)
- [`xv::net::NetworkManager::ExecuteFlow::missing`](execute.cppm.md#fn-52836a2f2eb7449a)
- [`xv::net::NetworkManager::ExecuteFlow::mutate`](execute.cppm.md#fn-436b7877c10e038f)
- [`xv::net::NetworkManager::ExecuteFlow::needs_invalidation`](execute.cppm.md#fn-a487a8a6f6e85b66)
- [`xv::net::NetworkManager::ExecuteFlow::neighbor`](execute.cppm.md#fn-5420eca1ea98975a)
- [`xv::net::NetworkManager::ExecuteFlow::neighbor_operation`](execute.cppm.md#fn-dd63e78bb7d9bb15)
- [`xv::net::NetworkManager::ExecuteFlow::valid`](execute.cppm.md#fn-0e67e8bf33318cb1)

<a id="fn-64c5b931ad4b98c2"></a>

## `xv::net::NetworkManager::ExecuteFlow::bridge`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::bridge(Context &c) noexcept
@brief 检查管理操作是否属于网桥命令范围；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-890bfa31711ecf4a"></a>

## `xv::net::NetworkManager::ExecuteFlow::bridge_command`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::bridge_command(Context &c) noexcept
@brief 向注入的网桥管理服务转交命令；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3f2efc1df7bce8f5"></a>

## `xv::net::NetworkManager::ExecuteFlow::complete`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf03604b9bf51fbe"></a>

## `xv::net::NetworkManager::ExecuteFlow::configured`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::configured(Context &c) noexcept
@brief 检查启动配置结果或执行锁内配置阶段；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-b42dcedf830212b7"></a>

## `xv::net::NetworkManager::ExecuteFlow::dhcp`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::dhcp(Context &c) noexcept
@brief 检查管理操作是否属于 DHCP 命令范围；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-513aa7c6399836f6"></a>

## `xv::net::NetworkManager::ExecuteFlow::dhcp_command`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::dhcp_command(Context &c) noexcept
@brief 向注入的 DHCP 管理服务转交命令；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5004a3d380254ed4"></a>

## `xv::net::NetworkManager::ExecuteFlow::find_interface`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::find_interface(Context &c) noexcept
@brief 按管理请求选择接口对象；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-de5b1c668e8cc020"></a>

## `xv::net::NetworkManager::ExecuteFlow::found`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9d01536fa286c9f4"></a>

## `xv::net::NetworkManager::ExecuteFlow::inspect`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2cf61e5cba982108"></a>

## `xv::net::NetworkManager::ExecuteFlow::inspect_neighbor`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::inspect_neighbor(Context &c) noexcept
@brief 序列化一条邻居快照到管理请求；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bfc6b02da954ed65"></a>

## `xv::net::NetworkManager::ExecuteFlow::inspection`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::inspection(Context &c) noexcept
@brief 检查本次管理命令是否为只读查询；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-41dab7e3b4af2c74"></a>

## `xv::net::NetworkManager::ExecuteFlow::invalid`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ddf2754e720c0c36"></a>

## `xv::net::NetworkManager::ExecuteFlow::invalidate`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-52836a2f2eb7449a"></a>

## `xv::net::NetworkManager::ExecuteFlow::missing`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-436b7877c10e038f"></a>

## `xv::net::NetworkManager::ExecuteFlow::mutate`

```doxygen
@fn static inline void xv::net::NetworkManager::ExecuteFlow::mutate(Context &c) noexcept
@brief 执行网络管理的受保护修改操作；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a487a8a6f6e85b66"></a>

## `xv::net::NetworkManager::ExecuteFlow::needs_invalidation`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::needs_invalidation(Context &c) noexcept
@brief 检查成功修改是否需要锁外失效通知；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5420eca1ea98975a"></a>

## `xv::net::NetworkManager::ExecuteFlow::neighbor`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::neighbor(Context &c) noexcept
@brief 检查本次命令是否查询邻居记录；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dd63e78bb7d9bb15"></a>

## `xv::net::NetworkManager::ExecuteFlow::neighbor_operation`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::neighbor_operation(Context &c) noexcept
@brief 检查管理命令是否只涉及邻居查询或清空；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0e67e8bf33318cb1"></a>

## `xv::net::NetworkManager::ExecuteFlow::valid`

```doxygen
@fn static inline bool xv::net::NetworkManager::ExecuteFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::NetworkManager::ExecuteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
