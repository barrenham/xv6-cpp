# `kernel/net/socket/tcp/options.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpMssFlow::advance`](options.cppm.md#fn-7ae095eb5737c2b2)
- [`xv::net::TcpMssFlow::complete`](options.cppm.md#fn-6264947cba50f83f)
- [`xv::net::TcpMssFlow::end`](options.cppm.md#fn-60fcd3adc5a1867a)
- [`xv::net::TcpMssFlow::inspect`](options.cppm.md#fn-0b503816a1738653)
- [`xv::net::TcpMssFlow::more`](options.cppm.md#fn-ead9f9ba223aed49)
- [`xv::net::TcpMssFlow::mss`](options.cppm.md#fn-7ada9c01f672990c)
- [`xv::net::TcpMssFlow::nonzero`](options.cppm.md#fn-029e5a960f254cdf)
- [`xv::net::TcpMssFlow::padding`](options.cppm.md#fn-53992eac0251a7e1)
- [`xv::net::TcpMssFlow::read_mss`](options.cppm.md#fn-e82016bd5e3c6a50)
- [`xv::net::TcpMssFlow::remember`](options.cppm.md#fn-3213e32bee1ee29a)
- [`xv::net::TcpMssFlow::skip_padding`](options.cppm.md#fn-25af35db5cf9b7bc)
- [`xv::net::TcpMssFlow::unique_mss`](options.cppm.md#fn-b1a7965c44c91717)
- [`xv::net::TcpMssFlow::valid_header`](options.cppm.md#fn-f5de0773248bcbf7)
- [`xv::net::TcpMssFlow::valid_length`](options.cppm.md#fn-08963a4d48e1eb32)

<a id="fn-7ae095eb5737c2b2"></a>

## `xv::net::TcpMssFlow::advance`

```doxygen
@fn static inline void xv::net::TcpMssFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6264947cba50f83f"></a>

## `xv::net::TcpMssFlow::complete`

```doxygen
@fn static inline void xv::net::TcpMssFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-60fcd3adc5a1867a"></a>

## `xv::net::TcpMssFlow::end`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::end(Context &c) noexcept
@brief 检查选项迭代是否遇到结束标记；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0b503816a1738653"></a>

## `xv::net::TcpMssFlow::inspect`

```doxygen
@fn static inline void xv::net::TcpMssFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ead9f9ba223aed49"></a>

## `xv::net::TcpMssFlow::more`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7ada9c01f672990c"></a>

## `xv::net::TcpMssFlow::mss`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::mss(Context &c) noexcept
@brief 检查当前 TCP 选项是否为 MSS；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-029e5a960f254cdf"></a>

## `xv::net::TcpMssFlow::nonzero`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::nonzero(Context &c) noexcept
@brief 检查解码后的 MSS 是否非零；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-53992eac0251a7e1"></a>

## `xv::net::TcpMssFlow::padding`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::padding(Context &c) noexcept
@brief 检查当前选项是否为填充字节；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e82016bd5e3c6a50"></a>

## `xv::net::TcpMssFlow::read_mss`

```doxygen
@fn static inline void xv::net::TcpMssFlow::read_mss(Context &c) noexcept
@brief 解码当前 MSS 选项数值；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3213e32bee1ee29a"></a>

## `xv::net::TcpMssFlow::remember`

```doxygen
@fn static inline void xv::net::TcpMssFlow::remember(Context &c) noexcept
@brief 记录已经见过 MSS 选项；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-25af35db5cf9b7bc"></a>

## `xv::net::TcpMssFlow::skip_padding`

```doxygen
@fn static inline void xv::net::TcpMssFlow::skip_padding(Context &c) noexcept
@brief 跳过 TCP 选项填充字节；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b1a7965c44c91717"></a>

## `xv::net::TcpMssFlow::unique_mss`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::unique_mss(Context &c) noexcept
@brief 检查 MSS 选项长度正确且未重复；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f5de0773248bcbf7"></a>

## `xv::net::TcpMssFlow::valid_header`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::valid_header(Context &c) noexcept
@brief 检查本协议固定首部与长度约束；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-08963a4d48e1eb32"></a>

## `xv::net::TcpMssFlow::valid_length`

```doxygen
@fn static inline bool xv::net::TcpMssFlow::valid_length(Context &c) noexcept
@brief 检查当前 TCP 选项长度未越过首部范围；所属流程为 xv::net::TcpMssFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
