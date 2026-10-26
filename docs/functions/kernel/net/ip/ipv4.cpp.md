# `kernel/net/ip/ipv4.cpp`

## 函数导航

- [`xv::net::Ipv4Layer::allocate`](ipv4.cpp.md#fn-9f3ac76c2a449fd0)
- [`xv::net::Ipv4Layer::attach`](ipv4.cpp.md#fn-117533788806052c)
- [`xv::net::Ipv4Layer::fragment`](ipv4.cpp.md#fn-501941cbcccc2bee)
- [`xv::net::Ipv4Layer::initialize`](ipv4.cpp.md#fn-7415b9e5fc7d3a4f)
- [`xv::net::Ipv4Layer::mtu_for`](ipv4.cpp.md#fn-3a0cbb19a9a3cb97)
- [`xv::net::Ipv4Layer::receive`](ipv4.cpp.md#fn-0a3554f93eae9eb7)
- [`xv::net::Ipv4Layer::send`](ipv4.cpp.md#fn-ec39a5d6caadee7f)
- [`xv::net::Ipv4Layer::source_for`](ipv4.cpp.md#fn-e20391cb067e3d28)
- [`xv::net::Ipv4Layer::tick`](ipv4.cpp.md#fn-a0dc11b7e7393d13)

<a id="fn-7415b9e5fc7d3a4f"></a>

## `xv::net::Ipv4Layer::initialize`

```doxygen
@fn void xv::net::Ipv4Layer::initialize()
@brief Ipv4Layer：启动时初始化分片重组与回环队列锁。
@details 启动时初始化分片重组与回环队列锁。
```

<a id="fn-117533788806052c"></a>

## `xv::net::Ipv4Layer::attach`

```doxygen
@fn void xv::net::Ipv4Layer::attach(unsigned protocol, IpProtocol &handler)
@brief Ipv4Layer：启动时登记协议号与 IpProtocol 实例，当前挂接 ICMP(1)、TCP(6)、UDP(17)。
@details 启动时登记协议号与 IpProtocol 实例，当前挂接 ICMP(1)、TCP(6)、UDP(17)。
```

<a id="fn-9f3ac76c2a449fd0"></a>

## `xv::net::Ipv4Layer::allocate`

```doxygen
@fn Packet xv::net::Ipv4Layer::allocate(unsigned length)
@brief 分配清零的纯 IP 页面，包含二十字节首部和至多 4062 字节负载；不预留以太首部，分配失败返回空 Packet。
```

<a id="fn-e20391cb067e3d28"></a>

## `xv::net::Ipv4Layer::source_for`

```doxygen
@fn unsigned xv::net::Ipv4Layer::source_for(unsigned destination, unsigned requested, const NetworkInterface *via) const
@brief 按目标、可选绑定源和强制出口查询实际源地址；无路由返回零，传输校验和须使用同一源。
```

<a id="fn-3a0cbb19a9a3cb97"></a>

## `xv::net::Ipv4Layer::mtu_for`

```doxygen
@fn unsigned xv::net::Ipv4Layer::mtu_for(unsigned destination, unsigned requested) const
@brief 查询目标与可选绑定源所选择出口的 MTU，无路由返回零。
```

<a id="fn-501941cbcccc2bee"></a>

## `xv::net::Ipv4Layer::fragment`

```doxygen
@fn void xv::net::Ipv4Layer::fragment(NetworkInterface &interface, Packet &packet, unsigned header, unsigned total)
@brief 按入站接口、源/目的、标识和协议隔离四个重组槽；拒绝选项分片、重叠及超长包，完成后解锁再递交纯 IP 报文。
```

<a id="fn-a0dc11b7e7393d13"></a>

## `xv::net::Ipv4Layer::tick`

```doxygen
@fn void xv::net::Ipv4Layer::tick(unsigned long now)
@brief 在重组锁内清理已到期分片页面；回环队列由 LoopbackInterface 独立维护。
```
<a id="fn-ec39a5d6caadee7f"></a>

## `xv::net::Ipv4Layer::send`

```doxygen
@fn int xv::net::Ipv4Layer::send(Packet &packet, unsigned destination, unsigned protocol, unsigned source, const NetworkInterface *via)
@brief 在选路快照后填写纯 IP 首部并通过统一出口 send 计数；使用同一选中源与 MTU，DF 禁止出站分片，无路由返回 -ENETUNREACH。
```

<a id="fn-0a3554f93eae9eb7"></a>

## `xv::net::Ipv4Layer::receive`

```doxygen
@fn void xv::net::Ipv4Layer::receive(NetworkInterface &interface, Packet &packet)
@brief 检查注册入口、状态和 IPv4 基础字段，拒绝时累计接口丢弃；验证后携带入口身份重组或分派，不持配置锁调用上层。
```
