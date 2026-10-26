# `kernel/net/transport/tcp.cppm`

## 函数导航

- [`xv::net::tcp_protocol`](../core/service.cpp.md#fn-955e03b182fca925)
- [`xv::net::TcpProtocol::TcpProtocol`](tcp.cppm.md#fn-5f97e075ff831330)
- [`xv::net::TcpProtocol::receive`](tcp_protocol.cpp.md#fn-b649250825e39e1b)
- [`xv::net::TcpProtocol::transmit`](tcp_protocol.cpp.md#fn-08feda01927d4d5e)

<a id="fn-5f97e075ff831330"></a>

## `xv::net::TcpProtocol::TcpProtocol`

```doxygen
@fn xv::net::TcpProtocol::TcpProtocol(SocketRegistry &registry)
@brief 构造 TcpProtocol 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```
