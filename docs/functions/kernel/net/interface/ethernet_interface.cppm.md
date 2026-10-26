# `kernel/net/interface/ethernet_interface.cppm`

## 函数导航

- [`xv::net::EthernetInterface::EthernetInterface`](ethernet_interface.cppm.md#fn-e8752742067697dc)
- [`xv::net::EthernetInterface::carrier`](ethernet_interface.cppm.md#fn-1d271cadfb212572)
- [`xv::net::EthernetInterface::initialize`](ethernet_interface.cpp.md#fn-6aa7b41af4f709cb)
- [`xv::net::EthernetInterface::mac`](ethernet_interface.cppm.md#fn-5768eaba9cc42609)
- [`xv::net::EthernetInterface::receive`](ethernet_interface.cpp.md#fn-00530c658ce99424)
- [`xv::net::EthernetInterface::receiver`](ethernet_interface.cppm.md#fn-b39f14a27d219261)
- [`xv::net::EthernetInterface::tick`](ethernet_interface.cppm.md#fn-df47067df667684e)
- [`xv::net::EthernetInterface::transmit`](ethernet_interface.cpp.md#fn-3e698439f28e1935)

<a id="fn-e8752742067697dc"></a>

## `xv::net::EthernetInterface::EthernetInterface`

```doxygen
@fn xv::net::EthernetInterface::EthernetInterface(unsigned id, const char *name, unsigned mtu = 1500)
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
@fn const MacAddress *xv::net::EthernetInterface::mac() const
@brief 借用从设备读取的 MAC 地址；该属性只存在于以太接口。
```

<a id="fn-1d271cadfb212572"></a>

## `xv::net::EthernetInterface::carrier`

```doxygen
@fn bool xv::net::EthernetInterface::carrier() const
@brief 查询本接口绑定设备的实际链路状态，与接口管理状态分开。
```

<a id="fn-df47067df667684e"></a>

## `xv::net::EthernetInterface::tick`

```doxygen
@fn void xv::net::EthernetInterface::tick(unsigned long now)
@brief 维护本接口独立 ARP 待解析队列，按单调毫秒时刻重试或释放超时报文。
```
