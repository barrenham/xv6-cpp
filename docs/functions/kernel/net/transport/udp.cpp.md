# `kernel/net/transport/udp.cpp`

## 函数导航

- [`xv::net::UdpProtocol::attach`](udp.cpp.md#fn-b5cbaf0f6126b7bd)
- [`xv::net::UdpProtocol::receive`](udp.cpp.md#fn-89d047edb11f0bae)
- [`xv::net::UdpProtocol::send`](udp.cpp.md#fn-178312cdd1239398)

<a id="fn-b5cbaf0f6126b7bd"></a>

## `xv::net::UdpProtocol::attach`

```doxygen
@fn void xv::net::UdpProtocol::attach(DatagramSink &sink)
@brief UdpProtocol：启动时登记最多四个 DatagramSink；兼容 PID 端口与文件描述符 socket 依次参与分派。
@details 启动时登记最多四个 DatagramSink；兼容 PID 端口与文件描述符 socket 依次参与分派。
```

<a id="fn-89d047edb11f0bae"></a>

## `xv::net::UdpProtocol::receive`

```doxygen
@fn void xv::net::UdpProtocol::receive(Packet &packet, const IpDatagram &d)
@brief 从纯 IP 页面验证 UDP 长度、目的端口及可选校验和，再按地址/端口分派；未绑定时交 ICMP 不可达路径。
```

<a id="fn-178312cdd1239398"></a>

## `xv::net::UdpProtocol::send`

```doxygen
@fn int xv::net::UdpProtocol::send(Endpoint source, Endpoint destination, const void *data, unsigned length)
@brief 先按路由确定源地址，再编码 UDP 及使用同一源的伪首部校验和；IP 层继续检查出口 MTU，避免跨接口校验和错误。
```
