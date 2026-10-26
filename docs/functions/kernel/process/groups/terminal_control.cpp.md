# `kernel/process/groups/terminal_control.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/terminal_control.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:groups`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 本函数可见的状态更新包括：`session->terminal = terminal;`；`session->foreground = caller.group_->id;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!terminal || !caller.group_`；`caller.pid_ != session->id`；`session->terminal`；`group->session->terminal == terminal`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`session->terminal == terminal ? 0 : -1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `find_group`。
@details 本函数可见的状态更新包括：`group->session->foreground = id;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`id <= 0 || !caller.group_ || caller.group_->session->terminal != terminal`；`!group || group->session != caller.group_->session`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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
