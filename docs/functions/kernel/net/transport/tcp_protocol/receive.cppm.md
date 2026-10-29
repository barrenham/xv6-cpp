# `kernel/net/transport/tcp_protocol/receive.cppm`

TcpProtocol 的内部具名动作与条件。流程关系和所有权约定见
[声明式协议流程](../../../../../network-tcp-protocol-flow.md)。

## 函数导航

- [`xv::net::TcpProtocol::ReceiveFlow::ChecksumValid::test`](receive.cppm.md#fn-511d1ab12d8ba750)
- [`xv::net::TcpProtocol::ReceiveFlow::DeliverValidatedSegment::run`](receive.cppm.md#fn-0b5c9b6898da19fa)
- [`xv::net::TcpProtocol::ReceiveFlow::HasMinimumHeader::test`](receive.cppm.md#fn-c45b14dc6aea20b4)
- [`xv::net::TcpProtocol::ReceiveFlow::HeaderLayoutValid::test`](receive.cppm.md#fn-5fd442c7fe583d51)
- [`xv::net::TcpProtocol::ReceiveFlow::InspectHeader::run`](receive.cppm.md#fn-b6d447558d9a5fc4)
- [`xv::net::TcpProtocol::ReceiveFlow::IsReset::test`](receive.cppm.md#fn-fd1e3a4d79738feb)
- [`xv::net::TcpProtocol::ReceiveFlow::PortsValid::test`](receive.cppm.md#fn-689621011b61623f)
- [`xv::net::TcpProtocol::ReceiveFlow::PrepareResetResponse::run`](receive.cppm.md#fn-fe06f4789edc2f16)
- [`xv::net::TcpProtocol::ReceiveFlow::SendResetResponse::run`](receive.cppm.md#fn-f7d2cc1cd578c09e)
- [`xv::net::TcpProtocol::ReceiveFlow::WasHandled::test`](receive.cppm.md#fn-0544232eb0f741fa)

<a id="fn-c45b14dc6aea20b4"></a>

## `xv::net::TcpProtocol::ReceiveFlow::HasMinimumHeader::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::HasMinimumHeader::test(Context &ctx) noexcept
@brief 检查数据报至少容纳固定 TCP 首部。
@details 先于任何 TCP 字节读取执行；少于 20 字节时流程直接结束。
```

<a id="fn-b6d447558d9a5fc4"></a>

## `xv::net::TcpProtocol::ReceiveFlow::InspectHeader::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::ReceiveFlow::InspectHeader::run(Context &ctx) noexcept
@brief 定位借用的 TCP 字节并解码首部长度。
@details 依赖最短长度检查；不转移 Packet 所有权。
```

<a id="fn-5fd442c7fe583d51"></a>

## `xv::net::TcpProtocol::ReceiveFlow::HeaderLayoutValid::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::HeaderLayoutValid::test(Context &ctx) noexcept
@brief 检查 TCP 数据偏移范围和保留位。
@details 首部长度必须至少 20 且不超过数据报长度，低四位必须为零。
```

<a id="fn-511d1ab12d8ba750"></a>

## `xv::net::TcpProtocol::ReceiveFlow::ChecksumValid::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::ChecksumValid::test(Context &ctx) noexcept
@brief 检查 TCP 段及 IPv4 地址伪首部的校验和。
@details 保留原实现的伪首部构造，校验失败不分派接收器或回复 RST。
```

<a id="fn-689621011b61623f"></a>

## `xv::net::TcpProtocol::ReceiveFlow::PortsValid::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::PortsValid::test(Context &ctx) noexcept
@brief 检查源端口与目的端口均非零。
@details 在首部布局和校验和通过之后检查。
```

<a id="fn-0b5c9b6898da19fa"></a>

## `xv::net::TcpProtocol::ReceiveFlow::DeliverValidatedSegment::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::ReceiveFlow::DeliverValidatedSegment::run(Context &ctx) noexcept
@brief 解码端点并同步向接收器借用已验证 TCP 段。
@details 接收器不存在时 handled 为 false；true 包括监听资源不足后的静默丢弃。
```

<a id="fn-0544232eb0f741fa"></a>

## `xv::net::TcpProtocol::ReceiveFlow::WasHandled::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::WasHandled::test(Context &ctx) noexcept
@brief 查询接收器是否已经处理该报文。
@details 为 true 时结束接收图，不构造未匹配复位。
```

<a id="fn-fd1e3a4d79738feb"></a>

## `xv::net::TcpProtocol::ReceiveFlow::IsReset::test`

```doxygen
@fn static inline bool xv::net::TcpProtocol::ReceiveFlow::IsReset::test(Context &ctx) noexcept
@brief 检查未匹配报文是否带 RST。
@details 只在接收器未处理后执行，为 true 时禁止复位应答。
```

<a id="fn-fe06f4789edc2f16"></a>

## `xv::net::TcpProtocol::ReceiveFlow::PrepareResetResponse::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::ReceiveFlow::PrepareResetResponse::run(Context &ctx) noexcept
@brief 计算未匹配报文的 RST 序号、确认号和标志。
@details 带 ACK 时复位序号取收到的确认号；否则确认载荷与 SYN/FIN 的序号占用，保持 unsigned 回绕。
```

<a id="fn-f7d2cc1cd578c09e"></a>

## `xv::net::TcpProtocol::ReceiveFlow::SendResetResponse::run`

```doxygen
@fn static inline void xv::net::TcpProtocol::ReceiveFlow::SendResetResponse::run(Context &ctx) noexcept
@brief 在接收器返回后通过当前协议对象发送复位。
@details 复用本实例发送流程；发送失败保持 best effort，不影响借用接收 Packet 的所有权。
```
