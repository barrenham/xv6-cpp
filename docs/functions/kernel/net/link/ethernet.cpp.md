# `kernel/net/link/ethernet.cpp`

## 函数导航

- [`xv::net::EthernetLayer::attach`](ethernet.cpp.md#fn-d01f4eb93cf246f8)
- [`xv::net::EthernetLayer::initialize`](ethernet.cpp.md#fn-c7b38042d08f668f)
- [`xv::net::EthernetLayer::receive_frame`](ethernet.cpp.md#fn-bf5e575793115a66)
- [`xv::net::EthernetLayer::send`](ethernet.cpp.md#fn-468ec89c64e20c7c)
- [`xv::net::EthernetLayer::send_ip`](ethernet.cpp.md#fn-2f3a15c921db001f)

<a id="fn-d01f4eb93cf246f8"></a>

## `xv::net::EthernetLayer::attach`

```doxygen
@fn void xv::net::EthernetLayer::attach(unsigned type, EthernetProtocol &protocol)
@brief EthernetLayer：启动时在最多四项协议表中登记 EtherType 与借用的 EthernetProtocol 实例；不支持运行期卸载。
@details 启动时在最多四项协议表中登记 EtherType 与借用的 EthernetProtocol 实例；不支持运行期卸载。
```

<a id="fn-c7b38042d08f668f"></a>

## `xv::net::EthernetLayer::initialize`

```doxygen
@fn void xv::net::EthernetLayer::initialize(xv::drivers::EthernetDevice &device)
@brief 借用指定以太设备并读取其 MAC，后续收发始终使用该设备，避免全局首网卡依赖。
```

<a id="fn-2f3a15c921db001f"></a>

## `xv::net::EthernetLayer::send_ip`

```doxygen
@fn int xv::net::EthernetLayer::send_ip(Packet &packet, const unsigned char *destination)
@brief 为纯 IPv4 包前插十四字节以太首部并发送；失败撤销首部，未移交的页面保持归调用者所有。
```

<a id="fn-468ec89c64e20c7c"></a>

## `xv::net::EthernetLayer::send`

```doxygen
@fn int xv::net::EthernetLayer::send(Packet &packet, const unsigned char *destination, unsigned type)
@brief 填写指定目标、本设备 MAC 与 EtherType；检查帧长度和设备就绪，成功 DMA 发送后显式 release 页面。
```

<a id="fn-bf5e575793115a66"></a>

## `xv::net::EthernetLayer::receive_frame`

```doxygen
@fn void xv::net::EthernetLayer::receive_frame(void *page, unsigned length)
@brief 接管驱动移交的 RX 页面，检查帧长、本设备 MAC/广播目标，再按 EtherType 分派；未消费页面由 RAII 释放。
```
