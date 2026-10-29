# `kernel/net/link/ethernet.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::EthernetLayer::attach`](ethernet.cpp.md#fn-d01f4eb93cf246f8)
- [`xv::net::EthernetLayer::initialize`](ethernet.cpp.md#fn-c7b38042d08f668f)
- [`xv::net::EthernetLayer::receive_frame`](ethernet.cpp.md#fn-bf5e575793115a66)
- [`xv::net::EthernetLayer::send`](ethernet.cpp.md#fn-468ec89c64e20c7c)
- [`xv::net::EthernetLayer::send_ip`](ethernet.cpp.md#fn-2f3a15c921db001f)
- [`xv::net::EthernetLayer::transmit_frame`](ethernet.cpp.md#fn-d7050414ee46b952)

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
@details 实现通过 EncapsulationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-d01f4eb93cf246f8"></a>

## `xv::net::EthernetLayer::attach`

```doxygen
@fn void xv::net::EthernetLayer::attach(unsigned type, EthernetProtocol &protocol)
@brief EthernetLayer：启动时在最多四项协议表中登记 EtherType 与借用的 EthernetProtocol 实例；不支持运行期卸载。
@details 启动时在最多四项协议表中登记 EtherType 与借用的 EthernetProtocol 实例；不支持运行期卸载。
```

<a id="fn-468ec89c64e20c7c"></a>

## `xv::net::EthernetLayer::send`

```doxygen
@fn int xv::net::EthernetLayer::send(Packet &packet, const unsigned char *destination, unsigned type)
@brief 为本机生成的帧填写目的 MAC、本机源 MAC 和 EtherType，再委托原始帧发送；网桥过境帧必须直接用 transmit_frame，不能经过此源 MAC 填写步骤。
```

<a id="fn-d7050414ee46b952"></a>

## `xv::net::EthernetLayer::transmit_frame`

```doxygen
@fn int xv::net::EthernetLayer::transmit_frame(Packet &packet)
@brief 检查完整帧长度、设备就绪和载波后，原样交给设备；成功 release 页面，失败保留调用者所有权，不修改任何帧字节。
@param[in,out] packet 完整以太帧，调用成功后页面被设备接管。
@return 零或 -EMSGSIZE/-ENETUNREACH/-EAGAIN，失败保留页面。
@details 实现通过 SendFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-bf5e575793115a66"></a>

## `xv::net::EthernetLayer::receive_frame`

```doxygen
@fn void xv::net::EthernetLayer::receive_frame(void *page, unsigned length)
@brief 接管 RX 页面并验证基本帧长。
@details 桥成员先按接入代次分派完整帧，否则按本机 MAC/广播过滤后进行 EtherType 分派，未消费页由 RAII 释放。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```
