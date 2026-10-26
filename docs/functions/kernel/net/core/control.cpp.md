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
@brief 在配置锁内生成接口/地址/路由的零填充快照；LINK 额外输出 bridge/slave 标记和 master ID，不暴露指针，枚举结束为零。
```

<a id="fn-4bb10e3a91b412f9"></a>

## `xv::net::NetworkManager::mutate`

```doxygen
@fn int xv::net::NetworkManager::mutate(unsigned operation, const netctl_request &r, NetworkInterface &interface)
@brief 在配置锁内执行地址、管理状态或路由事务；桥成员禁止修改 IPv4，地址与自动路由一并提交或回滚，删地址级联清理归属和首选源路由。
```

<a id="fn-77ff706bfd0958c0"></a>

## `xv::net::NetworkManager::execute`

```doxygen
@fn int xv::net::NetworkManager::execute(unsigned operation, netctl_request &request)
@brief 校验固定 ABI，先把桥专用操作在未持配置锁时委托 BridgeManager；普通管理沿用短配置事务及锁外邻居清理，查询不在网络锁内复制用户数据。
@param operation NETCTL_* 操作号，含桥专用操作。
@param[in,out] request 内核中的固定请求或零填充查询记录。
@return 查询 1/0，修改成功 0，失败负 errno。
@pre 不持配置或桥管理锁。
```
