# `kernel/net/ip/ipv4/send.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::Ipv4Layer::SendFlow::allowed`](send.cppm.md#fn-24606a66436b93e0)
- [`xv::net::Ipv4Layer::SendFlow::bootstrap_allowed`](send.cppm.md#fn-c1275fde81a85b66)
- [`xv::net::Ipv4Layer::SendFlow::forbidden`](send.cppm.md#fn-ebc7333ee0ff1522)
- [`xv::net::Ipv4Layer::SendFlow::invalid`](send.cppm.md#fn-af39c90c7d07eeeb)
- [`xv::net::Ipv4Layer::SendFlow::select_route`](send.cppm.md#fn-9c054422afe627fd)
- [`xv::net::Ipv4Layer::SendFlow::send`](send.cppm.md#fn-7f9ed04217d4f257)
- [`xv::net::Ipv4Layer::SendFlow::send_bootstrap`](send.cppm.md#fn-22f8821a0cd84633)
- [`xv::net::Ipv4Layer::SendFlow::too_large`](send.cppm.md#fn-f8246aacb40bd079)
- [`xv::net::Ipv4Layer::SendFlow::unreachable`](send.cppm.md#fn-af893104602b21e1)
- [`xv::net::Ipv4Layer::SendFlow::valid_bootstrap_interface`](send.cppm.md#fn-d12742cabb58ede7)
- [`xv::net::Ipv4Layer::SendFlow::valid_bootstrap_packet`](send.cppm.md#fn-20c5c7fd0222f875)
- [`xv::net::Ipv4Layer::SendFlow::valid_bootstrap_ports`](send.cppm.md#fn-b917ded37dce995d)
- [`xv::net::Ipv4Layer::SendFlow::valid_packet`](send.cppm.md#fn-ee338dd942d3315e)
- [`xv::net::Ipv4Layer::SendFlow::write_bootstrap_header`](send.cppm.md#fn-34d0af948d2d63b2)
- [`xv::net::Ipv4Layer::SendFlow::write_header`](send.cppm.md#fn-ab6d13fa42aa9bbf)

<a id="fn-24606a66436b93e0"></a>

## `xv::net::Ipv4Layer::SendFlow::allowed`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::allowed(Context &c) noexcept
@brief 检查发送报文是否通过 OUTPUT 过滤；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1275fde81a85b66"></a>

## `xv::net::Ipv4Layer::SendFlow::bootstrap_allowed`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::bootstrap_allowed(Context &c) noexcept
@brief 检查引导报文是否通过 OUTPUT 过滤；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ebc7333ee0ff1522"></a>

## `xv::net::Ipv4Layer::SendFlow::forbidden`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::forbidden(Context &c) noexcept
@brief 记录输出过滤拒绝的负 EACCES；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-af39c90c7d07eeeb"></a>

## `xv::net::Ipv4Layer::SendFlow::invalid`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9c054422afe627fd"></a>

## `xv::net::Ipv4Layer::SendFlow::select_route`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::select_route(Context &c) noexcept
@brief 为发送报文选择路由；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7f9ed04217d4f257"></a>

## `xv::net::Ipv4Layer::SendFlow::send`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-22f8821a0cd84633"></a>

## `xv::net::Ipv4Layer::SendFlow::send_bootstrap`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::send_bootstrap(Context &c) noexcept
@brief 通过接口引导发送能力提交报文；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f8246aacb40bd079"></a>

## `xv::net::Ipv4Layer::SendFlow::too_large`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-af893104602b21e1"></a>

## `xv::net::Ipv4Layer::SendFlow::unreachable`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d12742cabb58ede7"></a>

## `xv::net::Ipv4Layer::SendFlow::valid_bootstrap_interface`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::valid_bootstrap_interface(Context &c) noexcept
@brief 检查引导接口身份、状态与源地址；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-20c5c7fd0222f875"></a>

## `xv::net::Ipv4Layer::SendFlow::valid_bootstrap_packet`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::valid_bootstrap_packet(Context &c) noexcept
@brief 检查引导报文最短长度与出口 MTU；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b917ded37dce995d"></a>

## `xv::net::Ipv4Layer::SendFlow::valid_bootstrap_ports`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::valid_bootstrap_ports(Context &c) noexcept
@brief 检查引导 UDP 的 68 到 67 端口；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ee338dd942d3315e"></a>

## `xv::net::Ipv4Layer::SendFlow::valid_packet`

```doxygen
@fn static inline bool xv::net::Ipv4Layer::SendFlow::valid_packet(Context &c) noexcept
@brief 检查发送 Packet 的有效性、最短长度与 MTU；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-34d0af948d2d63b2"></a>

## `xv::net::Ipv4Layer::SendFlow::write_bootstrap_header`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::write_bootstrap_header(Context &c) noexcept
@brief 编码 DHCP 引导广播的 IPv4 首部；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ab6d13fa42aa9bbf"></a>

## `xv::net::Ipv4Layer::SendFlow::write_header`

```doxygen
@fn static inline void xv::net::Ipv4Layer::SendFlow::write_header(Context &c) noexcept
@brief 编码普通发送的 IPv4 首部；所属流程为 xv::net::Ipv4Layer::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
