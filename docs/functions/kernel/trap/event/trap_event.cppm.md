# `kernel/trap/event/trap_event.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/event/trap_event.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap:event`。

## 函数导航

- [`xv::trap::TrapCause::TrapCause`](trap_event.cppm.md#fn-f06098b36dba35b2)
- [`xv::trap::TrapCause::code`](trap_event.cppm.md#fn-cb63a037ce21c570)
- [`xv::trap::TrapCause::is_interrupt`](trap_event.cppm.md#fn-4e9bb1d4b6b8a168)
- [`xv::trap::TrapCause::kind`](trap_event.cppm.md#fn-bb08adb0909794a4)
- [`xv::trap::TrapCause::raw`](trap_event.cppm.md#fn-e3e2eb4e8049d2e5)
- [`xv::trap::TrapEvent::from_supervisor`](trap_event.cppm.md#fn-b2e7b3fb3cbc4c03)
- [`xv::trap::TrapEvent::interrupts_enabled`](trap_event.cppm.md#fn-5addef74e108fed6)

<a id="fn-f06098b36dba35b2"></a>

## `xv::trap::TrapCause::TrapCause`

```doxygen
@fn constexpr explicit xv::trap::TrapCause::TrapCause(Word value = 0)
@brief 构造陷阱原因值对象，建立本重载要求的成员初值和借用关系。
@param value 本次操作使用的 value 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`value_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e3e2eb4e8049d2e5"></a>

## `xv::trap::TrapCause::raw`

```doxygen
@fn constexpr Word xv::trap::TrapCause::raw() const
@brief 陷阱原因值对象：读取或计算 `value_`，返回当前查询结果。
@return 各分支实际返回：`value_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4e9bb1d4b6b8a168"></a>

## `xv::trap::TrapCause::is_interrupt`

```doxygen
@fn constexpr bool xv::trap::TrapCause::is_interrupt() const
@brief 陷阱原因值对象：读取或计算 `(value_ & InterruptBit) != 0`，返回当前查询结果。
@return 各分支实际返回：`(value_ & InterruptBit) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cb63a037ce21c570"></a>

## `xv::trap::TrapCause::code`

```doxygen
@fn constexpr Word xv::trap::TrapCause::code() const
@brief 陷阱原因值对象：读取或计算 `value_ & ~InterruptBit`，返回当前查询结果。
@return 各分支实际返回：`value_ & ~InterruptBit`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bb08adb0909794a4"></a>

## `xv::trap::TrapCause::kind`

```doxygen
@fn constexpr CauseKind xv::trap::TrapCause::kind() const
@brief 陷阱原因值对象：种类。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`is_interrupt` → `code`。
@details 需要特别跟踪的条件：`is_interrupt()`；`code() == 9`；`code() == 5`；`code() == 8`；`code() == 13`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`CauseKind::ExternalInterrupt`；`CauseKind::TimerInterrupt`；`CauseKind::UserSystemCall`；`CauseKind::LoadPageFault`；`CauseKind::StorePageFault`；`CauseKind::Other`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b2e7b3fb3cbc4c03"></a>

## `xv::trap::TrapEvent::from_supervisor`

```doxygen
@fn constexpr bool xv::trap::TrapEvent::from_supervisor() const
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `(status & SupervisorPrevious) != 0`，返回当前查询结果。
@return 各分支实际返回：`(status & SupervisorPrevious) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5addef74e108fed6"></a>

## `xv::trap::TrapEvent::interrupts_enabled`

```doxygen
@fn constexpr bool xv::trap::TrapEvent::interrupts_enabled() const
@brief 陷阱快照、事件分派、设备服务及特权级返回：读取或计算 `(status & InterruptEnable) != 0`，返回当前查询结果。
@return 各分支实际返回：`(status & InterruptEnable) != 0`。应结合上面的条件检查区分正常值、空值和错误值。
```
