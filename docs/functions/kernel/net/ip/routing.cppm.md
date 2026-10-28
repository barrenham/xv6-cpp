# `kernel/net/ip/routing.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::RoutingTable::RoutingTable`](routing.cppm.md#fn-77c6a2a9198776a1)
- [`xv::net::RoutingTable::add`](routing.cpp.md#fn-9936922a5ae03a29)
- [`xv::net::RoutingTable::seal`](routing.cppm.md#fn-591a3db6e9fe9bc5)
- [`xv::net::RoutingTable::lookup`](routing.cpp.md#fn-8144db493f085cfd)
- [`xv::net::RoutingTable::at`](routing.cpp.md#fn-b8409f8dc1870a38)
- [`xv::net::RoutingTable::available`](routing.cppm.md#fn-bd6eac3a212c4775)
- [`xv::net::RoutingTable::references`](routing.cpp.md#fn-bb34c84c11c1b132)
- [`xv::net::RoutingTable::add_static`](routing.cpp.md#fn-dbbb529d2a5000b6)
- [`xv::net::RoutingTable::remove_static`](routing.cpp.md#fn-36c109e5285e1dc2)
- [`xv::net::RoutingTable::add_address_routes`](routing.cpp.md#fn-d2237136a2190524)
- [`xv::net::RoutingTable::remove_address_routes`](routing.cpp.md#fn-baf01748347dc4e9)
- [`xv::net::RoutingTable::lease_routes_match`](routing.cpp.md#fn-6c00bff74534bbcb)
- [`xv::net::RoutingTable::replace_lease_routes`](routing.cpp.md#fn-eb063be1375b395c)
- [`xv::net::RoutingTable::remove_lease_routes`](routing.cpp.md#fn-7ce648c7ca6190da)
- [`xv::net::RoutingTable::address_routes`](routing.cpp.md#fn-fc66c407a7ebe8aa)
- [`xv::net::RoutingTable::valid`](routing.cpp.md#fn-cadd66a2b3d00d62)
- [`xv::net::RoutingTable::same_path`](routing.cpp.md#fn-aebc669c97dde387)
- [`xv::net::RoutingTable::find`](routing.cpp.md#fn-51f31c949311b75a)
- [`xv::net::RoutingTable::erase`](routing.cpp.md#fn-3fc57a9eef8fb259)
- [`xv::net::RoutingTable::lease_route_count`](routing.cpp.md#fn-81139327de9a7d47)

<a id="fn-77c6a2a9198776a1"></a>

## `xv::net::RoutingTable::RoutingTable`

```doxygen
@fn constexpr explicit xv::net::RoutingTable::RoutingTable(const InterfaceRegistry &interfaces)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-591a3db6e9fe9bc5"></a>

## `xv::net::RoutingTable::seal`

```doxygen
@fn void xv::net::RoutingTable::seal()
@brief 在配置锁内封存启动添加入口；运行期操作由路由表自身的具名方法完成，不取消封存。
```

<a id="fn-bd6eac3a212c4775"></a>

## `xv::net::RoutingTable::available`

```doxygen
@fn unsigned xv::net::RoutingTable::available() const
@brief 在配置锁内返回剩余路由槽位数。
@note 跨调用的容量检查与提交须由外层 ConfigurationGuard 覆盖，单独查询不预留槽位。
```
