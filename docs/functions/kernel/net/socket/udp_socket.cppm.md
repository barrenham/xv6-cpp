# `kernel/net/socket/udp_socket.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::UdpSocket::UdpSocket`](udp_socket.cppm.md#fn-2b7d32d9e1d8d3b4)
- [`xv::net::UdpSocket::connect`](udp_socket.cpp.md#fn-f72fc8c5d0e8ba8d)
- [`xv::net::UdpSocket::read`](udp_socket.cpp.md#fn-be5662d5a59833f9)
- [`xv::net::UdpSocket::write`](udp_socket.cpp.md#fn-c8204ec2498ef987)
- [`xv::net::UdpSocket::send_to`](udp_socket.cpp.md#fn-a5d64b3c6ae3d978)
- [`xv::net::UdpSocket::receive_from`](udp_socket.cpp.md#fn-36db1e9161281aff)
- [`xv::net::UdpSocket::poll`](udp_socket.cpp.md#fn-b5e462b9bd861861)
- [`xv::net::UdpSocket::close`](udp_socket.cpp.md#fn-eed8d538bf40d970)
- [`xv::net::UdpSocket::destroy`](udp_socket.cpp.md#fn-b15ea2d9e6042036)
- [`xv::net::UdpSocket::deliver`](udp_socket.cpp.md#fn-1939981af256f2db)

<a id="fn-2b7d32d9e1d8d3b4"></a>

## `xv::net::UdpSocket::UdpSocket`

```doxygen
@fn explicit xv::net::UdpSocket::UdpSocket(SocketRegistry &registry)
@brief 构造 UdpSocket 的状态与依赖。
@details 构造 UDP 协议标识与空的十六项数据报队列；对象尚未自动绑定端口。
```
