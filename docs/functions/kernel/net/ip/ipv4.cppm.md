# `kernel/net/ip/ipv4.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IpProtocol::receive`](ipv4.cppm.md#fn-56b2966d4e378d05)
- [`xv::net::IpProtocol::~IpProtocol`](ipv4.cppm.md#fn-a85a20734f631a87)
- [`xv::net::Ipv4Bootstrap::receive_bootstrap`](ipv4.cppm.md#fn-3bebdee13a445ad8)
- [`xv::net::Ipv4Bootstrap::~Ipv4Bootstrap`](ipv4.cppm.md#fn-dc28cbdeba540453)
- [`xv::net::Ipv4Layer::Ipv4Layer`](ipv4.cppm.md#fn-1842ecb0929516e0)
- [`xv::net::Ipv4Layer::allocate`](ipv4.cpp.md#fn-9f3ac76c2a449fd0)
- [`xv::net::Ipv4Layer::attach`](ipv4.cpp.md#fn-117533788806052c)
- [`xv::net::Ipv4Layer::attach_bootstrap`](ipv4.cppm.md#fn-6a0a31ea650c206e)
- [`xv::net::Ipv4Layer::attach_filter`](ipv4.cppm.md#fn-9c7739c914926daa)
- [`xv::net::Ipv4Layer::initialize`](ipv4.cpp.md#fn-7415b9e5fc7d3a4f)
- [`xv::net::Ipv4Layer::local`](ipv4.cppm.md#fn-5728bbcf34020003)
- [`xv::net::Ipv4Layer::mtu_for`](ipv4.cpp.md#fn-3a0cbb19a9a3cb97)
- [`xv::net::Ipv4Layer::output_allowed`](ipv4.cpp.md#fn-df3cc0c85232f9e7)
- [`xv::net::Ipv4Layer::receive`](ipv4.cpp.md#fn-0a3554f93eae9eb7)
- [`xv::net::Ipv4Layer::send`](ipv4.cpp.md#fn-ec39a5d6caadee7f)
- [`xv::net::Ipv4Layer::send_bootstrap`](ipv4.cpp.md#fn-d38f175c28a36890)
- [`xv::net::Ipv4Layer::source_for`](ipv4.cpp.md#fn-e20391cb067e3d28)
- [`xv::net::Ipv4Layer::tick`](ipv4.cpp.md#fn-a0dc11b7e7393d13)
- [`xv::net::Ipv4Layer::write_header`](ipv4.cpp.md#fn-38cbc784b1b54011)

<a id="fn-3bebdee13a445ad8"></a>

## `xv::net::Ipv4Bootstrap::receive_bootstrap`

```doxygen
@fn virtual bool xv::net::Ipv4Bootstrap::receive_bootstrap(NetworkInterface &interface, Packet &packet, const IpDatagram &datagram)
@brief 启动协议接收契约，在常规本机目的地址过滤前有条件消费已验证的 IPv4 数据报。
@param interface 实际入接口。
@param packet 借用的完整纯 IP 页面。
@param datagram 已检查的协议号及有效负载范围。
@return true 消费候选，false 继续正常目的地址校验；不得扩大普通 socket 收包范围。
```

<a id="fn-dc28cbdeba540453"></a>

## `xv::net::Ipv4Bootstrap::~Ipv4Bootstrap`

```doxygen
@fn xv::net::Ipv4Bootstrap::~Ipv4Bootstrap()
@brief 结束启动协议接收器基类部分，protected 非虚析构禁止通过接口 delete。
@details 具体接收器由静态网络服务或受控测试上下文持有。
```

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
@fn constexpr xv::net::Ipv4Layer::Ipv4Layer(InterfaceRegistry &interfaces, RoutingTable &routes)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-9c7739c914926daa"></a>

## `xv::net::Ipv4Layer::attach_filter`

```doxygen
@fn void xv::net::Ipv4Layer::attach_filter(PacketFilter &filter)
@brief 在启动组装阶段注入借用过滤器，要求其覆盖 IPv4 层的生命周期。
```

<a id="fn-6a0a31ea650c206e"></a>

## `xv::net::Ipv4Layer::attach_bootstrap`

```doxygen
@fn void xv::net::Ipv4Layer::attach_bootstrap(Ipv4Bootstrap &handler)
@brief 启动期借用一个长期存活的 IPv4 启动协议接收器。
@details 运行期不更换、不销毁接收器，普通协议处理表继续独立按协议号分派。
```

<a id="fn-5728bbcf34020003"></a>

## `xv::net::Ipv4Layer::local`

```doxygen
@fn bool xv::net::Ipv4Layer::local(unsigned address) const
@brief 查询已启用接口注册表的地址拥有关系，支持多个物理地址及回环前缀。
```
