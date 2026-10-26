# `kernel/process/groups/process_groups.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/process_groups.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:groups`, `:group_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessGroups::allocate_group`](process_groups.cpp.md#fn-1dbdc5852a1b5676)
- [`xv::process::ProcessGroups::attach`](process_groups.cpp.md#fn-99e88e769ce178e2)
- [`xv::process::ProcessGroups::create_initial`](process_groups.cpp.md#fn-0b49bfae1c638cc0)
- [`xv::process::ProcessGroups::create_session`](process_groups.cpp.md#fn-aa7ea8358c3e4e35)
- [`xv::process::ProcessGroups::detach`](process_groups.cpp.md#fn-73bbed264e0c09a5)
- [`xv::process::ProcessGroups::did_exec`](process_groups.cpp.md#fn-38e8e67dcb920d6f)
- [`xv::process::ProcessGroups::find_group`](process_groups.cpp.md#fn-2e0fd0545f91a391)
- [`xv::process::ProcessGroups::find_member`](process_groups.cpp.md#fn-17454b9eb064996b)
- [`xv::process::ProcessGroups::get_group`](process_groups.cpp.md#fn-ad044f30e0ca5762)
- [`xv::process::ProcessGroups::get_session`](process_groups.cpp.md#fn-8a811a35de5d5380)
- [`xv::process::ProcessGroups::inherit`](process_groups.cpp.md#fn-fb259ac360aabda0)
- [`xv::process::ProcessGroups::initialize`](process_groups.cpp.md#fn-64a3ff755b49e0b6)
- [`xv::process::ProcessGroups::release`](process_groups.cpp.md#fn-59b7282a49e6dc1a)
- [`xv::process::ProcessGroups::reparent`](process_groups.cpp.md#fn-02fe7151a3441d92)
- [`xv::process::ProcessGroups::set_group`](process_groups.cpp.md#fn-e0e5fac1455dc712)
- [`xv::process::ProcessGroups::visit_group`](process_groups.cpp.md#fn-13b8d39cbdbc6f32)

<a id="fn-64a3ff755b49e0b6"></a>

## `xv::process::ProcessGroups::initialize`

```doxygen
@fn void xv::process::ProcessGroups::initialize()
@brief 会话和进程组登记：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2e0fd0545f91a391"></a>

## `xv::process::ProcessGroups::find_group`

```doxygen
@fn ProcessGroup *xv::process::ProcessGroups::find_group(int id)
@brief 会话和进程组登记：查找进程组。
@param id 本次操作使用的 身份编号 值。
@details 需要特别跟踪的条件：`group->id == id`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`group`；`nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-17454b9eb064996b"></a>

## `xv::process::ProcessGroups::find_member`

```doxygen
@fn ProcessMembership *xv::process::ProcessGroups::find_member(int pid)
@brief 会话和进程组登记：查找成员。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 需要特别跟踪的条件：`member->pid_ == pid`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`member`；`nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1dbdc5852a1b5676"></a>

## `xv::process::ProcessGroups::allocate_group`

```doxygen
@fn ProcessGroup *xv::process::ProcessGroups::allocate_group(int id, Session &session)
@brief 会话和进程组登记：分配进程组。
@param id 本次操作使用的 身份编号 值。
@param session 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::allocate_group_page` → `new`。
@details 本函数可见的状态更新包括：`group->next = groups_;`；`groups_ = group;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!page`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`nullptr`；`group`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-99e88e769ce178e2"></a>

## `xv::process::ProcessGroups::attach`

```doxygen
@fn void xv::process::ProcessGroups::attach(ProcessMembership &member, ProcessGroup &group)
@brief 会话和进程组登记：关联后端对象、接收器或组成员。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param group 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 本函数可见的状态更新包括：`member.group_ = &group;`；`member.next_ = group.members;`；`group.members = &member;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-73bbed264e0c09a5"></a>

## `xv::process::ProcessGroups::detach`

```doxygen
@fn void xv::process::ProcessGroups::detach(ProcessMembership &member)
@brief 会话和进程组登记：解除关联。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::group_fail` → `ProcessGroup` → `detail::free_group_page` → `Session`。
@details 本函数可见的状态更新包括：`member.group_ = nullptr;`；`member.next_ = nullptr;`；`group->session->foreground = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`*link && *link != &member`；`!*link`；`group->members`；`*entry && *entry != group`；`!*entry`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0b49bfae1c638cc0"></a>

## `xv::process::ProcessGroups::create_initial`

```doxygen
@fn bool xv::process::ProcessGroups::create_initial(ProcessMembership &member, int pid)
@brief 会话和进程组登记：创建initial。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::group_fail` → `detail::allocate_group_page` → `new` → `allocate_group` → `Session` → `detail::free_group_page` → `attach`。
@details 本函数可见的状态更新包括：`member.pid_ = pid;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`member.group_ || groups_ || pid <= 0`；`!page`；`!group`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fb259ac360aabda0"></a>

## `xv::process::ProcessGroups::inherit`

```doxygen
@fn void xv::process::ProcessGroups::inherit(ProcessMembership &child, int pid, const ProcessMembership &parent)
@brief 会话和进程组登记：继承。
@param child 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param parent 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::group_fail` → `attach`。
@details 本函数可见的状态更新包括：`child.pid_ = pid;`；`child.parent_ = parent.pid_;`；`child.execed_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`child.group_ || !parent.group_ || pid <= 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-02fe7151a3441d92"></a>

## `xv::process::ProcessGroups::reparent`

```doxygen
@fn void xv::process::ProcessGroups::reparent(ProcessMembership &child, int parent)
@brief 会话和进程组登记：更换父关系。
@param child 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param parent 本次操作使用的 parent 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 本函数可见的状态更新包括：`child.parent_ = parent;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-38e8e67dcb920d6f"></a>

## `xv::process::ProcessGroups::did_exec`

```doxygen
@fn void xv::process::ProcessGroups::did_exec(ProcessMembership &member)
@brief 会话和进程组登记：did执行映像。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::group_fail`。
@details 本函数可见的状态更新包括：`member.execed_ = true;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!member.group_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-59b7282a49e6dc1a"></a>

## `xv::process::ProcessGroups::release`

```doxygen
@fn void xv::process::ProcessGroups::release(ProcessMembership &member)
@brief 会话和进程组登记：释放当前引用或归还对象占用的资源。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detach`。
@details 本函数可见的状态更新包括：`member.pid_ = member.parent_ = 0;`；`member.parent_ = 0;`；`member.execed_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`member.group_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ad044f30e0ca5762"></a>

## `xv::process::ProcessGroups::get_group`

```doxygen
@fn int xv::process::ProcessGroups::get_group(const ProcessMembership &caller)
@brief 会话和进程组登记：读取或计算 `caller.group_ ? caller.group_->id : -1`，返回当前查询结果。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@return 各分支实际返回：`caller.group_ ? caller.group_->id : -1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8a811a35de5d5380"></a>

## `xv::process::ProcessGroups::get_session`

```doxygen
@fn int xv::process::ProcessGroups::get_session(const ProcessMembership &caller, int pid)
@brief 会话和进程组登记：取得会话。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `find_member`。
@details 需要特别跟踪的条件：`pid < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`member && member->group_ ? member->group_->session->id : -1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e0e5fac1455dc712"></a>

## `xv::process::ProcessGroups::set_group`

```doxygen
@fn int xv::process::ProcessGroups::set_group(ProcessMembership &caller, int pid, int id)
@brief 会话和进程组登记：设置进程组。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param id 本次操作使用的 身份编号 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `find_member` → `find_group` → `allocate_group` → `detach` → `attach`。
@details 需要特别跟踪的条件：`pid < 0 || id < 0`；`!caller.group_`；`!member || !member->group_`；`member != &caller && (member->parent_ != caller.pid_ || member->execed_)`；`member->group_->session != session || member->pid_ == session->id`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-13b8d39cbdbc6f32"></a>

## `xv::process::ProcessGroups::visit_group`

```doxygen
@fn void xv::process::ProcessGroups::visit_group(const ProcessMembership &caller, int id, GroupVisitor &visitor)
@brief 会话和进程组登记：visit进程组。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param id 本次操作使用的 身份编号 值。
@param visitor 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `find_group` → `visitor.visit`。
@details 需要特别跟踪的条件：`!group`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-aa7ea8358c3e4e35"></a>

## `xv::process::ProcessGroups::create_session`

```doxygen
@fn int xv::process::ProcessGroups::create_session(ProcessMembership &caller)
@brief 会话和进程组登记：创建会话。
@param caller 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `find_group` → `detail::allocate_group_page` → `new` → `allocate_group` → `Session` → `detail::free_group_page` → `detach` → `attach`。
@details 需要特别跟踪的条件：`!caller.group_ || find_group(caller.pid_)`；`!page`；`!group`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`session->id`。应结合上面的条件检查区分正常值、空值和错误值。
```
