# `kernel/drivers/interrupt/interrupt_router.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/interrupt/interrupt_router.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::InterruptRouter::DispatchFlow::claim`](interrupt_router.cpp.md#fn-a41d9e8cebb17bcd)
- [`xv::drivers::InterruptRouter::DispatchFlow::claimed`](interrupt_router.cpp.md#fn-c7ba712428b47daf)
- [`xv::drivers::InterruptRouter::DispatchFlow::complete`](interrupt_router.cpp.md#fn-3568ef82a369f7bd)
- [`xv::drivers::InterruptRouter::DispatchFlow::route`](interrupt_router.cpp.md#fn-eca64cb56df4af52)
- [`xv::drivers::InterruptRouter::InitializeFlow::controller`](interrupt_router.cpp.md#fn-85aacb9aa7cfc4f0)
- [`xv::drivers::InterruptRouter::InitializeFlow::priorities`](interrupt_router.cpp.md#fn-52ba72002545d48f)
- [`xv::drivers::InterruptRouter::InitializeFlow::validate`](interrupt_router.cpp.md#fn-9d8e4da6d635c53c)
- [`xv::drivers::InterruptRouter::dispatch`](interrupt_router.cpp.md#fn-d2d94b71a3549485)
- [`xv::drivers::InterruptRouter::initialize`](interrupt_router.cpp.md#fn-8e59e19503e86de5)
- [`xv::drivers::InterruptRouter::initialize_hart`](interrupt_router.cpp.md#fn-4efe08b4f15644d0)

<a id="fn-8e59e19503e86de5"></a>

## `xv::drivers::InterruptRouter::initialize`

```doxygen
@fn void xv::drivers::InterruptRouter::initialize()
@brief 外部中断路由器：建立该对象开始工作所需的初始状态。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 InitializeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@return 各分支实际返回：`0`；`unknown`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DispatchFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-a41d9e8cebb17bcd"></a>

## `xv::drivers::InterruptRouter::DispatchFlow::claim`

```doxygen
@fn static void xv::drivers::InterruptRouter::DispatchFlow::claim(Context &c) noexcept
@brief 从原控制器为本 hart 取得一个 IRQ。
@details 只 claim 一次，零 IRQ 直接返回且不 complete；非零 IRQ 查找并调用最多一个原处理器，处理器返回后无论是否已知都 complete 一次。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c7ba712428b47daf"></a>

## `xv::drivers::InterruptRouter::DispatchFlow::claimed`

```doxygen
@fn static bool xv::drivers::InterruptRouter::DispatchFlow::claimed(Context &c) noexcept
@brief 检查本次 IRQ 是否非零。
@details 只 claim 一次，零 IRQ 直接返回且不 complete；非零 IRQ 查找并调用最多一个原处理器，处理器返回后无论是否已知都 complete 一次。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3568ef82a369f7bd"></a>

## `xv::drivers::InterruptRouter::DispatchFlow::complete`

```doxygen
@fn static void xv::drivers::InterruptRouter::DispatchFlow::complete(Context &c) noexcept
@brief 在处理器返回后完成本次非零 IRQ。
@details 只 claim 一次，零 IRQ 直接返回且不 complete；非零 IRQ 查找并调用最多一个原处理器，处理器返回后无论是否已知都 complete 一次。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eca64cb56df4af52"></a>

## `xv::drivers::InterruptRouter::DispatchFlow::route`

```doxygen
@fn static void xv::drivers::InterruptRouter::DispatchFlow::route(Context &c) noexcept
@brief 直接查找源并调用匹配处理器，保留未知 IRQ 结果。
@details 只 claim 一次，零 IRQ 直接返回且不 complete；非零 IRQ 查找并调用最多一个原处理器，处理器返回后无论是否已知都 complete 一次。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-85aacb9aa7cfc4f0"></a>

## `xv::drivers::InterruptRouter::InitializeFlow::controller`

```doxygen
@fn static void xv::drivers::InterruptRouter::InitializeFlow::controller(Context &c) noexcept
@brief 在全部源检查成功后初始化控制器。
@details 固定非拥有接线表由调用者保证寿命；先完成全部有效性与重复 IRQ 检查，再初始化控制器和设置优先级，不缓存或合并原 irq 读取。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-52ba72002545d48f"></a>

## `xv::drivers::InterruptRouter::InitializeFlow::priorities`

```doxygen
@fn static void xv::drivers::InterruptRouter::InitializeFlow::priorities(Context &c) noexcept
@brief 按原源次序设置控制器优先级。
@details 固定非拥有接线表由调用者保证寿命；先完成全部有效性与重复 IRQ 检查，再初始化控制器和设置优先级，不缓存或合并原 irq 读取。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9d8e4da6d635c53c"></a>

## `xv::drivers::InterruptRouter::InitializeFlow::validate`

```doxygen
@fn static void xv::drivers::InterruptRouter::InitializeFlow::validate(Context &c) noexcept
@brief 直接检查全部源指针、非零 IRQ 和重复接线。
@details 固定非拥有接线表由调用者保证寿命；先完成全部有效性与重复 IRQ 检查，再初始化控制器和设置优先级，不缓存或合并原 irq 读取。 下一节点由图策略声明，业务结果与控制信号分离。
```
