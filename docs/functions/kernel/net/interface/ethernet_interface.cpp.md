# `kernel/net/interface/ethernet_interface.cpp`

## 函数导航

- [`xv::net::EthernetInterface::initialize`](ethernet_interface.cpp.md#fn-6aa7b41af4f709cb)
- [`xv::net::EthernetInterface::receive`](ethernet_interface.cpp.md#fn-00530c658ce99424)
- [`xv::net::EthernetInterface::transmit`](ethernet_interface.cpp.md#fn-3e698439f28e1935)

<a id="fn-6aa7b41af4f709cb"></a>

## `xv::net::EthernetInterface::initialize`

```doxygen
@fn void xv::net::EthernetInterface::initialize(xv::drivers::EthernetDevice &device)
@brief 绑定指定设备，读取设备 MAC，初始化独立 ARP 缓存，挂接 EtherType 0x806 和 0x800。
@details 须在设备初始化后调用。
```

<a id="fn-3e698439f28e1935"></a>

## `xv::net::EthernetInterface::transmit`

```doxygen
@fn int xv::net::EthernetInterface::transmit(Packet &packet, unsigned next_hop)
@brief 检查管理状态、载波、IPv4 地址及 MTU；广播直接封装以太首部，单播经本接口 ARP 解析下一跳。
```

<a id="fn-00530c658ce99424"></a>

## `xv::net::EthernetInterface::receive`

```doxygen
@fn void xv::net::EthernetInterface::receive(Packet &packet)
@brief 从已验证以太帧移除十四字节首部，再以本接口身份向 IP 层投递。
```
