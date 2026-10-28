# `kernel/net/link/arp.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::ArpCache::ArpCache`](arp.cppm.md#fn-1deae4753a85af91)
- [`xv::net::ArpCache::initialize`](arp.cpp.md#fn-9716d9e48bbba0d6)
- [`xv::net::ArpCache::send`](arp.cpp.md#fn-ef8526d27ee87536)
- [`xv::net::ArpCache::receive`](arp.cpp.md#fn-704e5894f62445e0)
- [`xv::net::ArpCache::tick`](arp.cpp.md#fn-ab7954406875ed6d)
- [`xv::net::ArpCache::neighbor`](arp.cpp.md#fn-ee8e9de0c540ad2b)
- [`xv::net::ArpCache::clear`](arp.cpp.md#fn-f92a3d89216fa13d)
- [`xv::net::ArpCache::request`](arp.cpp.md#fn-d772ee0820984b06)

<a id="fn-1deae4753a85af91"></a>

## `xv::net::ArpCache::ArpCache`

```doxygen
@fn constexpr xv::net::ArpCache::ArpCache(EthernetLayer &link, const NetworkInterface &interface)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```
