# `kernel/drivers/input/virtio_keyboard.cppm`

<a id="fn-eb99a3764a571999"></a>

## `xv::drivers::VirtioKeyboard::VirtioKeyboard`

```doxygen
@fn constexpr xv::drivers::VirtioKeyboard::VirtioKeyboard(unsigned long base, unsigned irq) : registers_(base), irq_(irq)
@brief 保存板级MMIO地址和IRQ，初始化空设备状态，不访问硬件。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-887414e90a6beb63"></a>

## `xv::drivers::VirtioKeyboard::name`

```doxygen
@fn const char *xv::drivers::VirtioKeyboard::name() const override
@brief 返回键盘设备名称。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-5e201b7c6d1f4568"></a>

## `xv::drivers::VirtioKeyboard::irq`

```doxygen
@fn unsigned xv::drivers::VirtioKeyboard::irq() const override
@brief 返回板级键盘IRQ编号。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-5c28f6022d558625"></a>

## `xv::drivers::VirtioKeyboard::write`

```doxygen
@fn int xv::drivers::VirtioKeyboard::write(IoBuffer, int count) override
@brief 仅接受零长度写入，不提供LED或事件注入接口。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-0cdfbc6b56ddeb58"></a>

## `xv::drivers::VirtioKeyboard::put_sync`

```doxygen
@fn void xv::drivers::VirtioKeyboard::put_sync(int) override
@brief 无输出操作，输入设备不提供字符发送能力。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```
