# `kernel/drivers/board/qemu_virt.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/board/qemu_virt.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`xv.print`, `xv.fs`, `:uart`, `:n_tty`, `:tty`, `:plic`, `:virtio`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::block_device`](qemu_virt.cpp.md#fn-72e31996ffc2b022)
- [`xv::drivers::console`](qemu_virt.cpp.md#fn-cb18263197838102)
- [`xv::drivers::handle_external_interrupt`](qemu_virt.cpp.md#fn-8819b4e9b6bdacb6)
- [`xv::drivers::initialize_console`](qemu_virt.cpp.md#fn-cacccb8b1ed2107b)
- [`xv::drivers::initialize_hart`](qemu_virt.cpp.md#fn-bea9233c904d33be)
- [`xv::drivers::initialize_interrupts`](qemu_virt.cpp.md#fn-ea116721d09e5c14)
- [`xv::drivers::initialize_network`](qemu_virt.cpp.md#fn-b676d9440f8d6421)
- [`xv::drivers::interrupt_controller`](qemu_virt.cpp.md#fn-276393f76c615b60)
- [`xv::drivers::network_device`](qemu_virt.cpp.md#fn-59f08ec5772b85b6)
- [`xv::drivers::serial_port`](qemu_virt.cpp.md#fn-ee46a944352f0df2)
- [`xv::drivers::terminal_timer_tick`](qemu_virt.cpp.md#fn-a345578c92b3adb7)

<a id="fn-cacccb8b1ed2107b"></a>

## `xv::drivers::initialize_console`

```doxygen
@fn void xv::drivers::initialize_console()
@brief 设备对象、MMIO、设备中断与终端数据流：初始化控制台。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`terminal.initialize` → `initialize_ptys` → `uart.attach` → `uart.initialize` → `xv::fs::devices` → `attach`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ea116721d09e5c14"></a>

## `xv::drivers::initialize_interrupts`

```doxygen
@fn void xv::drivers::initialize_interrupts()
@brief 设备对象、MMIO、设备中断与终端数据流：初始化interrupts。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`router.initialize`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bea9233c904d33be"></a>

## `xv::drivers::initialize_hart`

```doxygen
@fn void xv::drivers::initialize_hart(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：建立当前 hart 的局部运行状态。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`router.initialize_hart`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8819b4e9b6bdacb6"></a>

## `xv::drivers::handle_external_interrupt`

```doxygen
@fn void xv::drivers::handle_external_interrupt(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：handle外部中断。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`router.dispatch` → `xv::print::printk`。
@details 需要特别跟踪的条件：`unknown`。这些条件决定检查、重试或提前返回路径。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`terminal.timer_tick` → `pty_timer_tick`。
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

<a id="fn-b676d9440f8d6421"></a>

## `xv::drivers::initialize_network`

```doxygen
@fn void xv::drivers::initialize_network(EthernetReceiver &receiver)
@brief 连接协议接收者并在启动阶段初始化板载 E1000。
@param receiver 生命周期覆盖整个运行期的协议接收者。
@pre 分配器、内核映射、进程等待机制已初始化，其他 hart 尚未进入调度。
@details 先 attach，再初始化网卡；没有设备或分配失败时保留离线状态。
```

<a id="fn-59f08ec5772b85b6"></a>

## `xv::drivers::network_device`

```doxygen
@fn EthernetDevice &xv::drivers::network_device()
@brief 取得 QEMU virt 板级唯一以太网设备。
@return 静态 EthernetDevice 引用；须通过 online 查询是否可收发。
```
