# `kernel/process/groups/terminal_control.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/terminal_control.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:groups`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessGroups::ClaimTerminalFlow::assigned`](terminal_control.cpp.md#fn-e67c259cdb67faac)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::available`](terminal_control.cpp.md#fn-b0041d173feecc77)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::existing`](terminal_control.cpp.md#fn-340dd2a7634ad639)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::leader`](terminal_control.cpp.md#fn-42c966b63b9a6364)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::publish`](terminal_control.cpp.md#fn-fbe84780b9c4e181)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::session`](terminal_control.cpp.md#fn-d0f3cbcba7186101)
- [`xv::process::ProcessGroups::ClaimTerminalFlow::valid`](terminal_control.cpp.md#fn-228f5d4dff30f7ae)
- [`xv::process::ProcessGroups::ForegroundFlow::find`](terminal_control.cpp.md#fn-5756a403fae1084e)
- [`xv::process::ProcessGroups::ForegroundFlow::permitted`](terminal_control.cpp.md#fn-b19016f4c8d8ff50)
- [`xv::process::ProcessGroups::ForegroundFlow::publish`](terminal_control.cpp.md#fn-79df6da45e3650e0)
- [`xv::process::ProcessGroups::ForegroundFlow::valid`](terminal_control.cpp.md#fn-a36124bedf7f6dae)
- [`xv::process::ProcessGroups::claim_terminal`](terminal_control.cpp.md#fn-8a56e2b14cf9b154)
- [`xv::process::ProcessGroups::detach_terminal`](terminal_control.cpp.md#fn-f799f3318f792fa1)
- [`xv::process::ProcessGroups::exit_terminal`](terminal_control.cpp.md#fn-a4ca6b6dd1789097)
- [`xv::process::ProcessGroups::foreground`](terminal_control.cpp.md#fn-da38cc2bc5cbb6be)
- [`xv::process::ProcessGroups::may_read_terminal`](terminal_control.cpp.md#fn-7e21e578da02ef0b)
- [`xv::process::ProcessGroups::set_foreground`](terminal_control.cpp.md#fn-aeb712077b7a077b)
- [`xv::process::ProcessGroups::visit_foreground`](terminal_control.cpp.md#fn-f94a9086969893f9)

<a id="fn-8a56e2b14cf9b154"></a>

## `xv::process::ProcessGroups::claim_terminal`

```doxygen
@fn int xv::process::ProcessGroups::claim_terminal(const ProcessMembership &caller, void *terminal)
@brief 会话和进程组登记：认领终端。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`-1`；`session->terminal == terminal ? 0 : -1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ClaimTerminalFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-da38cc2bc5cbb6be"></a>

## `xv::process::ProcessGroups::foreground`

```doxygen
@fn int xv::process::ProcessGroups::foreground(const ProcessMembership &caller, void *terminal)
@brief 会话和进程组登记：前台组。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 需要特别跟踪的条件：`!caller.group_ || caller.group_->session->terminal != terminal`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`caller.group_->session->foreground`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-aeb712077b7a077b"></a>

## `xv::process::ProcessGroups::set_foreground`

```doxygen
@fn int xv::process::ProcessGroups::set_foreground(const ProcessMembership &caller, void *terminal, int id)
@brief 会话和进程组登记：设置前台组。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param id 本次操作使用的 身份编号 值。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ForegroundFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-7e21e578da02ef0b"></a>

## `xv::process::ProcessGroups::may_read_terminal`

```doxygen
@fn bool xv::process::ProcessGroups::may_read_terminal(const ProcessMembership &caller, void *terminal)
@brief 会话和进程组登记：may读取终端。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 需要特别跟踪的条件：`!caller.group_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`session->terminal != terminal || session->foreground == caller.group_->id`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a4ca6b6dd1789097"></a>

## `xv::process::ProcessGroups::exit_terminal`

```doxygen
@fn void xv::process::ProcessGroups::exit_terminal(const ProcessMembership &caller)
@brief 会话和进程组登记：退出终端。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 本函数可见的状态更新包括：`caller.group_->session->terminal = nullptr;`；`caller.group_->session->foreground = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`caller.group_ && caller.pid_ == caller.group_->session->id`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f799f3318f792fa1"></a>

## `xv::process::ProcessGroups::detach_terminal`

```doxygen
@fn void xv::process::ProcessGroups::detach_terminal(void *terminal)
@brief 会话和进程组登记：解除关联终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 本函数可见的状态更新包括：`session->terminal = nullptr;`；`session->foreground = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`session->terminal == terminal`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f94a9086969893f9"></a>

## `xv::process::ProcessGroups::visit_foreground`

```doxygen
@fn void xv::process::ProcessGroups::visit_foreground(void *terminal, GroupVisitor &visitor)
@brief 会话和进程组登记：visit前台组。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param visitor 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `visitor.visit`。
@details 需要特别跟踪的条件：`group->session->terminal == terminal && group->session->foreground == group->id`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e67c259cdb67faac"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::assigned`

```doxygen
@fn static bool xv::process::ProcessGroups::ClaimTerminalFlow::assigned(Context &c) noexcept
@brief 检查会话是否已有控制终端。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b0041d173feecc77"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::available`

```doxygen
@fn static bool xv::process::ProcessGroups::ClaimTerminalFlow::available(Context &c) noexcept
@brief 直接扫描所有组，检查没有其他会话占用该终端。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-340dd2a7634ad639"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::existing`

```doxygen
@fn static void xv::process::ProcessGroups::ClaimTerminalFlow::existing(Context &c) noexcept
@brief 按现有控制终端是否相同记录零或负一结果。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-42c966b63b9a6364"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::leader`

```doxygen
@fn static bool xv::process::ProcessGroups::ClaimTerminalFlow::leader(Context &c) noexcept
@brief 检查调用者 PID 等于会话 ID。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fbe84780b9c4e181"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::publish`

```doxygen
@fn static void xv::process::ProcessGroups::ClaimTerminalFlow::publish(Context &c) noexcept
@brief 绑定会话控制终端与调用者前台组并记录成功。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d0f3cbcba7186101"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::session`

```doxygen
@fn static void xv::process::ProcessGroups::ClaimTerminalFlow::session(Context &c) noexcept
@brief 选择调用者所属会话。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-228f5d4dff30f7ae"></a>

## `xv::process::ProcessGroups::ClaimTerminalFlow::valid`

```doxygen
@fn static bool xv::process::ProcessGroups::ClaimTerminalFlow::valid(Context &c) noexcept
@brief 检查终端非空且调用者已经登记。
@details 入口 ScopedLock 持组锁；只允许会话 leader 认领非空终端，已有相同绑定直接成功，发布前直接检查全局会话的排他绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5756a403fae1084e"></a>

## `xv::process::ProcessGroups::ForegroundFlow::find`

```doxygen
@fn static void xv::process::ProcessGroups::ForegroundFlow::find(Context &c) noexcept
@brief 直接查找指定进程组。
@details 入口 ScopedLock 持组锁，按原 ID、成员和控制终端约束验证，再在同一会话内选择组并发布前台组号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b19016f4c8d8ff50"></a>

## `xv::process::ProcessGroups::ForegroundFlow::permitted`

```doxygen
@fn static bool xv::process::ProcessGroups::ForegroundFlow::permitted(Context &c) noexcept
@brief 检查组存在且属于调用者会话。
@details 入口 ScopedLock 持组锁，按原 ID、成员和控制终端约束验证，再在同一会话内选择组并发布前台组号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-79df6da45e3650e0"></a>

## `xv::process::ProcessGroups::ForegroundFlow::publish`

```doxygen
@fn static void xv::process::ProcessGroups::ForegroundFlow::publish(Context &c) noexcept
@brief 发布会话前台组号并记录零成功码。
@details 入口 ScopedLock 持组锁，按原 ID、成员和控制终端约束验证，再在同一会话内选择组并发布前台组号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a36124bedf7f6dae"></a>

## `xv::process::ProcessGroups::ForegroundFlow::valid`

```doxygen
@fn static bool xv::process::ProcessGroups::ForegroundFlow::valid(Context &c) noexcept
@brief 检查组号为正、调用者已登记且控制终端匹配。
@details 入口 ScopedLock 持组锁，按原 ID、成员和控制终端约束验证，再在同一会话内选择组并发布前台组号。 下一节点由图策略声明，业务结果与控制信号分离。
```
