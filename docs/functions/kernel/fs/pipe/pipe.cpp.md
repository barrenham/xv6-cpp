# `kernel/fs/pipe/pipe.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/pipe/pipe.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:pipe`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Pipe::close`](pipe.cpp.md#fn-bc84a084f8763d9f)
- [`xv::fs::Pipe::initialize`](pipe.cpp.md#fn-506325f4cc7a1e4d)
- [`xv::fs::Pipe::poll`](pipe.cpp.md#fn-394c4a6f8b4d5c75)
- [`xv::fs::Pipe::read`](pipe.cpp.md#fn-64b695cc57f76b14)
- [`xv::fs::Pipe::write`](pipe.cpp.md#fn-dd4843c53ed371db)

<a id="fn-394c4a6f8b4d5c75"></a>

## `xv::fs::Pipe::poll`

```doxygen
@fn unsigned xv::fs::Pipe::poll(bool writable, xv::sync::PollTable *table)
@brief 有界管道环形缓冲：查询当前就绪状态，并在提供表时登记通知依赖。
@param writable 本次操作使用的 可写条件 值。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `table->watch`。
@details 需要特别跟踪的条件：`table`；`writable`；`!reader_open_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`POLLERR`；`written_ - read_ < Capacity ? POLLOUT : 0`；`events | (writer_open_ ? 0 : POLLHUP)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-506325f4cc7a1e4d"></a>

## `xv::fs::Pipe::initialize`

```doxygen
@fn void xv::fs::Pipe::initialize()
@brief 有界管道环形缓冲：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bc84a084f8763d9f"></a>

## `xv::fs::Pipe::close`

```doxygen
@fn void xv::fs::Pipe::close(bool writable)
@brief 有界管道环形缓冲：关闭当前端点或释放文件关联。
@param writable 本次操作使用的 可写条件 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::wakeup` → `lock_.release` → `Pipe` → `detail::free_page`。
@details 本函数可见的状态更新包括：`writer_open_ = 0;`；`reader_open_ = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`writable`；`reader_open_ == 0 && writer_open_ == 0`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-dd4843c53ed371db"></a>

## `xv::fs::Pipe::write`

```doxygen
@fn int xv::fs::Pipe::write(uint64 addr, int n, bool nonblocking)
@brief 有界管道环形缓冲：提交输入数据并报告传输结果。
@param addr 本次操作使用的 addr 值。
@param n 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 不超过管道容量的小写请求按整体空间等待或检查，避免小写交错。大写可分段，边写边通知读端；无读者或信号中断时按已有进展返回，非阻塞无空间返回 EAGAIN。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::killed` → `lock_.release` → `detail::prepare_interruptible_sleep` → `detail::sleep` → `detail::wakeup` → `detail::copy_in`。
@details 本函数可见的状态更新包括：`data_[written_++ % Capacity] = ch;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`n <= 0`；`static_cast<unsigned>(n) <= Capacity && Capacity - (written_ - read_) < static_cast<unsigned>(n)`；`!reader_open_ || detail::killed()`；`nonblocking`；`i < n`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`n == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`i ? i : -1`；`i ? i : -EAGAIN`；`i`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-64b695cc57f76b14"></a>

## `xv::fs::Pipe::read`

```doxygen
@fn int xv::fs::Pipe::read(uint64 addr, int n, bool nonblocking)
@brief 有界管道环形缓冲：读取已有数据并按接口返回读取结果。
@param addr 本次操作使用的 addr 值。
@param n 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 持管道锁检查数据和写端存活；空且写端仍开时登记可中断等待或返回 EAGAIN。读取推进读游标并通知写端，写端关闭且排空后才是 EOF。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::killed` → `lock_.release` → `detail::prepare_interruptible_sleep` → `detail::sleep` → `detail::copy_out` → `detail::wakeup`。
@details 需要特别跟踪的条件：`n <= 0`；`read_ == written_ && writer_open_`；`detail::killed()`；`nonblocking`；`read_ == written_`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`n == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`i`。应结合上面的条件检查区分正常值、空值和错误值。
```
