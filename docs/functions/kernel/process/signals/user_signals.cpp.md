# `kernel/process/signals/user_signals.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/user_signals.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.memory`, `xv.runtime`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::begin_system_call`](user_signals.cpp.md#fn-58ff4ed2bf6d42b0)
- [`xv::process::Process::enter_signal_handler`](user_signals.cpp.md#fn-a4af1623ff74134d)
- [`xv::process::Process::finish_system_call`](user_signals.cpp.md#fn-2c3b9b20d500a9c5)
- [`xv::process::Process::signal_action`](user_signals.cpp.md#fn-bc793784f39c34ed)
- [`xv::process::Process::signal_mask`](user_signals.cpp.md#fn-ef5ff2ec7facd8e3)
- [`xv::process::Process::signal_pending`](user_signals.cpp.md#fn-ea7534a5b6b93ae7)
- [`xv::process::Process::signal_return`](user_signals.cpp.md#fn-5c2916a90196dfd3)
- [`xv::process::Process::signal_suspend`](user_signals.cpp.md#fn-824959f8e4a5aee2)
- [`xv::process::executable`](user_signals.cpp.md#fn-5a4c7fec2fbd833c)

<a id="fn-bc793784f39c34ed"></a>

## `xv::process::Process::signal_action`

```doxygen
@fn int xv::process::Process::signal_action(int signal, unsigned long address, unsigned long old, unsigned long restorer)
@brief 进程描述符：校验并安装或查询用户信号处理规则。
@param signal 待查询、投递或处理的信号编号。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param old 原有状态或源页表。
@param restorer 本次操作使用的 restorer 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supported_signal` → `memory_.copy_in` → `executable` → `pending_signals_.action` → `memory_.copy_out` → `guard` → `pending_signals_.set_action`。
@details 需要特别跟踪的条件：`!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP`；`address`；`handler > SIGNAL_IGNORE && (!executable(memory_, handler) || !executable(memory_, restorer))`；`old && memory_.copy_out(old, &result, sizeof(result)) < 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ef5ff2ec7facd8e3"></a>

## `xv::process::Process::signal_mask`

```doxygen
@fn int xv::process::Process::signal_mask(int how, unsigned long address, unsigned long old)
@brief 进程描述符：查询或按指定方式更新用户屏蔽字。
@param how 本次操作使用的 how 值。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param old 原有状态或源页表。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`memory_.copy_in` → `pending_signals_.mask` → `memory_.copy_out` → `guard` → `pending_signals_.set_mask`。
@details 需要特别跟踪的条件：`old && memory_.copy_out(old, &previous, sizeof(previous)) < 0`；`address`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ea7534a5b6b93ae7"></a>

## `xv::process::Process::signal_pending`

```doxygen
@fn int xv::process::Process::signal_pending(unsigned long address)
@brief 进程描述符：复制当前被屏蔽且待处理的信号集合。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pending_signals_.pending` → `pending_signals_.mask` → `memory_.copy_out`。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`memory_.copy_out(address, &value, sizeof(value))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-824959f8e4a5aee2"></a>

## `xv::process::Process::signal_suspend`

```doxygen
@fn int xv::process::Process::signal_suspend(unsigned long address)
@brief 进程描述符：用临时屏蔽字原子等待可投递信号。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`memory_.copy_in` → `guard` → `pending_signals_.mask` → `pending_signals_.set_mask` → `is_killed` → `pending_signals_.caught_signal` → `prepare_interruptible_sleep` → `sleep`。
@details 本函数可见的状态更新包括：`suspend_mask_ = pending_signals_.mask();`；`suspended_mask_ = true;`；`suspend_restarted_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`memory_.copy_in(&value, address, sizeof(value)) < 0 || (value & ~PendingSignals::SupportedMask)`；`!suspended_mask_`；`is_killed()`。这些条件决定检查、重试或提前返回路径。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`pending_signals_.caught_signal() ? -EINTR : -1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5c2916a90196dfd3"></a>

## `xv::process::Process::signal_return`

```doxygen
@fn unsigned long xv::process::Process::signal_return()
@brief 进程描述符：从用户信号帧恢复被打断的用户上下文。
@details 仅接受最内层活动帧并检查帧位置、标记、用户可执行 PC、SP 及屏蔽字。恢复用户寄存器和旧屏蔽字，返回恢复后的 a0 以避免被系统调用分发覆盖；内核 SATP、内核栈和特权状态不从用户帧恢复。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`memory_.copy_in` → `executable` → `memory_.size` → `guard` → `pending_signals_.set_mask` → `xv::runtime::move`。
@details 本函数可见的状态更新包括：`trapframe->epc = frame.pc;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!signal_depth_`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`trapframe->a0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-58ff4ed2bf6d42b0"></a>

## `xv::process::Process::begin_system_call`

```doxygen
@fn void xv::process::Process::begin_system_call()
@brief 进程描述符：清除本次系统调用的中断记录。
@details 本函数可见的状态更新包括：`syscall_interrupted_ = false;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2c3b9b20d500a9c5"></a>

## `xv::process::Process::finish_system_call`

```doxygen
@fn long xv::process::Process::finish_system_call(int number, unsigned long argument, long result)
@brief 进程描述符：根据捕获信号及重启策略整理系统调用结果。
@param number 本次操作使用的 编号 值。
@param argument 本次操作使用的 argument 值。
@param result 本次操作使用的 result 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`stop_requested` → `pending_signals_.caught_signal` → `pending_signals_.action`。
@details 本函数可见的状态更新包括：`suspend_restarted_ = true;`；`trapframe->epc -= 4;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`number == SYS_sigsuspend && result == -1 && stop_requested()`；`result != -1 || !syscall_interrupted_ || !signal`；`restartable && (pending_signals_.action(signal).flags & SA_RESTART)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`result`；`argument`；`-EINTR`。应结合上面的条件检查区分正常值、空值和错误值。
@details UDP recv 与 read/write/wait/waitpid 一样支持 SA_RESTART；捕获信号且未启用重启时返回 -EINTR，重启时回退系统调用 PC 并恢复首参数。
```

<a id="fn-a4af1623ff74134d"></a>

## `xv::process::Process::enter_signal_handler`

```doxygen
@fn bool xv::process::Process::enter_signal_handler()
@brief 进程描述符：保存用户现场并安排下一次用户态执行信号处理函数。
@details 从可投递捕获信号取得处置，校验处理函数/返回入口和用户栈，保存 PC、全部整数寄存器及旧屏蔽字到用户信号帧。消费待处理位、更新临时屏蔽字后修改 epc/sp/ra/a0，真正执行处理函数发生在返回用户态后。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pending_signals_.caught_signal` → `pending_signals_.action` → `executable` → `memory_.size` → `pending_signals_.mask` → `xv::runtime::move` → `memory_.copy_out` → `guard` → `pending_signals_.consume` → `pending_signals_.set_action` → `pending_signals_.set_mask`。
@details 本函数可见的状态更新包括：`frame.magic = FrameMagic;`；`frame.mask = suspended_mask_ ? suspend_mask_ : pending_signals_.mask();`；`frame.pc = trapframe->epc;`；`frame.pc += 4;`；`frame.registers[9] = -EINTR;`；`suspended_mask_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!signal`；`suspended_mask_ && suspend_restarted_`；`memory_.copy_out(stack, &frame, sizeof(frame)) < 0`；`action.flags & SA_RESETHAND`；`!(action.flags & SA_NODEFER)`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5a4c7fec2fbd833c"></a>

## `xv::process::executable`

```doxygen
@fn bool xv::process::executable(const AddressSpace &memory, unsigned long address)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `false`，返回当前查询结果。
@param memory 启动内存信息或地址空间对象。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`memory.size` → `xv::memory::walk_entry` → `memory.page_table`。
@details 需要特别跟踪的条件：`address >= memory.size() || (address & 1)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`entry && (*entry & (PTE_V | PTE_U | PTE_X)) == (PTE_V | PTE_U | PTE_X)`。应结合上面的条件检查区分正常值、空值和错误值。
```
