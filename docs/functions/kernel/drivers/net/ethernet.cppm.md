# `kernel/drivers/net/ethernet.cppm`

## 函数导航

- [`xv::drivers::initialize_network`](../board/qemu_virt.cpp.md#fn-b676d9440f8d6421)
- [`xv::drivers::network_device`](../board/qemu_virt.cpp.md#fn-59f08ec5772b85b6)
- [`xv::drivers::EthernetDevice::online`](ethernet.cppm.md#fn-759d5d8b3299891d)
- [`xv::drivers::EthernetDevice::transmit`](ethernet.cppm.md#fn-7340f363f0949924)
- [`xv::drivers::EthernetDevice::~EthernetDevice`](ethernet.cppm.md#fn-79a6be906dbdfcdd)
- [`xv::drivers::EthernetReceiver::receive_frame`](ethernet.cppm.md#fn-af5421b98e5d86b2)
- [`xv::drivers::EthernetReceiver::~EthernetReceiver`](ethernet.cppm.md#fn-0d52ec251209d344)


独立 Doxygen 开发文档；运行代码位于 `kernel/`。


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
