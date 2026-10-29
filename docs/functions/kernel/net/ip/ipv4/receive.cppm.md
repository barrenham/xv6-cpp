# `kernel/net/ip/ipv4/receive.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::Ipv4Layer::ReceiveFlow::accepted_destination`](receive.cppm.md#fn-e3f2a6ea434b0494)
- [`xv::net::Ipv4Layer::ReceiveFlow::accepts_fragment`](receive.cppm.md#fn-619519352270cd99)
- [`xv::net::Ipv4Layer::ReceiveFlow::classify_destination`](receive.cppm.md#fn-16f3cef0ccd6dc43)
- [`xv::net::Ipv4Layer::ReceiveFlow::describe_datagram`](receive.cppm.md#fn-cf9e17e31e4979a7)
- [`xv::net::Ipv4Layer::ReceiveFlow::dispatch`](receive.cppm.md#fn-2d75e783c40e0704)
- [`xv::net::Ipv4Layer::ReceiveFlow::drop`](receive.cppm.md#fn-9a0842b556439b06)
- [`xv::net::Ipv4Layer::ReceiveFlow::eligible`](receive.cppm.md#fn-e3dba34ffac79eda)
- [`xv::net::Ipv4Layer::ReceiveFlow::fragmented`](receive.cppm.md#fn-83ece5d3c8a0128b)
- [`xv::net::Ipv4Layer::ReceiveFlow::input_allowed`](receive.cppm.md#fn-4da4456b12fdc577)
- [`xv::net::Ipv4Layer::ReceiveFlow::inspect`](receive.cppm.md#fn-588e57f2318ac7b6)
- [`xv::net::Ipv4Layer::ReceiveFlow::local`](receive.cppm.md#fn-06fb6433beef359b)
- [`xv::net::Ipv4Layer::ReceiveFlow::offer_bootstrap`](receive.cppm.md#fn-f70619742b3de0e1)
- [`xv::net::Ipv4Layer::ReceiveFlow::reassemble`](receive.cppm.md#fn-eb0a7874855ca4e1)
- [`xv::net::Ipv4Layer::ReceiveFlow::valid_header`](receive.cppm.md#fn-f6eb74c23a5d274d)
- [`xv::net::Ipv4Layer::ReceiveFlow::was_handled`](receive.cppm.md#fn-38d66fd17237fcae)

<a id="fn-e3f2a6ea434b0494"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::accepted_destination`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::accepted_destination(Context &c) noexcept
@brief 检查本地或引导报文目的是否允许；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-619519352270cd99"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::accepts_fragment`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::accepts_fragment(Context &c) noexcept
@brief 检查接口是否接受该分片目的；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-16f3cef0ccd6dc43"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::classify_destination`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::classify_destination(Context &c) noexcept
@brief 判断本地目的与 DHCP 引导候选；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf9e17e31e4979a7"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::describe_datagram`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::describe_datagram(Context &c) noexcept
@brief 构造已验证的 IP 数据报描述；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2d75e783c40e0704"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::dispatch`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::dispatch(Context &c) noexcept
@brief 按 IP 协议号向已经挂接的处理器交付；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9a0842b556439b06"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::drop`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::drop(Context &c) noexcept
@brief 计入接口接收丢弃统计；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e3dba34ffac79eda"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::eligible`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-83ece5d3c8a0128b"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::fragmented`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::fragmented(Context &c) noexcept
@brief 检查 IP 分片标志或偏移是否存在；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4da4456b12fdc577"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::input_allowed`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::input_allowed(Context &c) noexcept
@brief 检查完整数据报是否通过 INPUT 过滤；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-588e57f2318ac7b6"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::inspect`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-06fb6433beef359b"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::local`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::local(Context &c) noexcept
@brief 检查报文是否应向本地协议路径交付；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f70619742b3de0e1"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::offer_bootstrap`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::offer_bootstrap(Context &c) noexcept
@brief 向引导接收器提供已描述的完整数据报；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-eb0a7874855ca4e1"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::reassemble`

```doxygen
@fn static inline void xv::net::Ipv4Layer::ReceiveFlow::reassemble(Context &c) noexcept
@brief 尝试重组分片并在完成后锁外重新进入 IPv4 接收；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f6eb74c23a5d274d"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::valid_header`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::valid_header(Context &c) noexcept
@brief 检查本协议固定首部与长度约束；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-38d66fd17237fcae"></a>

## `xv::net::Ipv4Layer::ReceiveFlow::was_handled`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::ReceiveFlow::was_handled(Context &c) noexcept
@brief 检查接收器的已处理结果；所属流程为 xv::net::Ipv4Layer::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
