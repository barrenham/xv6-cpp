# `kernel/net/socket/tcp_socket.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::tcp_peer_mss`](tcp_receiver.cpp.md#fn-ba073cc96aada617)
- [`xv::net::TcpSocket::TcpSocket`](tcp_socket.cpp.md#fn-feacfcf142e49326)
- [`xv::net::TcpSocket::initialize`](tcp_socket.cpp.md#fn-c62835414fcd2c60)
- [`xv::net::TcpSocket::connect`](tcp_socket.cpp.md#fn-9e07fc39cb3c17a2)
- [`xv::net::TcpSocket::listen`](tcp_socket.cpp.md#fn-7e3a04ae59672f21)
- [`xv::net::TcpSocket::accept`](tcp_socket.cpp.md#fn-eb2f93e4dada729f)
- [`xv::net::TcpSocket::read`](tcp_socket.cpp.md#fn-8c178dd34591a5e9)
- [`xv::net::TcpSocket::write`](tcp_socket.cpp.md#fn-633245e696cf6ebf)
- [`xv::net::TcpSocket::send_to`](tcp_socket.cpp.md#fn-231bbbb4f11fc234)
- [`xv::net::TcpSocket::receive_from`](tcp_socket.cpp.md#fn-12f33b275e0e4dec)
- [`xv::net::TcpSocket::poll`](tcp_socket.cpp.md#fn-31670288cdab90bc)
- [`xv::net::TcpSocket::shutdown`](tcp_socket.cpp.md#fn-1eb72bf0aca4a8a8)
- [`xv::net::TcpSocket::close`](tcp_socket.cpp.md#fn-c7164ce40d0f4ab4)
- [`xv::net::TcpSocket::destroy`](tcp_socket.cpp.md#fn-9f612ac9dc8ef596)
- [`xv::net::TcpSocket::tick`](tcp_socket.cpp.md#fn-b862f8511a782329)
- [`xv::net::TcpSocket::matches`](tcp_socket.cpp.md#fn-8d2f58bae366b18d)
- [`xv::net::TcpSocket::listening`](tcp_socket.cpp.md#fn-ecfbb01f47787bd0)
- [`xv::net::TcpSocket::incoming`](tcp_receiver.cpp.md#fn-e3735def055c42b7)
- [`xv::net::TcpSocket::state`](tcp_socket.cppm.md#fn-a430e78e02443614)
- [`xv::net::TcpSocket::passive`](tcp_socket.cpp.md#fn-c8523a7152fb629f)
- [`xv::net::TcpSocket::segment`](tcp_sender.cpp.md#fn-3533387c7fba430c)
- [`xv::net::TcpSocket::emit`](tcp_sender.cpp.md#fn-1e8c0fad1b1d52f4)
- [`xv::net::TcpSocket::synchronize`](tcp_sender.cpp.md#fn-8f8d8c5361cbc218)
- [`xv::net::TcpSocket::pump`](tcp_sender.cpp.md#fn-0a573b0a54d401fc)
- [`xv::net::TcpSocket::retransmit`](tcp_sender.cpp.md#fn-a3fb96da6efa099c)
- [`xv::net::TcpSocket::persist`](tcp_sender.cpp.md#fn-ddde9d68f5d02226)
- [`xv::net::TcpSocket::acknowledge`](tcp_sender.cpp.md#fn-908ef432e52e4bd8)
- [`xv::net::TcpSocket::update_window`](tcp_sender.cpp.md#fn-12a68173f8514fe3)
- [`xv::net::TcpSocket::finish`](tcp_sender.cpp.md#fn-cdfa0effd85a294d)
- [`xv::net::TcpSocket::fail`](tcp_socket.cpp.md#fn-42ede9b5fed95194)
- [`xv::net::TcpSocket::enter_time_wait`](tcp_socket.cpp.md#fn-d8a9c5a73ce2babb)
- [`xv::net::TcpSocket::window`](tcp_receiver.cpp.md#fn-92a05e6d9952e0c5)
- [`xv::net::TcpSocket::mss`](tcp_sender.cpp.md#fn-21edc71336b6198c)

<a id="fn-a430e78e02443614"></a>

## `xv::net::TcpSocket::state`

```doxygen
@fn TcpState xv::net::TcpSocket::state() const
@brief TcpSocket：持注册锁读取当前 TcpState 值；不提供运行期无锁状态快照保证。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁读取当前 TcpState 值；不提供运行期无锁状态快照保证。
```
