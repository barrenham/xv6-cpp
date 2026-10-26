# `kernel/net/interface/interface.cpp`

## 函数导航

- [`xv::net::InterfaceRegistry::add`](interface.cpp.md#fn-19629c883c7bf34c)
- [`xv::net::InterfaceRegistry::find`](interface.cpp.md#fn-bfd9e34fed60845d)
- [`xv::net::InterfaceRegistry::owns`](interface.cpp.md#fn-9875c43aa204f3b9)
- [`xv::net::InterfaceRegistry::seal`](interface.cpp.md#fn-6dc874d592d4900e)
- [`xv::net::InterfaceRegistry::tick`](interface.cpp.md#fn-11570218be60251c)
- [`xv::net::NetworkInterface::accepts_destination`](interface.cpp.md#fn-afdbdf714f5adc2b)
- [`xv::net::NetworkInterface::account_send`](interface.cpp.md#fn-c99a4c9b5b90a08c)
- [`xv::net::NetworkInterface::add_address`](interface.cpp.md#fn-29e4fdf0db63d8be)
- [`xv::net::NetworkInterface::broadcast`](interface.cpp.md#fn-aa7c7468ae954fd0)
- [`xv::net::NetworkInterface::deliver`](interface.cpp.md#fn-9b5c9773ed945053)
- [`xv::net::NetworkInterface::owns`](interface.cpp.md#fn-24e03455c304d9fd)
- [`xv::net::NetworkInterface::receive_drop`](interface.cpp.md#fn-72713f8a1c6e3db4)
- [`xv::net::NetworkInterface::send`](interface.cpp.md#fn-02e2a99c0121b11a)
- [`xv::net::NetworkInterface::send_bootstrap`](interface.cpp.md#fn-4486a19c308f9201)
- [`xv::net::NetworkInterface::set_up`](interface.cpp.md#fn-0dd1df949d25550a)
- [`xv::net::NetworkInterface::source_for`](interface.cpp.md#fn-1c833c16e2ec25f4)
- [`xv::net::NetworkInterface::statistics`](interface.cpp.md#fn-a1a1eb1c46a2f883)

<a id="fn-afdbdf714f5adc2b"></a>

## `xv::net::NetworkInterface::accepts_destination`

```doxygen
@fn bool xv::net::NetworkInterface::accepts_destination(unsigned address, const InterfaceRegistry &) const
@brief 按入站接口接收本机地址或该接口广播地址，避免普通以太接口跨接口接收。
```

<a id="fn-24e03455c304d9fd"></a>

## `xv::net::NetworkInterface::owns`

```doxygen
@fn bool xv::net::NetworkInterface::owns(unsigned value) const
@brief 在配置临界区内查询地址是否精确属于本接口；同一调用链嵌套查询复用外层锁，回环子类采用前缀拥有语义。
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

<a id="fn-9b5c9773ed945053"></a>

## `xv::net::NetworkInterface::deliver`

```doxygen
@fn void xv::net::NetworkInterface::deliver(Packet &packet)
@brief 在已启用且已挂接接收器时累计纯 IP 入站计数，再在配置锁外以本接口身份回调；禁止投递则累计丢弃。
```

<a id="fn-19629c883c7bf34c"></a>

## `xv::net::InterfaceRegistry::add`

```doxygen
@fn bool xv::net::InterfaceRegistry::add(NetworkInterface &interface)
@brief 启动期借用并注册接口，最多八个；检查非零 ID、名称唯一、MTU 68..4082 和已有地址冲突。
@details 对象须长期存活。
```

<a id="fn-6dc874d592d4900e"></a>

## `xv::net::InterfaceRegistry::seal`

```doxygen
@fn bool xv::net::InterfaceRegistry::seal()
@brief 启动结束时在配置锁内双向核对地址拥有冲突，封存启动注册及配置 API；运行期桥发布、地址和状态交管理器协调。
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
