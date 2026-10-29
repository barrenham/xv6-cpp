# `kernel/net/interface/tunnel.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IpInIpInterface::initialize`](tunnel.cpp.md#fn-414d9b2164bde204)
- [`xv::net::IpInIpInterface::receive_outer`](tunnel.cpp.md#fn-9e6c6c02ccc93ae4)
- [`xv::net::IpInIpInterface::transmit`](tunnel.cpp.md#fn-6e1ec61ddfefeb21)
- [`xv::net::IpInIpProtocol::add`](tunnel.cpp.md#fn-c1358ee430c56716)
- [`xv::net::IpInIpProtocol::receive`](tunnel.cpp.md#fn-0bbf7c9d366e4c64)

<a id="fn-414d9b2164bde204"></a>

## `xv::net::IpInIpInterface::initialize`

```doxygen
@fn bool xv::net::IpInIpInterface::initialize(Ipv4Layer &ip, NetworkInterface &underlay, unsigned local, unsigned remote)
@brief 启动期绑定 IPv4 服务、外层端点和非隧道下层。
@details 外层本地地址须属于下层；内层 MTU 至少留出二十字节外层首部，失败不安装依赖。
@param ip 共享 IPv4 协议服务。
@param underlay 承载外层报文的非隧道接口。
@param local 外层本地 IPv4，必须属于 underlay。
@param remote 外层远端 IPv4，非零且不同于 local。
@return 依赖和 MTU 校验成功返回 true；重复初始化返回 false。
```

<a id="fn-6e1ec61ddfefeb21"></a>

## `xv::net::IpInIpInterface::transmit`

```doxygen
@fn int xv::net::IpInIpInterface::transmit(Packet &packet, unsigned)
@brief 检查状态和内层 MTU，前插二十字节首部后以协议号 4 发送；强制使用指定下层及外层源地址，失败移除所插首部。
@details 实现通过 TransmitFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-9e6c6c02ccc93ae4"></a>

## `xv::net::IpInIpInterface::receive_outer`

```doxygen
@fn bool xv::net::IpInIpInterface::receive_outer(Packet &packet, const IpDatagram &d)
@brief 仅匹配指定入站下层及外层源/目的地址；匹配后裁去外层首部并以隧道身份重新投递。
@details 短包或解封深度超过四层时消费并丢弃。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@param[in,out] packet 包含完整外层首部的数据页。
@param d IPv4 层已验证的外层边界及入口身份。
@return false 表示端点或入口不匹配；true 表示本隧道已消费，包括匹配后丢弃。
```

<a id="fn-c1358ee430c56716"></a>

## `xv::net::IpInIpProtocol::add`

```doxygen
@fn bool xv::net::IpInIpProtocol::add(IpInIpInterface &interface)
@brief 启动期借用一个隧道接收器，拒绝重复和超过八项；隧道对象须长期存活。
@details 实现通过 DispatchFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-0bbf7c9d366e4c64"></a>

## `xv::net::IpInIpProtocol::receive`

```doxygen
@fn void xv::net::IpInIpProtocol::receive(Packet &packet, const IpDatagram &datagram)
@brief 按注册顺序匹配外层端点和入站接口，首个匹配隧道消费报文；未匹配由 Packet 析构释放。
@details 实现通过 DispatchFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
