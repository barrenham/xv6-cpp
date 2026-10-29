# `kernel/net/interface/loopback.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

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

<a id="fn-19bc88c16594dcb2"></a>

## `xv::net::LoopbackInterface::owns`

```doxygen
@fn bool xv::net::LoopbackInterface::owns(unsigned value) const
@brief 在配置锁内按本接口地址前缀查询拥有关系，默认覆盖整个 127/8；不会返回可悬空的地址引用。
```

<a id="fn-bb6135ec4e85a90a"></a>

## `xv::net::LoopbackInterface::transmit`

```doxygen
@fn int xv::net::LoopbackInterface::transmit(Packet &packet, unsigned)
@brief 在有界回环队列接管纯 IP 报文页面、长度及处理上下文。
@details 检查启用、MTU 与十六项队列容量，失败保留调用方拥有权；入队不直接回调协议。 实现通过 TransmitFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-49f2bb3530d94f42"></a>

## `xv::net::LoopbackInterface::tick`

```doxygen
@fn void xv::net::LoopbackInterface::tick(unsigned long)
@brief 在队列锁内摘除页面与元数据，锁外恢复完整 Packet 上下文并回调本机入口。
@details 每次最多处理六十四项，避免同步重入和丢失隧道预算。 实现通过 DrainFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
