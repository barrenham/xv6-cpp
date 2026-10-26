# `kernel/drivers/interrupt/plic.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/plic.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:plic`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Plic::claim`](plic.cpp.md#fn-2f59f51d7a3e8b75)
- [`xv::drivers::Plic::complete`](plic.cpp.md#fn-35f1dc5c24401299)
- [`xv::drivers::Plic::enable`](plic.cpp.md#fn-dde1bb3588499d54)
- [`xv::drivers::Plic::initialize`](plic.cpp.md#fn-8371618adfdd12ad)
- [`xv::drivers::Plic::initialize_hart`](plic.cpp.md#fn-ad5f053e65d2ed98)
- [`xv::drivers::Plic::set_priority`](plic.cpp.md#fn-cdb74cfb3aaaa599)

<a id="fn-8371618adfdd12ad"></a>

## `xv::drivers::Plic::initialize`

```doxygen
@fn void xv::drivers::Plic::initialize()
@brief PLIC 外部中断控制器：建立该对象开始工作所需的初始状态。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-cdb74cfb3aaaa599"></a>

## `xv::drivers::Plic::set_priority`

```doxygen
@fn void xv::drivers::Plic::set_priority(unsigned irq, unsigned priority)
@brief PLIC 外部中断控制器：配置中断源的投递优先级。
@param irq 本次操作使用的 中断号 值。
@param priority 本次操作使用的 priority 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_.write32`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ad5f053e65d2ed98"></a>

## `xv::drivers::Plic::initialize_hart`

```doxygen
@fn void xv::drivers::Plic::initialize_hart(int hart)
@brief PLIC 外部中断控制器：建立当前 hart 的局部运行状态。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_.write32`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-dde1bb3588499d54"></a>

## `xv::drivers::Plic::enable`

```doxygen
@fn void xv::drivers::Plic::enable(int hart, unsigned irq)
@brief PLIC 外部中断控制器：允许指定 hart 接收某个外部中断源。
@param hart 本次操作针对的 hart 编号。
@param irq 本次操作使用的 中断号 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_.read32` → `registers_.write32`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2f59f51d7a3e8b75"></a>

## `xv::drivers::Plic::claim`

```doxygen
@fn unsigned xv::drivers::Plic::claim(int hart)
@brief PLIC 外部中断控制器：领取目标 hart 待处理的外部中断源编号。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_.read32`。
@return 各分支实际返回：`registers_.read32(0x201004 + hart * 0x2000)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-35f1dc5c24401299"></a>

## `xv::drivers::Plic::complete`

```doxygen
@fn void xv::drivers::Plic::complete(int hart, unsigned irq)
@brief PLIC 外部中断控制器：向中断控制器确认中断处理完成。
@param hart 本次操作针对的 hart 编号。
@param irq 本次操作使用的 中断号 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_.write32`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
