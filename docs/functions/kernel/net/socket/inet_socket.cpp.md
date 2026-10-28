# `kernel/net/socket/inet_socket.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::InetSocket::bind`](inet_socket.cpp.md#fn-6067d2454125f722)
- [`xv::net::InetSocket::auto_bind`](inet_socket.cpp.md#fn-0b320212f6b53f92)
- [`xv::net::InetSocket::listen`](inet_socket.cpp.md#fn-effeaf7d08e9e569)
- [`xv::net::InetSocket::accept`](inet_socket.cpp.md#fn-d2da4b830d20ce65)
- [`xv::net::InetSocket::shutdown`](inet_socket.cpp.md#fn-9cf8b6db250e5ed8)
- [`xv::net::InetSocket::local`](inet_socket.cpp.md#fn-32c5ee2101635ca7)
- [`xv::net::InetSocket::peer`](inet_socket.cpp.md#fn-cec904cbda0c065a)
- [`xv::net::InetSocket::error`](inet_socket.cpp.md#fn-8bc83ef76882de09)
- [`xv::net::InetSocket::notify`](inet_socket.cpp.md#fn-16bc4fae651114d5)
- [`xv::net::InetSocket::wait`](inet_socket.cpp.md#fn-89b4a062cf12e43d)

<a id="fn-6067d2454125f722"></a>

## `xv::net::InetSocket::bind`

```doxygen
@fn int xv::net::InetSocket::bind(Endpoint local)
@brief 在注册锁内绑定 ANY 或任一已启用本机地址；不同具体地址可使用同一端口，ANY 与所有同协议地址冲突，零端口分配临时端口。
```

<a id="fn-0b320212f6b53f92"></a>

## `xv::net::InetSocket::auto_bind`

```doxygen
@fn int xv::net::InetSocket::auto_bind(unsigned destination)
@brief 按目标和已绑定地址选源并分配临时端口；无路由返回 -ENETUNREACH，耗尽返回 -EADDRINUSE。
@details TCP 固定源，未连接 UDP 保留 ANY。
```

<a id="fn-effeaf7d08e9e569"></a>

## `xv::net::InetSocket::listen`

```doxygen
@fn int xv::net::InetSocket::listen(int)
@brief InetSocket：默认拒绝不支持监听的具体协议，返回 -EOPNOTSUPP。
@details 默认拒绝不支持监听的具体协议，返回 -EOPNOTSUPP。
```

<a id="fn-d2da4b830d20ce65"></a>

## `xv::net::InetSocket::accept`

```doxygen
@fn Socket *xv::net::InetSocket::accept(bool, int &error)
@brief InetSocket：默认拒绝不支持被动连接的协议，以 error 输出 -EOPNOTSUPP 并返回 nullptr。
@details 默认拒绝不支持被动连接的协议，以 error 输出 -EOPNOTSUPP 并返回 nullptr。
```

<a id="fn-9cf8b6db250e5ed8"></a>

## `xv::net::InetSocket::shutdown`

```doxygen
@fn int xv::net::InetSocket::shutdown(int)
@brief InetSocket：默认拒绝不支持半关闭的协议，返回 -EOPNOTSUPP。
@details 默认拒绝不支持半关闭的协议，返回 -EOPNOTSUPP。
```

<a id="fn-32c5ee2101635ca7"></a>

## `xv::net::InetSocket::local`

```doxygen
@fn Endpoint xv::net::InetSocket::local()
@brief InetSocket：在注册锁保护下复制当前本地 Endpoint；返回值不暴露内部可变字段。
@details 在注册锁保护下复制当前本地 Endpoint；返回值不暴露内部可变字段。
```

<a id="fn-cec904cbda0c065a"></a>

## `xv::net::InetSocket::peer`

```doxygen
@fn Endpoint xv::net::InetSocket::peer()
@brief InetSocket：在注册锁保护下复制当前已连接 peer；未连接时端口为零。
@details 在注册锁保护下复制当前已连接 peer；未连接时端口为零。
```

<a id="fn-8bc83ef76882de09"></a>

## `xv::net::InetSocket::error`

```doxygen
@fn int xv::net::InetSocket::error()
@brief InetSocket：在注册锁内取出并清除挂起 socket 错误，供 SO_ERROR 使用，返回正 errno 或零。
@details 在注册锁内取出并清除挂起 socket 错误，供 SO_ERROR 使用，返回正 errno 或零。
```

<a id="fn-16bc4fae651114d5"></a>

## `xv::net::InetSocket::notify`

```doxygen
@fn void xv::net::InetSocket::notify()
@brief InetSocket：在条件状态变化后通知本对象通道的普通等待者和 poll 订阅者；通知不是数据计数。
@details 在条件状态变化后通知本对象通道的普通等待者和 poll 订阅者；通知不是数据计数。
```

<a id="fn-89b4a062cf12e43d"></a>

## `xv::net::InetSocket::wait`

```doxygen
@fn void xv::net::InetSocket::wait()
@brief InetSocket：调用者持注册锁，先登记可中断等待再解锁、睡眠并重新加锁，避免丢失唤醒；醒来必须循环复查条件。
@details 调用者持注册锁，先登记可中断等待再解锁、睡眠并重新加锁，避免丢失唤醒；醒来必须循环复查条件。
```
