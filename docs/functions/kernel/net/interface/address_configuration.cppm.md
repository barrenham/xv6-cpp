# `kernel/net/interface/address_configuration.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::AddressConfiguration::AddressConfiguration`](address_configuration.cppm.md#fn-669a7a64845e5939)
- [`xv::net::AddressConfiguration::add_static`](address_configuration.cpp.md#fn-61f8d692dbb019bd)
- [`xv::net::AddressConfiguration::install_lease`](address_configuration.cpp.md#fn-ec6332fe77a9c444)
- [`xv::net::AddressConfiguration::remove_lease`](address_configuration.cpp.md#fn-c8c67bede08a808c)
- [`xv::net::AddressConfiguration::remove_static`](address_configuration.cpp.md#fn-a4bbc7a46ce7f664)

<a id="fn-669a7a64845e5939"></a>

## `xv::net::AddressConfiguration::AddressConfiguration`

```doxygen
@fn constexpr xv::net::AddressConfiguration::AddressConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback)
@brief 借用接口注册表、路由表与回环出口，构造手工和租约地址共用的事务服务。
@details 服务只保存依赖引用，不分配、不发送报文；依赖对象须比服务存活更久。
```
