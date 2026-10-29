# `kernel/net/filter/flows/accept.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::Firewall::AcceptFlow::decode`](accept.cppm.md#fn-77b8784747c9cfc7)
- [`xv::net::Firewall::AcceptFlow::evaluate`](accept.cppm.md#fn-b8e5bad3d81a17b0)
- [`xv::net::Firewall::AcceptFlow::input`](accept.cppm.md#fn-240b143d0738a1f7)
- [`xv::net::Firewall::AcceptFlow::output`](accept.cppm.md#fn-8f988547c4b4d4fa)
- [`xv::net::Firewall::AcceptFlow::select_input`](accept.cppm.md#fn-e1a15d5c53e278c9)
- [`xv::net::Firewall::AcceptFlow::select_output`](accept.cppm.md#fn-20756f0369a92487)

<a id="fn-77b8784747c9cfc7"></a>

## `xv::net::Firewall::AcceptFlow::decode`

```doxygen
@fn static inline bool xv::net::Firewall::AcceptFlow::decode(Context &c) noexcept
@brief 按已验证格式解码本阶段字段；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b8e5bad3d81a17b0"></a>

## `xv::net::Firewall::AcceptFlow::evaluate`

```doxygen
@fn static inline void xv::net::Firewall::AcceptFlow::evaluate(Context &c) noexcept
@brief 在防火墙锁下执行纯规则核心；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-240b143d0738a1f7"></a>

## `xv::net::Firewall::AcceptFlow::input`

```doxygen
@fn static inline bool xv::net::Firewall::AcceptFlow::input(Context &c) noexcept
@brief 检查过滤 hook 是否是 INPUT；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8f988547c4b4d4fa"></a>

## `xv::net::Firewall::AcceptFlow::output`

```doxygen
@fn static inline bool xv::net::Firewall::AcceptFlow::output(Context &c) noexcept
@brief 检查过滤 hook 是否是 OUTPUT；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e1a15d5c53e278c9"></a>

## `xv::net::Firewall::AcceptFlow::select_input`

```doxygen
@fn static inline void xv::net::Firewall::AcceptFlow::select_input(Context &c) noexcept
@brief 选择 INPUT 内建链与入站接口元数据；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-20756f0369a92487"></a>

## `xv::net::Firewall::AcceptFlow::select_output`

```doxygen
@fn static inline void xv::net::Firewall::AcceptFlow::select_output(Context &c) noexcept
@brief 选择 OUTPUT 内建链与出站接口元数据；所属流程为 xv::net::Firewall::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
