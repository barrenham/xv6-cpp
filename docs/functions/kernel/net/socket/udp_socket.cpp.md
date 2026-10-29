# `kernel/net/socket/udp_socket.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::UdpSocket::close`](udp_socket.cpp.md#fn-eed8d538bf40d970)
- [`xv::net::UdpSocket::connect`](udp_socket.cpp.md#fn-f72fc8c5d0e8ba8d)
- [`xv::net::UdpSocket::deliver`](udp_socket.cpp.md#fn-1939981af256f2db)
- [`xv::net::UdpSocket::destroy`](udp_socket.cpp.md#fn-b15ea2d9e6042036)
- [`xv::net::UdpSocket::poll`](udp_socket.cpp.md#fn-b5e462b9bd861861)
- [`xv::net::UdpSocket::read`](udp_socket.cpp.md#fn-be5662d5a59833f9)
- [`xv::net::UdpSocket::receive_from`](udp_socket.cpp.md#fn-36db1e9161281aff)
- [`xv::net::UdpSocket::send_to`](udp_socket.cpp.md#fn-a5d64b3c6ae3d978)
- [`xv::net::UdpSocket::write`](udp_socket.cpp.md#fn-c8204ec2498ef987)

<a id="fn-f72fc8c5d0e8ba8d"></a>

## `xv::net::UdpSocket::connect`

```doxygen
@fn int xv::net::UdpSocket::connect(Endpoint peer, bool)
@brief 在注册锁内选择可路由的本地地址及端口，固定默认目标与本地源；后续接收按连接对端筛选。
@details 实现通过 ConnectFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-be5662d5a59833f9"></a>

## `xv::net::UdpSocket::read`

```doxygen
@fn int xv::net::UdpSocket::read(xv::io::Buffer destination, int count)
@brief UdpSocket：把 File read 转为无来源输出的 receive_from，保留数据报边界与空报返回 0 的语义。
@details 把 File read 转为无来源输出的 receive_from，保留数据报边界与空报返回 0 的语义。
```

<a id="fn-c8204ec2498ef987"></a>

## `xv::net::UdpSocket::write`

```doxygen
@fn int xv::net::UdpSocket::write(xv::io::Buffer source, int count)
@brief UdpSocket：把 File write 转为使用默认 peer 的 send_to，未 connect 返回 -ENOTCONN；一次 write 对应一个数据报。
@details 把 File write 转为使用默认 peer 的 send_to，未 connect 返回 -ENOTCONN；一次 write 对应一个数据报。
```

<a id="fn-a5d64b3c6ae3d978"></a>

## `xv::net::UdpSocket::send_to`

```doxygen
@fn int xv::net::UdpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *destination)
@brief 在注册锁外暂存用户负载，在锁内绑定并选择目标，再交 UDP 编码。
@details 未连接 ANY 可逐报选不同出口；负载最多 4042 字节，仍受出口 MTU 限制。 实现通过 SendFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-1939981af256f2db"></a>

## `xv::net::UdpSocket::deliver`

```doxygen
@fn bool xv::net::UdpSocket::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
@brief 在已匹配本地与可选对端约束后，有容量则接管报文页面、来源、载荷边界及处理上下文。
@details 匹配但队列满仍返回 true 并丢弃，避免生成错误的端口不可达；未匹配返回 false。 实现通过 DeliveryFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用方已持本注册表锁，报文及传输层边界已校验。
```

<a id="fn-36db1e9161281aff"></a>

## `xv::net::UdpSocket::receive_from`

```doxygen
@fn int xv::net::UdpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
@brief 在注册锁内取出一条数据报，锁外以原上下文恢复页面拥有权并复制有界载荷给用户。
@details 非阻塞空队列返回 -EAGAIN，阻塞按可中断条件等待处理；复制失败仍通过 Packet 析构释放已出队页面。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-b5e462b9bd861861"></a>

## `xv::net::UdpSocket::poll`

```doxygen
@fn unsigned xv::net::UdpSocket::poll(xv::sync::PollTable *table)
@brief UdpSocket：登记本对象通道后返回数据报就绪状态；队列非空为 POLLIN，发送调用可立即提交或返回错误，标为 POLLOUT。
@details 登记本对象通道后返回数据报就绪状态；队列非空为 POLLIN，发送调用可立即提交或返回错误，标为 POLLOUT。
```

<a id="fn-eed8d538bf40d970"></a>

## `xv::net::UdpSocket::close`

```doxygen
@fn void xv::net::UdpSocket::close()
@brief UdpSocket：最后一个 File 引用消失时，在注册锁内撤销并销毁 UDP 对象；fork/dup 的其他文件引用仍存活时不会触发。
@details 最后一个 File 引用消失时，在注册锁内撤销并销毁 UDP 对象；fork/dup 的其他文件引用仍存活时不会触发。
```

<a id="fn-b15ea2d9e6042036"></a>

## `xv::net::UdpSocket::destroy`

```doxygen
@fn void xv::net::UdpSocket::destroy()
@brief UdpSocket：归还所有排队数据报页，显式析构后归还对象页；调用后 this 不再有效。
@details 归还所有排队数据报页，显式析构后归还对象页；调用后 this 不再有效。
```
