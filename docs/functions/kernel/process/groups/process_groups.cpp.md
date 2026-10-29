# `kernel/process/groups/process_groups.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/process_groups.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:groups`, `:group_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessGroups::DetachFlow::drop`](process_groups.cpp.md#fn-43759bd3ddf66537)
- [`xv::process::ProcessGroups::DetachFlow::find`](process_groups.cpp.md#fn-07d45898f9aff9f1)
- [`xv::process::ProcessGroups::DetachFlow::found`](process_groups.cpp.md#fn-91c3662661a9789d)
- [`xv::process::ProcessGroups::DetachFlow::free_session`](process_groups.cpp.md#fn-6229937810dfc52a)
- [`xv::process::ProcessGroups::DetachFlow::invalid`](process_groups.cpp.md#fn-18fbce99f46ca84f)
- [`xv::process::ProcessGroups::DetachFlow::last`](process_groups.cpp.md#fn-f2d0c6e54a06dc67)
- [`xv::process::ProcessGroups::DetachFlow::unlink`](process_groups.cpp.md#fn-51897627af4bedad)
- [`xv::process::ProcessGroups::InitialFlow::invalid`](process_groups.cpp.md#fn-2151587501ad1974)
- [`xv::process::ProcessGroups::InitialFlow::publish`](process_groups.cpp.md#fn-dfa8d20884ac9987)
- [`xv::process::ProcessGroups::InitialFlow::valid`](process_groups.cpp.md#fn-7af03581b1ee35bc)
- [`xv::process::ProcessGroups::SessionFlow::eligible`](process_groups.cpp.md#fn-b1852c460ae23c0f)
- [`xv::process::ProcessGroups::SessionFlow::publish`](process_groups.cpp.md#fn-71020d2fc430284f)
- [`xv::process::ProcessGroups::SessionProtocol::allocate_group`](process_groups.cpp.md#fn-d0c9eac0f5ace98f)
- [`xv::process::ProcessGroups::SessionProtocol::allocate_session`](process_groups.cpp.md#fn-a329eb88bfd70aea)
- [`xv::process::ProcessGroups::SessionProtocol::has_group`](process_groups.cpp.md#fn-8ec0e59b7a34d40f)
- [`xv::process::ProcessGroups::SessionProtocol::has_session`](process_groups.cpp.md#fn-8afe32ea4a1c32e1)
- [`xv::process::ProcessGroups::SessionProtocol::rollback`](process_groups.cpp.md#fn-1eb3b27ede084152)
- [`xv::process::ProcessGroups::SetGroupFlow::allocate`](process_groups.cpp.md#fn-1a9d730fdfbc1c67)
- [`xv::process::ProcessGroups::SetGroupFlow::found`](process_groups.cpp.md#fn-0f3659f107815454)
- [`xv::process::ProcessGroups::SetGroupFlow::identical`](process_groups.cpp.md#fn-0184541753f171e5)
- [`xv::process::ProcessGroups::SetGroupFlow::may_create`](process_groups.cpp.md#fn-e0e11a9450db1f00)
- [`xv::process::ProcessGroups::SetGroupFlow::member`](process_groups.cpp.md#fn-b2cd4d6209fcde38)
- [`xv::process::ProcessGroups::SetGroupFlow::movable`](process_groups.cpp.md#fn-b8eb79cc3b7e87a4)
- [`xv::process::ProcessGroups::SetGroupFlow::move`](process_groups.cpp.md#fn-495dd0bc2ce6b8c9)
- [`xv::process::ProcessGroups::SetGroupFlow::permitted`](process_groups.cpp.md#fn-82828d48df6577b8)
- [`xv::process::ProcessGroups::SetGroupFlow::registered`](process_groups.cpp.md#fn-5ed7269856ad169e)
- [`xv::process::ProcessGroups::SetGroupFlow::same_session`](process_groups.cpp.md#fn-b9726007cb0dae69)
- [`xv::process::ProcessGroups::SetGroupFlow::session`](process_groups.cpp.md#fn-e03b107459f8f9a6)
- [`xv::process::ProcessGroups::SetGroupFlow::success`](process_groups.cpp.md#fn-2c0be1501bd7959d)
- [`xv::process::ProcessGroups::SetGroupFlow::target`](process_groups.cpp.md#fn-c191837a0c09b751)
- [`xv::process::ProcessGroups::SetGroupFlow::target_found`](process_groups.cpp.md#fn-2ba1383b2c391ed7)
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
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DetachFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-0b49bfae1c638cc0"></a>

## `xv::process::ProcessGroups::create_initial`

```doxygen
@fn bool xv::process::ProcessGroups::create_initial(ProcessMembership &member, int pid)
@brief 会话和进程组登记：创建initial。
@param member 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 InitialFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SetGroupFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@return 各分支实际返回：`-1`；`session->id`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SessionFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-43759bd3ddf66537"></a>

## `xv::process::ProcessGroups::DetachFlow::drop`

```doxygen
@fn static void xv::process::ProcessGroups::DetachFlow::drop(Context &c) noexcept
@brief 销毁并归还组页，然后减少会话的组计数并保存最后引用判断。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-07d45898f9aff9f1"></a>

## `xv::process::ProcessGroups::DetachFlow::find`

```doxygen
@fn static void xv::process::ProcessGroups::DetachFlow::find(Context &c) noexcept
@brief 直接查找空组在全局组链中的链接位置。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-91c3662661a9789d"></a>

## `xv::process::ProcessGroups::DetachFlow::found`

```doxygen
@fn static bool xv::process::ProcessGroups::DetachFlow::found(Context &c) noexcept
@brief 检查空组确实登记在组链中。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6229937810dfc52a"></a>

## `xv::process::ProcessGroups::DetachFlow::free_session`

```doxygen
@fn static void xv::process::ProcessGroups::DetachFlow::free_session(Context &c) noexcept
@brief 销毁并归还最后一个组释放的会话页。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-18fbce99f46ca84f"></a>

## `xv::process::ProcessGroups::DetachFlow::invalid`

```doxygen
@fn static void xv::process::ProcessGroups::DetachFlow::invalid(Context &) noexcept
@brief 诊断未登记的空进程组。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f2d0c6e54a06dc67"></a>

## `xv::process::ProcessGroups::DetachFlow::last`

```doxygen
@fn static bool xv::process::ProcessGroups::DetachFlow::last(Context &c) noexcept
@brief 检查会话是否已没有进程组。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-51897627af4bedad"></a>

## `xv::process::ProcessGroups::DetachFlow::unlink`

```doxygen
@fn static void xv::process::ProcessGroups::DetachFlow::unlink(Context &c) noexcept
@brief 从组链移除空组，按需清除前台组号并保存会话引用。
@details 调用者持组锁且已摘除成员、确认组已空；只有复杂空组回收进入图。归还组页前保存会话，最后一个组释放会话；释放后不再读取对应对象。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2151587501ad1974"></a>

## `xv::process::ProcessGroups::InitialFlow::invalid`

```doxygen
@fn static void xv::process::ProcessGroups::InitialFlow::invalid(Context &) noexcept
@brief 诊断无效初始成员关系。
@details 入口 ScopedLock 持组锁；无旧成员和旧组时才能初始化，分配失败不发布成员，参数或状态非法保留原诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dfa8d20884ac9987"></a>

## `xv::process::ProcessGroups::InitialFlow::publish`

```doxygen
@fn static void xv::process::ProcessGroups::InitialFlow::publish(Context &c) noexcept
@brief 设置成员 PID 并接入已登记初始组，记录成功。
@details 入口 ScopedLock 持组锁；无旧成员和旧组时才能初始化，分配失败不发布成员，参数或状态非法保留原诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7af03581b1ee35bc"></a>

## `xv::process::ProcessGroups::InitialFlow::valid`

```doxygen
@fn static bool xv::process::ProcessGroups::InitialFlow::valid(Context &c) noexcept
@brief 检查成员尚未登记、组表为空且 PID 为正。
@details 入口 ScopedLock 持组锁；无旧成员和旧组时才能初始化，分配失败不发布成员，参数或状态非法保留原诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b1852c460ae23c0f"></a>

## `xv::process::ProcessGroups::SessionFlow::eligible`

```doxygen
@fn static bool xv::process::ProcessGroups::SessionFlow::eligible(Context &c) noexcept
@brief 检查调用者已登记且不存在与其 PID 同号的存活进程组。
@details 入口 ScopedLock 持组锁；新会话与组全部取得并登记后才移除旧成员关系，保证失败原子性和旧会话的正确释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-71020d2fc430284f"></a>

## `xv::process::ProcessGroups::SessionFlow::publish`

```doxygen
@fn static void xv::process::ProcessGroups::SessionFlow::publish(Context &c) noexcept
@brief 从旧组摘除调用者并接入新组，返回新会话 ID。
@details 入口 ScopedLock 持组锁；新会话与组全部取得并登记后才移除旧成员关系，保证失败原子性和旧会话的正确释放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d0c9eac0f5ace98f"></a>

## `xv::process::ProcessGroups::SessionProtocol::allocate_group`

```doxygen
@fn static void xv::process::ProcessGroups::SessionProtocol::allocate_group(Context &c) noexcept
@brief 为会话候选分配并登记同 ID 的进程组。
@details 初始化和创建会话共用分配与回滚动作；会话页取得后再分配并登记组，失败仅回收新候选，不改旧成员关系。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a329eb88bfd70aea"></a>

## `xv::process::ProcessGroups::SessionProtocol::allocate_session`

```doxygen
@fn static void xv::process::ProcessGroups::SessionProtocol::allocate_session(Context &c) noexcept
@brief 尝试分配并原位构造指定 ID 的空会话。
@details 初始化和创建会话共用分配与回滚动作；会话页取得后再分配并登记组，失败仅回收新候选，不改旧成员关系。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8ec0e59b7a34d40f"></a>

## `xv::process::ProcessGroups::SessionProtocol::has_group`

```doxygen
@fn static bool xv::process::ProcessGroups::SessionProtocol::has_group(Context &c) noexcept
@brief 检查进程组候选是否已取得。
@details 初始化和创建会话共用分配与回滚动作；会话页取得后再分配并登记组，失败仅回收新候选，不改旧成员关系。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8afe32ea4a1c32e1"></a>

## `xv::process::ProcessGroups::SessionProtocol::has_session`

```doxygen
@fn static bool xv::process::ProcessGroups::SessionProtocol::has_session(Context &c) noexcept
@brief 检查会话候选是否已取得。
@details 初始化和创建会话共用分配与回滚动作；会话页取得后再分配并登记组，失败仅回收新候选，不改旧成员关系。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1eb3b27ede084152"></a>

## `xv::process::ProcessGroups::SessionProtocol::rollback`

```doxygen
@fn static void xv::process::ProcessGroups::SessionProtocol::rollback(Context &c) noexcept
@brief 在组分配失败时销毁并归还会话候选页。
@details 初始化和创建会话共用分配与回滚动作；会话页取得后再分配并登记组，失败仅回收新候选，不改旧成员关系。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1a9d730fdfbc1c67"></a>

## `xv::process::ProcessGroups::SetGroupFlow::allocate`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::allocate(Context &c) noexcept
@brief 尝试创建并登记目标组，先取得会话引用。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0f3659f107815454"></a>

## `xv::process::ProcessGroups::SetGroupFlow::found`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::found(Context &c) noexcept
@brief 检查目标成员存在且仍登记于组。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0184541753f171e5"></a>

## `xv::process::ProcessGroups::SetGroupFlow::identical`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::identical(Context &c) noexcept
@brief 检查成员是否已属于目标组，保持原有成员链次序。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e0e11a9450db1f00"></a>

## `xv::process::ProcessGroups::SetGroupFlow::may_create`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::may_create(Context &c) noexcept
@brief 检查新组只能使用目标进程自己的 PID 命名。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b2cd4d6209fcde38"></a>

## `xv::process::ProcessGroups::SetGroupFlow::member`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::member(Context &c) noexcept
@brief 按零 PID 选择调用者，否则直接查找指定成员。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b8eb79cc3b7e87a4"></a>

## `xv::process::ProcessGroups::SetGroupFlow::movable`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::movable(Context &c) noexcept
@brief 检查目标属于同一会话且不是会话 leader。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-495dd0bc2ce6b8c9"></a>

## `xv::process::ProcessGroups::SetGroupFlow::move`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::move(Context &c) noexcept
@brief 摘除旧成员关系并接入已保留会话引用的目标组。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-82828d48df6577b8"></a>

## `xv::process::ProcessGroups::SetGroupFlow::permitted`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::permitted(Context &c) noexcept
@brief 检查目标是自己，或尚未 exec 的直接子进程。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5ed7269856ad169e"></a>

## `xv::process::ProcessGroups::SetGroupFlow::registered`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::registered(Context &c) noexcept
@brief 检查调用者已有成员关系。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b9726007cb0dae69"></a>

## `xv::process::ProcessGroups::SetGroupFlow::same_session`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::same_session(Context &c) noexcept
@brief 检查已有目标组属于同一会话。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e03b107459f8f9a6"></a>

## `xv::process::ProcessGroups::SetGroupFlow::session`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::session(Context &c) noexcept
@brief 保存调用者所属会话。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2c0be1501bd7959d"></a>

## `xv::process::ProcessGroups::SetGroupFlow::success`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::success(Context &c) noexcept
@brief 记录零成功码。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c191837a0c09b751"></a>

## `xv::process::ProcessGroups::SetGroupFlow::target`

```doxygen
@fn static void xv::process::ProcessGroups::SetGroupFlow::target(Context &c) noexcept
@brief 将零组号转换为目标 PID，再查询已有进程组。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2ba1383b2c391ed7"></a>

## `xv::process::ProcessGroups::SetGroupFlow::target_found`

```doxygen
@fn static bool xv::process::ProcessGroups::SetGroupFlow::target_found(Context &c) noexcept
@brief 检查本阶段是否取得目标组。
@details 入口先拒绝负 PID/组号，再持组锁；目标组的引用在 detach 之前建立，使会话在最后成员迁移期间保持存活。原父子、exec、会话和 leader 规则按原顺序检查。 下一节点由图策略声明，业务结果与控制信号分离。
```
