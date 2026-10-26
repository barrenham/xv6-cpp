# `kernel/net/dhcp/client.cppm`

## 函数导航

- [`xv::net::DhcpManager::DhcpManager`](client.cppm.md#fn-956037c41b834553)
- [`xv::net::DhcpManager::execute`](client.cpp.md#fn-d4e4da766bebabfb)
- [`xv::net::DhcpManager::find`](client.cpp.md#fn-8786da367c47a7df)
- [`xv::net::DhcpManager::initialize`](client.cpp.md#fn-19acae0e87259110)
- [`xv::net::DhcpManager::receive_bootstrap`](client.cpp.md#fn-46570f71ac652ac6)
- [`xv::net::DhcpManager::select`](client.cpp.md#fn-5497dbe215ae9a52)
- [`xv::net::DhcpManager::tick`](client.cpp.md#fn-40a1e0ee6745de59)
- [`xv::net::DhcpManager::transmit`](client.cpp.md#fn-f736e585f22f8be7)

<a id="fn-956037c41b834553"></a>

## `xv::net::DhcpManager::DhcpManager`

```doxygen
@fn xv::net::DhcpManager::DhcpManager(InterfaceRegistry &interfaces,Ipv4Layer &ip,LeaseConfiguration &configuration)
@brief 借用注册表、IPv4 层和租约配置服务，初始化八个空客户端槽。
@details 不执行 I/O，不要求接口已获得地址；所有依赖和接口对象须长期存活。
```
