# `kernel/net/filter/flows/decoder.h`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::filter::MetadataDecodeFlow::describe`](decoder.h.md#fn-f25230c3bc18af2d)
- [`xv::net::filter::MetadataDecodeFlow::inspect`](decoder.h.md#fn-6df2b1d715db3da7)
- [`xv::net::filter::MetadataDecodeFlow::inspect_tcp`](decoder.h.md#fn-794763ad7751098b)
- [`xv::net::filter::MetadataDecodeFlow::layout`](decoder.h.md#fn-1e3155ddc59682fe)
- [`xv::net::filter::MetadataDecodeFlow::minimum`](decoder.h.md#fn-a64a05e219cbc428)
- [`xv::net::filter::MetadataDecodeFlow::ports`](decoder.h.md#fn-0ac45ec2d8800d63)
- [`xv::net::filter::MetadataDecodeFlow::success`](decoder.h.md#fn-77c097e9e769405b)
- [`xv::net::filter::MetadataDecodeFlow::tcp`](decoder.h.md#fn-65ef638a4322cada)
- [`xv::net::filter::MetadataDecodeFlow::tcp_layout`](decoder.h.md#fn-5552f9653a6f89af)
- [`xv::net::filter::MetadataDecodeFlow::tcp_minimum`](decoder.h.md#fn-444aad04bab0fbb1)
- [`xv::net::filter::MetadataDecodeFlow::udp`](decoder.h.md#fn-7c9846a9fad286c2)
- [`xv::net::filter::MetadataDecodeFlow::udp_layout`](decoder.h.md#fn-48228cd9d363c977)
- [`xv::net::filter::decode`](../decoder.h.md#fn-407caf51f07ec782)

<a id="fn-f25230c3bc18af2d"></a>

## `xv::net::filter::MetadataDecodeFlow::describe`

```doxygen
@fn static inline void xv::net::filter::MetadataDecodeFlow::describe(Context &c) noexcept
@brief 构造本阶段使用的具名值描述；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6df2b1d715db3da7"></a>

## `xv::net::filter::MetadataDecodeFlow::inspect`

```doxygen
@fn static inline void xv::net::filter::MetadataDecodeFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-794763ad7751098b"></a>

## `xv::net::filter::MetadataDecodeFlow::inspect_tcp`

```doxygen
@fn static inline void xv::net::filter::MetadataDecodeFlow::inspect_tcp(Context &c) noexcept
@brief 读取已满足最短长度的 TCP 数据偏移；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e3155ddc59682fe"></a>

## `xv::net::filter::MetadataDecodeFlow::layout`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::layout(Context &c) noexcept
@brief 检查 IP 数据报长度、首部与分片约束；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a64a05e219cbc428"></a>

## `xv::net::filter::MetadataDecodeFlow::minimum`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::minimum(Context &c) noexcept
@brief 检查过滤解码的 IPv4 最短首部与版本；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0ac45ec2d8800d63"></a>

## `xv::net::filter::MetadataDecodeFlow::ports`

```doxygen
@fn static inline void xv::net::filter::MetadataDecodeFlow::ports(Context &c) noexcept
@brief 写入已验证的传输层端口元数据；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-77c097e9e769405b"></a>

## `xv::net::filter::MetadataDecodeFlow::success`

```doxygen
@fn static inline void xv::net::filter::MetadataDecodeFlow::success(Context &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-65ef638a4322cada"></a>

## `xv::net::filter::MetadataDecodeFlow::tcp`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::tcp(Context &c) noexcept
@brief 检查当前协议或对象是否为 TCP；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5552f9653a6f89af"></a>

## `xv::net::filter::MetadataDecodeFlow::tcp_layout`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::tcp_layout(Context &c) noexcept
@brief 检查 TCP 数据偏移是否位于载荷范围；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-444aad04bab0fbb1"></a>

## `xv::net::filter::MetadataDecodeFlow::tcp_minimum`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::tcp_minimum(Context &c) noexcept
@brief 检查载荷是否至少容纳 TCP 固定首部；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7c9846a9fad286c2"></a>

## `xv::net::filter::MetadataDecodeFlow::udp`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::udp(Context &c) noexcept
@brief 检查当前报文是否为 UDP；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-48228cd9d363c977"></a>

## `xv::net::filter::MetadataDecodeFlow::udp_layout`

```doxygen
@fn static inline bool xv::net::filter::MetadataDecodeFlow::udp_layout(Context &c) noexcept
@brief 检查 UDP 固定首部和长度字段；所属流程为 xv::net::filter::MetadataDecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
