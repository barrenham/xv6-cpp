# `kernel/drivers/board/qemu_virt.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/board/qemu_virt.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`xv.print`, `xv.fs`, `:uart`, `:n_tty`, `:tty`, `:plic`, `:virtio`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::ConsoleFlow::devices`](qemu_virt.cpp.md#fn-f0e4ac986e868925)
- [`xv::drivers::ConsoleFlow::serial`](qemu_virt.cpp.md#fn-b54510fdb4f2206b)
- [`xv::drivers::ConsoleFlow::terminal`](qemu_virt.cpp.md#fn-0e7177c0981f3b7a)
- [`xv::drivers::InterruptsFlow::discover`](qemu_virt.cpp.md#fn-d29facf8ea670c0e)
- [`xv::drivers::InterruptsFlow::locate`](qemu_virt.cpp.md#fn-bbee61fe23e23e4a)
- [`xv::drivers::InterruptsFlow::route`](qemu_virt.cpp.md#fn-807dc2b114e2fef1)
- [`xv::drivers::PciNetworkInterrupt::PciNetworkInterrupt`](qemu_virt.cpp.md#fn-9d462dc2b56e9df5)
- [`xv::drivers::PciNetworkInterrupt::handle_interrupt`](qemu_virt.cpp.md#fn-3ef9ccf1cd9c3d06)
- [`xv::drivers::PciNetworkInterrupt::initialize`](qemu_virt.cpp.md#fn-5af1d4c6af322afc)
- [`xv::drivers::PciNetworkInterrupt::irq`](qemu_virt.cpp.md#fn-2f668257e023a5a7)
- [`xv::drivers::PciNetworkInterrupt::name`](qemu_virt.cpp.md#fn-47bf194bc003f4c4)
- [`xv::drivers::block_device`](qemu_virt.cpp.md#fn-72e31996ffc2b022)
- [`xv::drivers::console`](qemu_virt.cpp.md#fn-cb18263197838102)
- [`xv::drivers::data_device`](qemu_virt.cpp.md#fn-1f40f120d59e62df)
- [`xv::drivers::handle_external_interrupt`](qemu_virt.cpp.md#fn-8819b4e9b6bdacb6)
- [`xv::drivers::initialize_console`](qemu_virt.cpp.md#fn-cacccb8b1ed2107b)
- [`xv::drivers::initialize_hart`](qemu_virt.cpp.md#fn-bea9233c904d33be)
- [`xv::drivers::initialize_interrupts`](qemu_virt.cpp.md#fn-ea116721d09e5c14)
- [`xv::drivers::initialize_network`](qemu_virt.cpp.md#fn-a202b21c5c06ed23)
- [`xv::drivers::interrupt_controller`](qemu_virt.cpp.md#fn-276393f76c615b60)
- [`xv::drivers::network_device`](qemu_virt.cpp.md#fn-0dd9355aa5e189f6)
- [`xv::drivers::network_device_count`](qemu_virt.cpp.md#fn-0f6db43d3d7dce07)
- [`xv::drivers::serial_port`](qemu_virt.cpp.md#fn-ee46a944352f0df2)
- [`xv::drivers::terminal_timer_tick`](qemu_virt.cpp.md#fn-a345578c92b3adb7)

<a id="fn-cacccb8b1ed2107b"></a>

## `xv::drivers::initialize_console`

```doxygen
@fn void xv::drivers::initialize_console()
@brief 初始化板级终端并登记 console 与 ptmx。
@details 在启动阶段初始化 TTY、PTY 注册锁及 UART，登记静态控制台 1/0 和 PTY 多路复用工厂 4/0；devfs 发布目录前封存注册表。实际 PTY 会话只在打开 ptmx 时分配。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
@details 流程由 ConsoleFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-ea116721d09e5c14"></a>

## `xv::drivers::initialize_interrupts`

```doxygen
@fn void xv::drivers::initialize_interrupts()
@brief 启动期扫描最多四个 E1000，设置插槽及共享 IRQ，然后初始化统一中断路由。
@details 流程由 InterruptsFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-bea9233c904d33be"></a>

## `xv::drivers::initialize_hart`

```doxygen
@fn void xv::drivers::initialize_hart(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：建立当前 hart 的局部运行状态。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`router.initialize_hart`。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8819b4e9b6bdacb6"></a>

## `xv::drivers::handle_external_interrupt`

```doxygen
@fn void xv::drivers::handle_external_interrupt(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：handle外部中断。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`router.dispatch` → `xv::print::printk`。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
@details 需要特别跟踪的条件：`unknown`。这些条件决定检查、重试或提前返回路径。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-cb18263197838102"></a>

## `xv::drivers::console`

```doxygen
@fn CharacterDevice &xv::drivers::console()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `terminal`，返回当前查询结果。
@return 各分支实际返回：`terminal`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a345578c92b3adb7"></a>

## `xv::drivers::terminal_timer_tick`

```doxygen
@fn void xv::drivers::terminal_timer_tick()
@brief 设备对象、MMIO、设备中断与终端数据流：终端定时器tick。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`terminal.timer_tick` → `pty_timer_tick`。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ee46a944352f0df2"></a>

## `xv::drivers::serial_port`

```doxygen
@fn SerialPort &xv::drivers::serial_port()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `uart`，返回当前查询结果。
@return 各分支实际返回：`uart`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-72e31996ffc2b022"></a>

## `xv::drivers::block_device`

```doxygen
@fn BlockDevice &xv::drivers::block_device()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `disk`，返回当前查询结果。
@return 各分支实际返回：`disk`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-276393f76c615b60"></a>

## `xv::drivers::interrupt_controller`

```doxygen
@fn InterruptController &xv::drivers::interrupt_controller()
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `plic`，返回当前查询结果。
@return 各分支实际返回：`plic`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9d462dc2b56e9df5"></a>

## `xv::drivers::PciNetworkInterrupt::PciNetworkInterrupt`

```doxygen
@fn constexpr explicit xv::drivers::PciNetworkInterrupt::PciNetworkInterrupt(unsigned irq) : irq_(irq)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-47bf194bc003f4c4"></a>

## `xv::drivers::PciNetworkInterrupt::name`

```doxygen
@fn const char *xv::drivers::PciNetworkInterrupt::name() const override
@brief 返回共享 PCI 以太 INTx 中断源的诊断名称。
```

<a id="fn-2f668257e023a5a7"></a>

## `xv::drivers::PciNetworkInterrupt::irq`

```doxygen
@fn unsigned xv::drivers::PciNetworkInterrupt::irq() const override
@brief 返回该中断源服务的 PLIC 线路号。
```

<a id="fn-5af1d4c6af322afc"></a>

## `xv::drivers::PciNetworkInterrupt::initialize`

```doxygen
@fn void xv::drivers::PciNetworkInterrupt::initialize() override
@brief 共享中断源无需独立硬件初始化；E1000 初始化负责设备寄存器。
```

<a id="fn-3ef9ccf1cd9c3d06"></a>

## `xv::drivers::PciNetworkInterrupt::handle_interrupt`

```doxygen
@fn void xv::drivers::PciNetworkInterrupt::handle_interrupt() override
@brief 遍历所有已发现网卡，对 IRQ 与本线路一致的设备逐个处理；支持多个 PCI 插槽共享 INTx。
```

<a id="fn-0f6db43d3d7dce07"></a>

## `xv::drivers::network_device_count`

```doxygen
@fn unsigned xv::drivers::network_device_count()
@brief 返回启动扫描发现的 E1000 数量，范围零至四。
```

<a id="fn-a202b21c5c06ed23"></a>

## `xv::drivers::initialize_network`

```doxygen
@fn void xv::drivers::initialize_network(EthernetReceiver &receiver, unsigned index)
@brief 为指定已发现设备挂接接收器并初始化硬件，超出发现数量则不操作。
```

<a id="fn-0dd9355aa5e189f6"></a>

## `xv::drivers::network_device`

```doxygen
@fn EthernetDevice &xv::drivers::network_device(unsigned index)
@brief 借用指定静态 E1000 对象；索引必须小于四，使用设备前还须核对发现数量。
```

<a id="fn-1f40f120d59e62df"></a>

## `xv::drivers::data_device`

```doxygen
@fn BlockDevice *xv::drivers::data_device()
@brief 取得已连接的第二个 VirtIO 块设备。
@details 板级静态实例位于根盘之后的 MMIO 页和 IRQ；未发现设备时返回 nullptr。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-f0e4ac986e868925"></a>

## `xv::drivers::ConsoleFlow::devices`

```doxygen
@fn static void xv::drivers::ConsoleFlow::devices(Context &) noexcept
@brief 按原顺序登记 console 与 ptmx。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-b54510fdb4f2206b"></a>

## `xv::drivers::ConsoleFlow::serial`

```doxygen
@fn static void xv::drivers::ConsoleFlow::serial(Context &) noexcept
@brief 连接原终端并初始化 UART。
@details 固定板级实例仍静态构造；图只连接原终端/PTY、UART 和设备登记顺序，不改变 kernel/boot。 下一节点由图策略声明，业务结果与控制信号分离。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-0e7177c0981f3b7a"></a>

## `xv::drivers::ConsoleFlow::terminal`

```doxygen
@fn static void xv::drivers::ConsoleFlow::terminal(Context &) noexcept
@brief 初始化板级 TTY 与 PTY 列表。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-d29facf8ea670c0e"></a>

## `xv::drivers::InterruptsFlow::discover`

```doxygen
@fn static void xv::drivers::InterruptsFlow::discover(Context &c) noexcept
@brief 按原四设备容量扫描 PCI 并保存网卡数。
@details 调用帧保持原固定未初始化插槽数组；发现数量先发布，再定位固定网卡实例，最后验证并初始化原路由。 下一节点由图策略声明，业务结果与控制信号分离。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-bbee61fe23e23e4a"></a>

## `xv::drivers::InterruptsFlow::locate`

```doxygen
@fn static void xv::drivers::InterruptsFlow::locate(Context &c) noexcept
@brief 将发现插槽绑定到对应固定网卡及 INTx 编号。
@details 调用帧保持原固定未初始化插槽数组；发现数量先发布，再定位固定网卡实例，最后验证并初始化原路由。 下一节点由图策略声明，业务结果与控制信号分离。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-807dc2b114e2fef1"></a>

## `xv::drivers::InterruptsFlow::route`

```doxygen
@fn static void xv::drivers::InterruptsFlow::route(Context &) noexcept
@brief 初始化原控制器与中断路由。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-98149e18194be74a"></a>

## `xv::drivers::realtime_seconds`

```doxygen
@fn unsigned long xv::drivers::realtime_seconds()
@brief 读取Goldfish RTC实时时间。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```

<a id="fn-295e9f1fac671bce"></a>

## `xv::drivers::boot_time_seconds`

```doxygen
@fn unsigned long xv::drivers::boot_time_seconds()
@brief 返回已记录的启动epoch秒数。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。 本轮板级装配增加kbd0(30:0)0600及slot4/IRQ5，在中断启用前初始化键盘，其余设备顺序保持。
```
