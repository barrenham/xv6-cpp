# `kernel/net/ip/ipv4.cppm`

## 函数导航

- [`xv::net::IpProtocol::receive`](ipv4.cppm.md#fn-56b2966d4e378d05)
- [`xv::net::IpProtocol::~IpProtocol`](ipv4.cppm.md#fn-a85a20734f631a87)
- [`xv::net::Ipv4Layer::Ipv4Layer`](ipv4.cppm.md#fn-1842ecb0929516e0)
- [`xv::net::Ipv4Layer::allocate`](ipv4.cpp.md#fn-9f3ac76c2a449fd0)
- [`xv::net::Ipv4Layer::attach`](ipv4.cpp.md#fn-117533788806052c)
- [`xv::net::Ipv4Layer::fragment`](ipv4.cpp.md#fn-501941cbcccc2bee)
- [`xv::net::Ipv4Layer::initialize`](ipv4.cpp.md#fn-7415b9e5fc7d3a4f)
- [`xv::net::Ipv4Layer::local`](ipv4.cppm.md#fn-5728bbcf34020003)
- [`xv::net::Ipv4Layer::mtu_for`](ipv4.cpp.md#fn-3a0cbb19a9a3cb97)
- [`xv::net::Ipv4Layer::receive`](ipv4.cpp.md#fn-0a3554f93eae9eb7)
- [`xv::net::Ipv4Layer::send`](ipv4.cpp.md#fn-ec39a5d6caadee7f)
- [`xv::net::Ipv4Layer::source_for`](ipv4.cpp.md#fn-e20391cb067e3d28)
- [`xv::net::Ipv4Layer::tick`](ipv4.cpp.md#fn-a0dc11b7e7393d13)

<a id="fn-56b2966d4e378d05"></a>

## `xv::net::IpProtocol::receive`

```doxygen
@fn void xv::net::IpProtocol::receive(Packet &packet, const IpDatagram &datagram)
@brief 已验证 IPv4 数据报的多态接收契约。
@param packet 持有完整纯 IP 页面，起点不含以太首部的唯一对象。
@param datagram 已验证的源/目标、协议号、负载边界和入站接口。
@details 只访问 datagram 给定范围；若排队必须显式移交 Packet 所有权，不允许睡眠或用户地址复制。
```

<a id="fn-a85a20734f631a87"></a>

## `xv::net::IpProtocol::~IpProtocol`

```doxygen
@fn xv::net::IpProtocol::~IpProtocol()
@brief 结束 IpProtocol 的基类或资源生命周期。
@details protected 非虚析构只销毁基类部分；不能通过接口 delete。对象拥有权按工厂/注册表/File 契约转交。
```

<a id="fn-1842ecb0929516e0"></a>

## `xv::net::Ipv4Layer::Ipv4Layer`

```doxygen
@fn xv::net::Ipv4Layer::Ipv4Layer(InterfaceRegistry &interfaces, RoutingTable &routes)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-5728bbcf34020003"></a>

## `xv::net::Ipv4Layer::local`

```doxygen
@fn bool xv::net::Ipv4Layer::local(unsigned address) const
@brief 查询已启用接口注册表的地址拥有关系，支持多个物理地址及回环前缀。
```
