# `kernel/drivers/net/ethernet.cppm`

## 函数导航

- [`xv::drivers::initialize_network`](../board/qemu_virt.cpp.md#fn-a202b21c5c06ed23)
- [`xv::drivers::network_device`](../board/qemu_virt.cpp.md#fn-0dd9355aa5e189f6)
- [`xv::drivers::network_device_count`](../board/qemu_virt.cpp.md#fn-0f6db43d3d7dce07)
- [`xv::drivers::EthernetDevice::carrier`](ethernet.cppm.md#fn-0a75b7ad9c57762f)
- [`xv::drivers::EthernetDevice::mac_address`](ethernet.cppm.md#fn-835a79075d06aef8)
- [`xv::drivers::EthernetDevice::online`](ethernet.cppm.md#fn-759d5d8b3299891d)
- [`xv::drivers::EthernetDevice::promiscuous`](ethernet.cppm.md#fn-180eea3b76ffadd5)
- [`xv::drivers::EthernetDevice::transmit`](ethernet.cppm.md#fn-7340f363f0949924)
- [`xv::drivers::EthernetDevice::~EthernetDevice`](ethernet.cppm.md#fn-79a6be906dbdfcdd)
- [`xv::drivers::EthernetReceiver::receive_frame`](ethernet.cppm.md#fn-af5421b98e5d86b2)
- [`xv::drivers::EthernetReceiver::~EthernetReceiver`](ethernet.cppm.md#fn-0d52ec251209d344)

<a id="fn-af5421b98e5d86b2"></a>

## `xv::drivers::EthernetReceiver::receive_frame`

```doxygen
@fn void xv::drivers::EthernetReceiver::receive_frame(void *page, unsigned length)
@brief 接收并取得一个完整以太网帧页面的所有权。
@param page 页首为 Ethernet 头的独占页。
@param length 实际有效字节数，不含 FCS。
@details 在中断上下文调用；实现必须入队或释放页面，不能睡眠。驱动回调前已释放设备锁。
```

<a id="fn-0d52ec251209d344"></a>

## `xv::drivers::EthernetReceiver::~EthernetReceiver`

```doxygen
@fn xv::drivers::EthernetReceiver::~EthernetReceiver()
@brief 销毁接收接口基类部分。
@details protected 非虚析构禁止通过接口删除派生对象；板级协议实例静态存活，不以基类指针拥有对象。
```

<a id="fn-759d5d8b3299891d"></a>

## `xv::drivers::EthernetDevice::online`

```doxygen
@fn bool xv::drivers::EthernetDevice::online() const
@brief 查询设备能否接受以太网帧。
@return 初始化成功后为 true；离线时发送方不得依赖数据传输。
```

<a id="fn-7340f363f0949924"></a>

## `xv::drivers::EthernetDevice::transmit`

```doxygen
@fn bool xv::drivers::EthernetDevice::transmit(void *page, unsigned length)
@brief 尝试提交独占页中的以太网帧，并明确页面所有权。
@param page 分配器取得的 DMA 页。
@param length 页首有效帧长度，至多 1514。
@return true 后页面归设备；false 后页面仍归调用者。
@note 此接口不阻塞；协议层把提交失败映射为 -EAGAIN。
```

<a id="fn-79a6be906dbdfcdd"></a>

## `xv::drivers::EthernetDevice::~EthernetDevice`

```doxygen
@fn xv::drivers::EthernetDevice::~EthernetDevice()
@brief 销毁以太网设备接口基类部分。
@details protected 非虚析构禁止通过接口删除；板级设备静态存活，接口只提供借用访问。
```
<a id="fn-0a75b7ad9c57762f"></a>

## `xv::drivers::EthernetDevice::carrier`

```doxygen
@fn bool xv::drivers::EthernetDevice::carrier() const
@brief 默认把驱动就绪视作载波可用；具体硬件可覆盖实际链路检测。
```

<a id="fn-835a79075d06aef8"></a>

## `xv::drivers::EthernetDevice::mac_address`

```doxygen
@fn void xv::drivers::EthernetDevice::mac_address(unsigned char *destination) const
@brief 以太设备契约：向至少六字节目标缓冲区提供此设备 MAC，不把该属性施加给三层虚拟接口。
```
<a id="fn-180eea3b76ffadd5"></a>

## `xv::drivers::EthernetDevice::promiscuous`

```doxygen
@fn bool xv::drivers::EthernetDevice::promiscuous(bool)
@brief 以太设备的可选混杂接收能力，默认不支持并返回 false；桥接必须先成功启用该能力。
```
