# `kernel/trap/controller/trap_controller.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/controller/trap_controller.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::TrapController::handle_kernel`](trap_controller.cpp.md#fn-1e01ca83cbbc98f6)
- [`xv::trap::TrapController::handle_user`](trap_controller.cpp.md#fn-91cdce7331da1c4b)
- [`xv::trap::TrapController::initialize_hart`](trap_controller.cpp.md#fn-afc1472bab28e034)
- [`xv::trap::TrapController::prepare_return`](trap_controller.cpp.md#fn-0713205e94c82bbd)
- [`xv::trap::TrapController::return_to_user`](trap_controller.cpp.md#fn-d181f139ca490928)

<a id="fn-afc1472bab28e034"></a>

## `xv::trap::TrapController::initialize_hart`

```doxygen
@fn void xv::trap::TrapController::initialize_hart()
@brief 陷阱控制器：建立当前 hart 的局部运行状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::install_kernel_vector`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-91cdce7331da1c4b"></a>

## `xv::trap::TrapController::handle_user`

```doxygen
@fn Word xv::trap::TrapController::handle_user()
@brief 陷阱控制器：处理来自用户态的陷阱并准备用户页表返回值。
@details 保存用户 PC，分派系统调用/中断/缺页，再在安全边界处理信号。未知用户异常走旧退出路径；需要让出 CPU 时先调度，随后统一 prepare_return 并返回用户页表 token。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::snapshot` → `event.from_supervisor` → `detail::fatal` → `detail::install_kernel_vector` → `detail::has_process` → `detail::save_user_pc` → `dispatcher_.dispatch` → `detail::report_user_fault` → `detail::request_kill` → `detail::exit_current` → `detail::deliver_signals` → `detail::killed`。
@details 需要特别跟踪的条件：`event.from_supervisor()`；`!detail::has_process()`；`action == TrapAction::Unhandled`；`action == TrapAction::Terminate || detail::killed()`；`action == TrapAction::Yield`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`detail::user_page_table_token()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1e01ca83cbbc98f6"></a>

## `xv::trap::TrapController::handle_kernel`

```doxygen
@fn void xv::trap::TrapController::handle_kernel()
@brief 陷阱控制器：处理内核陷阱并恢复原内核 trap 快照。
@details 验证 trap 来自内核且进入时中断关闭，只接受 Resume/Yield。允许定时事件导致调度后，再恢复最初 trap 的 sepc/sstatus，防止嵌套陷阱或迁移覆盖返回现场。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::snapshot` → `event.from_supervisor` → `detail::fatal` → `event.interrupts_enabled` → `dispatcher_.dispatch` → `detail::has_process` → `detail::yield_current` → `detail::restore_kernel`。
@details 需要特别跟踪的条件：`!event.from_supervisor()`；`event.interrupts_enabled()`；`action != TrapAction::Resume && action != TrapAction::Yield`；`action == TrapAction::Yield && detail::has_process()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0713205e94c82bbd"></a>

## `xv::trap::TrapController::prepare_return`

```doxygen
@fn void xv::trap::TrapController::prepare_return()
@brief 陷阱控制器：完成返回用户态前的信号检查与架构准备。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::deliver_signals` → `detail::prepare_user_return`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d181f139ca490928"></a>

## `xv::trap::TrapController::return_to_user`

```doxygen
@fn void xv::trap::TrapController::return_to_user()
@brief 陷阱控制器：准备并进入不返回的用户态切换路径。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`prepare_return` → `detail::enter_user` → `detail::user_page_table_token`。
@return 各分支实际返回：`_to_user() { prepare_return()`。应结合上面的条件检查区分正常值、空值和错误值。
```
