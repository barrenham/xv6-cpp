# `kernel/net/transport/udp.cppm`

## 函数导航

- [`xv::net::DatagramSink::deliver`](udp.cppm.md#fn-e96d99e2a20cb8c0)
- [`xv::net::DatagramSink::~DatagramSink`](udp.cppm.md#fn-29988a1b5f410ade)
- [`xv::net::UdpProtocol::UdpProtocol`](udp.cppm.md#fn-1ca7e5679b67ce43)
- [`xv::net::UdpProtocol::attach`](udp.cpp.md#fn-b5cbaf0f6126b7bd)
- [`xv::net::UdpProtocol::receive`](udp.cpp.md#fn-89d047edb11f0bae)
- [`xv::net::UdpProtocol::send`](udp.cpp.md#fn-178312cdd1239398)

<a id="fn-e96d99e2a20cb8c0"></a>

## `xv::net::DatagramSink::deliver`

```doxygen
@fn bool xv::net::DatagramSink::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
@brief 已验证 UDP 数据报的消费与命中契约。
@details 按 source/destination Endpoint 和 offset/length 查找端口；返回 true 表示已命中（即使因队列满而丢包），false 允许下一 sink 尝试。保留数据时必须 release 页面到有界队列。
```

<a id="fn-29988a1b5f410ade"></a>

## `xv::net::DatagramSink::~DatagramSink`

```doxygen
@fn xv::net::DatagramSink::~DatagramSink()
@brief 结束 DatagramSink 的基类或资源生命周期。
@details protected 非虚析构只销毁基类部分；不能通过接口 delete。对象拥有权按工厂/注册表/File 契约转交。
```

<a id="fn-1ca7e5679b67ce43"></a>

## `xv::net::UdpProtocol::UdpProtocol`

```doxygen
@fn xv::net::UdpProtocol::UdpProtocol(Ipv4Layer &ip, IcmpProtocol &icmp)
@brief 构造 UdpProtocol 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```
