# `kernel/net/dhcp/lease.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::LeaseConfiguration::LeaseConfiguration`](lease.cppm.md#fn-9dce38b4e813e716)
- [`xv::net::LeaseConfiguration::install`](lease.cpp.md#fn-585913184002ca08)
- [`xv::net::LeaseConfiguration::remove`](lease.cpp.md#fn-02f977e5647590a0)

<a id="fn-9dce38b4e813e716"></a>

## `xv::net::LeaseConfiguration::LeaseConfiguration`

```doxygen
@fn constexpr xv::net::LeaseConfiguration::LeaseConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback)
@brief 借用网络配置依赖并组合通用地址事务服务，作为 DHCP 租约适配器。
@details 不持有地址数组或路由存储，不分配、不发送报文。
```
