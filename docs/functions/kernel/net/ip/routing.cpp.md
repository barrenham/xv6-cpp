# `kernel/net/ip/routing.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::RoutingTable::add`](routing.cpp.md#fn-9936922a5ae03a29)
- [`xv::net::RoutingTable::add_address_routes`](routing.cpp.md#fn-d2237136a2190524)
- [`xv::net::RoutingTable::add_static`](routing.cpp.md#fn-dbbb529d2a5000b6)
- [`xv::net::RoutingTable::address_routes`](routing.cpp.md#fn-fc66c407a7ebe8aa)
- [`xv::net::RoutingTable::at`](routing.cpp.md#fn-b8409f8dc1870a38)
- [`xv::net::RoutingTable::erase`](routing.cpp.md#fn-3fc57a9eef8fb259)
- [`xv::net::RoutingTable::find`](routing.cpp.md#fn-51f31c949311b75a)
- [`xv::net::RoutingTable::lease_route_count`](routing.cpp.md#fn-81139327de9a7d47)
- [`xv::net::RoutingTable::lease_routes_match`](routing.cpp.md#fn-6c00bff74534bbcb)
- [`xv::net::RoutingTable::lookup`](routing.cpp.md#fn-8144db493f085cfd)
- [`xv::net::RoutingTable::references`](routing.cpp.md#fn-bb34c84c11c1b132)
- [`xv::net::RoutingTable::remove_address_routes`](routing.cpp.md#fn-baf01748347dc4e9)
- [`xv::net::RoutingTable::remove_lease_routes`](routing.cpp.md#fn-7ce648c7ca6190da)
- [`xv::net::RoutingTable::remove_static`](routing.cpp.md#fn-36c109e5285e1dc2)
- [`xv::net::RoutingTable::replace_lease_routes`](routing.cpp.md#fn-eb063be1375b395c)
- [`xv::net::RoutingTable::same_path`](routing.cpp.md#fn-aebc669c97dde387)
- [`xv::net::RoutingTable::valid`](routing.cpp.md#fn-cadd66a2b3d00d62)

<a id="fn-cadd66a2b3d00d62"></a>

## `xv::net::RoutingTable::valid`

```doxygen
@fn bool xv::net::RoutingTable::valid(const Route &route) const
@brief 检查待添加路由的前缀、出口注册身份、桥成员状态、首选源和网关有效性。
@pre route.interface 非空，调用方已持配置锁。
@note 不修改表，也不检查容量或启动封存状态，供启动添加和运行期静态添加共用。
@details 实现通过 ValidationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-9936922a5ae03a29"></a>

## `xv::net::RoutingTable::add`

```doxygen
@fn bool xv::net::RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface, unsigned gateway, unsigned metric, unsigned source, unsigned owner, unsigned local_address)
@brief 在配置锁内校验并追加启动路由；封存、容量或字段校验失败均返回 false。
@details 前缀归一化后存入私有数组，保留地址归属和本机地址标记。运行期管理使用 add_static 等具名操作，不解除启动封存。
```

<a id="fn-b8409f8dc1870a38"></a>

## `xv::net::RoutingTable::at`

```doxygen
@fn bool xv::net::RoutingTable::at(unsigned index, Route &result) const
@brief 在配置锁内复制指定位置的路由值快照，越界返回 false 且不修改结果。
@note 快照不暴露数组引用；接口指针为借用对象，跨接口生命周期使用时需遵守外层配置锁约定。
```

<a id="fn-bb34c84c11c1b132"></a>

## `xv::net::RoutingTable::references`

```doxygen
@fn bool xv::net::RoutingTable::references(const NetworkInterface &interface) const
@brief 在配置锁内判断是否有路由以指定接口为出口或地址归属者，供网桥判断接口占用。
```

<a id="fn-aebc669c97dde387"></a>

## `xv::net::RoutingTable::same_path`

```doxygen
@fn bool xv::net::RoutingTable::same_path(const Route &left, const Route &right)
@brief 比较网络、前缀、出口、网关、metric 和首选源，不把归属标记计入路径身份。
@pre 网络地址已经按前缀归一化。
```

<a id="fn-51f31c949311b75a"></a>

## `xv::net::RoutingTable::find`

```doxygen
@fn int xv::net::RoutingTable::find(const Route &route) const
@brief 按路径身份查找首个相同路由，未找到返回负值。
@pre 调用方已持配置锁，网络已按前缀归一化。
```

<a id="fn-3fc57a9eef8fb259"></a>

## `xv::net::RoutingTable::erase`

```doxygen
@fn void xv::net::RoutingTable::erase(unsigned index)
@brief 删除一个有效槽位并压紧路由数组、清零尾部，保持其他路由次序。
@pre 调用方已持配置锁且 index 小于 count_。
```

<a id="fn-dbbb529d2a5000b6"></a>

## `xv::net::RoutingTable::add_static`

```doxygen
@fn int xv::net::RoutingTable::add_static(unsigned network, unsigned prefix, NetworkInterface &interface, unsigned gateway, unsigned metric, unsigned source)
@brief 在配置锁内添加运行期手工路由，不改变启动封存状态。
@details 归一化网络后依次检查重复路径、容量和路由有效性；失败保持原表。 实现通过 MutationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 成功零，前缀或路由无效 -EINVAL，重复 -EEXIST，满表 -ENOBUFS。
```

<a id="fn-36c109e5285e1dc2"></a>

## `xv::net::RoutingTable::remove_static`

```doxygen
@fn int xv::net::RoutingTable::remove_static(unsigned network, unsigned prefix, NetworkInterface &interface, unsigned gateway, unsigned metric, unsigned source)
@brief 在配置锁内删除匹配的手工路由，保护地址自动路由及租约路由。
@return 成功零，前缀无效 -EINVAL，未找到 -ENOENT，托管路由 -EBUSY。
@details 实现通过 MutationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-fc66c407a7ebe8aa"></a>

## `xv::net::RoutingTable::address_routes`

```doxygen
@fn RoutingTable::RouteSet xv::net::RoutingTable::address_routes(NetworkInterface &interface, NetworkInterface &loopback, Ipv4Address address, unsigned lease_owner, unsigned gateway, unsigned metric)
@brief 构造本机 /32 回环路由、接口直连路由及可选默认路由的有界值集合。
@details 自动路由带地址归属，本机路由和默认路由使用该地址为首选源；lease_owner 非零时整组标记为租约所有。
@pre 地址前缀已验证，接口对象在配置事务中存活。
```

<a id="fn-d2237136a2190524"></a>

## `xv::net::RoutingTable::add_address_routes`

```doxygen
@fn bool xv::net::RoutingTable::add_address_routes(NetworkInterface &interface, NetworkInterface &loopback, Ipv4Address address)
@brief 在配置锁内预检两条槽位后追加本机与直连自动路由，失败不修改路由表。
@pre 调用方已验证接口与地址，并以外层 ConfigurationGuard 覆盖地址提交及本次路由更新。
@return 前缀超限或容量不足返回 false，成功返回 true。
```

<a id="fn-baf01748347dc4e9"></a>

## `xv::net::RoutingTable::remove_address_routes`

```doxygen
@fn void xv::net::RoutingTable::remove_address_routes(const NetworkInterface &interface, unsigned address)
@brief 在配置锁内删除指定接口地址的自动路由及显式以该地址为首选源的路由。
@pre 调用方以外层 ConfigurationGuard 覆盖地址移除及本次路由更新。
```

<a id="fn-81139327de9a7d47"></a>

## `xv::net::RoutingTable::lease_route_count`

```doxygen
@fn unsigned xv::net::RoutingTable::lease_route_count(unsigned owner) const
@brief 统计指定租约归属的路由数量，用于容量预检及相同配置判断。
@pre owner 为非零的接口 ID，调用方已持配置锁。
```

<a id="fn-6c00bff74534bbcb"></a>

## `xv::net::RoutingTable::lease_routes_match`

```doxygen
@fn bool xv::net::RoutingTable::lease_routes_match(NetworkInterface &interface, NetworkInterface &loopback, Ipv4Address address, unsigned gateway, unsigned metric) const
@brief 在配置锁内比较本接口现有租约路由与期望的地址、前缀、网关和 metric。
@details 要求数量相同且每条期望路径存在；不改变路由次序或计数。 实现通过 LeaseFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 接口已注册且 ID 非零；跨地址和路由比较由外层 ConfigurationGuard 覆盖。
```

<a id="fn-eb063be1375b395c"></a>

## `xv::net::RoutingTable::replace_lease_routes`

```doxygen
@fn bool xv::net::RoutingTable::replace_lease_routes(NetworkInterface &interface, NetworkInterface &loopback, Ipv4Address address, unsigned gateway, unsigned metric)
@brief 在配置锁内预检可复用的旧租约容量，再整组替换本接口租约路由。
@details 预检失败不修改表；成功保留其他路由相对顺序并在尾部追加新路由。 实现通过 LeaseFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 接口已注册且 ID 非零，地址和网关已验证；调用方以外层 ConfigurationGuard 覆盖地址提交及本次替换。
@return 前缀超限或容量不足返回 false，否则返回 true。
```

<a id="fn-7ce648c7ca6190da"></a>

## `xv::net::RoutingTable::remove_lease_routes`

```doxygen
@fn bool xv::net::RoutingTable::remove_lease_routes(const NetworkInterface &interface)
@brief 在配置锁内压缩路由数组，只移除本接口租约所属的记录并清零尾部。
@details 保留手工路由，包括引用旧租约源地址的手工路由。
@pre 接口 ID 非零；联合地址撤销由外层 ConfigurationGuard 覆盖。
@return 是否确实移除了记录。
```

<a id="fn-8144db493f085cfd"></a>

## `xv::net::RoutingTable::lookup`

```doxygen
@fn bool xv::net::RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source, const NetworkInterface *via) const
@brief 在一致配置快照内排除桥成员、禁用/断链、无源及失效网关路由，再按最长前缀和较小 metric 选择；桥本机作为普通以太 IP 出口参与选路。
@details 实现通过 LookupFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
