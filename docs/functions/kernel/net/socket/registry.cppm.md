# `kernel/net/socket/registry.cppm`

## 函数导航

- [`xv::net::socket_registry`](../core/service.cpp.md#fn-a6dd2cdb97db1848)
- [`xv::net::InetSocket::InetSocket`](registry.cppm.md#fn-d46e3a4682fc0941)
- [`xv::net::InetSocket::accept`](inet_socket.cpp.md#fn-d2da4b830d20ce65)
- [`xv::net::InetSocket::auto_bind`](inet_socket.cpp.md#fn-0b320212f6b53f92)
- [`xv::net::InetSocket::bind`](inet_socket.cpp.md#fn-6067d2454125f722)
- [`xv::net::InetSocket::bound`](registry.cppm.md#fn-9f53cd3638df33c8)
- [`xv::net::InetSocket::destroy`](registry.cppm.md#fn-2420c65d9c46bc5f)
- [`xv::net::InetSocket::error`](inet_socket.cpp.md#fn-8bc83ef76882de09)
- [`xv::net::InetSocket::listen`](inet_socket.cpp.md#fn-effeaf7d08e9e569)
- [`xv::net::InetSocket::local`](inet_socket.cpp.md#fn-32c5ee2101635ca7)
- [`xv::net::InetSocket::notify`](inet_socket.cpp.md#fn-16bc4fae651114d5)
- [`xv::net::InetSocket::owned`](registry.cppm.md#fn-26c167116bbe39c8)
- [`xv::net::InetSocket::peer`](inet_socket.cpp.md#fn-cec904cbda0c065a)
- [`xv::net::InetSocket::protocol`](registry.cppm.md#fn-12aa64f3b8def06f)
- [`xv::net::InetSocket::shutdown`](inet_socket.cpp.md#fn-9cf8b6db250e5ed8)
- [`xv::net::InetSocket::tick`](registry.cppm.md#fn-16b6f7514517e4b8)
- [`xv::net::InetSocket::wait`](inet_socket.cpp.md#fn-89b4a062cf12e43d)
- [`xv::net::SocketRegistry::SocketRegistry`](registry.cppm.md#fn-0926fac492970a06)
- [`xv::net::SocketRegistry::add`](registry.cpp.md#fn-8ebcbaf798cf3b49)
- [`xv::net::SocketRegistry::at`](registry.cppm.md#fn-4c94f5be09f01218)
- [`xv::net::SocketRegistry::create`](registry.cpp.md#fn-b6e4a131f2727ec9)
- [`xv::net::SocketRegistry::create_child`](registry.cpp.md#fn-312d78452a442d45)
- [`xv::net::SocketRegistry::deliver`](registry.cpp.md#fn-9bba05169d63bfe0)
- [`xv::net::SocketRegistry::destroy`](registry.cpp.md#fn-5728efd1d2787600)
- [`xv::net::SocketRegistry::ephemeral`](registry.cpp.md#fn-5f18bd69e55bbc95)
- [`xv::net::SocketRegistry::initialize`](registry.cpp.md#fn-659351b0c1327217)
- [`xv::net::SocketRegistry::ip`](registry.cppm.md#fn-16744c8763b01524)
- [`xv::net::SocketRegistry::lock`](registry.cppm.md#fn-0fa9280129146c53)
- [`xv::net::SocketRegistry::port_used`](registry.cpp.md#fn-843c6c84c36d20b9)
- [`xv::net::SocketRegistry::tick`](registry.cpp.md#fn-c06a8341d8d22f53)
- [`xv::net::SocketRegistry::udp`](registry.cppm.md#fn-5bf9bee61e4d9914)

<a id="fn-0926fac492970a06"></a>

## `xv::net::SocketRegistry::SocketRegistry`

```doxygen
@fn xv::net::SocketRegistry::SocketRegistry(Ipv4Layer &ip, UdpProtocol &udp)
@brief 构造 SocketRegistry 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```

<a id="fn-4c94f5be09f01218"></a>

## `xv::net::SocketRegistry::at`

```doxygen
@fn InetSocket *xv::net::SocketRegistry::at(unsigned index)
@brief SocketRegistry：持注册锁借用指定索引对象；调用者确保 index 小于 64，空槽返回 nullptr。
@pre 调用者持有统一注册锁。
@details 持注册锁借用指定索引对象；调用者确保 index 小于 64，空槽返回 nullptr。
```

<a id="fn-0fa9280129146c53"></a>

## `xv::net::SocketRegistry::lock`

```doxygen
@fn xv::sync::SpinLock &xv::net::SocketRegistry::lock()
@brief SocketRegistry：借用统一网络条件自旋锁；阻塞路径须先登记等待，再释放此锁并睡眠。
@details 借用统一网络条件自旋锁；阻塞路径须先登记等待，再释放此锁并睡眠。
```

<a id="fn-16744c8763b01524"></a>

## `xv::net::SocketRegistry::ip`

```doxygen
@fn Ipv4Layer &xv::net::SocketRegistry::ip()
@brief SocketRegistry：借用上层唯一 Ipv4Layer 供组包与路由使用。
@details 借用上层唯一 Ipv4Layer 供组包与路由使用。
```

<a id="fn-5bf9bee61e4d9914"></a>

## `xv::net::SocketRegistry::udp`

```doxygen
@fn UdpProtocol &xv::net::SocketRegistry::udp()
@brief SocketRegistry：借用上层唯一 UdpProtocol 供数据报组包与发送使用。
@details 借用上层唯一 UdpProtocol 供数据报组包与发送使用。
```

<a id="fn-d46e3a4682fc0941"></a>

## `xv::net::InetSocket::InetSocket`

```doxygen
@fn xv::net::InetSocket::InetSocket(SocketRegistry &registry, unsigned protocol)
@brief 构造 InetSocket 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```

<a id="fn-12aa64f3b8def06f"></a>

## `xv::net::InetSocket::protocol`

```doxygen
@fn unsigned xv::net::InetSocket::protocol() const
@brief InetSocket：取得构造时固定的 IP 传输协议号（TCP6/UDP17）。
@details 取得构造时固定的 IP 传输协议号（TCP6/UDP17）。
```

<a id="fn-9f53cd3638df33c8"></a>

## `xv::net::InetSocket::bound`

```doxygen
@fn Endpoint xv::net::InetSocket::bound() const
@brief InetSocket：持注册锁借用状态的值副本，供端口查重使用；本函数不额外加锁。
@details 持注册锁借用状态的值副本，供端口查重使用；本函数不额外加锁。
```

<a id="fn-2420c65d9c46bc5f"></a>

## `xv::net::InetSocket::destroy`

```doxygen
@fn void xv::net::InetSocket::destroy()
@brief InetSocket：具体类必须显式析构并归还自身及独占缓冲页；只能由已撤销注册的对象触发，不以非虚基类析构删除。
@details 具体类必须显式析构并归还自身及独占缓冲页；只能由已撤销注册的对象触发，不以非虚基类析构删除。
```

<a id="fn-16b6f7514517e4b8"></a>

## `xv::net::InetSocket::tick`

```doxygen
@fn void xv::net::InetSocket::tick(unsigned long)
@brief InetSocket：默认无计时工作；具体 TCP 实现覆盖此接口。
@details 默认无计时工作；具体 TCP 实现覆盖此接口。
```

<a id="fn-26c167116bbe39c8"></a>

## `xv::net::InetSocket::owned`

```doxygen
@fn bool xv::net::InetSocket::owned() const
@brief InetSocket：持注册锁查询是否仍由应用 File 或待安装的调用者持有；未接受及最终 close 后为 false。
@details 持注册锁查询是否仍由应用 File 或待安装的调用者持有；未接受及最终 close 后为 false。
```
