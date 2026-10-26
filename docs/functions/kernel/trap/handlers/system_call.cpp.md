# `kernel/trap/handlers/system_call.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/handlers/system_call.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:handlers`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::SystemCallHandler::accepts`](system_call.cpp.md#fn-9f95e4acf0aa1611)
- [`xv::trap::SystemCallHandler::handle`](system_call.cpp.md#fn-92d0af62cc7bf29f)

<a id="fn-9f95e4acf0aa1611"></a>

## `xv::trap::SystemCallHandler::accepts`

```doxygen
@fn bool xv::trap::SystemCallHandler::accepts(const TrapEvent &event) const
@brief 用户系统调用处理器：判断该处理器是否负责当前陷阱原因。
@param event 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`event.from_supervisor` → `event.cause.kind`。
@return 各分支实际返回：`!event.from_supervisor() && event.cause.kind() == CauseKind::UserSystemCall`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-92d0af62cc7bf29f"></a>

## `xv::trap::SystemCallHandler::handle`

```doxygen
@fn TrapAction xv::trap::SystemCallHandler::handle(const TrapEvent &)
@brief 用户系统调用处理器：handle。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::killed` → `detail::advance_user_pc` → `detail::enable_interrupts` → `detail::dispatch_system_call`。
@details 需要特别跟踪的条件：`detail::killed()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`TrapAction::Terminate`；`TrapAction::Resume`。应结合上面的条件检查区分正常值、空值和错误值。
```
