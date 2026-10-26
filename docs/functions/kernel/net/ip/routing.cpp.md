# `kernel/net/ip/routing.cpp`

## 函数导航

- [`xv::net::RoutingTable::add`](routing.cpp.md#fn-fcb92b0f65b7a82b)
- [`xv::net::RoutingTable::lookup`](routing.cpp.md#fn-8144db493f085cfd)

<a id="fn-fcb92b0f65b7a82b"></a>

## `xv::net::RoutingTable::add`

```doxygen
@fn bool xv::net::RoutingTable::add(unsigned network, unsigned prefix, NetworkInterface &interface, unsigned gateway, unsigned metric, unsigned source)
@brief 启动期增加最多三十二条静态路由，规范化前缀；出口必须已注册，网关须在出口链路且非自身/广播，首选源须属于已启用本机接口。
@param network 目标网络，以主机字节序表示；按 prefix 自动规范化。
@param prefix 前缀位数，范围 0..32。
@param interface 已注册且长期存活的出口对象。
@param gateway 零表示直连；非零必须是出口链路上的单播下一跳。
@param metric 同前缀长度下较小值优先。
@param source 首选本机源；零表示由出口选源，无编号接口需显式指定。
@return 成功保存返回 true，配置已封存、容量不足或校验失败返回 false。
@pre 仅在启动配置阶段调用，不与 lookup 并发修改。
```

<a id="fn-8144db493f085cfd"></a>

## `xv::net::RoutingTable::lookup`

```doxygen
@fn bool xv::net::RoutingTable::lookup(unsigned destination, RouteSelection &result, unsigned source, const NetworkInterface *via) const
@brief 先筛选管理状态、载波、源地址与强制出口，再按最长前缀和较小 metric 选路；输出出口、下一跳和源地址，无可用源或路由返回 false。
@param destination 主机字节序目标 IPv4。
@param[out] result 出口、下一跳和源地址；失败时清零。
@param source 绑定源约束，零表示自动选源。
@param via 可选强制出口，用于固定隧道承载接口。
@return 找到可用路由且具有源地址时返回 true。
@pre 路由及接口配置已封存，或调用者保证无并发修改。
```
