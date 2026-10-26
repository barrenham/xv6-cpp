# `kernel/trap/handlers/external_interrupt.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/handlers/external_interrupt.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:handlers`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::ExternalInterruptHandler::accepts`](external_interrupt.cpp.md#fn-3dbf6fac56b35930)
- [`xv::trap::ExternalInterruptHandler::handle`](external_interrupt.cpp.md#fn-1178044b9798eeb1)

<a id="fn-3dbf6fac56b35930"></a>

## `xv::trap::ExternalInterruptHandler::accepts`

```doxygen
@fn bool xv::trap::ExternalInterruptHandler::accepts(const TrapEvent &event) const
@brief 外部中断处理器：判断该处理器是否负责当前陷阱原因。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`event.cause.kind`。
@return 各分支实际返回：`event.cause.kind() == CauseKind::ExternalInterrupt`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1178044b9798eeb1"></a>

## `xv::trap::ExternalInterruptHandler::handle`

```doxygen
@fn TrapAction xv::trap::ExternalInterruptHandler::handle(const TrapEvent &)
@brief 外部中断处理器：读取或计算 `TrapAction::Resume`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::dispatch_external_interrupt`。
@return 各分支实际返回：`TrapAction::Resume`。应结合上面的条件检查区分正常值、空值和错误值。
```
