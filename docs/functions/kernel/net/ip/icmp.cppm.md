# `kernel/net/ip/icmp.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IcmpProtocol::IcmpProtocol`](icmp.cppm.md#fn-1a829fe9267b19a5)
- [`xv::net::IcmpProtocol::receive`](icmp.cpp.md#fn-e81d21f708c3ced8)
- [`xv::net::IcmpProtocol::unreachable`](icmp.cpp.md#fn-a19fb566f8d31417)

- [`xv::net::IcmpEchoSink::deliver_echo`](icmp.cppm.md#fn-ab5ffb9ccdc58bf4)

- [`xv::net::IcmpEchoSink::~IcmpEchoSink`](icmp.cppm.md#fn-e432f07f5230e8f3)

- [`xv::net::IcmpProtocol::attach`](icmp.cppm.md#fn-9965b95a1048705f)

<a id="fn-1a829fe9267b19a5"></a>

## `xv::net::IcmpProtocol::IcmpProtocol`

```doxygen
@fn constexpr explicit xv::net::IcmpProtocol::IcmpProtocol(Ipv4Layer &ip)
@brief 构造 IcmpProtocol 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```

<a id="fn-ab5ffb9ccdc58bf4"></a>

## `xv::net::IcmpEchoSink::deliver_echo`

```doxygen
@fn virtual bool xv::net::IcmpEchoSink::deliver_echo(Packet &packet, const IpDatagram &datagram) = 0
@brief 消费已校验的 Echo Reply 或拒绝接收。
@details 成功时接收端接管 Packet 页面；拒绝时保留原所有权。调用方已保证至少八字节 ICMP 头部。
```

<a id="fn-e432f07f5230e8f3"></a>

## `xv::net::IcmpEchoSink::~IcmpEchoSink`

```doxygen
@fn xv::net::IcmpEchoSink::~IcmpEchoSink() = default
@brief 保护非拥有接口的析构，接收端生命周期由网络服务管理。
```

<a id="fn-9965b95a1048705f"></a>

## `xv::net::IcmpProtocol::attach`

```doxygen
@fn void xv::net::IcmpProtocol::attach(IcmpEchoSink &sink)
@brief 启动期间绑定长期存活的 Echo Reply 接收端。
@pre 协议接收尚未开始，sink 的生命周期覆盖协议服务。
```
