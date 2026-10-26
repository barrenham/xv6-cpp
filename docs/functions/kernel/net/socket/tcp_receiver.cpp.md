# `kernel/net/socket/tcp_receiver.cpp`

## 函数导航

- [`xv::net::TcpSocket::incoming`](tcp_receiver.cpp.md#fn-e3735def055c42b7)
- [`xv::net::TcpSocket::window`](tcp_receiver.cpp.md#fn-92a05e6d9952e0c5)
- [`xv::net::tcp_peer_mss`](tcp_receiver.cpp.md#fn-ba073cc96aada617)

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
@brief 处理 TCP 握手、ACK、窗口、数据重排和 FIN。
@details 按状态与序号验证报文，支持同时打开和挑战 ACK；ACK 驱动 NewReno、累计释放及窗口恢复。载荷写入接收位图，连续前沿越过 FIN 后产生 EOF，最后推进发送。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-ba073cc96aada617"></a>

## `xv::net::tcp_peer_mss`

```doxygen
@fn unsigned xv::net::tcp_peer_mss(const unsigned char *p, unsigned length)
@brief 在 SYN 首部范围内解析对端 MSS。
@details 逐项验证选项长度，跳过 NOP 和未知项；MSS 为零、重复或长度错误时拒绝，缺省 536，发送协商上限 1460。
```
