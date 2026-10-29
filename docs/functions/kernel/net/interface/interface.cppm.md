# `kernel/net/interface/interface.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::InterfaceRegistry::add`](interface.cpp.md#fn-19629c883c7bf34c)
- [`xv::net::InterfaceRegistry::at`](interface.cppm.md#fn-b76c86a7bfb7aabc)
- [`xv::net::InterfaceRegistry::count`](interface.cppm.md#fn-9f93947e373b767e)
- [`xv::net::InterfaceRegistry::find`](interface.cpp.md#fn-bfd9e34fed60845d)
- [`xv::net::InterfaceRegistry::owns`](interface.cpp.md#fn-9875c43aa204f3b9)
- [`xv::net::InterfaceRegistry::publish`](interface.cpp.md#fn-10d3ef393b2b8755)
- [`xv::net::InterfaceRegistry::registration_error`](interface.cpp.md#fn-371622e603f8de22)
- [`xv::net::InterfaceRegistry::seal`](interface.cpp.md#fn-6dc874d592d4900e)
- [`xv::net::InterfaceRegistry::tick`](interface.cpp.md#fn-11570218be60251c)
- [`xv::net::InterfaceRegistry::withdraw`](interface.cpp.md#fn-8fd44ee2a0436af7)
- [`xv::net::IpReceiver::receive`](interface.cppm.md#fn-4db4082c70dfe156)
- [`xv::net::IpReceiver::~IpReceiver`](interface.cppm.md#fn-e35112b1f1b79196)
- [`xv::net::NetworkInterface::NetworkInterface`](interface.cppm.md#fn-7f76e185fefb2835)
- [`xv::net::NetworkInterface::NetworkInterface`](interface.cppm.md#fn-75d62bfd584e1c73)
- [`xv::net::NetworkInterface::accepts_destination`](interface.cpp.md#fn-afdbdf714f5adc2b)
- [`xv::net::NetworkInterface::account_send`](interface.cpp.md#fn-c99a4c9b5b90a08c)
- [`xv::net::NetworkInterface::add_address`](interface.cpp.md#fn-29e4fdf0db63d8be)
- [`xv::net::NetworkInterface::address`](interface.cppm.md#fn-61d3aff01263ffa7)
- [`xv::net::NetworkInterface::address_count`](interface.cppm.md#fn-94e3c574ca835a7a)
- [`xv::net::NetworkInterface::attach`](interface.cppm.md#fn-1c1cf46241f614d1)
- [`xv::net::NetworkInterface::broadcast`](interface.cpp.md#fn-aa7c7468ae954fd0)
- [`xv::net::NetworkInterface::carrier`](interface.cppm.md#fn-b5047586873434d7)
- [`xv::net::NetworkInterface::configure_link`](interface.cpp.md#fn-38a3be6d9f984d65)
- [`xv::net::NetworkInterface::deliver`](interface.cpp.md#fn-9b5c9773ed945053)
- [`xv::net::NetworkInterface::dhcp_active`](interface.cppm.md#fn-85aa8fb35261e0b3)
- [`xv::net::NetworkInterface::generation`](interface.cppm.md#fn-f3c8aa01de8e327c)
- [`xv::net::NetworkInterface::id`](interface.cppm.md#fn-b511fe96a0cf54c6)
- [`xv::net::NetworkInterface::invalidate`](interface.cppm.md#fn-9b47e77a19f3fcde)
- [`xv::net::NetworkInterface::is_bridge`](interface.cppm.md#fn-c281e79486e261b6)
- [`xv::net::NetworkInterface::is_tunnel`](interface.cppm.md#fn-48100c513b8eb3f7)
- [`xv::net::NetworkInterface::layer2`](interface.cppm.md#fn-3239b118d8b0f543)
- [`xv::net::NetworkInterface::mac`](interface.cppm.md#fn-11e021ef3340ef33)
- [`xv::net::NetworkInterface::master`](interface.cppm.md#fn-df4b298eb8ae885a)
- [`xv::net::NetworkInterface::mtu`](interface.cppm.md#fn-0c6eee747ef53549)
- [`xv::net::NetworkInterface::name`](interface.cppm.md#fn-f67f92c3b57f37e1)
- [`xv::net::NetworkInterface::neighbor`](interface.cppm.md#fn-db1c14ebd06114d2)
- [`xv::net::NetworkInterface::operator=`](interface.cppm.md#fn-2686b037f2ab5fda)
- [`xv::net::NetworkInterface::owns`](interface.cpp.md#fn-24e03455c304d9fd)
- [`xv::net::NetworkInterface::receive_drop`](interface.cpp.md#fn-72713f8a1c6e3db4)
- [`xv::net::NetworkInterface::send`](interface.cpp.md#fn-02e2a99c0121b11a)
- [`xv::net::NetworkInterface::send_bootstrap`](interface.cpp.md#fn-4486a19c308f9201)
- [`xv::net::NetworkInterface::set_dhcp_active`](interface.cpp.md#fn-d319bc6cf8fb7b63)
- [`xv::net::NetworkInterface::set_up`](interface.cpp.md#fn-0dd1df949d25550a)
- [`xv::net::NetworkInterface::source_for`](interface.cpp.md#fn-1c833c16e2ec25f4)
- [`xv::net::NetworkInterface::statistics`](interface.cpp.md#fn-a1a1eb1c46a2f883)
- [`xv::net::NetworkInterface::tick`](interface.cppm.md#fn-d7cce1ea0cdf5e32)
- [`xv::net::NetworkInterface::topology_changed`](interface.cpp.md#fn-0aaeed5dbb643a04)
- [`xv::net::NetworkInterface::transmit`](interface.cppm.md#fn-01938802e0c33e15)
- [`xv::net::NetworkInterface::transmit_bootstrap`](interface.cppm.md#fn-6d7f4d82c2a0b4ef)
- [`xv::net::NetworkInterface::up`](interface.cppm.md#fn-500511d922186f66)
- [`xv::net::NetworkInterface::~NetworkInterface`](interface.cppm.md#fn-03de2f61c61d9eae)

<a id="fn-4db4082c70dfe156"></a>

## `xv::net::IpReceiver::receive`

```doxygen
@fn void xv::net::IpReceiver::receive(NetworkInterface &interface, Packet &packet)
@brief 纯 IP 入站契约。
@details interface 标明真实入口，packet 起始字节为 IP 首部；同步借用页面，若异步保留必须显式接管所有权。
@param interface 本次接收的真实入口对象，解封后为隧道对象。
@param[in,out] packet 起点为 IP 首部的页面，可显式移交。
@warning 不允许睡眠或直接复制用户地址。
```

<a id="fn-e35112b1f1b79196"></a>

## `xv::net::IpReceiver::~IpReceiver`

```doxygen
@fn xv::net::IpReceiver::~IpReceiver()
@brief 结束接口基类生命周期；protected 非虚析构禁止通过接口 delete，具体对象由启动期长期存储管理。
```

<a id="fn-75d62bfd584e1c73"></a>

## `xv::net::NetworkInterface::NetworkInterface`

```doxygen
@fn constexpr xv::net::NetworkInterface::NetworkInterface(unsigned id, const char *name, unsigned mtu)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-7f76e185fefb2835"></a>

## `xv::net::NetworkInterface::NetworkInterface`

```doxygen
@fn xv::net::NetworkInterface::NetworkInterface(const NetworkInterface &)
@brief 禁止复制或赋值此接口对象，保持注册身份、借用关系和队列所有权唯一。
```

<a id="fn-2686b037f2ab5fda"></a>

## `xv::net::NetworkInterface::operator=`

```doxygen
@fn NetworkInterface &xv::net::NetworkInterface::operator=(const NetworkInterface &)
@brief 禁止复制或赋值此接口对象，保持注册身份、借用关系和队列所有权唯一。
```

<a id="fn-b511fe96a0cf54c6"></a>

## `xv::net::NetworkInterface::id`

```doxygen
@fn unsigned xv::net::NetworkInterface::id() const
@brief 返回启动期固定的非零接口标识，接收路径据此核对注册表中的对象身份。
```

<a id="fn-f67f92c3b57f37e1"></a>

## `xv::net::NetworkInterface::name`

```doxygen
@fn const char *xv::net::NetworkInterface::name() const
@brief 借用接口名称字符串；调用者须保证字符串在接口整个生命周期内有效。
```

<a id="fn-0c6eee747ef53549"></a>

## `xv::net::NetworkInterface::mtu`

```doxygen
@fn unsigned xv::net::NetworkInterface::mtu() const
@brief 返回接口可承载的最大纯 IP 报文长度，不包含以太首部。
```

<a id="fn-85aa8fb35261e0b3"></a>

## `xv::net::NetworkInterface::dhcp_active`

```doxygen
@fn bool xv::net::NetworkInterface::dhcp_active() const
@brief 在配置锁下查询接口是否由 DHCP 客户端占用。
@details 尚在发现或失败重试阶段也算活动，桥管理必须要求客户端停止后才接入成员或注销。
```

<a id="fn-500511d922186f66"></a>

## `xv::net::NetworkInterface::up`

```doxygen
@fn bool xv::net::NetworkInterface::up() const
@brief 在配置锁下读取接口管理状态；carrier 单独表示物理或下层链路可用性。
```

<a id="fn-f3c8aa01de8e327c"></a>

## `xv::net::NetworkInterface::generation`

```doxygen
@fn unsigned xv::net::NetworkInterface::generation() const
@brief 在配置锁下读取接口配置代次；ARP 待解析包据此检测启停或地址变化，拒绝迟到的旧配置包。
```

<a id="fn-db1c14ebd06114d2"></a>

## `xv::net::NetworkInterface::neighbor`

```doxygen
@fn virtual bool xv::net::NetworkInterface::neighbor(unsigned, NeighborInfo &)
@brief 无邻居解析能力的接口默认返回 false，不虚构 MAC 或邻居记录；以太子类提供真实缓存。
```

<a id="fn-9b47e77a19f3fcde"></a>

## `xv::net::NetworkInterface::invalidate`

```doxygen
@fn virtual void xv::net::NetworkInterface::invalidate()
@brief 接口配置变更后的清理钩子，基类不执行操作；调用者必须在配置锁外调用，子类可取得自己的队列锁。
```

<a id="fn-c281e79486e261b6"></a>

## `xv::net::NetworkInterface::is_bridge`

```doxygen
@fn virtual bool xv::net::NetworkInterface::is_bridge() const
@brief 基类默认返回 false；BridgeInterface 覆盖为 true，独立于有无 MAC、IP 或隧道能力。
```

<a id="fn-3239b118d8b0f543"></a>

## `xv::net::NetworkInterface::layer2`

```doxygen
@fn virtual Layer2Port *xv::net::NetworkInterface::layer2()
@brief 基类返回 nullptr，表示没有完整以太帧端口能力；回环和三层隧道不能因此被强行接入网桥。
```

<a id="fn-df4b298eb8ae885a"></a>

## `xv::net::NetworkInterface::master`

```doxygen
@fn virtual unsigned xv::net::NetworkInterface::master() const
@brief 基类默认没有二层 master，返回零；以太接口从自己的 Layer2Port 查询桥归属。
```

<a id="fn-48100c513b8eb3f7"></a>

## `xv::net::NetworkInterface::is_tunnel`

```doxygen
@fn virtual bool xv::net::NetworkInterface::is_tunnel() const
@brief 基类返回 false；隧道覆盖此能力查询，用于拒绝递归隧道下层。
```

<a id="fn-b5047586873434d7"></a>

## `xv::net::NetworkInterface::carrier`

```doxygen
@fn virtual bool xv::net::NetworkInterface::carrier() const
@brief 基类默认链路可用，具体物理接口查询设备，隧道查询下层链路。
```

<a id="fn-11e021ef3340ef33"></a>

## `xv::net::NetworkInterface::mac`

```doxygen
@fn virtual const MacAddress *xv::net::NetworkInterface::mac() const
@brief 基类返回 nullptr，表示该三层接口没有以太网 MAC 属性。
```

<a id="fn-94e3c574ca835a7a"></a>

## `xv::net::NetworkInterface::address_count`

```doxygen
@fn unsigned xv::net::NetworkInterface::address_count() const
@brief 在配置锁下读取零至四个地址的数量；零表示尚无 IPv4 或无编号配置，不影响设备和 MAC 身份。
```

<a id="fn-61d3aff01263ffa7"></a>

## `xv::net::NetworkInterface::address`

```doxygen
@fn Ipv4Address xv::net::NetworkInterface::address(unsigned index) const
@brief 在配置锁下复制一个 IPv4 地址项，越界返回空值；返回副本避免并发删地址后悬空引用，循环查询需留意并发修改。
```

<a id="fn-1c1cf46241f614d1"></a>

## `xv::net::NetworkInterface::attach`

```doxygen
@fn void xv::net::NetworkInterface::attach(IpReceiver &receiver)
@brief 启动期借用纯 IP 接收器；接收器须比接口存活更久，运行期不得并发更换。
```

<a id="fn-6d7f4d82c2a0b4ef"></a>

## `xv::net::NetworkInterface::transmit_bootstrap`

```doxygen
@fn virtual int xv::net::NetworkInterface::transmit_bootstrap(Packet &)
@brief 不具备以太广播能力的接口默认返回不支持。
@details 回环和三层隧道不会通过普通 IP 发送契约伪造 DHCP 二层能力。
```

<a id="fn-01938802e0c33e15"></a>

## `xv::net::NetworkInterface::transmit`

```doxygen
@fn int xv::net::NetworkInterface::transmit(Packet &packet, unsigned next_hop)
@brief 多态发送纯 IP 报文。
@details next_hop 是选路结果；返回零表示已经接管或发送，负值表示失败，未接管页面仍由 Packet 释放。
@param[in,out] packet 起点为 IP 首部的独占页面；实现可封装或移交。
@param next_hop 路由器计算的主机字节序下一跳；三层虚拟接口可忽略。
@return 零表示接受；负 errno 表示失败。
@warning 该路径可在 IRQ/时钟上下文运行，不得睡眠或访问用户地址。
```

<a id="fn-d7cce1ea0cdf5e32"></a>

## `xv::net::NetworkInterface::tick`

```doxygen
@fn virtual void xv::net::NetworkInterface::tick(unsigned long)
@brief 接口周期服务钩子；基类不执行操作，子类驱动邻居重试或回环排队投递。
```

<a id="fn-03de2f61c61d9eae"></a>

## `xv::net::NetworkInterface::~NetworkInterface`

```doxygen
@fn xv::net::NetworkInterface::~NetworkInterface()
@brief 结束接口基类生命周期；protected 非虚析构禁止通过接口 delete，具体对象由启动期长期存储管理。
```

<a id="fn-9f93947e373b767e"></a>

## `xv::net::InterfaceRegistry::count`

```doxygen
@fn unsigned xv::net::InterfaceRegistry::count() const
@brief 在配置锁内读取当前已发布接口数，运行期创建/删除桥可使该值变化，逐项枚举不是整轮冻结。
```

<a id="fn-b76c86a7bfb7aabc"></a>

## `xv::net::InterfaceRegistry::at`

```doxygen
@fn NetworkInterface *xv::net::InterfaceRegistry::at(unsigned index) const
@brief 在配置锁内按发布顺序借用接口，越界返回 nullptr；对象长期存活，但并发注销后可能已不在注册表内。
```
