# `kernel/net/transport/tcp_protocol.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpProtocol::receive`](tcp_protocol.cpp.md#fn-b649250825e39e1b)
- [`xv::net::TcpProtocol::transmit`](tcp_protocol.cpp.md#fn-08feda01927d4d5e)

<a id="fn-b649250825e39e1b"></a>

## `xv::net::TcpProtocol::receive`

```doxygen
@fn void xv::net::TcpProtocol::receive(Packet &packet, const IpDatagram &datagram)
@brief 校验 TCP 首部、保留位、非零端口和伪首部校验和，将借用 TCP 段交给接收器。
@details 运行接收模板图，具名动作和条件位于 tcp_protocol/receive.cppm，连接关系由类的私有流程分区声明。不扫描注册表或访问连接私有字段。无接收器或接收器未匹配时，除收到 RST 外按 ACK、SYN、FIN 和载荷长度构造复位响应；未匹配 RST 的发送位于接收器锁作用域外。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-08feda01927d4d5e"></a>

## `xv::net::TcpProtocol::transmit`

```doxygen
@fn int xv::net::TcpProtocol::transmit(Endpoint local, Endpoint peer, unsigned sequence, unsigned acknowledgement, unsigned flags, unsigned window, const void *data, unsigned length)
@brief 通过本实例 IPv4 服务编码并发送 TCP 段，包含首部、可选 MSS 及伪首部校验和。
@details 运行发送模板图，具名动作和条件位于 tcp_protocol/transmit.cppm，业务 errno 从 ctx.result 返回，Packet 由上下文统一持有和析构。同步借用并复制载荷；IPv4 分配与发送错误原样返回。协议对象不查找全局注册表或 TCP 单例。 实现通过 TransmitFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 发送成功零或 IPv4、容量错误的负 errno。
```

节点关系、错误出口、分区与验证见 [声明式协议流程](../../../../network-tcp-protocol-flow.md)。
