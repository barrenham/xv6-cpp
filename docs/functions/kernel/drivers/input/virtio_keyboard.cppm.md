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
@brief 按实例能力返回键盘或鼠标滚轮的设备名称。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-5e201b7c6d1f4568"></a>

## `xv::drivers::VirtioKeyboard::irq`

```doxygen
@fn unsigned xv::drivers::VirtioKeyboard::irq() const override
@brief 返回该输入实例的板级IRQ编号。
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

- [`xv::drivers::VirtioKeyboard::initialize_wheel`](virtio_keyboard.cpp.md#fn-567537edde1d045a)

- [`xv::drivers::VirtioKeyboard::VirtioKeyboard`](virtio_keyboard.cppm.md#fn-eb99a3764a571999)

- [`xv::drivers::VirtioKeyboard::allocate`](virtio_keyboard.cpp.md#fn-73b833e10fb4ab7f)

- [`xv::drivers::VirtioKeyboard::close`](virtio_keyboard.cpp.md#fn-9a985696697d3148)

- [`xv::drivers::VirtioKeyboard::disable`](virtio_keyboard.cpp.md#fn-9b6ea0d81d296171)

- [`xv::drivers::VirtioKeyboard::drain`](virtio_keyboard.cpp.md#fn-ab9498cbf73790d0)

- [`xv::drivers::VirtioKeyboard::enqueue`](virtio_keyboard.cpp.md#fn-4037f84d6005e2a0)

- [`xv::drivers::VirtioKeyboard::handle_interrupt`](virtio_keyboard.cpp.md#fn-28da51adebb88eb3)

- [`xv::drivers::VirtioKeyboard::identify`](virtio_keyboard.cpp.md#fn-ab277fa7bf126252)

- [`xv::drivers::VirtioKeyboard::initialize`](virtio_keyboard.cpp.md#fn-d3fd356e92516882)

- [`xv::drivers::VirtioKeyboard::irq`](virtio_keyboard.cppm.md#fn-5e201b7c6d1f4568)

- [`xv::drivers::VirtioKeyboard::name`](virtio_keyboard.cppm.md#fn-887414e90a6beb63)

- [`xv::drivers::VirtioKeyboard::negotiate`](virtio_keyboard.cpp.md#fn-b66af90a2a625682)

- [`xv::drivers::VirtioKeyboard::open`](virtio_keyboard.cpp.md#fn-294a49f5f12d6658)

- [`xv::drivers::VirtioKeyboard::poll`](virtio_keyboard.cpp.md#fn-1c07778e5175b796)

- [`xv::drivers::VirtioKeyboard::publish`](virtio_keyboard.cpp.md#fn-08afe6ccb6f0785a)

- [`xv::drivers::VirtioKeyboard::put_sync`](virtio_keyboard.cppm.md#fn-0cdfbc6b56ddeb58)

- [`xv::drivers::VirtioKeyboard::read`](virtio_keyboard.cpp.md#fn-922d28769507196c)

- [`xv::drivers::VirtioKeyboard::rollback`](virtio_keyboard.cpp.md#fn-f901e356d0436d0e)

- [`xv::drivers::VirtioKeyboard::write`](virtio_keyboard.cppm.md#fn-5c28f6022d558625)
