# `kernel/net/core/control.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::NetworkManager::NetworkManager`](control.cppm.md#fn-3e5feadf3cb71a73)
- [`xv::net::NetworkManager::execute`](control.cpp.md#fn-77ff706bfd0958c0)
- [`xv::net::NetworkManager::inspect`](control.cpp.md#fn-2482ef9e05a0b870)
- [`xv::net::NetworkManager::mutate`](control.cpp.md#fn-4bb10e3a91b412f9)

<a id="fn-3e5feadf3cb71a73"></a>

## `xv::net::NetworkManager::NetworkManager`

```doxygen
@fn constexpr xv::net::NetworkManager::NetworkManager(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback, BridgeManager *bridges = nullptr, DhcpManager *dhcp = nullptr)
@brief 借用网络服务并组合地址事务服务，供管理请求分派、快照转换及状态协调使用。
@details AddressConfiguration 与其他成员只保存长期依赖引用，构造不执行网络 I/O。
```
