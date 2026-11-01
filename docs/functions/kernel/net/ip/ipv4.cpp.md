# `kernel/net/ip/ipv4.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Ipv4Layer::allocate`](ipv4.cpp.md#fn-9f3ac76c2a449fd0)
- [`xv::net::Ipv4Layer::attach`](ipv4.cpp.md#fn-117533788806052c)
- [`xv::net::Ipv4Layer::initialize`](ipv4.cpp.md#fn-7415b9e5fc7d3a4f)
- [`xv::net::Ipv4Layer::mtu_for`](ipv4.cpp.md#fn-3a0cbb19a9a3cb97)
- [`xv::net::Ipv4Layer::output_allowed`](ipv4.cpp.md#fn-df3cc0c85232f9e7)
- [`xv::net::Ipv4Layer::receive`](ipv4.cpp.md#fn-0a3554f93eae9eb7)
- [`xv::net::Ipv4Layer::send`](ipv4.cpp.md#fn-ec39a5d6caadee7f)
- [`xv::net::Ipv4Layer::send_bootstrap`](ipv4.cpp.md#fn-d38f175c28a36890)
- [`xv::net::Ipv4Layer::source_for`](ipv4.cpp.md#fn-e20391cb067e3d28)
- [`xv::net::Ipv4Layer::tick`](ipv4.cpp.md#fn-a0dc11b7e7393d13)
- [`xv::net::Ipv4Layer::write_header`](ipv4.cpp.md#fn-38cbc784b1b54011)

<a id="fn-7415b9e5fc7d3a4f"></a>

## `xv::net::Ipv4Layer::initialize`

```doxygen
@fn void xv::net::Ipv4Layer::initialize()
@brief 在发布收发服务前初始化所属重组组件的锁，不重置已配置的协议和过滤接收入口。
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
@brief 为有界 IPv4 载荷预留二十字节基础首部并清零有效字节，载荷上限与重组组件一致。
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

<a id="fn-38cbc784b1b54011"></a>

## `xv::net::Ipv4Layer::write_header`

```doxygen
@fn void xv::net::Ipv4Layer::write_header(Packet &packet, unsigned source, unsigned destination, unsigned protocol)
@brief 统一构造出站基础 IPv4 首部：版本、TTL 64、协议、总长、原子 ID、DF、地址和校验和。
@pre 报文拥有页面且至少二十字节；调用者已完成普通选路或引导发送的边界检查。
```

<a id="fn-df3cc0c85232f9e7"></a>

## `xv::net::Ipv4Layer::output_allowed`

```doxygen
@fn bool xv::net::Ipv4Layer::output_allowed(const Packet &packet, const NetworkInterface &interface)
@brief 对已编码的完整出站数据报执行 OUTPUT 过滤，无过滤器时直接放行。
@pre 报文首部已完成，interface 是选定的出口，不转移页面。
```

<a id="fn-ec39a5d6caadee7f"></a>

## `xv::net::Ipv4Layer::send`

```doxygen
@fn int xv::net::Ipv4Layer::send(Packet &packet, unsigned destination, unsigned protocol, unsigned source, const NetworkInterface *via)
@brief 选出出口与源地址，核对报文和 MTU，共用首部编码及 OUTPUT 检查后交给接口。
@details IPv4 收发后通过 TimerChange 通知网络任务；分片重组下一截止时间由受锁保护的重组器查询。
@return 选路失败 -ENETUNREACH，报文或 MTU 错误 -EMSGSIZE，过滤拒绝 -EACCES，其他结果由接口返回。
```

<a id="fn-0a3554f93eae9eb7"></a>

## `xv::net::Ipv4Layer::receive`

```doxygen
@fn void xv::net::Ipv4Layer::receive(NetworkInterface &interface, Packet &packet)
@brief 校验入口与 IPv4 首部，委托重组完整报文，再依次执行本机/引导目的判断、INPUT 过滤和协议分派。
@details IPv4 收发后通过 TimerChange 通知网络任务；分片重组下一截止时间由受锁保护的重组器查询。
```

<a id="fn-a0dc11b7e7393d13"></a>

## `xv::net::Ipv4Layer::tick`

```doxygen
@fn void xv::net::Ipv4Layer::tick(unsigned long now)
@brief 委托所属重组组件释放到期片段页面，不在重组锁内回调协议。
```

<a id="fn-d38f175c28a36890"></a>

## `xv::net::Ipv4Layer::send_bootstrap`

```doxygen
@fn int xv::net::Ipv4Layer::send_bootstrap(NetworkInterface &interface, Packet &packet, unsigned source)
@brief 校验活动 DHCP 接口及 UDP 68→67 条件，使用共用首部编码与 OUTPUT 检查发送引导广播。
@details IPv4 收发后通过 TimerChange 通知网络任务；分片重组下一截止时间由受锁保护的重组器查询。
```

<a id="fn-11b147b885cba8ca"></a>

## `xv::net::Ipv4Layer::next_deadline`

```doxygen
@fn unsigned long xv::net::Ipv4Layer::next_deadline()
@brief IPv4 收发后通过 TimerChange 通知网络任务；分片重组下一截止时间由受锁保护的重组器查询。
@details IPv4 收发后通过 TimerChange 通知网络任务；分片重组下一截止时间由受锁保护的重组器查询。
```

- [`xv::net::Ipv4Layer::next_deadline`](ipv4.cpp.md#fn-11b147b885cba8ca)
