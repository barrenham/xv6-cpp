# `kernel/process/signals/user_signals.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/user_signals.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.memory`, `xv.runtime`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::EnterSignalFlow::consume`](user_signals.cpp.md#fn-5a5ea9360734ee8f)
- [`xv::process::Process::EnterSignalFlow::copied`](user_signals.cpp.md#fn-c999bc795d0552fa)
- [`xv::process::Process::EnterSignalFlow::copy`](user_signals.cpp.md#fn-c07beeb73babc94e)
- [`xv::process::Process::EnterSignalFlow::frame`](user_signals.cpp.md#fn-4a5032ed47c71b3d)
- [`xv::process::Process::EnterSignalFlow::publish`](user_signals.cpp.md#fn-ae61b07007ecd45e)
- [`xv::process::Process::EnterSignalFlow::valid`](user_signals.cpp.md#fn-991e5a900d4b5667)
- [`xv::process::Process::ReturnSignalFlow::restore_mask`](user_signals.cpp.md#fn-d3418ca89fd5b3d0)
- [`xv::process::Process::ReturnSignalFlow::restore_registers`](user_signals.cpp.md#fn-3061289ea3192ccc)
- [`xv::process::Process::ReturnSignalFlow::valid`](user_signals.cpp.md#fn-22ad303c2ccad376)
- [`xv::process::Process::SignalActionFlow::copy_previous`](user_signals.cpp.md#fn-9deb31be5c9afe61)
- [`xv::process::Process::SignalActionFlow::executable_handler`](user_signals.cpp.md#fn-16d34b4510b3911c)
- [`xv::process::Process::SignalActionFlow::input`](user_signals.cpp.md#fn-2edc6330b2aa6072)
- [`xv::process::Process::SignalActionFlow::install`](user_signals.cpp.md#fn-998111ac902ee8b9)
- [`xv::process::Process::SignalActionFlow::previous`](user_signals.cpp.md#fn-7538dd7b024c4671)
- [`xv::process::Process::SignalActionFlow::read`](user_signals.cpp.md#fn-143f87676544e8c1)
- [`xv::process::Process::SignalActionFlow::success`](user_signals.cpp.md#fn-2c9b8c59ddfa92ac)
- [`xv::process::Process::SignalActionFlow::valid`](user_signals.cpp.md#fn-0d0a223afa77f111)
- [`xv::process::Process::SignalMaskFlow::copy_previous`](user_signals.cpp.md#fn-b6955826000911da)
- [`xv::process::Process::SignalMaskFlow::input`](user_signals.cpp.md#fn-e2f03a752a11752b)
- [`xv::process::Process::SignalMaskFlow::install`](user_signals.cpp.md#fn-df8b9189247e6b3c)
- [`xv::process::Process::SignalMaskFlow::previous`](user_signals.cpp.md#fn-fc7d91ce3b345376)
- [`xv::process::Process::SignalMaskFlow::read`](user_signals.cpp.md#fn-d9029ef5a6692696)
- [`xv::process::Process::SignalMaskFlow::success`](user_signals.cpp.md#fn-226c4bd5a2b5bee7)
- [`xv::process::Process::SignalMaskFlow::valid`](user_signals.cpp.md#fn-3aaa333f3a7d891b)
- [`xv::process::Process::SignalSuspendFlow::install`](user_signals.cpp.md#fn-498a32ce7ffe0f6d)
- [`xv::process::Process::SignalSuspendFlow::interrupted`](user_signals.cpp.md#fn-264a4e3109554201)
- [`xv::process::Process::SignalSuspendFlow::killed`](user_signals.cpp.md#fn-c5a4f6f1e8fa6d8d)
- [`xv::process::Process::SignalSuspendFlow::read`](user_signals.cpp.md#fn-99bfaf425df19913)
- [`xv::process::Process::SignalSuspendFlow::valid`](user_signals.cpp.md#fn-3a8fff6d91e5efc0)
- [`xv::process::Process::SignalSuspendFlow::wait`](user_signals.cpp.md#fn-418899cfb7ed6006)
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
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SignalActionFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-ef5ff2ec7facd8e3"></a>

## `xv::process::Process::signal_mask`

```doxygen
@fn int xv::process::Process::signal_mask(int how, unsigned long address, unsigned long old)
@brief 进程描述符：查询或按指定方式更新用户屏蔽字。
@param how 本次操作使用的 how 值。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param old 原有状态或源页表。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SignalMaskFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`pending_signals_.caught_signal() ? -EINTR : -1`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SignalSuspendFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5c2916a90196dfd3"></a>

## `xv::process::Process::signal_return`

```doxygen
@fn unsigned long xv::process::Process::signal_return()
@brief 进程描述符：从用户信号帧恢复被打断的用户上下文。
@details 仅接受最内层活动帧并检查帧位置、标记、用户可执行 PC、SP 及屏蔽字。恢复用户寄存器和旧屏蔽字，返回恢复后的 a0 以避免被系统调用分发覆盖；内核 SATP、内核栈和特权状态不从用户帧恢复。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`trapframe->a0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReturnSignalFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-58ff4ed2bf6d42b0"></a>

## `xv::process::Process::begin_system_call`

```doxygen
@fn void xv::process::Process::begin_system_call()
@brief 进程描述符：清除本次系统调用的中断记录。
@details 本函数可见的状态更新包括：`syscall_interrupted_ = false;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a4af1623ff74134d"></a>

## `xv::process::Process::enter_signal_handler`

```doxygen
@fn bool xv::process::Process::enter_signal_handler()
@brief 进程描述符：保存用户现场并安排下一次用户态执行信号处理函数。
@details 从可投递捕获信号取得处置，校验处理函数/返回入口和用户栈，保存 PC、全部整数寄存器及旧屏蔽字到用户信号帧。消费待处理位、更新临时屏蔽字后修改 epc/sp/ra/a0，真正执行处理函数发生在返回用户态后。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 EnterSignalFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details UDP socket 的 recvfrom 与 read/write/wait/waitpid 一样支持 SA_RESTART；捕获信号且未启用重启时返回 -EINTR，重启时回退系统调用 PC 并恢复首参数。
@details accept、recvfrom、sendto 加入可 SA_RESTART 重启的阻塞调用；read/write 已覆盖 Socket 字节流。connect 中断保留在途连接，应用可 poll 查询，避免自动重复发起 connect。
```

<a id="fn-5a5ea9360734ee8f"></a>

## `xv::process::Process::EnterSignalFlow::consume`

```doxygen
@fn static void xv::process::Process::EnterSignalFlow::consume(Context &c) noexcept
@brief 在进程锁下消费信号、应用动作标志及掩码并登记帧。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c999bc795d0552fa"></a>

## `xv::process::Process::EnterSignalFlow::copied`

```doxygen
@fn static bool xv::process::Process::EnterSignalFlow::copied(Context &c) noexcept
@brief 检查用户信号帧是否复制成功。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c07beeb73babc94e"></a>

## `xv::process::Process::EnterSignalFlow::copy`

```doxygen
@fn static void xv::process::Process::EnterSignalFlow::copy(Context &c) noexcept
@brief 向用户栈复制完整信号帧并保存结果。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4a5032ed47c71b3d"></a>

## `xv::process::Process::EnterSignalFlow::frame`

```doxygen
@fn static void xv::process::Process::EnterSignalFlow::frame(Context &c) noexcept
@brief 构造对齐的用户信号帧并处理挂起系统调用的恢复位置。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ae61b07007ecd45e"></a>

## `xv::process::Process::EnterSignalFlow::publish`

```doxygen
@fn static void xv::process::Process::EnterSignalFlow::publish(Context &c) noexcept
@brief 设置处理器入口、用户栈、返回地址与信号参数。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-991e5a900d4b5667"></a>

## `xv::process::Process::EnterSignalFlow::valid`

```doxygen
@fn static bool xv::process::Process::EnterSignalFlow::valid(Context &c) noexcept
@brief 检查嵌套深度、处理器和 restorer 映射及用户栈边界。
@details 保持原用户信号帧 ABI；复制用户帧成功后才加进程锁消费信号并记录帧，最后发布用户寄存器。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d3418ca89fd5b3d0"></a>

## `xv::process::Process::ReturnSignalFlow::restore_mask`

```doxygen
@fn static void xv::process::Process::ReturnSignalFlow::restore_mask(Context &c) noexcept
@brief 在进程锁下弹出已验证信号帧并恢复掩码。
@details 按登记帧地址复制并校验用户帧，成功才减深度与恢复掩码；只恢复用户寄存器，不接受内核 CSR。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3061289ea3192ccc"></a>

## `xv::process::Process::ReturnSignalFlow::restore_registers`

```doxygen
@fn static void xv::process::Process::ReturnSignalFlow::restore_registers(Context &c) noexcept
@brief 恢复用户 PC 与寄存器并保留原系统调用返回寄存器。
@details 按登记帧地址复制并校验用户帧，成功才减深度与恢复掩码；只恢复用户寄存器，不接受内核 CSR。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-22ad303c2ccad376"></a>

## `xv::process::Process::ReturnSignalFlow::valid`

```doxygen
@fn static bool xv::process::Process::ReturnSignalFlow::valid(Context &c) noexcept
@brief 验证帧地址、复制结果、魔数、恢复 PC、用户栈及信号掩码。
@details 按登记帧地址复制并校验用户帧，成功才减深度与恢复掩码；只恢复用户寄存器，不接受内核 CSR。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9deb31be5c9afe61"></a>

## `xv::process::Process::SignalActionFlow::copy_previous`

```doxygen
@fn static bool xv::process::Process::SignalActionFlow::copy_previous(Context &c) noexcept
@brief 按需向用户地址回写旧动作并检查成功。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-16d34b4510b3911c"></a>

## `xv::process::Process::SignalActionFlow::executable_handler`

```doxygen
@fn static bool xv::process::Process::SignalActionFlow::executable_handler(Context &c) noexcept
@brief 验证自定义处理器与 restorer 的用户可执行映射。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2edc6330b2aa6072"></a>

## `xv::process::Process::SignalActionFlow::input`

```doxygen
@fn static bool xv::process::Process::SignalActionFlow::input(Context &c) noexcept
@brief 检查是否提供新动作地址。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-998111ac902ee8b9"></a>

## `xv::process::Process::SignalActionFlow::install`

```doxygen
@fn static void xv::process::Process::SignalActionFlow::install(Context &c) noexcept
@brief 在进程锁下发布新动作与 restorer。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7538dd7b024c4671"></a>

## `xv::process::Process::SignalActionFlow::previous`

```doxygen
@fn static void xv::process::Process::SignalActionFlow::previous(Context &c) noexcept
@brief 读取原信号动作快照。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-143f87676544e8c1"></a>

## `xv::process::Process::SignalActionFlow::read`

```doxygen
@fn static void xv::process::Process::SignalActionFlow::read(Context &c) noexcept
@brief 复制新动作并验证掩码与受支持标志。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2c9b8c59ddfa92ac"></a>

## `xv::process::Process::SignalActionFlow::success`

```doxygen
@fn static void xv::process::Process::SignalActionFlow::success(Context &c) noexcept
@brief 记录零成功码。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0d0a223afa77f111"></a>

## `xv::process::Process::SignalActionFlow::valid`

```doxygen
@fn static bool xv::process::Process::SignalActionFlow::valid(Context &c) noexcept
@brief 检查输入动作的复制与标志验证是否成功。
@details 先复制并验证输入，再回写旧动作，最后在进程锁下安装；用户复制不持自旋锁，输入输出地址允许相同。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b6955826000911da"></a>

## `xv::process::Process::SignalMaskFlow::copy_previous`

```doxygen
@fn static bool xv::process::Process::SignalMaskFlow::copy_previous(Context &c) noexcept
@brief 按需回写旧掩码并检查复制成功。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e2f03a752a11752b"></a>

## `xv::process::Process::SignalMaskFlow::input`

```doxygen
@fn static bool xv::process::Process::SignalMaskFlow::input(Context &c) noexcept
@brief 检查是否提供新掩码地址。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-df8b9189247e6b3c"></a>

## `xv::process::Process::SignalMaskFlow::install`

```doxygen
@fn static void xv::process::Process::SignalMaskFlow::install(Context &c) noexcept
@brief 在进程锁下按操作类型合成并发布掩码。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fc7d91ce3b345376"></a>

## `xv::process::Process::SignalMaskFlow::previous`

```doxygen
@fn static void xv::process::Process::SignalMaskFlow::previous(Context &c) noexcept
@brief 读取当前掩码快照。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d9029ef5a6692696"></a>

## `xv::process::Process::SignalMaskFlow::read`

```doxygen
@fn static void xv::process::Process::SignalMaskFlow::read(Context &c) noexcept
@brief 复制并验证新掩码和操作类型。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-226c4bd5a2b5bee7"></a>

## `xv::process::Process::SignalMaskFlow::success`

```doxygen
@fn static void xv::process::Process::SignalMaskFlow::success(Context &c) noexcept
@brief 记录零成功码。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3aaa333f3a7d891b"></a>

## `xv::process::Process::SignalMaskFlow::valid`

```doxygen
@fn static bool xv::process::Process::SignalMaskFlow::valid(Context &c) noexcept
@brief 检查输入掩码是否有效。
@details 先读取新掩码和旧掩码，再复制旧值，最后加进程锁发布；没有输入地址时忽略 how。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-498a32ce7ffe0f6d"></a>

## `xv::process::Process::SignalSuspendFlow::install`

```doxygen
@fn static void xv::process::Process::SignalSuspendFlow::install(Context &c) noexcept
@brief 在进程锁下保存原掩码并安装临时掩码。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-264a4e3109554201"></a>

## `xv::process::Process::SignalSuspendFlow::interrupted`

```doxygen
@fn static void xv::process::Process::SignalSuspendFlow::interrupted(Context &c) noexcept
@brief 根据是否有捕获信号记录 EINTR 或原失败值。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c5a4f6f1e8fa6d8d"></a>

## `xv::process::Process::SignalSuspendFlow::killed`

```doxygen
@fn static bool xv::process::Process::SignalSuspendFlow::killed(Context &c) noexcept
@brief 检查可中断状态并保留系统调用中断记账。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-99bfaf425df19913"></a>

## `xv::process::Process::SignalSuspendFlow::read`

```doxygen
@fn static void xv::process::Process::SignalSuspendFlow::read(Context &c) noexcept
@brief 复制并验证临时信号掩码。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3a8fff6d91e5efc0"></a>

## `xv::process::Process::SignalSuspendFlow::valid`

```doxygen
@fn static bool xv::process::Process::SignalSuspendFlow::valid(Context &c) noexcept
@brief 检查临时掩码复制与验证是否成功。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-418899cfb7ed6006"></a>

## `xv::process::Process::SignalSuspendFlow::wait`

```doxygen
@fn static void xv::process::Process::SignalSuspendFlow::wait(Context &c) noexcept
@brief 登记可中断等待并睡眠。
@details 保存首次挂起前掩码，安装临时掩码；睡眠后迭代复查中断，不增加调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```
