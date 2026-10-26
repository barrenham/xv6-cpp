# `kernel/net/ip/icmp.cpp`

## 函数导航

- [`xv::net::IcmpProtocol::receive`](icmp.cpp.md#fn-e81d21f708c3ced8)
- [`xv::net::IcmpProtocol::unreachable`](icmp.cpp.md#fn-a19fb566f8d31417)

<a id="fn-e81d21f708c3ced8"></a>

## `xv::net::IcmpProtocol::receive`

```doxygen
@fn void xv::net::IcmpProtocol::receive(Packet &packet, const IpDatagram &d)
@brief 处理已验证本机目的的 Echo 请求，在纯 IP 页面构造回复，并以请求命中的本地地址作为回复源。
```

<a id="fn-a19fb566f8d31417"></a>

## `xv::net::IcmpProtocol::unreachable`

```doxygen
@fn void xv::net::IcmpProtocol::unreachable(Packet &packet, const IpDatagram &d)
@brief 对发往任一本机地址的无监听 UDP 报生成端口不可达，引用原始纯 IP 首部及最多八字节负载。
```
