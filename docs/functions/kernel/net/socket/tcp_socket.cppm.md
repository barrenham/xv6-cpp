# `kernel/net/socket/tcp_socket.cppm`

## 函数导航

- [`xv::net::TcpSocket::TcpSocket`](tcp_socket.cpp.md#fn-feacfcf142e49326)
- [`xv::net::TcpSocket::accept`](tcp_socket.cpp.md#fn-eb2f93e4dada729f)
- [`xv::net::TcpSocket::acknowledge`](tcp_socket.cpp.md#fn-908ef432e52e4bd8)
- [`xv::net::TcpSocket::close`](tcp_socket.cpp.md#fn-c7164ce40d0f4ab4)
- [`xv::net::TcpSocket::connect`](tcp_socket.cpp.md#fn-9e07fc39cb3c17a2)
- [`xv::net::TcpSocket::destroy`](tcp_socket.cpp.md#fn-9f612ac9dc8ef596)
- [`xv::net::TcpSocket::emit`](tcp_socket.cpp.md#fn-1e8c0fad1b1d52f4)
- [`xv::net::TcpSocket::enter_time_wait`](tcp_socket.cpp.md#fn-d8a9c5a73ce2babb)
- [`xv::net::TcpSocket::fail`](tcp_socket.cpp.md#fn-42ede9b5fed95194)
- [`xv::net::TcpSocket::finish`](tcp_socket.cpp.md#fn-cdfa0effd85a294d)
- [`xv::net::TcpSocket::incoming`](tcp_socket.cpp.md#fn-e3735def055c42b7)
- [`xv::net::TcpSocket::initialize`](tcp_socket.cpp.md#fn-c62835414fcd2c60)
- [`xv::net::TcpSocket::listen`](tcp_socket.cpp.md#fn-7e3a04ae59672f21)
- [`xv::net::TcpSocket::listening`](tcp_socket.cpp.md#fn-ecfbb01f47787bd0)
- [`xv::net::TcpSocket::matches`](tcp_socket.cpp.md#fn-8d2f58bae366b18d)
- [`xv::net::TcpSocket::passive`](tcp_socket.cpp.md#fn-c8523a7152fb629f)
- [`xv::net::TcpSocket::poll`](tcp_socket.cpp.md#fn-31670288cdab90bc)
- [`xv::net::TcpSocket::queue`](tcp_socket.cpp.md#fn-2d9f1c832b17f59b)
- [`xv::net::TcpSocket::read`](tcp_socket.cpp.md#fn-8c178dd34591a5e9)
- [`xv::net::TcpSocket::receive_from`](tcp_socket.cpp.md#fn-12f33b275e0e4dec)
- [`xv::net::TcpSocket::send_to`](tcp_socket.cpp.md#fn-231bbbb4f11fc234)
- [`xv::net::TcpSocket::shutdown`](tcp_socket.cpp.md#fn-1eb72bf0aca4a8a8)
- [`xv::net::TcpSocket::state`](tcp_socket.cppm.md#fn-a430e78e02443614)
- [`xv::net::TcpSocket::tick`](tcp_socket.cpp.md#fn-b862f8511a782329)
- [`xv::net::TcpSocket::window`](tcp_socket.cpp.md#fn-92a05e6d9952e0c5)
- [`xv::net::TcpSocket::write`](tcp_socket.cpp.md#fn-633245e696cf6ebf)

<a id="fn-a430e78e02443614"></a>

## `xv::net::TcpSocket::state`

```doxygen
@fn TcpState xv::net::TcpSocket::state() const
@brief TcpSocket：持注册锁读取当前 TcpState 值；不提供运行期无锁状态快照保证。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁读取当前 TcpState 值；不提供运行期无锁状态快照保证。
```
