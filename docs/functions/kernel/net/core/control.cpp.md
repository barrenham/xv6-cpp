# `kernel/net/core/control.cpp`

## 函数导航

- [`xv::net::NetworkManager::execute`](control.cpp.md#fn-77ff706bfd0958c0)
- [`xv::net::NetworkManager::inspect`](control.cpp.md#fn-2482ef9e05a0b870)
- [`xv::net::NetworkManager::mutate`](control.cpp.md#fn-4bb10e3a91b412f9)
- [`xv::net::NetworkManager::remove_route`](control.cpp.md#fn-6f6a373445d7f0ae)

<a id="fn-6f6a373445d7f0ae"></a>

## `xv::net::NetworkManager::remove_route`

```doxygen
@fn void xv::net::NetworkManager::remove_route(unsigned index)
@brief 在配置锁内删除指定有效路由项并压紧表，清零尾部；调用者保证 index 小于当前路由数。
```

<a id="fn-2482ef9e05a0b870"></a>

## `xv::net::NetworkManager::inspect`

```doxygen
@fn int xv::net::NetworkManager::inspect(unsigned operation, netctl_request &request)
@brief 在配置锁内生成清零的单条接口、地址或路由快照。
@details 地址和路由额外报告 NET_CONFIG_DHCP 归属标记，原有桥/master 字段不变；查询结束返回零，不暴露内核指针。
```

<a id="fn-4bb10e3a91b412f9"></a>

## `xv::net::NetworkManager::mutate`

```doxygen
@fn int xv::net::NetworkManager::mutate(unsigned operation, const netctl_request &request, NetworkInterface &interface)
@brief 执行接口、手工地址及路由的配置事务，并保护 DHCP 所有的配置。
@details DHCP 地址和路由不可用普通删除操作移除，应停止客户端；手工别名仍可共存，桥成员不能新增地址。容量和自动路由更新在配置锁下完成。
@pre 调用者已持配置锁。
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
