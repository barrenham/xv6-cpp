# `kernel/net/interface/loopback.cpp`

## 函数导航

- [`xv::net::LoopbackInterface::initialize`](loopback.cpp.md#fn-21e76ce52e463582)
- [`xv::net::LoopbackInterface::owns`](loopback.cpp.md#fn-19bc88c16594dcb2)
- [`xv::net::LoopbackInterface::tick`](loopback.cpp.md#fn-49f2bb3530d94f42)
- [`xv::net::LoopbackInterface::transmit`](loopback.cpp.md#fn-bb6135ec4e85a90a)

<a id="fn-21e76ce52e463582"></a>

## `xv::net::LoopbackInterface::initialize`

```doxygen
@fn void xv::net::LoopbackInterface::initialize()
@brief 初始化回环队列自旋锁；在任何并发收发前执行。
```

<a id="fn-bb6135ec4e85a90a"></a>

## `xv::net::LoopbackInterface::transmit`

```doxygen
@fn int xv::net::LoopbackInterface::transmit(Packet &packet, unsigned)
@brief 把纯 IP 页面移交十六项有界队列；检查管理状态和 MTU，队满返回 -ENOBUFS。
```

<a id="fn-49f2bb3530d94f42"></a>

## `xv::net::LoopbackInterface::tick`

```doxygen
@fn void xv::net::LoopbackInterface::tick(unsigned long)
@brief 每轮最多出队六十四报，取出后释放队列锁再调用 IP 接收器，避免协议递归持锁。
```
<a id="fn-19bc88c16594dcb2"></a>

## `xv::net::LoopbackInterface::owns`

```doxygen
@fn bool xv::net::LoopbackInterface::owns(unsigned value) const
@brief 在配置锁内按本接口地址前缀查询拥有关系，默认覆盖整个 127/8；不会返回可悬空的地址引用。
```
