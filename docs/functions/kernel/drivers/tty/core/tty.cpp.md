# `kernel/drivers/tty/core/tty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/core/tty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:tty`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Tty::InjectFlow::accepted`](tty.cpp.md#fn-89b9fa3002e98003)
- [`xv::drivers::Tty::InjectFlow::again`](tty.cpp.md#fn-03a923b1716583ee)
- [`xv::drivers::Tty::InjectFlow::nonblocking`](tty.cpp.md#fn-ebc8e519c4657d53)
- [`xv::drivers::Tty::InjectFlow::receive`](tty.cpp.md#fn-31cfc06200a27cc1)
- [`xv::drivers::Tty::InjectFlow::retry`](tty.cpp.md#fn-9da97960fc707e3f)
- [`xv::drivers::Tty::InjectFlow::stopped`](tty.cpp.md#fn-969c0b68f4b781bd)
- [`xv::drivers::Tty::InjectFlow::wait`](tty.cpp.md#fn-171540264f3b2d65)
- [`xv::drivers::Tty::IoctlFlow::apply`](tty.cpp.md#fn-a5eb611a1a50b87d)
- [`xv::drivers::Tty::IoctlFlow::claim`](tty.cpp.md#fn-c4d0e95a2a3ab488)
- [`xv::drivers::Tty::IoctlFlow::claim_request`](tty.cpp.md#fn-559fa9a7aca1be86)
- [`xv::drivers::Tty::IoctlFlow::drain`](tty.cpp.md#fn-b4707816cd44fc43)
- [`xv::drivers::Tty::IoctlFlow::draining`](tty.cpp.md#fn-e135004a4843cdd1)
- [`xv::drivers::Tty::IoctlFlow::flush`](tty.cpp.md#fn-78d1777b4886b03e)
- [`xv::drivers::Tty::IoctlFlow::flush_request`](tty.cpp.md#fn-5f4100acfed1bc19)
- [`xv::drivers::Tty::IoctlFlow::group`](tty.cpp.md#fn-e9d2287dd8f033cf)
- [`xv::drivers::Tty::IoctlFlow::group_request`](tty.cpp.md#fn-28df49f4810b7684)
- [`xv::drivers::Tty::IoctlFlow::ok`](tty.cpp.md#fn-3cb230c39f45623d)
- [`xv::drivers::Tty::IoctlFlow::read_settings`](tty.cpp.md#fn-2e609e88e5333fd9)
- [`xv::drivers::Tty::IoctlFlow::readable`](tty.cpp.md#fn-6cc37a8416f17d38)
- [`xv::drivers::Tty::IoctlFlow::readable_request`](tty.cpp.md#fn-e44787feb0e4fff9)
- [`xv::drivers::Tty::IoctlFlow::release_writer`](tty.cpp.md#fn-ac7668158f77c27c)
- [`xv::drivers::Tty::IoctlFlow::set_group`](tty.cpp.md#fn-b3668124f1fa5de4)
- [`xv::drivers::Tty::IoctlFlow::set_group_request`](tty.cpp.md#fn-66499fc7cf663fcb)
- [`xv::drivers::Tty::IoctlFlow::set_settings_request`](tty.cpp.md#fn-8b7a8aa9f137753c)
- [`xv::drivers::Tty::IoctlFlow::set_window`](tty.cpp.md#fn-6205757eeae20c5a)
- [`xv::drivers::Tty::IoctlFlow::set_window_request`](tty.cpp.md#fn-5605b39a8322d1bd)
- [`xv::drivers::Tty::IoctlFlow::settings`](tty.cpp.md#fn-314457c85ca90adb)
- [`xv::drivers::Tty::IoctlFlow::settings_request`](tty.cpp.md#fn-2b68718b449d5f0d)
- [`xv::drivers::Tty::IoctlFlow::success`](tty.cpp.md#fn-1e98834c0eb86b73)
- [`xv::drivers::Tty::IoctlFlow::window`](tty.cpp.md#fn-3a1d95e781e7ad36)
- [`xv::drivers::Tty::IoctlFlow::window_request`](tty.cpp.md#fn-51eb7247eb684fa6)
- [`xv::drivers::Tty::IoctlFlow::writer`](tty.cpp.md#fn-8d730ff5cc5a5c67)
- [`xv::drivers::Tty::PollFlow::disconnected`](tty.cpp.md#fn-d3fc42014bdba2b9)
- [`xv::drivers::Tty::PollFlow::hangup`](tty.cpp.md#fn-e2baa78fdba75444)
- [`xv::drivers::Tty::PollFlow::reader`](tty.cpp.md#fn-d6279f7200d99eea)
- [`xv::drivers::Tty::PollFlow::snapshot`](tty.cpp.md#fn-8e4c597200bbbbfe)
- [`xv::drivers::Tty::PollFlow::writer`](tty.cpp.md#fn-a47cdf7e4d4c46f9)
- [`xv::drivers::Tty::ReadFlow::again`](tty.cpp.md#fn-f8bdc564b635b0d6)
- [`xv::drivers::Tty::ReadFlow::complete`](tty.cpp.md#fn-6960a3fd4c45a1cf)
- [`xv::drivers::Tty::ReadFlow::drain`](tty.cpp.md#fn-acc80bb629e49df3)
- [`xv::drivers::Tty::ReadFlow::fault`](tty.cpp.md#fn-2f79074b8b4a5e9a)
- [`xv::drivers::Tty::ReadFlow::finished`](tty.cpp.md#fn-a73fdeaad4e82ae2)
- [`xv::drivers::Tty::ReadFlow::generation`](tty.cpp.md#fn-54c6c4f4c5f84f47)
- [`xv::drivers::Tty::ReadFlow::killed`](tty.cpp.md#fn-6b56fbdea3714ed9)
- [`xv::drivers::Tty::ReadFlow::mode`](tty.cpp.md#fn-13e3ea7421e169a6)
- [`xv::drivers::Tty::ReadFlow::nonblocking`](tty.cpp.md#fn-f1759c6d1b62fa25)
- [`xv::drivers::Tty::ReadFlow::permitted`](tty.cpp.md#fn-9d039abb34ca1eed)
- [`xv::drivers::Tty::ReadFlow::raw_complete`](tty.cpp.md#fn-bb7c1541bc3cfecd)
- [`xv::drivers::Tty::ReadFlow::wait`](tty.cpp.md#fn-6e987700133ae1a7)
- [`xv::drivers::Tty::WriteFlow::advance`](tty.cpp.md#fn-bdcc164ee45db38b)
- [`xv::drivers::Tty::WriteFlow::failure`](tty.cpp.md#fn-ac037a5d16228bda)
- [`xv::drivers::Tty::WriteFlow::killed`](tty.cpp.md#fn-af06972a91dd22ef)
- [`xv::drivers::Tty::WriteFlow::ok`](tty.cpp.md#fn-d2fd97b989103e96)
- [`xv::drivers::Tty::WriteFlow::read`](tty.cpp.md#fn-a43322a79841239a)
- [`xv::drivers::Tty::WriteFlow::remaining`](tty.cpp.md#fn-4fa7fa627ac5a23c)
- [`xv::drivers::Tty::WriteFlow::sent`](tty.cpp.md#fn-f7d55c565a63d098)
- [`xv::drivers::Tty::WriteFlow::short_write`](tty.cpp.md#fn-5f51e911ea771f67)
- [`xv::drivers::Tty::WriteFlow::success`](tty.cpp.md#fn-87171b41092901ed)
- [`xv::drivers::Tty::WriteFlow::transform`](tty.cpp.md#fn-adfe8ebbd243677f)
- [`xv::drivers::Tty::WriteFlow::transmit`](tty.cpp.md#fn-fce2cc597b0d2794)
- [`xv::drivers::Tty::hangup`](tty.cpp.md#fn-3acc0f0e4a2096c9)
- [`xv::drivers::Tty::initialize`](tty.cpp.md#fn-2a48d8f477ebb1b7)
- [`xv::drivers::Tty::inject`](tty.cpp.md#fn-258d0222cee0993a)
- [`xv::drivers::Tty::input_writable`](tty.cpp.md#fn-ec3f40ec3ea02161)
- [`xv::drivers::Tty::ioctl`](tty.cpp.md#fn-3625e17fbe0b06ab)
- [`xv::drivers::Tty::poll`](tty.cpp.md#fn-f52d9721bfdab4d5)
- [`xv::drivers::Tty::put_sync`](tty.cpp.md#fn-d96051693f210b8e)
- [`xv::drivers::Tty::read`](tty.cpp.md#fn-2b943588ba88bb06)
- [`xv::drivers::Tty::receive`](tty.cpp.md#fn-a871a5e9e5abafda)
- [`xv::drivers::Tty::timer_tick`](tty.cpp.md#fn-eaea2170608ca21d)
- [`xv::drivers::Tty::write`](tty.cpp.md#fn-7a0276d465b23c53)
- [`xv::drivers::valid`](tty.cpp.md#fn-e69f7ea6763472d1)

- [`xv::drivers::Tty::WriteFlow::retry`](tty.cpp.md#fn-205cdd66dd1702c0)

- [`xv::drivers::Tty::WriteFlow::shrink`](tty.cpp.md#fn-0365b3b870b056cb)

<a id="fn-e69f7ea6763472d1"></a>

## `xv::drivers::valid`

```doxygen
@fn bool xv::drivers::valid(const termios &value)
@brief 验证TTY支持的输入、输出、本地模式及固定控制配置。
@param value 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 支持常规行规程标志及私有XV_PROXY代理声明位。XV_PROXY只保存/恢复，不改变字符处理；外层用户态代理通过子PTY的TCGETS查询退出键归属。其余未知位仍拒绝。
@return 所有标志均受支持、c_cflag等于FixedControl且c_line为0时返回true，否则false。
```

<a id="fn-2a48d8f477ebb1b7"></a>

## `xv::drivers::Tty::initialize`

```doxygen
@fn void xv::drivers::Tty::initialize()
@brief 终端核心：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init` → `read_lock_.init` → `write_lock_.init` → `discipline_.flush`。
@details 本函数可见的状态更新包括：`settings_.c_iflag = ICRNL;`；`settings_.c_oflag = OPOST;`；`settings_.c_cflag = FixedControl;`；`settings_.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;`；`settings_.c_cc[VINTR] = 3;`；`settings_.c_cc[VSUSP] = 26;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d96051693f210b8e"></a>

## `xv::drivers::Tty::put_sync`

```doxygen
@fn void xv::drivers::Tty::put_sync(int character)
@brief 终端核心：通过不能睡眠的输出路径提交一个字符。
@param character 要输出的单个字符值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`output_.put_sync`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a871a5e9e5abafda"></a>

## `xv::drivers::Tty::receive`

```doxygen
@fn void xv::drivers::Tty::receive(int character)
@brief 终端核心：按终端模式解释输入并更新记录缓冲。
@param character 要输出的单个字符值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `discipline_.receive` → `detail::post_terminal_interrupt` → `discipline_.available` → `detail::wake_processes`。
@details 需要特别跟踪的条件：`action == InputAction::Interrupt || action == InputAction::Suspend`；`discipline_.available()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-258d0222cee0993a"></a>

## `xv::drivers::Tty::inject`

```doxygen
@fn int xv::drivers::Tty::inject(unsigned char character, bool nonblocking)
@brief 终端核心：把输入字节送入终端行规程。
@param character 要输出的单个字符值。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：`-1`；`0`；`-EAGAIN`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 InjectFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-3acc0f0e4a2096c9"></a>

## `xv::drivers::Tty::hangup`

```doxygen
@fn void xv::drivers::Tty::hangup()
@brief 终端核心：标记终端挂断并解除等待者的阻塞。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `discipline_.change_mode` → `detail::detach_terminal` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`hungup_ = true;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ec3f40ec3ea02161"></a>

## `xv::drivers::Tty::input_writable`

```doxygen
@fn bool xv::drivers::Tty::input_writable(xv::sync::PollTable *table)
@brief 终端核心：查询 PTY 输入是否可以继续注入。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `table->watch` → `discipline_.input_room`。
@details 需要特别跟踪的条件：`table`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`!hungup_ && discipline_.input_room(settings_.c_lflag & ICANON)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f52d9721bfdab4d5"></a>

## `xv::drivers::Tty::poll`

```doxygen
@fn unsigned xv::drivers::Tty::poll(xv::sync::PollTable *table)
@brief 终端核心：查询当前就绪状态，并在提供表时登记通知依赖。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@return 各分支实际返回：`events | POLLIN | POLLHUP`；`events`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 PollFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-eaea2170608ca21d"></a>

## `xv::drivers::Tty::timer_tick`

```doxygen
@fn void xv::drivers::Tty::timer_tick()
@brief 终端核心：定时器tick。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::wake_processes`。
@details 需要特别跟踪的条件：`timed_wait_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2b943588ba88bb06"></a>

## `xv::drivers::Tty::read`

```doxygen
@fn int xv::drivers::Tty::read(IoBuffer destination, int count)
@brief 终端核心：读取已有数据并按接口返回读取结果。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 读取由序列锁避免多个读者互相消费记录，状态锁保护行规程。规范模式等待可提交记录；非规范模式按 VMIN/VTIME 组合决定完成条件。非阻塞路径消费已有数据，不能等待序列锁或定时器。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : (fault ? -1 : (again ? -EAGAIN : 0))`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReadFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-7a0276d465b23c53"></a>

## `xv::drivers::Tty::write`

```doxygen
@fn int xv::drivers::Tty::write(IoBuffer source, int count)
@brief 终端核心：提交输入数据并报告传输结果。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 写序列锁维护输出顺序，按 termios 转换原始输入字符。非阻塞转换单元要求后端完整接受或失败，返回值按原始输入字节计算，防止重试把半个 CRLF 重复写出。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : -1`；`done ? done : result`；`done`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 WriteFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-3625e17fbe0b06ab"></a>

## `xv::drivers::Tty::ioctl`

```doxygen
@fn int xv::drivers::Tty::ioctl(unsigned request, IoBuffer argument)
@brief 终端核心：分派设备控制请求并处理参数复制。
@param request 本次操作使用的 请求 值。
@param argument 本次操作使用的 argument 值。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`hungup_ ? -1 : detail::claim_terminal(this)`；`detail::copy_out(user, argument.address, &group, sizeof(group))`；`result`；`detail::copy_out(user, argument.address, &value, sizeof(value))`；`0`；`detail::copy_out(user, argument.address, &count, sizeof(count))`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 IoctlFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-89b9fa3002e98003"></a>

## `xv::drivers::Tty::InjectFlow::accepted`

```doxygen
@fn static void xv::drivers::Tty::InjectFlow::accepted(Context &c) noexcept
@brief 按动作投递前台信号、唤醒等待并记录成功。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03a923b1716583ee"></a>

## `xv::drivers::Tty::InjectFlow::again`

```doxygen
@fn static void xv::drivers::Tty::InjectFlow::again(Context &c) noexcept
@brief 记录缓冲暂不可用的 EAGAIN。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ebc8e519c4657d53"></a>

## `xv::drivers::Tty::InjectFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::Tty::InjectFlow::nonblocking(Context &c) noexcept
@brief 检查本次软件注入是否非阻塞。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-31cfc06200a27cc1"></a>

## `xv::drivers::Tty::InjectFlow::receive`

```doxygen
@fn static void xv::drivers::Tty::InjectFlow::receive(Context &c) noexcept
@brief 将一个软件输入字节交给原行规程。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9da97960fc707e3f"></a>

## `xv::drivers::Tty::InjectFlow::retry`

```doxygen
@fn static bool xv::drivers::Tty::InjectFlow::retry(Context &c) noexcept
@brief 检查行规程是否要求等待缓冲空间。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-969c0b68f4b781bd"></a>

## `xv::drivers::Tty::InjectFlow::stopped`

```doxygen
@fn static bool xv::drivers::Tty::InjectFlow::stopped(Context &c) noexcept
@brief 按原顺序检查挂断与进程中断。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-171540264f3b2d65"></a>

## `xv::drivers::Tty::InjectFlow::wait`

```doxygen
@fn static void xv::drivers::Tty::InjectFlow::wait(Context &c) noexcept
@brief 在条件锁内登记等待后解锁睡眠并复取锁。
@details 入口取得状态锁，退出统一解锁；软件输入满时在锁内登记可中断等待，释放锁睡眠并复取锁后重试，硬件接收仍不睡眠。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a5eb611a1a50b87d"></a>

## `xv::drivers::Tty::IoctlFlow::apply`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::apply(Context &c) noexcept
@brief 在状态锁下按请求清输入、切模式、发布设置并增加代数和唤醒。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c4d0e95a2a3ab488"></a>

## `xv::drivers::Tty::IoctlFlow::claim`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::claim(Context &c) noexcept
@brief 验证原参数并在状态锁内按挂断状态认领终端。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-559fa9a7aca1be86"></a>

## `xv::drivers::Tty::IoctlFlow::claim_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::claim_request(Context &c) noexcept
@brief 检查是否请求认领控制终端。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b4707816cd44fc43"></a>

## `xv::drivers::Tty::IoctlFlow::drain`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::drain(Context &c) noexcept
@brief 不持状态锁地排空输出。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e135004a4843cdd1"></a>

## `xv::drivers::Tty::IoctlFlow::draining`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::draining(Context &c) noexcept
@brief 检查本次 termios 设置是否要求排空输出。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-78d1777b4886b03e"></a>

## `xv::drivers::Tty::IoctlFlow::flush`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::flush(Context &c) noexcept
@brief 在状态锁内清输入、增加代数并唤醒。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5f4100acfed1bc19"></a>

## `xv::drivers::Tty::IoctlFlow::flush_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::flush_request(Context &c) noexcept
@brief 检查是否为受支持的输入清空请求。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e9d2287dd8f033cf"></a>

## `xv::drivers::Tty::IoctlFlow::group`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::group(Context &c) noexcept
@brief 读取前台组并向调用者复制结果。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-28df49f4810b7684"></a>

## `xv::drivers::Tty::IoctlFlow::group_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::group_request(Context &c) noexcept
@brief 检查是否读取前台进程组。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3cb230c39f45623d"></a>

## `xv::drivers::Tty::IoctlFlow::ok`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::ok(Context &c) noexcept
@brief 检查最近复制验证或写序列锁获取是否成功。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2e609e88e5333fd9"></a>

## `xv::drivers::Tty::IoctlFlow::read_settings`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::read_settings(Context &c) noexcept
@brief 复制并验证候选 termios。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6cc37a8416f17d38"></a>

## `xv::drivers::Tty::IoctlFlow::readable`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::readable(Context &c) noexcept
@brief 快照行规程可读字节数后复制输出。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e44787feb0e4fff9"></a>

## `xv::drivers::Tty::IoctlFlow::readable_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::readable_request(Context &c) noexcept
@brief 检查是否查询可读字节数。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ac7668158f77c27c"></a>

## `xv::drivers::Tty::IoctlFlow::release_writer`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::release_writer(Context &c) noexcept
@brief 在设置发布后释放写序列锁。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b3668124f1fa5de4"></a>

## `xv::drivers::Tty::IoctlFlow::set_group`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::set_group(Context &c) noexcept
@brief 先复制组号，再持状态锁设置并按成功结果唤醒。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-66499fc7cf663fcb"></a>

## `xv::drivers::Tty::IoctlFlow::set_group_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::set_group_request(Context &c) noexcept
@brief 检查是否设置前台进程组。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8b7a8aa9f137753c"></a>

## `xv::drivers::Tty::IoctlFlow::set_settings_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::set_settings_request(Context &c) noexcept
@brief 检查是否为三种 termios 设置请求。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6205757eeae20c5a"></a>

## `xv::drivers::Tty::IoctlFlow::set_window`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::set_window(Context &c) noexcept
@brief 先复制候选窗口大小，再在状态锁内发布。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5605b39a8322d1bd"></a>

## `xv::drivers::Tty::IoctlFlow::set_window_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::set_window_request(Context &c) noexcept
@brief 检查是否设置窗口大小。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-314457c85ca90adb"></a>

## `xv::drivers::Tty::IoctlFlow::settings`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::settings(Context &c) noexcept
@brief 在状态锁内快照 termios，解锁后复制输出。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2b68718b449d5f0d"></a>

## `xv::drivers::Tty::IoctlFlow::settings_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::settings_request(Context &c) noexcept
@brief 检查是否读取 termios。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1e98834c0eb86b73"></a>

## `xv::drivers::Tty::IoctlFlow::success`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::success(Context &c) noexcept
@brief 记录零成功码。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3a1d95e781e7ad36"></a>

## `xv::drivers::Tty::IoctlFlow::window`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::window(Context &c) noexcept
@brief 在状态锁内快照窗口大小，解锁后复制输出。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-51eb7247eb684fa6"></a>

## `xv::drivers::Tty::IoctlFlow::window_request`

```doxygen
@fn static bool xv::drivers::Tty::IoctlFlow::window_request(Context &c) noexcept
@brief 检查是否读取窗口大小。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8d730ff5cc5a5c67"></a>

## `xv::drivers::Tty::IoctlFlow::writer`

```doxygen
@fn static void xv::drivers::Tty::IoctlFlow::writer(Context &c) noexcept
@brief 可中断地取得写序列锁。
@details 模板嵌套分支声明请求去向；各命令保留原复制/锁次序。TCSETSW/F 先取写序列锁再 drain，状态锁只在发布设置时持有，发布后再释放写序列锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d3fc42014bdba2b9"></a>

## `xv::drivers::Tty::PollFlow::disconnected`

```doxygen
@fn static bool xv::drivers::Tty::PollFlow::disconnected(Context &c) noexcept
@brief 检查快照是否已挂断。
@details 状态快照和通道订阅在状态锁内完成；解锁后检查读写序列锁与输出容量，输出订阅即使写锁忙也执行，挂断保留可读与 HUP。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e2baa78fdba75444"></a>

## `xv::drivers::Tty::PollFlow::hangup`

```doxygen
@fn static void xv::drivers::Tty::PollFlow::hangup(Context &c) noexcept
@brief 发布原可读与挂断事件。
@details 状态快照和通道订阅在状态锁内完成；解锁后检查读写序列锁与输出容量，输出订阅即使写锁忙也执行，挂断保留可读与 HUP。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d6279f7200d99eea"></a>

## `xv::drivers::Tty::PollFlow::reader`

```doxygen
@fn static void xv::drivers::Tty::PollFlow::reader(Context &c) noexcept
@brief 按请求检查读序列锁并修正可读事件。
@details 状态快照和通道订阅在状态锁内完成；解锁后检查读写序列锁与输出容量，输出订阅即使写锁忙也执行，挂断保留可读与 HUP。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8e4c597200bbbbfe"></a>

## `xv::drivers::Tty::PollFlow::snapshot`

```doxygen
@fn static void xv::drivers::Tty::PollFlow::snapshot(Context &c) noexcept
@brief 订阅输入通道并按模式阈值保存事件与挂断快照。
@details 状态快照和通道订阅在状态锁内完成；解锁后检查读写序列锁与输出容量，输出订阅即使写锁忙也执行，挂断保留可读与 HUP。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a47cdf7e4d4c46f9"></a>

## `xv::drivers::Tty::PollFlow::writer`

```doxygen
@fn static void xv::drivers::Tty::PollFlow::writer(Context &c) noexcept
@brief 分别订阅写序列锁和输出容量，二者均满足才置可写。
@details 状态快照和通道订阅在状态锁内完成；解锁后检查读写序列锁与输出容量，输出订阅即使写锁忙也执行，挂断保留可读与 HUP。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f8bdc564b635b0d6"></a>

## `xv::drivers::Tty::ReadFlow::again`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::again(Context &c) noexcept
@brief 无已读字节时记录 EAGAIN 条件。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6960a3fd4c45a1cf"></a>

## `xv::drivers::Tty::ReadFlow::complete`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::complete(Context &c) noexcept
@brief 检查记录结束、请求满足或挂断。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-acc80bb629e49df3"></a>

## `xv::drivers::Tty::ReadFlow::drain`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::drain(Context &c) noexcept
@brief 直接消费当前可用记录，先复制再消费并处理 EOF 边界。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2f79074b8b4a5e9a"></a>

## `xv::drivers::Tty::ReadFlow::fault`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::fault(Context &c) noexcept
@brief 记录权限或进程中断造成的读失败。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a73fdeaad4e82ae2"></a>

## `xv::drivers::Tty::ReadFlow::finished`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::finished(Context &c) noexcept
@brief 检查是否因配置变化结束本次读取。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-54c6c4f4c5f84f47"></a>

## `xv::drivers::Tty::ReadFlow::generation`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::generation(Context &c) noexcept
@brief 按配置代数变化决定返回已有字节或重置计时快照。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6b56fbdea3714ed9"></a>

## `xv::drivers::Tty::ReadFlow::killed`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::killed(Context &) noexcept
@brief 在真正等待前检查进程中断。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-13e3ea7421e169a6"></a>

## `xv::drivers::Tty::ReadFlow::mode`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::mode(Context &c) noexcept
@brief 保存本轮规范模式、VMIN/VTIME 并按请求长度夹取最小量。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f1759c6d1b62fa25"></a>

## `xv::drivers::Tty::ReadFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::nonblocking(Context &c) noexcept
@brief 检查本次读取是否非阻塞。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9d039abb34ca1eed"></a>

## `xv::drivers::Tty::ReadFlow::permitted`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::permitted(Context &c) noexcept
@brief 复查前台读取权限。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bb7c1541bc3cfecd"></a>

## `xv::drivers::Tty::ReadFlow::raw_complete`

```doxygen
@fn static bool xv::drivers::Tty::ReadFlow::raw_complete(Context &c) noexcept
@brief 按原 VMIN/VTIME 条件决定原始模式读取是否完成。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6e987700133ae1a7"></a>

## `xv::drivers::Tty::ReadFlow::wait`

```doxygen
@fn static void xv::drivers::Tty::ReadFlow::wait(Context &c) noexcept
@brief 发布计时等待、登记依赖、解锁睡眠并复取锁清计时标志。
@details 读者 SleepLock 保持原词法作用域；图运行时持状态锁，等待阶段按原协议释放再复取。字节批量消费保留复制失败不消费、EOF 边界和部分读优先返回。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bdcc164ee45db38b"></a>

## `xv::drivers::Tty::WriteFlow::advance`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::advance(Context &c) noexcept
@brief 增加已完成源字节数。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ac037a5d16228bda"></a>

## `xv::drivers::Tty::WriteFlow::failure`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::failure(Context &c) noexcept
@brief 按已完成源字节数优先返回或记录失败。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-af06972a91dd22ef"></a>

## `xv::drivers::Tty::WriteFlow::killed`

```doxygen
@fn static bool xv::drivers::Tty::WriteFlow::killed(Context &) noexcept
@brief 在下一块复制前检查进程中断。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d2fd97b989103e96"></a>

## `xv::drivers::Tty::WriteFlow::ok`

```doxygen
@fn static bool xv::drivers::Tty::WriteFlow::ok(Context &c) noexcept
@brief 检查本块源复制是否成功。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a43322a79841239a"></a>

## `xv::drivers::Tty::WriteFlow::read`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::read(Context &c) noexcept
@brief 选择至多32个源字节并复制；非阻塞输出也先尝试批量发送。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4fa7fa627ac5a23c"></a>

## `xv::drivers::Tty::WriteFlow::remaining`

```doxygen
@fn static bool xv::drivers::Tty::WriteFlow::remaining(Context &c) noexcept
@brief 检查是否仍有源字节未输出。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f7d55c565a63d098"></a>

## `xv::drivers::Tty::WriteFlow::sent`

```doxygen
@fn static bool xv::drivers::Tty::WriteFlow::sent(Context &c) noexcept
@brief 检查后端是否接受完整变换块。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5f51e911ea771f67"></a>

## `xv::drivers::Tty::WriteFlow::short_write`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::short_write(Context &c) noexcept
@brief 保留原部分源字节或后端短写结果。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-87171b41092901ed"></a>

## `xv::drivers::Tty::WriteFlow::success`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::success(Context &c) noexcept
@brief 记录整个写请求完成。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-adfe8ebbd243677f"></a>

## `xv::drivers::Tty::WriteFlow::transform`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::transform(Context &c) noexcept
@brief 直接按 OPOST/ONLCR 规则展开当前块。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fce2cc597b0d2794"></a>

## `xv::drivers::Tty::WriteFlow::transmit`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::transmit(Context &c) noexcept
@brief 向原后端提交完整变换块。
@details 写者 SleepLock 和未初始化的固定缓冲由原调用帧持有；输出标志按原规则只快照一次。每块先检查中断、复制和转换，再发送；返回值按原源字节与部分成功规则计算。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-205cdd66dd1702c0"></a>

## `xv::drivers::Tty::WriteFlow::retry`

```doxygen
@fn static bool xv::drivers::Tty::WriteFlow::retry(Context &c) noexcept
@brief 判断是否因非阻塞队列空间不足而重试更小的源前缀。
@details 仅对nonblocking、EAGAIN及part大于1重试；其他错误或最小前缀失败保留已有源字节计数后返回。
```

<a id="fn-0365b3b870b056cb"></a>

## `xv::drivers::Tty::WriteFlow::shrink`

```doxygen
@fn static void xv::drivers::Tty::WriteFlow::shrink(Context &c) noexcept
@brief 将待发送源前缀向上取半，保持进度不变。
@details 失败块未被TerminalOutput消费；保留input缓冲，返回Transform重新展开ONLCR。每次减小part，最迟缩至1个源字节；完整CRLF不会拆分或重复。
```
