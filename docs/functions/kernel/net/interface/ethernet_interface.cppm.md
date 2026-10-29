# `kernel/net/interface/ethernet_interface.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::EthernetInterface::EthernetInterface`](ethernet_interface.cppm.md#fn-e8752742067697dc)
- [`xv::net::EthernetInterface::carrier`](ethernet_interface.cppm.md#fn-1d271cadfb212572)
- [`xv::net::EthernetInterface::initialize`](ethernet_interface.cpp.md#fn-6aa7b41af4f709cb)
- [`xv::net::EthernetInterface::invalidate`](ethernet_interface.cppm.md#fn-f6d1075dc84faef7)
- [`xv::net::EthernetInterface::layer2`](ethernet_interface.cppm.md#fn-f8c668746d15b180)
- [`xv::net::EthernetInterface::mac`](ethernet_interface.cppm.md#fn-5768eaba9cc42609)
- [`xv::net::EthernetInterface::master`](ethernet_interface.cppm.md#fn-6abccc5d2585ce1c)
- [`xv::net::EthernetInterface::neighbor`](ethernet_interface.cppm.md#fn-6595b5d65f41b1c5)
- [`xv::net::EthernetInterface::receive`](ethernet_interface.cpp.md#fn-00530c658ce99424)
- [`xv::net::EthernetInterface::receiver`](ethernet_interface.cppm.md#fn-b39f14a27d219261)
- [`xv::net::EthernetInterface::tick`](ethernet_interface.cppm.md#fn-df47067df667684e)
- [`xv::net::EthernetInterface::transmit`](ethernet_interface.cpp.md#fn-3e698439f28e1935)
- [`xv::net::EthernetInterface::transmit_bootstrap`](ethernet_interface.cpp.md#fn-6a23b3040e6fa382)

<a id="fn-e8752742067697dc"></a>

## `xv::net::EthernetInterface::EthernetInterface`

```doxygen
@fn constexpr xv::net::EthernetInterface::EthernetInterface(unsigned id, const char *name, unsigned mtu = 1500)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-b39f14a27d219261"></a>

## `xv::net::EthernetInterface::receiver`

```doxygen
@fn xv::drivers::EthernetReceiver &xv::net::EthernetInterface::receiver()
@brief 借用本接口的以太接收适配器，用于把驱动 RX 回调接入正确接口。
```

<a id="fn-5768eaba9cc42609"></a>

## `xv::net::EthernetInterface::mac`

```doxygen
@fn const MacAddress *xv::net::EthernetInterface::mac() const override
@brief 借用从设备读取的 MAC 地址；该属性只存在于以太接口。
```

<a id="fn-1d271cadfb212572"></a>

## `xv::net::EthernetInterface::carrier`

```doxygen
@fn bool xv::net::EthernetInterface::carrier() const override
@brief 查询本接口绑定设备的实际链路状态，与接口管理状态分开。
```

<a id="fn-f8c668746d15b180"></a>

## `xv::net::EthernetInterface::layer2`

```doxygen
@fn Layer2Port *xv::net::EthernetInterface::layer2() override
@brief 借用本接口的 EthernetLayer 二层端口能力，供桥成员管理与原始帧发送使用，不转移对象所有权。
```

<a id="fn-6abccc5d2585ce1c"></a>

## `xv::net::EthernetInterface::master`

```doxygen
@fn unsigned xv::net::EthernetInterface::master() const override
@brief 在二层端口配置快照下查询当前所属桥 ID；普通独立接口和桥本机入口为零。
```

<a id="fn-6595b5d65f41b1c5"></a>

## `xv::net::EthernetInterface::neighbor`

```doxygen
@fn bool xv::net::EthernetInterface::neighbor(unsigned index, NeighborInfo &result) override
@brief 从本接口 ARP 缓存取得指定有效邻居副本，未找到返回 false；不持配置锁调用。
```

<a id="fn-f6d1075dc84faef7"></a>

## `xv::net::EthernetInterface::invalidate`

```doxygen
@fn void xv::net::EthernetInterface::invalidate() override
@brief 在配置锁外清空本接口邻居和待解析包，释放被取消的页面；用于停接口、改地址和显式清空。
```

<a id="fn-df47067df667684e"></a>

## `xv::net::EthernetInterface::tick`

```doxygen
@fn void xv::net::EthernetInterface::tick(unsigned long now) override
@brief 维护本接口独立 ARP 待解析队列，按单调毫秒时刻重试或释放超时报文。
```
