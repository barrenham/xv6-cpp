# `kernel/net/interface/interface.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::InterfaceRegistry::add`](interface.cpp.md#fn-19629c883c7bf34c)
- [`xv::net::InterfaceRegistry::find`](interface.cpp.md#fn-bfd9e34fed60845d)
- [`xv::net::InterfaceRegistry::owns`](interface.cpp.md#fn-9875c43aa204f3b9)
- [`xv::net::InterfaceRegistry::publish`](interface.cpp.md#fn-10d3ef393b2b8755)
- [`xv::net::InterfaceRegistry::registration_error`](interface.cpp.md#fn-371622e603f8de22)
- [`xv::net::InterfaceRegistry::seal`](interface.cpp.md#fn-6dc874d592d4900e)
- [`xv::net::InterfaceRegistry::tick`](interface.cpp.md#fn-11570218be60251c)
- [`xv::net::InterfaceRegistry::withdraw`](interface.cpp.md#fn-8fd44ee2a0436af7)
- [`xv::net::NetworkInterface::accepts_destination`](interface.cpp.md#fn-afdbdf714f5adc2b)
- [`xv::net::NetworkInterface::account_send`](interface.cpp.md#fn-c99a4c9b5b90a08c)
- [`xv::net::NetworkInterface::add_address`](interface.cpp.md#fn-29e4fdf0db63d8be)
- [`xv::net::NetworkInterface::broadcast`](interface.cpp.md#fn-aa7c7468ae954fd0)
- [`xv::net::NetworkInterface::configure_link`](interface.cpp.md#fn-38a3be6d9f984d65)
- [`xv::net::NetworkInterface::deliver`](interface.cpp.md#fn-9b5c9773ed945053)
- [`xv::net::NetworkInterface::owns`](interface.cpp.md#fn-24e03455c304d9fd)
- [`xv::net::NetworkInterface::receive_drop`](interface.cpp.md#fn-72713f8a1c6e3db4)
- [`xv::net::NetworkInterface::send`](interface.cpp.md#fn-02e2a99c0121b11a)
- [`xv::net::NetworkInterface::send_bootstrap`](interface.cpp.md#fn-4486a19c308f9201)
- [`xv::net::NetworkInterface::set_dhcp_active`](interface.cpp.md#fn-d319bc6cf8fb7b63)
- [`xv::net::NetworkInterface::set_up`](interface.cpp.md#fn-0dd1df949d25550a)
- [`xv::net::NetworkInterface::source_for`](interface.cpp.md#fn-1c833c16e2ec25f4)
- [`xv::net::NetworkInterface::statistics`](interface.cpp.md#fn-a1a1eb1c46a2f883)
- [`xv::net::NetworkInterface::topology_changed`](interface.cpp.md#fn-0aaeed5dbb643a04)

<a id="fn-24e03455c304d9fd"></a>

## `xv::net::NetworkInterface::owns`

```doxygen
@fn bool xv::net::NetworkInterface::owns(unsigned value) const
@brief 在配置临界区内查询地址是否精确属于本接口；同一调用链嵌套查询复用外层锁，回环子类采用前缀拥有语义。
```

<a id="fn-afdbdf714f5adc2b"></a>

## `xv::net::NetworkInterface::accepts_destination`

```doxygen
@fn bool xv::net::NetworkInterface::accepts_destination(unsigned address, const InterfaceRegistry &) const
@brief 按入站接口接收本机地址或该接口广播地址，避免普通以太接口跨接口接收。
```

<a id="fn-aa7c7468ae954fd0"></a>

## `xv::net::NetworkInterface::broadcast`

```doxygen
@fn bool xv::net::NetworkInterface::broadcast(unsigned value) const
@brief 在配置锁下识别全一广播和接口前缀广播；/31、/32 无定向广播，无地址接口不接收广播。
```

<a id="fn-1c833c16e2ec25f4"></a>

## `xv::net::NetworkInterface::source_for`

```doxygen
@fn unsigned xv::net::NetworkInterface::source_for(unsigned destination) const
@brief 在一致配置快照内优先选择覆盖目标且前缀最长的地址，否则取首个；无地址返回零。
```

<a id="fn-29e4fdf0db63d8be"></a>

## `xv::net::NetworkInterface::add_address`

```doxygen
@fn bool xv::net::NetworkInterface::add_address(Ipv4Address value)
@brief 启动期添加 IPv4 地址与前缀。
@details 拒绝封存后修改、第五个地址、零地址、组播/保留地址、无效前缀及重复地址。
```

<a id="fn-0dd1df949d25550a"></a>

## `xv::net::NetworkInterface::set_up`

```doxygen
@fn bool xv::net::NetworkInterface::set_up(bool value)
@brief 启动期设置管理状态；封存后本入口返回 false，运行期修改必须走 NetworkManager 事务。
```

<a id="fn-38a3be6d9f984d65"></a>

## `xv::net::NetworkInterface::configure_link`

```doxygen
@fn void xv::net::NetworkInterface::configure_link(bool value)
@brief 在配置锁内设置运行期启停状态，并为每次管理请求推进接口代次。
@details 与启动期 set_up 不同，本操作可用于已封存接口；即使状态相同也保持原有的代次推进行为。
@pre 管理层已检查接口保护与请求权限；涉及拓扑的联合更新由外层配置锁覆盖。
@note 不调用 invalidate，邻居清理须在调用方解锁后进行。
```

<a id="fn-d319bc6cf8fb7b63"></a>

## `xv::net::NetworkInterface::set_dhcp_active`

```doxygen
@fn void xv::net::NetworkInterface::set_dhcp_active(bool active)
@brief 在配置锁内设置 DHCP 活动标记，不改变接口代次。
@pre DHCP 状态机以外层配置锁覆盖客户端状态及该标记的联合更新。
```

<a id="fn-0aaeed5dbb643a04"></a>

## `xv::net::NetworkInterface::topology_changed`

```doxygen
@fn void xv::net::NetworkInterface::topology_changed()
@brief 在配置锁内推进接口代次，标记网桥成员等拓扑变化。
@pre 调用方以外层配置锁覆盖拓扑状态及代次的联合更新，解锁后再清理邻居。
```

<a id="fn-9b5c9773ed945053"></a>

## `xv::net::NetworkInterface::deliver`

```doxygen
@fn void xv::net::NetworkInterface::deliver(Packet &packet)
@brief 在已启用且已挂接接收器时累计纯 IP 入站计数，再在配置锁外以本接口身份回调；禁止投递则累计丢弃。
@details 实现通过 DeliverFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-371622e603f8de22"></a>

## `xv::net::InterfaceRegistry::registration_error`

```doxygen
@fn int xv::net::InterfaceRegistry::registration_error(const NetworkInterface &interface) const
@brief 检查注册所需的非零身份、名称、MTU、地址冲突和容量，供启动及运行期发布共用。
@return 合法返回零，字段无效 -EINVAL，ID、名称或地址冲突 -EEXIST，满表 -ENOBUFS。
@pre 调用方已持配置锁，接口对象及名称稳定存活。
@details 实现通过 RegistrationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-19629c883c7bf34c"></a>

## `xv::net::InterfaceRegistry::add`

```doxygen
@fn bool xv::net::InterfaceRegistry::add(NetworkInterface &interface)
@brief 在配置锁内校验并借用注册启动接口；封存或身份、地址、容量校验失败返回 false。
@details 本操作不推进接口代次，不解除启动封存；对象及名称须长期存活。 实现通过 UpdateFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-10d3ef393b2b8755"></a>

## `xv::net::InterfaceRegistry::publish`

```doxygen
@fn int xv::net::InterfaceRegistry::publish(NetworkInterface &interface)
@brief 在配置锁内校验并发布运行期接口，成功时封存该接口的启动配置并推进代次。
@details 校验失败不改变注册表、接口代次或封存状态。接口由调用方长期持有，注册表只借用指针。 实现通过 UpdateFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 成功零，字段、冲突或容量错误为负 errno。
@pre 调用方以外层配置锁覆盖具体接口的 active 状态及本次发布。
```

<a id="fn-8fd44ee2a0436af7"></a>

## `xv::net::InterfaceRegistry::withdraw`

```doxygen
@fn bool xv::net::InterfaceRegistry::withdraw(NetworkInterface &interface)
@brief 在配置锁内按对象身份撤销接口注册，压紧数组并保持其他接口次序。
@details 成功推进被撤销接口的代次，保留其启动封存状态；不销毁对象，不清理协议、地址或路由。 实现通过 UpdateFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用方已确认接口无地址、DHCP、路由及成员引用，并保证所有存活的借用指针仍指向有效对象。
@return 找到并撤销返回 true，未注册返回 false 且不改变状态。
```

<a id="fn-6dc874d592d4900e"></a>

## `xv::net::InterfaceRegistry::seal`

```doxygen
@fn bool xv::net::InterfaceRegistry::seal()
@brief 启动结束时在配置锁内双向核对地址拥有冲突，封存启动注册及配置 API；运行期桥发布、地址和状态交管理器协调。
@details 实现通过 SealFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-bfd9e34fed60845d"></a>

## `xv::net::InterfaceRegistry::find`

```doxygen
@fn NetworkInterface *xv::net::InterfaceRegistry::find(unsigned id) const
@brief 在配置锁内按稳定 ID 查找当前已发布接口，支持桥运行期加入与注销；返回长期存活对象或 nullptr。
```

<a id="fn-9875c43aa204f3b9"></a>

## `xv::net::InterfaceRegistry::owns`

```doxygen
@fn bool xv::net::InterfaceRegistry::owns(unsigned address) const
@brief 在一致配置快照内查询已发布且启用的接口地址拥有关系，支持桥发布/注销与多地址，不返回内部地址引用。
```

<a id="fn-11570218be60251c"></a>

## `xv::net::InterfaceRegistry::tick`

```doxygen
@fn void xv::net::InterfaceRegistry::tick(unsigned long now)
@brief 先在配置锁内复制最多八个稳定接口指针，再解锁调用各接口定时维护；桥注销不释放对象，避免回调期间悬空或反向持锁。
```

<a id="fn-a1a1eb1c46a2f883"></a>

## `xv::net::NetworkInterface::statistics`

```doxygen
@fn InterfaceStatistics xv::net::NetworkInterface::statistics() const
@brief 分别原子读取六个纯 IP 接口计数。
@details 字段无数据竞争，但不保证所有计数来自同一时刻，不等同于硬件帧统计。
```

<a id="fn-72713f8a1c6e3db4"></a>

## `xv::net::NetworkInterface::receive_drop`

```doxygen
@fn void xv::net::NetworkInterface::receive_drop()
@brief 原子增加接口或 IP 基础验证拒绝计数；不包含所有驱动和传输层丢包。
```

<a id="fn-02e2a99c0121b11a"></a>

## `xv::net::NetworkInterface::send`

```doxygen
@fn int xv::net::NetworkInterface::send(Packet &packet, unsigned next_hop)
@brief 通过具体接口发送普通 IP 报文并统一累计纯 IP 发送统计。
@details 成功代表出口已接管或排队，负返回计作错误；按发送前长度计字节，页面所有权由具体出口契约决定。
```

<a id="fn-4486a19c308f9201"></a>

## `xv::net::NetworkInterface::send_bootstrap`

```doxygen
@fn int xv::net::NetworkInterface::send_bootstrap(Packet &packet)
@brief 调用启动协议发送能力并按发送前纯 IP 长度累计统一统计。
@details 默认接口不支持；调用者只能通过受限的 IPv4 DHCP 入口使用，不能把无地址发送开放给普通 socket。
```

<a id="fn-c99a4c9b5b90a08c"></a>

## `xv::net::NetworkInterface::account_send`

```doxygen
@fn int xv::net::NetworkInterface::account_send(unsigned length, int result)
@brief 根据出口返回值原子累计发送成功、字节或错误。
@param length 发送前纯 IP 长度。
@param result 出口返回值。
@return 原样返回 result，不修改页面所有权。
```
