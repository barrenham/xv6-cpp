# `kernel/net/ip/routing.cpp`

## 函数导航

- [`xv::net::RoutingTable::add`](routing.cpp.md#fn-9936922a5ae03a29)
- [`xv::net::RoutingTable::lookup`](routing.cpp.md#fn-8144db493f085cfd)

<a id="fn-9936922a5ae03a29"></a>

## `xv::net::RoutingTable::add`

```doxygen
@fn bool xv::net::RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface, unsigned gateway, unsigned metric, unsigned source, unsigned owner, unsigned local_address)
@brief 在配置锁内验证并添加路由，拒绝以桥成员为出口。
@details 网关需位于出口链路，以太首选源须属于出口，自动路由携带地址归属，运行期由管理事务协调。
```

<a id="fn-8144db493f085cfd"></a>

## `xv::net::RoutingTable::lookup`

```doxygen
@fn bool xv::net::RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source, const NetworkInterface *via) const
@brief 在一致配置快照内排除桥成员、禁用/断链、无源及失效网关路由，再按最长前缀和较小 metric 选择；桥本机作为普通以太 IP 出口参与选路。
```
