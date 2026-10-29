# `kernel/net/transport/tcp_protocol/transmit.cppm`

TcpProtocol 的内部具名动作与条件。流程关系和所有权约定见
[声明式协议流程](../../../../../network-tcp-protocol-flow.md)。

## 函数导航

- [`xv::net::TcpProtocol::TransmitFlow::AllocateSegment::run`](transmit.cppm.md#fn-dad4d9b77398de49)
- [`xv::net::TcpProtocol::TransmitFlow::CopyPayload::run`](transmit.cppm.md#fn-e464d81ed00cc2eb)
- [`xv::net::TcpProtocol::TransmitFlow::EncodeFixedHeader::run`](transmit.cppm.md#fn-834ff475a0a15669)
- [`xv::net::TcpProtocol::TransmitFlow::EncodeMssOption::run`](transmit.cppm.md#fn-1080bf11aca1f396)
- [`xv::net::TcpProtocol::TransmitFlow::HasPacket::test`](transmit.cppm.md#fn-c1ff861407f1e5df)
- [`xv::net::TcpProtocol::TransmitFlow::HasPayload::test`](transmit.cppm.md#fn-a27f47cf63f6c834)
- [`xv::net::TcpProtocol::TransmitFlow::HasSyn::test`](transmit.cppm.md#fn-d5ae9be22c833154)
- [`xv::net::TcpProtocol::TransmitFlow::ReportNoBuffer::run`](transmit.cppm.md#fn-1c184d5128d9fb63)
- [`xv::net::TcpProtocol::TransmitFlow::ReportUnreachable::run`](transmit.cppm.md#fn-a212b439655631c2)
- [`xv::net::TcpProtocol::TransmitFlow::RouteUsable::test`](transmit.cppm.md#fn-4db53562c71900b9)
- [`xv::net::TcpProtocol::TransmitFlow::SelectRoute::run`](transmit.cppm.md#fn-2b8200984323770e)
- [`xv::net::TcpProtocol::TransmitFlow::SendPacket::run`](transmit.cppm.md#fn-6b74254a01ff078b)
- [`xv::net::TcpProtocol::TransmitFlow::WriteChecksum::run`](transmit.cppm.md#fn-e0916b30280592e7)

<a id="fn-2b8200984323770e"></a>

## `xv::net::TcpProtocol::TransmitFlow::SelectRoute::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::SelectRoute::run(Context &ctx) noexcept
@brief 读取所选出口 MTU 并决定 TCP 首部长度。
@details 通过本实例 IPv4 路由查询；带 SYN 时首部 24 字节，否则 20 字节。
```

<a id="fn-4db53562c71900b9"></a>

## `xv::net::TcpProtocol::TransmitFlow::RouteUsable::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::TransmitFlow::RouteUsable::test(Context &ctx) noexcept
@brief 检查出口 MTU 达到 IPv4 最小值。
@details MTU 小于 68 时由流程进入不可达结果节点，不分配 Packet。
```

<a id="fn-a212b439655631c2"></a>

## `xv::net::TcpProtocol::TransmitFlow::ReportUnreachable::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::ReportUnreachable::run(Context &ctx) noexcept
@brief 将发送业务结果设为负 ENETUNREACH。
@details 动作只保存 errno，结束控制由节点策略 stop 表达。
```

<a id="fn-dad4d9b77398de49"></a>

## `xv::net::TcpProtocol::TransmitFlow::AllocateSegment::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::AllocateSegment::run(Context &ctx) noexcept
@brief 在上下文的 Packet 存储中构造分配结果。
@details 结束初始空 Packet 生命周期后使用原位构造，不额外分配存储；分配结果由 ctx 唯一持有并在入口退出时析构。
```

<a id="fn-c1ff861407f1e5df"></a>

## `xv::net::TcpProtocol::TransmitFlow::HasPacket::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::TransmitFlow::HasPacket::test(Context &ctx) noexcept
@brief 检查发送 Packet 是否分配成功。
@details 为 false 时进入 ENOBUFS 结果节点，不读取报文字节或载荷。
```

<a id="fn-1c184d5128d9fb63"></a>

## `xv::net::TcpProtocol::TransmitFlow::ReportNoBuffer::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::ReportNoBuffer::run(Context &ctx) noexcept
@brief 将发送业务结果设为负 ENOBUFS。
@details 动作只保存 errno，结束控制由节点策略 stop 表达。
```

<a id="fn-834ff475a0a15669"></a>

## `xv::net::TcpProtocol::TransmitFlow::EncodeFixedHeader::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::EncodeFixedHeader::run(Context &ctx) noexcept
@brief 编码 TCP 固定首部的端口、序号、标志和窗口。
@details IPv4 分配器已将报文清零；MSS、载荷和校验和由后续独立动作写入。
```

<a id="fn-d5ae9be22c833154"></a>

## `xv::net::TcpProtocol::TransmitFlow::HasSyn::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::TransmitFlow::HasSyn::test(Context &ctx) noexcept
@brief 检查本次发送是否带 SYN。
@details 控制可选 MSS 编码路径；不改变 flags 或选择具体下一节点。
```

<a id="fn-1080bf11aca1f396"></a>

## `xv::net::TcpProtocol::TransmitFlow::EncodeMssOption::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::EncodeMssOption::run(Context &ctx) noexcept
@brief 编码 SYN 的四字节 MSS 选项。
@details MSS 使用已选出口 MTU 减 40，与原发送行为一致。
```

<a id="fn-a27f47cf63f6c834"></a>

## `xv::net::TcpProtocol::TransmitFlow::HasPayload::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::TransmitFlow::HasPayload::test(Context &ctx) noexcept
@brief 检查本次发送是否有载荷。
@details 零长度直接汇合到校验和节点，不访问 data。
```

<a id="fn-e464d81ed00cc2eb"></a>

## `xv::net::TcpProtocol::TransmitFlow::CopyPayload::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::CopyPayload::run(Context &ctx) noexcept
@brief 将借用载荷复制到发送 Packet 的 TCP 首部之后。
@details 调用者保证非零长度 data 可读；按原始长度同步复制。
```

<a id="fn-e0916b30280592e7"></a>

## `xv::net::TcpProtocol::TransmitFlow::WriteChecksum::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::WriteChecksum::run(Context &ctx) noexcept
@brief 写入 TCP 段及地址伪首部的反码校验和。
@details 在可选 MSS 和载荷完成后执行，校验范围包含全部 TCP 字节。
```

<a id="fn-6b74254a01ff078b"></a>

## `xv::net::TcpProtocol::TransmitFlow::SendPacket::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::TransmitFlow::SendPacket::run(Context &ctx) noexcept
@brief 通过上下文 IPv4 服务发送并保存业务返回码。
@details 路由、MTU、过滤和设备错误原样写入 result；Packet 在流程出口统一析构。
```
