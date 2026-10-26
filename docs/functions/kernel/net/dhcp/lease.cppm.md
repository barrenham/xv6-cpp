# `kernel/net/dhcp/lease.cppm`

## 函数导航

- [`xv::net::LeaseConfiguration::LeaseConfiguration`](lease.cppm.md#fn-9dce38b4e813e716)
- [`xv::net::LeaseConfiguration::install`](lease.cpp.md#fn-585913184002ca08)
- [`xv::net::LeaseConfiguration::remove`](lease.cpp.md#fn-02f977e5647590a0)

<a id="fn-9dce38b4e813e716"></a>

## `xv::net::LeaseConfiguration::LeaseConfiguration`

```doxygen
@fn xv::net::LeaseConfiguration::LeaseConfiguration(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback)
@brief 借用接口注册表、路由表和本机回环出口，构造租约事务服务。
@details 不创建对象、不执行网络 I/O；配置归属以稳定且唯一的接口 ID 标记。
```
