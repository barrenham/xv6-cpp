# `kernel/drivers/tty/core/tty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/core/tty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:tty`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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

<a id="fn-e69f7ea6763472d1"></a>

## `xv::drivers::valid`

```doxygen
@fn bool xv::drivers::valid(const termios &value)
@brief 设备对象、MMIO、设备中断与终端数据流：有效性。
@param value 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 返回类型为 `bool (const termios &)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::process_killed` → `lock_.release` → `discipline_.receive` → `detail::post_terminal_interrupt` → `detail::wake_processes` → `detail::prepare_interruptible_sleep` → `detail::sleep_process`。
@details 需要特别跟踪的条件：`hungup_ || detail::process_killed()`；`action != InputAction::Retry`；`action == InputAction::Interrupt || action == InputAction::Suspend`；`nonblocking`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：`-1`；`0`；`-EAGAIN`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `table->watch` → `discipline_.available` → `lock_.release` → `table->events` → `read_lock_.available` → `write_lock_.available` → `output_.writable`。
@details 需要特别跟踪的条件：`table`；`!(settings_.c_lflag & ICANON) && !settings_.c_cc[VTIME] && settings_.c_cc[VMIN]`；`disconnected`；`(!table || (table->events() & POLLIN)) && !read_lock_.available(table)`；`!table || (table->events() & POLLOUT)`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`events | POLLIN | POLLHUP`；`events`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::may_read_terminal` → `detail::process_killed` → `read_lock_.try_acquire` → `read_lock_.acquire_interruptible` → `reader` → `lock_.acquire` → `detail::terminal_milliseconds` → `discipline_.available` → `discipline_.front` → `discipline_.consume` → `detail::copy_out` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`timed_wait_ = timeout && (!minimum || done);`；`timed_wait_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`count <= 0`；`!detail::may_read_terminal(this)`；`detail::process_killed()`；`destination.nonblocking`；`!read_lock_.try_acquire()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`read_lock_.try_acquire` → `read_lock_.acquire_interruptible` → `lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : (fault ? -1 : (again ? -EAGAIN : 0))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7a0276d465b23c53"></a>

## `xv::drivers::Tty::write`

```doxygen
@fn int xv::drivers::Tty::write(IoBuffer source, int count)
@brief 终端核心：提交输入数据并报告传输结果。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 写序列锁维护输出顺序，按 termios 转换原始输入字符。非阻塞转换单元要求后端完整接受或失败，返回值按原始输入字节计算，防止重试把半个 CRLF 重复写出。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::process_killed` → `write_lock_.try_acquire` → `write_lock_.acquire_interruptible` → `writer` → `lock_.acquire` → `lock_.release` → `detail::copy_in` → `output_.transmit`。
@details 本函数可见的状态更新包括：`output[used++] = '\r';`；`output[used++] = input[i];`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`count <= 0`；`detail::process_killed()`；`source.nonblocking`；`!write_lock_.try_acquire()`；`!write_lock_.acquire_interruptible()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`write_lock_.try_acquire` → `write_lock_.acquire_interruptible` → `lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : -1`；`done ? done : result`；`done`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3625e17fbe0b06ab"></a>

## `xv::drivers::Tty::ioctl`

```doxygen
@fn int xv::drivers::Tty::ioctl(unsigned request, IoBuffer argument)
@brief 终端核心：分派设备控制请求并处理参数复制。
@param request 本次操作使用的 请求 值。
@param argument 本次操作使用的 argument 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::claim_terminal` → `detail::terminal_foreground` → `detail::copy_out` → `detail::copy_in` → `detail::set_terminal_foreground` → `detail::wake_processes` → `lock_.acquire` → `lock_.release` → `valid` → `write_lock_.acquire_interruptible` → `output_.drain`。
@details 本函数可见的状态更新包括：`settings_ = value;`；`size_ = value;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`request == TIOCSCTTY`；`argument.address != 0`；`request == TIOCGPGRP`；`group < 0`；`request == TIOCSPGRP`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release` → `write_lock_.acquire_interruptible` → `write_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`hungup_ ? -1 : detail::claim_terminal(this)`；`detail::copy_out(user, argument.address, &group, sizeof(group))`；`result`；`detail::copy_out(user, argument.address, &value, sizeof(value))`；`0`；`detail::copy_out(user, argument.address, &count, sizeof(count))`。应结合上面的条件检查区分正常值、空值和错误值。
```
