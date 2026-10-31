# `kernel/drivers/input/virtio_keyboard.cpp`

<a id="fn-ab277fa7bf126252"></a>

## `xv::drivers::VirtioKeyboard::identify`

```doxygen
@fn bool xv::drivers::VirtioKeyboard::identify() const
@brief 检查VirtIO magic、modern版本及输入设备ID，不读取设备DMA。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-b66af90a2a625682"></a>

## `xv::drivers::VirtioKeyboard::negotiate`

```doxygen
@fn bool xv::drivers::VirtioKeyboard::negotiate()
@brief 协商VERSION_1并查询KEY事件能力，要求A与Enter键后才接入。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-73b833e10fb4ab7f"></a>

## `xv::drivers::VirtioKeyboard::allocate`

```doxygen
@fn bool xv::drivers::VirtioKeyboard::allocate()
@brief 逐页申请并清零7个DMA候选；部分失败留给初始化回滚。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-08afe6ccb6f0785a"></a>

## `xv::drivers::VirtioKeyboard::publish`

```doxygen
@fn bool xv::drivers::VirtioKeyboard::publish()
@brief 配置64槽eventq与空statusq，发布设备可写事件槽后设置DRIVER_OK。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-f901e356d0436d0e"></a>

## `xv::drivers::VirtioKeyboard::rollback`

```doxygen
@fn void xv::drivers::VirtioKeyboard::rollback()
@brief 复位设备并最多等待100ms，确认停止DMA后释放候选页，超时保留。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-e7eebabe174b748d"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::found`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::InitializeFlow::found(Context &context) noexcept
@brief 检查当前初始化候选是否为modern VirtIO输入设备。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-9f88ac8a16f7c3fa"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::negotiate`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InitializeFlow::negotiate(Context &context) noexcept
@brief 协商VERSION_1并查询KEY事件能力，要求A与Enter键后才接入。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-e331db210bb765fd"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InitializeFlow::allocate(Context &context) noexcept
@brief 逐页申请并清零7个DMA候选；部分失败留给初始化回滚。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-4181858db623c7bb"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::publish`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InitializeFlow::publish(Context &context) noexcept
@brief 配置64槽eventq与空statusq，发布设备可写事件槽后设置DRIVER_OK。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-59354aeb47174641"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::InitializeFlow::okay(Context &context) noexcept
@brief 返回本阶段操作是否成功，驱动图中的继续或失败分支。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-081b3c22114ece74"></a>

## `xv::drivers::VirtioKeyboard::InitializeFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InitializeFlow::fail(Context &context) noexcept
@brief 运行所属流程的失败清理：初始化复位回滚，IRQ离线保留DMA。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-d3fd356e92516882"></a>

## `xv::drivers::VirtioKeyboard::initialize`

```doxygen
@fn void xv::drivers::VirtioKeyboard::initialize()
@brief 初始化spin锁并运行设备发现、协商、分配、发布及失败回滚图。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-294a49f5f12d6658"></a>

## `xv::drivers::VirtioKeyboard::open`

```doxygen
@fn CharacterDevice *xv::drivers::VirtioKeyboard::open(bool)
@brief 在锁内验证在线状态与root身份，独占打开并丢弃旧输入。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-9a985696697d3148"></a>

## `xv::drivers::VirtioKeyboard::close`

```doxygen
@fn void xv::drivers::VirtioKeyboard::close()
@brief 最后一个打开文件引用释放时清除独占状态和旧事件。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-4037f84d6005e2a0"></a>

## `xv::drivers::VirtioKeyboard::enqueue`

```doxygen
@fn void xv::drivers::VirtioKeyboard::enqueue(const KeyboardEvent &event)
@brief 把事件放入有界队列；溢出丢弃旧输入并插入SYN_DROPPED。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-ab9498cbf73790d0"></a>

## `xv::drivers::VirtioKeyboard::drain`

```doxygen
@fn bool xv::drivers::VirtioKeyboard::drain()
@brief 验证used批次增量、ID、长度与唯一性，缓存ID后复制事件并回填描述符。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-9b6ea0d81d296171"></a>

## `xv::drivers::VirtioKeyboard::disable`

```doxygen
@fn void xv::drivers::VirtioKeyboard::disable()
@brief 异常时停止设备、保留DMA页并唤醒等待者，IRQ上下文不等待复位。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-3bc5b7104875dfc4"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::acknowledge`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InterruptFlow::acknowledge(Context &context) noexcept
@brief 确认设备中断状态并执行I/O内存屏障。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-e31a6d48936f1137"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::ready`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::InterruptFlow::ready(Context &context) noexcept
@brief 检查设备是否仍在线，离线设备不读取队列。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-58f8f6ea9e4f8f31"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::drain`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InterruptFlow::drain(Context &context) noexcept
@brief 验证used批次增量、ID、长度与唯一性，缓存ID后复制事件并回填描述符。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-8ff031886657a649"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::okay`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::InterruptFlow::okay(Context &context) noexcept
@brief 返回本阶段操作是否成功，驱动图中的继续或失败分支。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-1701c960ec8a4b33"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::fail`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InterruptFlow::fail(Context &context) noexcept
@brief 运行所属流程的失败清理：初始化复位回滚，IRQ离线保留DMA。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-a5d51841406643ad"></a>

## `xv::drivers::VirtioKeyboard::InterruptFlow::wake`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::InterruptFlow::wake(Context &context) noexcept
@brief 有缓冲事件时唤醒读者和poll等待者。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-28da51adebb88eb3"></a>

## `xv::drivers::VirtioKeyboard::handle_interrupt`

```doxygen
@fn void xv::drivers::VirtioKeyboard::handle_interrupt()
@brief 在spin锁内运行ACK、完成验证、重填与唤醒控制图。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-d25acdb30e4e0f86"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::stopped`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::ReadFlow::stopped(Context &context) noexcept
@brief 判断离线、未打开或进程终止条件，停止读取流程。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-e22a1352d3c6d779"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::available`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::ReadFlow::available(Context &context) noexcept
@brief 检查软件事件队列中是否存在待消费事件。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-c96c10ec48962273"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::VirtioKeyboard::ReadFlow::nonblocking(Context &context) noexcept
@brief 读取当前IoBuffer的非阻塞标志。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-bad6553a6542114f"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::again`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::ReadFlow::again(Context &context) noexcept
@brief 返回本项目直接使用的-EAGAIN状态。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-5c06b6403c3d3940"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::sleep`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::ReadFlow::sleep(Context &context) noexcept
@brief 持锁登记可中断睡眠，释放锁睡眠并在返回后重新持锁。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-c8ceed208d6cca22"></a>

## `xv::drivers::VirtioKeyboard::ReadFlow::copy`

```doxygen
@fn static void xv::drivers::VirtioKeyboard::ReadFlow::copy(Context &context) noexcept
@brief 有界复制至多32个完整事件，用户复制失败时不消费队列。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-922d28769507196c"></a>

## `xv::drivers::VirtioKeyboard::read`

```doxygen
@fn int xv::drivers::VirtioKeyboard::read(IoBuffer destination, int count)
@brief 按完整8字节事件读取，检查root身份，支持可中断等待与非阻塞EAGAIN。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```

<a id="fn-1c07778e5175b796"></a>

## `xv::drivers::VirtioKeyboard::poll`

```doxygen
@fn unsigned xv::drivers::VirtioKeyboard::poll(xv::sync::PollTable *table)
@brief 先登记等待通道，再在锁内报告可读事件或离线/权限错误。
@details 7页DMA由设备持有，运行期队列及独占状态受spin锁保护；eventq描述符只指向内核DMA事件槽。无设备不分配，初始化回滚须先确认reset；IRQ错误保留DMA。用户ABI为8字节type/code/value，256事件软件队列溢出显式报告SYN_DROPPED，消费者应清除修饰键状态。
```
