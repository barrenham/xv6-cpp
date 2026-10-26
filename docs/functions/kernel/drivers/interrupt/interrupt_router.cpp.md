# `kernel/drivers/interrupt/interrupt_router.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/interrupt_router.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::InterruptRouter::dispatch`](interrupt_router.cpp.md#fn-d2d94b71a3549485)
- [`xv::drivers::InterruptRouter::initialize`](interrupt_router.cpp.md#fn-8e59e19503e86de5)
- [`xv::drivers::InterruptRouter::initialize_hart`](interrupt_router.cpp.md#fn-4efe08b4f15644d0)

<a id="fn-8e59e19503e86de5"></a>

## `xv::drivers::InterruptRouter::initialize`

```doxygen
@fn void xv::drivers::InterruptRouter::initialize()
@brief 外部中断路由器：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`irq` → `detail::fail` → `controller_.initialize` → `controller_.set_priority`。
@details 需要特别跟踪的条件：`!devices_[i] || devices_[i]->irq() == 0`；`devices_[i]->irq() == devices_[j]->irq()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4efe08b4f15644d0"></a>

## `xv::drivers::InterruptRouter::initialize_hart`

```doxygen
@fn void xv::drivers::InterruptRouter::initialize_hart(int hart)
@brief 外部中断路由器：建立当前 hart 的局部运行状态。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`controller_.initialize_hart` → `controller_.enable` → `irq`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d2d94b71a3549485"></a>

## `xv::drivers::InterruptRouter::dispatch`

```doxygen
@fn unsigned xv::drivers::InterruptRouter::dispatch(int hart)
@brief 外部中断路由器：按输入事件或系统调用编号选择处理路径。
@param hart 本次操作针对的 hart 编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`controller_.claim` → `irq` → `handle_interrupt` → `controller_.complete`。
@details 需要特别跟踪的条件：`irq == 0`；`devices_[i]->irq() == irq`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`unknown`。应结合上面的条件检查区分正常值、空值和错误值。
```
