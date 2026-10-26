# `kernel/net/core/control.cppm`

## 函数导航

- [`xv::net::NetworkManager::NetworkManager`](control.cppm.md#fn-3e5feadf3cb71a73)
- [`xv::net::NetworkManager::execute`](control.cpp.md#fn-77ff706bfd0958c0)
- [`xv::net::NetworkManager::inspect`](control.cpp.md#fn-2482ef9e05a0b870)
- [`xv::net::NetworkManager::mutate`](control.cpp.md#fn-4bb10e3a91b412f9)
- [`xv::net::NetworkManager::remove_route`](control.cpp.md#fn-6f6a373445d7f0ae)

<a id="fn-3e5feadf3cb71a73"></a>

## `xv::net::NetworkManager::NetworkManager`

```doxygen
@fn xv::net::NetworkManager::NetworkManager(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback, BridgeManager *bridges = nullptr, DhcpManager *dhcp = nullptr)
@brief 借用接口、路由、回环及可选桥和 DHCP 管理器。
@details 依赖必须比管理器存活更久；未提供对应管理器时，其专用操作返回不支持。构造不执行 I/O。
```
