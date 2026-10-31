# `kernel/process/groups/group_operations.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/groups/group_operations.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessManager::claim_terminal`](group_operations.cpp.md#fn-dc2905888d371e3b)
- [`xv::process::ProcessManager::create_session`](group_operations.cpp.md#fn-ac54851324d72c43)
- [`xv::process::ProcessManager::detach_terminal`](group_operations.cpp.md#fn-82dd9a0dcde32e3a)
- [`xv::process::ProcessManager::did_exec`](group_operations.cpp.md#fn-026cc66345c86e17)
- [`xv::process::ProcessManager::get_process_group`](group_operations.cpp.md#fn-5273c2c990423d82)
- [`xv::process::ProcessManager::get_session`](group_operations.cpp.md#fn-4d575d2c1c9debae)
- [`xv::process::ProcessManager::may_read_terminal`](group_operations.cpp.md#fn-6ef8f28b8535c2e1)
- [`xv::process::ProcessManager::set_process_group`](group_operations.cpp.md#fn-da222928090ae0f5)
- [`xv::process::ProcessManager::set_terminal_foreground`](group_operations.cpp.md#fn-855c9b132f03c0ea)
- [`xv::process::ProcessManager::terminal_foreground`](group_operations.cpp.md#fn-58daa43bb91047a3)

<a id="fn-5273c2c990423d82"></a>

## `xv::process::ProcessManager::get_process_group`

```doxygen
@fn int xv::process::ProcessManager::get_process_group()
@brief 进程生命周期管理器：读取或计算 `groups_.get_group(current()->membership_)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.get_group` → `current`。
@return 各分支实际返回：`groups_.get_group(current()->membership_)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-da222928090ae0f5"></a>

## `xv::process::ProcessManager::set_process_group`

```doxygen
@fn int xv::process::ProcessManager::set_process_group(int pid, int group)
@brief 进程生命周期管理器：读取或计算 `groups_.set_group(current()->membership_, pid, group)`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param group 本次操作使用的 进程组 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.set_group` → `current`。
@return 各分支实际返回：`groups_.set_group(current()->membership_, pid, group)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4d575d2c1c9debae"></a>

## `xv::process::ProcessManager::get_session`

```doxygen
@fn int xv::process::ProcessManager::get_session(int pid)
@brief 进程生命周期管理器：读取或计算 `groups_.get_session(current()->membership_, pid)`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.get_session` → `current`。
@return 各分支实际返回：`groups_.get_session(current()->membership_, pid)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ac54851324d72c43"></a>

## `xv::process::ProcessManager::create_session`

```doxygen
@fn int xv::process::ProcessManager::create_session()
@brief 进程生命周期管理器：读取或计算 `groups_.create_session(current()->membership_)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.create_session` → `current`。
@return 各分支实际返回：`groups_.create_session(current()->membership_)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-026cc66345c86e17"></a>

## `xv::process::ProcessManager::did_exec`

```doxygen
@fn void xv::process::ProcessManager::did_exec(Process &process)
@brief 进程生命周期管理器：did执行映像。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.did_exec`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-82dd9a0dcde32e3a"></a>

## `xv::process::ProcessManager::detach_terminal`

```doxygen
@fn void xv::process::ProcessManager::detach_terminal(void *terminal)
@brief 进程生命周期管理器：解除关联终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.detach_terminal`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-dc2905888d371e3b"></a>

## `xv::process::ProcessManager::claim_terminal`

```doxygen
@fn int xv::process::ProcessManager::claim_terminal(void *terminal)
@brief 进程生命周期管理器：读取或计算 `groups_.claim_terminal(current()->membership_, terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.claim_terminal` → `current`。
@return 各分支实际返回：`groups_.claim_terminal(current()->membership_, terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-58daa43bb91047a3"></a>

## `xv::process::ProcessManager::terminal_foreground`

```doxygen
@fn int xv::process::ProcessManager::terminal_foreground(void *terminal)
@brief 进程生命周期管理器：读取或计算 `groups_.foreground(current()->membership_, terminal)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.foreground` → `current`。
@return 各分支实际返回：`groups_.foreground(current()->membership_, terminal)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-855c9b132f03c0ea"></a>

## `xv::process::ProcessManager::set_terminal_foreground`

```doxygen
@fn int xv::process::ProcessManager::set_terminal_foreground(void *terminal, int group)
@brief 进程生命周期管理器：读取或计算 `groups_.set_foreground(current()->membership_, terminal, group)`，返回当前查询结果。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param group 本次操作使用的 进程组 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`groups_.set_foreground` → `current`。
@return 各分支实际返回：`groups_.set_foreground(current()->membership_, terminal, group)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6ef8f28b8535c2e1"></a>

## `xv::process::ProcessManager::may_read_terminal`

```doxygen
@fn bool xv::process::ProcessManager::may_read_terminal(void *terminal)
@brief 进程生命周期管理器：may读取终端。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `groups_.may_read_terminal` → `process->pending_signals_.post`。
@details 需要特别跟踪的条件：`groups_.may_read_terminal(process->membership_, terminal)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-6d7c7b69f9c072ca"></a>

## `xv::process::ProcessManager::pin_controlling_terminal`

```doxygen
@fn void *ProcessManager::xv::process::ProcessManager::pin_controlling_terminal(void *(*pin)(void *))
@brief 在进程组锁内查找并固定当前session的控制终端。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
