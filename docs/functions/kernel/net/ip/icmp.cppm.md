# `kernel/net/ip/icmp.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IcmpProtocol::IcmpProtocol`](icmp.cppm.md#fn-1a829fe9267b19a5)
- [`xv::net::IcmpProtocol::receive`](icmp.cpp.md#fn-e81d21f708c3ced8)
- [`xv::net::IcmpProtocol::unreachable`](icmp.cpp.md#fn-a19fb566f8d31417)

<a id="fn-1a829fe9267b19a5"></a>

## `xv::net::IcmpProtocol::IcmpProtocol`

```doxygen
@fn constexpr explicit xv::net::IcmpProtocol::IcmpProtocol(Ipv4Layer &ip)
@brief 构造 IcmpProtocol 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```
