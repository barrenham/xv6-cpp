# `kernel/drivers/interrupt/interrupt_controller.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/interrupt_controller.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:interrupt`。

直接模块依赖：`:device`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::InterruptController::InterruptController`](interrupt_controller.cppm.md#fn-61fb86332d4a34d8)
- [`xv::drivers::InterruptController::claim`](interrupt_controller.cppm.md#fn-fb3b2e0b091e77c3)
- [`xv::drivers::InterruptController::complete`](interrupt_controller.cppm.md#fn-b1ef31032e99e282)
- [`xv::drivers::InterruptController::enable`](interrupt_controller.cppm.md#fn-b5571316b525cb4f)
- [`xv::drivers::InterruptController::initialize_hart`](interrupt_controller.cppm.md#fn-fed35aad42080609)
- [`xv::drivers::InterruptController::set_priority`](interrupt_controller.cppm.md#fn-ea5f39f32832642f)
- [`xv::drivers::InterruptController::~InterruptController`](interrupt_controller.cppm.md#fn-4c8dc54ec88b68ca)
- [`xv::drivers::InterruptDevice::InterruptDevice`](interrupt_controller.cppm.md#fn-fd35a627dc0f31ca)
- [`xv::drivers::InterruptDevice::handle_interrupt`](interrupt_controller.cppm.md#fn-aecd87060492cbd2)
- [`xv::drivers::InterruptDevice::irq`](interrupt_controller.cppm.md#fn-c2a81e02cefb24f3)
- [`xv::drivers::InterruptDevice::~InterruptDevice`](interrupt_controller.cppm.md#fn-2c730f5af34bfeb0)

<a id="fn-c2a81e02cefb24f3"></a>

## `xv::drivers::InterruptDevice::irq`

```doxygen
@fn virtual unsigned xv::drivers::InterruptDevice::irq() const
@brief 设备对象、MMIO、设备中断与终端数据流：中断号。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `unsigned int () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-aecd87060492cbd2"></a>

## `xv::drivers::InterruptDevice::handle_interrupt`

```doxygen
@fn virtual void xv::drivers::InterruptDevice::handle_interrupt()
@brief 设备对象、MMIO、设备中断与终端数据流：响应设备中断并完成或推进挂起的设备操作。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-fd35a627dc0f31ca"></a>

## `xv::drivers::InterruptDevice::InterruptDevice`

```doxygen
@fn constexpr xv::drivers::InterruptDevice::InterruptDevice()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2c730f5af34bfeb0"></a>

## `xv::drivers::InterruptDevice::~InterruptDevice`

```doxygen
@fn xv::drivers::InterruptDevice::~InterruptDevice()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ea5f39f32832642f"></a>

## `xv::drivers::InterruptController::set_priority`

```doxygen
@fn virtual void xv::drivers::InterruptController::set_priority(unsigned irq, unsigned priority)
@brief 设备对象、MMIO、设备中断与终端数据流：配置中断源的投递优先级。
@param irq 本次操作使用的 中断号 值。
@param priority 本次操作使用的 priority 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-fed35aad42080609"></a>

## `xv::drivers::InterruptController::initialize_hart`

```doxygen
@fn virtual void xv::drivers::InterruptController::initialize_hart(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：建立当前 hart 的局部运行状态。
@param hart 本次操作针对的 hart 编号。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b5571316b525cb4f"></a>

## `xv::drivers::InterruptController::enable`

```doxygen
@fn virtual void xv::drivers::InterruptController::enable(int hart, unsigned irq)
@brief 设备对象、MMIO、设备中断与终端数据流：允许指定 hart 接收某个外部中断源。
@param hart 本次操作针对的 hart 编号。
@param irq 本次操作使用的 中断号 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-fb3b2e0b091e77c3"></a>

## `xv::drivers::InterruptController::claim`

```doxygen
@fn virtual unsigned xv::drivers::InterruptController::claim(int hart)
@brief 设备对象、MMIO、设备中断与终端数据流：领取待处理外部中断或认领空闲存储块。
@param hart 本次操作针对的 hart 编号。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `unsigned int (int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-b1ef31032e99e282"></a>

## `xv::drivers::InterruptController::complete`

```doxygen
@fn virtual void xv::drivers::InterruptController::complete(int hart, unsigned irq)
@brief 设备对象、MMIO、设备中断与终端数据流：向中断控制器确认中断处理完成。
@param hart 本次操作针对的 hart 编号。
@param irq 本次操作使用的 中断号 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-61fb86332d4a34d8"></a>

## `xv::drivers::InterruptController::InterruptController`

```doxygen
@fn constexpr xv::drivers::InterruptController::InterruptController()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4c8dc54ec88b68ca"></a>

## `xv::drivers::InterruptController::~InterruptController`

```doxygen
@fn xv::drivers::InterruptController::~InterruptController()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
