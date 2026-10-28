# `kernel/net/interface/ethernet_interface.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::EthernetInterface::initialize`](ethernet_interface.cpp.md#fn-6aa7b41af4f709cb)
- [`xv::net::EthernetInterface::transmit`](ethernet_interface.cpp.md#fn-3e698439f28e1935)
- [`xv::net::EthernetInterface::receive`](ethernet_interface.cpp.md#fn-00530c658ce99424)
- [`xv::net::EthernetInterface::transmit_bootstrap`](ethernet_interface.cpp.md#fn-6a23b3040e6fa382)

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
@brief 拒绝已作为桥成员的接口直接发送 IP，并检查 UP、载波、地址与 MTU；本机广播直接编码，单播经本接口 ARP，桥本机子类复用相同流程。
```

<a id="fn-00530c658ce99424"></a>

## `xv::net::EthernetInterface::receive`

```doxygen
@fn void xv::net::EthernetInterface::receive(Packet &packet)
@brief 从已验证以太帧移除十四字节首部，再以本接口身份向 IP 层投递。
```

<a id="fn-6a23b3040e6fa382"></a>

## `xv::net::EthernetInterface::transmit_bootstrap`

```doxygen
@fn int xv::net::EthernetInterface::transmit_bootstrap(Packet &packet)
@brief 以广播 MAC 发送指定接口上的完整 IPv4 DHCP 包，允许接口尚无 IPv4 地址。
@details 仍检查 UP、载波、MTU 和桥归属；以太接口与桥本机入口共用，桥成员不能直接发起 DHCP。IPv4 层负责受限端口及源地址校验。
```
