# `kernel/net/transport/tcp_protocol.cpp`

## 函数导航

- [`xv::net::TcpProtocol::receive`](tcp_protocol.cpp.md#fn-b649250825e39e1b)
- [`xv::net::TcpProtocol::transmit`](tcp_protocol.cpp.md#fn-08feda01927d4d5e)

<a id="fn-b649250825e39e1b"></a>

## `xv::net::TcpProtocol::receive`

```doxygen
@fn void xv::net::TcpProtocol::receive(Packet &packet, const IpDatagram &d)
@brief 从纯 IP 页面验证 TCP 首部与伪首部校验和，再按地址/端口交注册表状态机；伪首部源/目的位于页面偏移十二。
```

<a id="fn-08feda01927d4d5e"></a>

## `xv::net::TcpProtocol::transmit`

```doxygen
@fn int xv::net::TcpProtocol::transmit(Endpoint local, Endpoint peer, unsigned sequence, unsigned acknowledgement, unsigned flags, unsigned window, const void *data, unsigned length)
@brief 先查询绑定源对应出口 MTU，再构造纯 IP TCP 段；SYN 通告出口 MTU 推导的 MSS，无路由返回错误，校验和使用实际本地源。
```
