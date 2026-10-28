# `kernel/net/bridge/bridge.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::BridgeInterface::initialize`](bridge.cpp.md#fn-ce4d08412e7d6ce2)
- [`xv::net::BridgeInterface::mac_address`](bridge.cpp.md#fn-e2285e8b355a3998)
- [`xv::net::BridgeInterface::carrier`](bridge.cpp.md#fn-c626d20e219e7087)
- [`xv::net::BridgeInterface::tick`](bridge.cpp.md#fn-ba92da7fc5981118)
- [`xv::net::BridgeInterface::ingress`](bridge.cpp.md#fn-84c2d9daab914b31)
- [`xv::net::BridgeInterface::transmit`](bridge.cpp.md#fn-26b371580e62fdc8)
- [`xv::net::BridgeInterface::forward`](bridge.cpp.md#fn-87db7052d871e63a)

<a id="fn-ce4d08412e7d6ce2"></a>

## `xv::net::BridgeInterface::initialize`

```doxygen
@fn void xv::net::BridgeInterface::initialize(IpReceiver &ip)
@brief 启动期仅执行一次，复用 EthernetInterface 的链路与 ARP 初始化，以本对象的软件 EthernetDevice 作为下层，并借用共享 IP 接收器。
```

<a id="fn-e2285e8b355a3998"></a>

## `xv::net::BridgeInterface::mac_address`

```doxygen
@fn void xv::net::BridgeInterface::mac_address(unsigned char *out) const
@brief 复制本桥槽的六字节稳定 MAC；供本机 EthernetLayer 初始化使用，不读取成员当前源 MAC。
```

<a id="fn-c626d20e219e7087"></a>

## `xv::net::BridgeInterface::carrier`

```doxygen
@fn bool xv::net::BridgeInterface::carrier() const
@brief 在配置锁内确认桥已发布，且至少一个成员管理状态和链路均可用；桥本身 UP 由选路层另行检查。
```

<a id="fn-ba92da7fc5981118"></a>

## `xv::net::BridgeInterface::tick`

```doxygen
@fn void xv::net::BridgeInterface::tick(unsigned long now)
@brief 在配置锁内清除已到期动态 MAC 项，再解锁维护本机 ARP 待发队列；不在配置锁内取得邻居锁。
```

<a id="fn-84c2d9daab914b31"></a>

## `xv::net::BridgeInterface::ingress`

```doxygen
@fn void xv::net::BridgeInterface::ingress(Layer2Port &port, unsigned epoch, Packet &frame)
@brief 把完整帧、真实入端口和接入代次交给二层转发；不预先做目的 MAC 本机过滤，页面仍由输入 Packet 管理。
@param port 入端口，必须仍属于本桥。
@param epoch RX 分派时记录的接入代次。
@param[in,out] frame 同步借用的完整帧，只有显式移交后可以保留。
```

<a id="fn-26b371580e62fdc8"></a>

## `xv::net::BridgeInterface::transmit`

```doxygen
@fn bool xv::net::BridgeInterface::transmit(void *page, unsigned length)
@brief 软件 EthernetDevice 的本机帧发送入口。
@details 以 Packet 临时接管页面并调用桥转发，至少一个出口成功才返回 true 并消费原页；失败 release 后把所有权留给调用者。
@param page 软件设备暂时接管的完整以太帧页。
@param length 不含 FCS 的帧长度。
@return true 表示页面已消费；false 时调用者仍拥有原页。
```

<a id="fn-87db7052d871e63a"></a>

## `xv::net::BridgeInterface::forward`

```doxygen
@fn bool xv::net::BridgeInterface::forward(Layer2Port *ingress, unsigned epoch, Packet &frame)
@brief 验证帧长、单播非零源、桥状态与接入代次，学习源 MAC 并形成泛洪或定向目标快照。
@details 解锁后逐出口克隆且复核归属/代次，保留 MAC 和内容原样发送；必要时把输入页移交桥本机协议栈。
@param ingress 入端口，本机发送为 nullptr。
@param epoch 接收器快照对应的接入代次，本机发送传零。
@param[in,out] frame 完整以太帧的独占页面；本机交付可消费输入页。
@return 至少一个出口成功或本机已消费返回 true，否则 false。
@note 不持配置锁分配副本、发送或回调本机协议；同入口过滤、VLAN/保留控制帧拒绝也返回 false。
```
