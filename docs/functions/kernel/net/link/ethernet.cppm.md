# `kernel/net/link/ethernet.cppm`

## 函数导航

- [`xv::net::EthernetLayer::EthernetLayer`](ethernet.cppm.md#fn-fd21ba02652da4a6)
- [`xv::net::EthernetLayer::attach`](ethernet.cpp.md#fn-d01f4eb93cf246f8)
- [`xv::net::EthernetLayer::carrier`](ethernet.cppm.md#fn-1460430a2e15ffcf)
- [`xv::net::EthernetLayer::frame_mtu`](ethernet.cppm.md#fn-8f0aba35e6de7dc3)
- [`xv::net::EthernetLayer::initialize`](ethernet.cpp.md#fn-c7b38042d08f668f)
- [`xv::net::EthernetLayer::mac`](ethernet.cppm.md#fn-e1f5e310ce433151)
- [`xv::net::EthernetLayer::online`](ethernet.cppm.md#fn-825314c376f22dde)
- [`xv::net::EthernetLayer::promiscuous`](ethernet.cppm.md#fn-7d5390e8fba95b1e)
- [`xv::net::EthernetLayer::receive_frame`](ethernet.cpp.md#fn-bf5e575793115a66)
- [`xv::net::EthernetLayer::send`](ethernet.cpp.md#fn-468ec89c64e20c7c)
- [`xv::net::EthernetLayer::send_ip`](ethernet.cpp.md#fn-2f3a15c921db001f)
- [`xv::net::EthernetLayer::transmit_frame`](ethernet.cpp.md#fn-d7050414ee46b952)
- [`xv::net::EthernetProtocol::receive`](ethernet.cppm.md#fn-e953565e24bf5559)
- [`xv::net::EthernetProtocol::~EthernetProtocol`](ethernet.cppm.md#fn-e3cf138ee8656e53)

<a id="fn-e953565e24bf5559"></a>

## `xv::net::EthernetProtocol::receive`

```doxygen
@fn void xv::net::EthernetProtocol::receive(Packet &packet)
@brief 按 EtherType 分派的链路协议接收契约。
@param packet 已验证目标 MAC 与有效帧长度的唯一页面。
@details 方法返回后未移交的页面由上层 RAII 回收；实现不能假设默认已取得网络注册锁。
```

<a id="fn-e3cf138ee8656e53"></a>

## `xv::net::EthernetProtocol::~EthernetProtocol`

```doxygen
@fn xv::net::EthernetProtocol::~EthernetProtocol()
@brief 结束 EthernetProtocol 的基类或资源生命周期。
@details protected 非虚析构只销毁基类部分；不能通过接口 delete。对象拥有权按工厂/注册表/File 契约转交。
```

<a id="fn-fd21ba02652da4a6"></a>

## `xv::net::EthernetLayer::EthernetLayer`

```doxygen
@fn xv::net::EthernetLayer::EthernetLayer(unsigned mtu)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-e1f5e310ce433151"></a>

## `xv::net::EthernetLayer::mac`

```doxygen
@fn const MacAddress &xv::net::EthernetLayer::mac() const
@brief 借用初始化时从设备读取的 MAC。
```

<a id="fn-1460430a2e15ffcf"></a>

## `xv::net::EthernetLayer::carrier`

```doxygen
@fn bool xv::net::EthernetLayer::carrier() const
@brief 返回所绑定设备的物理载波状态，尚未绑定返回 false。
```

<a id="fn-825314c376f22dde"></a>

## `xv::net::EthernetLayer::online`

```doxygen
@fn bool xv::net::EthernetLayer::online() const
@brief 返回所绑定设备的驱动就绪状态，尚未绑定返回 false。
```
<a id="fn-8f0aba35e6de7dc3"></a>

## `xv::net::EthernetLayer::frame_mtu`

```doxygen
@fn unsigned xv::net::EthernetLayer::frame_mtu() const
@brief 返回该二层端口的有效载荷 MTU，不含十四字节以太首部；桥据此拒绝不能承载自身 MTU 的成员。
```

<a id="fn-7d5390e8fba95b1e"></a>

## `xv::net::EthernetLayer::promiscuous`

```doxygen
@fn bool xv::net::EthernetLayer::promiscuous(bool enable)
@brief 把混杂接收开关交给所绑定设备，未绑定或设备不支持返回 false；由桥管理在配置锁外调用。
```
