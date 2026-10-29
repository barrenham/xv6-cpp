# `kernel/net/transport/tcp.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpProtocol::TcpProtocol`](tcp.cppm.md#fn-7ee8dd9bb0c00f97)
- [`xv::net::TcpProtocol::attach`](tcp.cppm.md#fn-cd1363e17f15f5a2)
- [`xv::net::TcpProtocol::receive`](tcp_protocol.cpp.md#fn-b649250825e39e1b)
- [`xv::net::TcpProtocol::transmit`](tcp_protocol.cpp.md#fn-08feda01927d4d5e)
- [`xv::net::TcpSegmentSink::deliver_segment`](tcp.cppm.md#fn-49f79078ce0f513c)
- [`xv::net::TcpSegmentSink::~TcpSegmentSink`](tcp.cppm.md#fn-6cfd12c3758cde47)

<a id="fn-49f79078ce0f513c"></a>

## `xv::net::TcpSegmentSink::deliver_segment`

```doxygen
@fn virtual bool xv::net::TcpSegmentSink::deliver_segment(const TcpSegment &segment)
@brief 同步接收已校验的借用 TCP 段，并告知协议层是否已处理。
@details true 表示匹配连接或监听入口，包括 backlog 或分配不足导致的静默丢弃；false 表示无接收对象，协议层按 TCP 规则决定是否回复 RST。
@pre 首部、校验和及端口已由 TcpProtocol 校验；回调不得睡眠、复制用户地址或保留借用字节指针。
```

<a id="fn-6cfd12c3758cde47"></a>

## `xv::net::TcpSegmentSink::~TcpSegmentSink`

```doxygen
@fn xv::net::TcpSegmentSink::~TcpSegmentSink()
@brief 结束接收接口基类生命周期；protected 非虚析构禁止通过基类 delete，具体接收器由组装方持有。
```

<a id="fn-7ee8dd9bb0c00f97"></a>

## `xv::net::TcpProtocol::TcpProtocol`

```doxygen
@fn constexpr explicit xv::net::TcpProtocol::TcpProtocol(Ipv4Layer &ip)
@brief 借用本实例的 IPv4 服务构造 TCP 编解码协议，不依赖 socket 注册表。
@pre ip 比协议对象存活更久，接收入口在发布前通过 attach 装配。
```

<a id="fn-cd1363e17f15f5a2"></a>

## `xv::net::TcpProtocol::attach`

```doxygen
@fn void xv::net::TcpProtocol::attach(TcpSegmentSink &sink)
@brief 在发布收发服务前注入 TCP 段接收器，借用其稳定对象身份。
@pre sink 比协议对象存活更久，不与正在运行的接收回调并发重绑定。
```
