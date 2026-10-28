# `kernel/net/core/address.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Ipv4Address::mask`](address.cppm.md#fn-30462e0981ae3632)
- [`xv::net::Ipv4Address::contains`](address.cppm.md#fn-a3948cdaef51ae79)

<a id="fn-30462e0981ae3632"></a>

## `xv::net::Ipv4Address::mask`

```doxygen
@fn unsigned xv::net::Ipv4Address::mask() const
@brief 把 0..32 位前缀转换为网络掩码；零及无效前缀返回零，避免非法移位。
```

<a id="fn-a3948cdaef51ae79"></a>

## `xv::net::Ipv4Address::contains`

```doxygen
@fn bool xv::net::Ipv4Address::contains(unsigned value) const
@brief 用前缀掩码判断地址是否落入本地址的子网；配置入口负责拒绝大于 32 的前缀。
```
