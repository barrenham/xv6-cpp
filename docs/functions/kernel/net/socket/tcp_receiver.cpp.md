# `kernel/net/socket/tcp_receiver.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpSocket::incoming`](tcp_receiver.cpp.md#fn-e3735def055c42b7)
- [`xv::net::TcpSocket::window`](tcp_receiver.cpp.md#fn-92a05e6d9952e0c5)
- [`xv::net::tcp_peer_mss`](tcp_receiver.cpp.md#fn-ba073cc96aada617)

<a id="fn-ba073cc96aada617"></a>

## `xv::net::tcp_peer_mss`

```doxygen
@fn unsigned xv::net::tcp_peer_mss(const unsigned char *p, unsigned length)
@brief 在 SYN 首部范围内解析对端 MSS。
@details 逐项验证选项长度，跳过 NOP 和未知项；MSS 为零、重复或长度错误时拒绝，缺省 536，发送协商上限 1460。 实现通过 TcpMssFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-92a05e6d9952e0c5"></a>

## `xv::net::TcpSocket::window`

```doxygen
@fn unsigned xv::net::TcpSocket::window() const
@brief 返回当前可通告的接收窗口。
@details 容量为 16384 字节减去尚未交给应用的连续数据；乱序字节保存在该序号窗口内，不另行改变右边缘。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-e3735def055c42b7"></a>

## `xv::net::TcpSocket::incoming`

```doxygen
@fn void xv::net::TcpSocket::incoming(Endpoint, Endpoint, const unsigned char *p, unsigned length)
@brief 处理本连接的握手、ACK、窗口、重排数据及 FIN，按状态与序号约束推进收发。
@details 注册表负责连接分派；异常 SYN-SENT 确认的复位响应也通过所属注册表的 TCP 对象发送。支持原有同时打开、挑战 ACK、NewReno 及接收位图语义。 实现通过 IncomingFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持本注册表锁，传入字节已经协议层验证。
```
