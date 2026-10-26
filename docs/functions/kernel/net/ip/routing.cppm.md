# `kernel/net/ip/routing.cppm`

## 函数导航

- [`xv::net::RoutingTable::RoutingTable`](routing.cppm.md#fn-77c6a2a9198776a1)
- [`xv::net::RoutingTable::add`](routing.cpp.md#fn-9936922a5ae03a29)
- [`xv::net::RoutingTable::lookup`](routing.cpp.md#fn-8144db493f085cfd)
- [`xv::net::RoutingTable::seal`](routing.cppm.md#fn-591a3db6e9fe9bc5)

<a id="fn-77c6a2a9198776a1"></a>

## `xv::net::RoutingTable::RoutingTable`

```doxygen
@fn xv::net::RoutingTable::RoutingTable(const InterfaceRegistry &interfaces)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-591a3db6e9fe9bc5"></a>

## `xv::net::RoutingTable::seal`

```doxygen
@fn void xv::net::RoutingTable::seal()
@brief 在配置锁内封存启动期添加入口；之后的动态修改由 NetworkManager 在同一锁下验证和提交。
```
