# `kernel/net/interface/interface.cpp`

## 函数导航

- [`xv::net::InterfaceRegistry::add`](interface.cpp.md#fn-19629c883c7bf34c)
- [`xv::net::InterfaceRegistry::find`](interface.cpp.md#fn-bfd9e34fed60845d)
- [`xv::net::InterfaceRegistry::owns`](interface.cpp.md#fn-9875c43aa204f3b9)
- [`xv::net::InterfaceRegistry::seal`](interface.cpp.md#fn-6dc874d592d4900e)
- [`xv::net::InterfaceRegistry::tick`](interface.cpp.md#fn-11570218be60251c)
- [`xv::net::NetworkInterface::accepts_destination`](interface.cpp.md#fn-afdbdf714f5adc2b)
- [`xv::net::NetworkInterface::add_address`](interface.cpp.md#fn-29e4fdf0db63d8be)
- [`xv::net::NetworkInterface::broadcast`](interface.cpp.md#fn-aa7c7468ae954fd0)
- [`xv::net::NetworkInterface::deliver`](interface.cpp.md#fn-9b5c9773ed945053)
- [`xv::net::NetworkInterface::owns`](interface.cpp.md#fn-24e03455c304d9fd)
- [`xv::net::NetworkInterface::set_up`](interface.cpp.md#fn-0dd1df949d25550a)
- [`xv::net::NetworkInterface::source_for`](interface.cpp.md#fn-1c833c16e2ec25f4)

<a id="fn-24e03455c304d9fd"></a>

## `xv::net::NetworkInterface::owns`

```doxygen
@fn bool xv::net::NetworkInterface::owns(unsigned value) const
@brief 判断地址是否精确属于本接口的 IPv4 地址列表。
@details 回环子类可覆盖为前缀拥有语义。
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
@brief 识别全一广播和接口前缀广播；/31、/32 不生成定向广播，无地址接口不接收广播。
```

<a id="fn-1c833c16e2ec25f4"></a>

## `xv::net::NetworkInterface::source_for`

```doxygen
@fn unsigned xv::net::NetworkInterface::source_for(unsigned destination) const
@brief 优先选择覆盖目标且前缀最长的本地地址，否则取首个地址；没有地址时返回零。
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
@brief 启动期设置管理状态；封存后返回 false，运行期不支持动态重配置。
```

<a id="fn-9b5c9773ed945053"></a>

## `xv::net::NetworkInterface::deliver`

```doxygen
@fn void xv::net::NetworkInterface::deliver(Packet &packet)
@brief 仅在接口已启用且挂接接收器时，把纯 IP 包连同本接口身份同步交给接收器。
@param[in,out] packet 起点为 IP 首部的独占页面。
@pre 页面内容已经去除链路层首部；不在接口队列锁内调用。
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
@brief 双向核对所有接口地址拥有关系，成功后冻结注册表及各接口地址和管理状态；冲突时返回 false。
@return 所有接口地址拥有关系不冲突时返回 true，否则不封存。
@pre 配置结束且尚未开始并发收发。
```

<a id="fn-bfd9e34fed60845d"></a>

## `xv::net::InterfaceRegistry::find`

```doxygen
@fn NetworkInterface *xv::net::InterfaceRegistry::find(unsigned id) const
@brief 按稳定 ID 查找接口，未注册返回 nullptr，不转移对象所有权。
```

<a id="fn-9875c43aa204f3b9"></a>

## `xv::net::InterfaceRegistry::owns`

```doxygen
@fn bool xv::net::InterfaceRegistry::owns(unsigned address) const
@brief 查询任一已启用接口是否拥有指定地址；供 bind、本地路由及无编号隧道接收使用。
```

<a id="fn-11570218be60251c"></a>

## `xv::net::InterfaceRegistry::tick`

```doxygen
@fn void xv::net::InterfaceRegistry::tick(unsigned long now)
@brief 把单调毫秒时刻转交每个已注册接口；接口自行完成队列或邻居维护。
```
