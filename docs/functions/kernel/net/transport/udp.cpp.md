# `kernel/net/transport/udp.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::UdpProtocol::attach`](udp.cpp.md#fn-b5cbaf0f6126b7bd)
- [`xv::net::UdpProtocol::receive`](udp.cpp.md#fn-89d047edb11f0bae)
- [`xv::net::UdpProtocol::send`](udp.cpp.md#fn-178312cdd1239398)

<a id="fn-b5cbaf0f6126b7bd"></a>

## `xv::net::UdpProtocol::attach`

```doxygen
@fn void xv::net::UdpProtocol::attach(DatagramSink &sink)
@brief 在启动装配阶段设置唯一的 UDP 数据报接收端。
@param sink 借用的接收端，必须比协议对象存活更久；当前装配对象为 SocketRegistry。
@details 替换保存的接收端指针，不维护接收端列表、PID 端口队列或并行兼容路径。
@pre 启动期间调用，不与收包并发修改绑定。
```

<a id="fn-89d047edb11f0bae"></a>

## `xv::net::UdpProtocol::receive`

```doxygen
@fn void xv::net::UdpProtocol::receive(Packet &packet, const IpDatagram &d)
@brief 校验 UDP 数据报并交给唯一 SocketRegistry 接收端按地址和端口匹配。
@param packet 包含完整 IPv4 报文的独占页面。
@param d IPv4 层已验证的数据报描述，包含载荷范围、源地址和目的地址。
@details 检查最小头长度、UDP 长度、非零目的端口与可选伪首部校验和。匹配 socket 后接收端可转移页面到其有界队列；没有匹配接收者时走 ICMP 不可达路径。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@note 不维护独立于 socket 的 UDP 端口表。
```

<a id="fn-178312cdd1239398"></a>

## `xv::net::UdpProtocol::send`

```doxygen
@fn int xv::net::UdpProtocol::send(Endpoint source, Endpoint destination, const void *data, unsigned length)
@brief 先按路由确定源地址，再编码 UDP 及使用同一源的伪首部校验和；IP 层继续检查出口 MTU，避免跨接口校验和错误。
@details 实现通过 SendFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
