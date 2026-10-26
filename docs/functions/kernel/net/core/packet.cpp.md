# `kernel/net/core/packet.cpp`

## 函数导航

- [`xv::net::Packet::allocate`](packet.cppm.md#fn-5633cc7891c8536f)
- [`xv::net::Packet::clone`](packet.cpp.md#fn-b249acc31eb1ae38)
- [`xv::net::Packet::~Packet`](packet.cppm.md#fn-38f798f54b47cbb2)

<a id="fn-b249acc31eb1ae38"></a>

## `xv::net::Packet::clone`

```doxygen
@fn Packet xv::net::Packet::clone() const
@brief 为有效数据分配独立页面并复制字节及长度，无效源页或分配失败返回空 Packet；不转移或释放原页，供二层多出口转发使用。
@return 拥有独立页面的副本，失败时 bool 转换为 false。
@note 原始 Packet 的页面和生命周期不变，输出副本由调用者管理。
```
