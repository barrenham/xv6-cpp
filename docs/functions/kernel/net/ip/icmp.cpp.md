# `kernel/net/ip/icmp.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IcmpProtocol::receive`](icmp.cpp.md#fn-e81d21f708c3ced8)
- [`xv::net::IcmpProtocol::unreachable`](icmp.cpp.md#fn-a19fb566f8d31417)

- [`xv::net::IcmpProtocol::send_echo`](icmp.cpp.md#fn-c471b5b630c269e3)

<a id="fn-e81d21f708c3ced8"></a>

## `xv::net::IcmpProtocol::receive`

```doxygen
@fn void xv::net::IcmpProtocol::receive(Packet &packet, const IpDatagram &d)
@brief 校验并向 Echo socket 接收端交付本机 Echo Reply，其余交给原应答流程。
@details 只向 sink 交付 type 0/code 0、完整头部且校验和有效的报文；未被 socket 接收时仍由 Packet 回收。原 Echo 请求应答和不可达流程保留。
```

<a id="fn-a19fb566f8d31417"></a>

## `xv::net::IcmpProtocol::unreachable`

```doxygen
@fn void xv::net::IcmpProtocol::unreachable(Packet &packet, const IpDatagram &d)
@brief 对发往任一本机地址的无监听 UDP 报生成端口不可达，引用原始纯 IP 首部及最多八字节负载。
@details 实现通过 ReplyFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-c471b5b630c269e3"></a>

## `xv::net::IcmpProtocol::send_echo`

```doxygen
@fn int xv::net::IcmpProtocol::send_echo(const unsigned char *data, unsigned length, Endpoint source, Endpoint destination)
@brief 校验 Echo 请求类型和长度，重写标识符与校验和，再经 IPv4 发送。
@param source 本地地址及 socket 分配的唯一 Echo 标识符。
@return 成功为 IPv4 发送结果，失败为负 errno；拒绝非 type 8/code 0 与 8..1472 之外的长度。
@note 与 UDP 共用路由、ARP、过滤和回环；临时报文由 Packet 管理。
```
