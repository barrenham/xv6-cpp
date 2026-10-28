# `kernel/net/core/packet.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Packet::~Packet`](packet.cpp.md#fn-38f798f54b47cbb2)
- [`xv::net::Packet::allocate`](packet.cpp.md#fn-5633cc7891c8536f)
- [`xv::net::Packet::clone`](packet.cpp.md#fn-b249acc31eb1ae38)

<a id="fn-38f798f54b47cbb2"></a>

## `xv::net::Packet::~Packet`

```doxygen
@fn xv::net::Packet::~Packet()
@brief 在仍持有页面时归还物理页；已移动或 release 的对象不再释放页面。
```

<a id="fn-5633cc7891c8536f"></a>

## `xv::net::Packet::allocate`

```doxygen
@fn Packet xv::net::Packet::allocate(unsigned length)
@brief 仅为不超过 Capacity 的长度申请一个物理页，失败或超限返回无拥有权报文。
@note 不初始化载荷字节；有效性以拥有权 bool 检查，不以 size() 判断。
```

<a id="fn-b249acc31eb1ae38"></a>

## `xv::net::Packet::clone`

```doxygen
@fn Packet xv::net::Packet::clone() const
@brief 分配独立页面并复制有效字节和完整上下文，不改变原报文拥有权。
@return 成功返回独立报文；原报文为空、长度超限或分配失败返回空。
@details 复制上下文保留剩余隧道预算，修改克隆载荷不会影响原页面。
```
