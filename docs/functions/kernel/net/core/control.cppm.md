# `kernel/net/core/control.cppm`

## 函数导航

- [`xv::net::NetworkManager::NetworkManager`](control.cppm.md#fn-e2eba91bdd66e7c2)
- [`xv::net::NetworkManager::execute`](control.cpp.md#fn-77ff706bfd0958c0)
- [`xv::net::NetworkManager::inspect`](control.cpp.md#fn-2482ef9e05a0b870)
- [`xv::net::NetworkManager::mutate`](control.cpp.md#fn-4bb10e3a91b412f9)
- [`xv::net::NetworkManager::remove_route`](control.cpp.md#fn-6f6a373445d7f0ae)

<a id="fn-e2eba91bdd66e7c2"></a>

## `xv::net::NetworkManager::NetworkManager`

```doxygen
@fn xv::net::NetworkManager::NetworkManager(InterfaceRegistry &interfaces, RoutingTable &routes, NetworkInterface &loopback, BridgeManager *bridges = nullptr)
@brief 借用接口、路由、回环及可选 BridgeManager；桥专用操作委托给该管理器，未提供时返回不支持，所有依赖须长期存活。
```
