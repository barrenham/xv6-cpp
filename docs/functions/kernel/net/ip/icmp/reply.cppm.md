# `kernel/net/ip/icmp/reply.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::IcmpProtocol::ReplyFlow::allocate_echo`](reply.cppm.md#fn-3cb0adef8addeba6)
- [`xv::net::IcmpProtocol::ReplyFlow::allocate_unreachable`](reply.cppm.md#fn-dcbcc042baba21a0)
- [`xv::net::IcmpProtocol::ReplyFlow::allocated`](reply.cppm.md#fn-852dd5c1c93a4140)
- [`xv::net::IcmpProtocol::ReplyFlow::encode_echo`](reply.cppm.md#fn-4d43e9115d7aae99)
- [`xv::net::IcmpProtocol::ReplyFlow::encode_unreachable`](reply.cppm.md#fn-ed33d834910684ef)
- [`xv::net::IcmpProtocol::ReplyFlow::local`](reply.cppm.md#fn-762340452778955f)
- [`xv::net::IcmpProtocol::ReplyFlow::send`](reply.cppm.md#fn-8ff0f9e2741ca51d)
- [`xv::net::IcmpProtocol::ReplyFlow::valid_echo`](reply.cppm.md#fn-7e5c8b4419b74bbb)

<a id="fn-3cb0adef8addeba6"></a>

## `xv::net::IcmpProtocol::ReplyFlow::allocate_echo`

```doxygen
@fn static inline void xv::net::IcmpProtocol::ReplyFlow::allocate_echo(Context &c) noexcept
@brief 分配与请求长度一致的 Echo 回复包；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-dcbcc042baba21a0"></a>

## `xv::net::IcmpProtocol::ReplyFlow::allocate_unreachable`

```doxygen
@fn static inline void xv::net::IcmpProtocol::ReplyFlow::allocate_unreachable(Context &c) noexcept
@brief 计算引用长度并分配 ICMP 不可达回复包；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-852dd5c1c93a4140"></a>

## `xv::net::IcmpProtocol::ReplyFlow::allocated`

```doxygen
@fn static inline bool xv::net::IcmpProtocol::ReplyFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4d43e9115d7aae99"></a>

## `xv::net::IcmpProtocol::ReplyFlow::encode_echo`

```doxygen
@fn static inline void xv::net::IcmpProtocol::ReplyFlow::encode_echo(Context &c) noexcept
@brief 复制 ICMP Echo 载荷并更新类型和校验和；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ed33d834910684ef"></a>

## `xv::net::IcmpProtocol::ReplyFlow::encode_unreachable`

```doxygen
@fn static inline void xv::net::IcmpProtocol::ReplyFlow::encode_unreachable(Context &c) noexcept
@brief 编码端口不可达及原报文引用；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-762340452778955f"></a>

## `xv::net::IcmpProtocol::ReplyFlow::local`

```doxygen
@fn static inline bool xv::net::IcmpProtocol::ReplyFlow::local(Context &c) noexcept
@brief 检查报文是否应向本地协议路径交付；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8ff0f9e2741ca51d"></a>

## `xv::net::IcmpProtocol::ReplyFlow::send`

```doxygen
@fn static inline void xv::net::IcmpProtocol::ReplyFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7e5c8b4419b74bbb"></a>

## `xv::net::IcmpProtocol::ReplyFlow::valid_echo`

```doxygen
@fn static inline bool xv::net::IcmpProtocol::ReplyFlow::valid_echo(Context &c) noexcept
@brief 检查 ICMP Echo 格式、校验和与本地目的；所属流程为 xv::net::IcmpProtocol::ReplyFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
