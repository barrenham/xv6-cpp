# `kernel/net/link/port.cppm`

完整以太帧能力与 IPv4 接口分离，FrameReceiver 接收带入端口与代次的完整帧。对象长期存活，成员归属受配置锁保护；见 [网桥设计](../../../../network-bridge.md)。

## 函数导航

- [`xv::net::FrameReceiver::bridge_id`](port.cppm.md#fn-fb43e6c9cb22bef1)
- [`xv::net::FrameReceiver::ingress`](port.cppm.md#fn-28fb707209eed9d2)
- [`xv::net::FrameReceiver::~FrameReceiver`](port.cppm.md#fn-4e1f94bec7f4b837)
- [`xv::net::Layer2Port::Layer2Port`](port.cppm.md#fn-4c3bdc34e6492dc5)
- [`xv::net::Layer2Port::Layer2Port`](port.cppm.md#fn-a31c5f8cb6d1c2d9)
- [`xv::net::Layer2Port::dispatch_frame`](port.cppm.md#fn-44bb72ed36d8c701)
- [`xv::net::Layer2Port::frame_mtu`](port.cppm.md#fn-e70dc727cfb90b3b)
- [`xv::net::Layer2Port::master`](port.cppm.md#fn-e7c68ec98e3aded2)
- [`xv::net::Layer2Port::operator=`](port.cppm.md#fn-8b8039b739cb6cd3)
- [`xv::net::Layer2Port::promiscuous`](port.cppm.md#fn-1ba7e3257a634714)
- [`xv::net::Layer2Port::transmit_frame`](port.cppm.md#fn-767ef4bfb16e4fd7)
- [`xv::net::Layer2Port::~Layer2Port`](port.cppm.md#fn-57083e6d7b38b205)

<a id="fn-fb43e6c9cb22bef1"></a>

## `xv::net::FrameReceiver::bridge_id`

```doxygen
@fn unsigned xv::net::FrameReceiver::bridge_id() const
@brief 原始帧接收器契约：提供所属桥的稳定非零接口 ID，供成员 master 查询。
```

<a id="fn-28fb707209eed9d2"></a>

## `xv::net::FrameReceiver::ingress`

```doxygen
@fn void xv::net::FrameReceiver::ingress(Layer2Port &port, unsigned epoch, Packet &frame)
@brief 原始帧入桥契约，显式携带入端口及取接收器时的接入代次；实现须在配置验证后处理，不得假设目的 MAC 是本机。
@param port 稳定的二层入端口对象。
@param epoch 分派快照代次，接收器必须核对当前归属。
@param[in,out] frame 完整以太帧，不预先进行本机 MAC 筛选。
@pre 回调期间没有持配置锁。
```

<a id="fn-4e1f94bec7f4b837"></a>

## `xv::net::FrameReceiver::~FrameReceiver`

```doxygen
@fn xv::net::FrameReceiver::~FrameReceiver()
@brief 结束接口基类生命周期；protected 非虚析构禁止通过接口 delete，具体对象由启动期长期存储管理。
```

<a id="fn-a31c5f8cb6d1c2d9"></a>

## `xv::net::Layer2Port::Layer2Port`

```doxygen
@fn xv::net::Layer2Port::Layer2Port(const Layer2Port &)
@brief 禁止复制或赋值此接口对象，保持注册身份、借用关系和队列所有权唯一。
```

<a id="fn-8b8039b739cb6cd3"></a>

## `xv::net::Layer2Port::operator=`

```doxygen
@fn Layer2Port &xv::net::Layer2Port::operator=(const Layer2Port &)
@brief 禁止复制或赋值此接口对象，保持注册身份、借用关系和队列所有权唯一。
```

<a id="fn-e70dc727cfb90b3b"></a>

## `xv::net::Layer2Port::frame_mtu`

```doxygen
@fn unsigned xv::net::Layer2Port::frame_mtu() const
@brief 二层能力契约：报告可承载的有效载荷 MTU，不含以太首部。
```

<a id="fn-1ba7e3257a634714"></a>

## `xv::net::Layer2Port::promiscuous`

```doxygen
@fn bool xv::net::Layer2Port::promiscuous(bool enable)
@brief 二层能力契约：启停完整单播/组播接收，成功返回 true；不支持时不能加入网桥，调用者不持配置锁。
```

<a id="fn-767ef4bfb16e4fd7"></a>

## `xv::net::Layer2Port::transmit_frame`

```doxygen
@fn int xv::net::Layer2Port::transmit_frame(Packet &frame)
@brief 二层原始发送契约：保持 MAC、EtherType 和负载原样，成功接管页面返回零；失败负 errno 并保留页面所有权。
@param[in,out] frame 完整以太帧，源/目的 MAC 和负载均不可重写。
@return 零表示已接管；负 errno 时页面仍由 Packet 管理。
@pre 不持配置锁。
```

<a id="fn-e7c68ec98e3aded2"></a>

## `xv::net::Layer2Port::master`

```doxygen
@fn unsigned xv::net::Layer2Port::master() const
@brief 在配置锁内从接收器获取稳定桥 ID，无桥归属返回零，不暴露可变成员引用。
```

<a id="fn-4c3bdc34e6492dc5"></a>

## `xv::net::Layer2Port::Layer2Port`

```doxygen
@fn xv::net::Layer2Port::Layer2Port()
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-57083e6d7b38b205"></a>

## `xv::net::Layer2Port::~Layer2Port`

```doxygen
@fn xv::net::Layer2Port::~Layer2Port()
@brief 结束接口基类生命周期；protected 非虚析构禁止通过接口 delete，具体对象由启动期长期存储管理。
```

<a id="fn-44bb72ed36d8c701"></a>

## `xv::net::Layer2Port::dispatch_frame`

```doxygen
@fn bool xv::net::Layer2Port::dispatch_frame(Packet &frame)
@brief 在配置锁内快照接收器与接入代次，解锁后回调。
@details 没有桥返回 false，存在接收器返回 true，即使旧代次帧随后被桥拒绝也不再投递独立本机路径。
```
