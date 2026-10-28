# `kernel/net/core/control.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::NetworkManager::inspect`](control.cpp.md#fn-2482ef9e05a0b870)
- [`xv::net::NetworkManager::mutate`](control.cpp.md#fn-4bb10e3a91b412f9)
- [`xv::net::NetworkManager::execute`](control.cpp.md#fn-77ff706bfd0958c0)

<a id="fn-2482ef9e05a0b870"></a>

## `xv::net::NetworkManager::inspect`

```doxygen
@fn int xv::net::NetworkManager::inspect(unsigned operation, netctl_request &request)
@brief 在配置锁内生成清零的单条接口、地址或路由管理快照。
@details 路由通过 RoutingTable::at 复制为值，再转换为无指针 ABI 字段及自动/DHCP 归属标记；查询结束返回零。
@pre 调用方已持配置锁。
```

<a id="fn-4bb10e3a91b412f9"></a>

## `xv::net::NetworkManager::mutate`

```doxygen
@fn int xv::net::NetworkManager::mutate(unsigned operation, const netctl_request &r, NetworkInterface &interface)
@brief 把管理请求分派为接口链路、地址事务或静态路由操作。
@details 链路请求先检查回环保护和 flags，再调用 configure_link；地址请求委托 AddressConfiguration，路由请求委托 RoutingTable。管理层不访问接口地址或路由存储。
@pre 调用方已持配置锁。
```

<a id="fn-77ff706bfd0958c0"></a>

## `xv::net::NetworkManager::execute`

```doxygen
@fn int xv::net::NetworkManager::execute(unsigned operation, netctl_request &request)
@brief 校验固定管理 ABI 后分别委托桥或 DHCP 操作，普通配置沿用短事务。
@details 专用管理器在配置锁外调用，避免发送、ARP 清理和配置锁反向依赖。
@param operation NETCTL_* 操作号。
@param[in,out] request 内核中的 128 字节请求或查询结果。
@return 查询一条为 1，枚举结束或修改成功为 0，失败负 errno。
```
