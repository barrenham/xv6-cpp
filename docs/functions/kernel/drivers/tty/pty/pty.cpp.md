# `kernel/drivers/tty/pty/pty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/pty/pty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:pty`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::CreatePtyFlow::allocate`](pty.cpp.md#fn-9fca282de39b5271)
- [`xv::drivers::CreatePtyFlow::allocated`](pty.cpp.md#fn-344b4c00f4005b69)
- [`xv::drivers::CreatePtyFlow::construct`](pty.cpp.md#fn-b21951d44177cf08)
- [`xv::drivers::CreatePtyFlow::publish`](pty.cpp.md#fn-3dfbe41e687d5f54)
- [`xv::drivers::CreatePtyFlow::register_pair`](pty.cpp.md#fn-5801084a71a447cd)
- [`xv::drivers::OpenNamedPtyFlow::attach`](pty.cpp.md#fn-9b43999f37a48661)
- [`xv::drivers::OpenNamedPtyFlow::find`](pty.cpp.md#fn-8df6da1be0582016)
- [`xv::drivers::OpenNamedPtyFlow::found`](pty.cpp.md#fn-f9bd84b466348a78)
- [`xv::drivers::OpenNamedPtyFlow::publish`](pty.cpp.md#fn-49eb6b864474f842)
- [`xv::drivers::OpenNamedPtyFlow::retain`](pty.cpp.md#fn-53cf4fee37e8a00b)
- [`xv::drivers::Pty::CloseFlow::disconnect`](pty.cpp.md#fn-c18209482fe2e356)
- [`xv::drivers::Pty::CloseFlow::hangup`](pty.cpp.md#fn-c13e0e67f0b44287)
- [`xv::drivers::Pty::CloseFlow::master`](pty.cpp.md#fn-d007f6b16e6e7bfe)
- [`xv::drivers::Pty::CloseFlow::release`](pty.cpp.md#fn-a9cb03713786c4db)
- [`xv::drivers::Pty::CloseFlow::session`](pty.cpp.md#fn-6c37cfa75a436efc)
- [`xv::drivers::Pty::CloseFlow::terminal`](pty.cpp.md#fn-91e2ed6ff45ccb5f)
- [`xv::drivers::Pty::DrainFlow::pending`](pty.cpp.md#fn-5ce9e203ece17acb)
- [`xv::drivers::Pty::DrainFlow::wait`](pty.cpp.md#fn-867cf6113fa32eb6)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::delegate`](pty.cpp.md#fn-9113e62d2a85b3e3)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::lock`](pty.cpp.md#fn-decfdf34bc961dcc)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::lock_request`](pty.cpp.md#fn-efa66fd4a7d4a43b)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::master_claim`](pty.cpp.md#fn-4b2793e6576382f8)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::master_readable`](pty.cpp.md#fn-3db5ecc93b11e834)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::number`](pty.cpp.md#fn-0cdf173f081047ca)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::number_request`](pty.cpp.md#fn-81cfff93d301d426)
- [`xv::drivers::Pty::Endpoint::IoctlFlow::readable`](pty.cpp.md#fn-8832ff9cac5ff276)
- [`xv::drivers::Pty::Endpoint::PollFlow::connected`](pty.cpp.md#fn-8e0beb055af0e7fa)
- [`xv::drivers::Pty::Endpoint::PollFlow::hangup`](pty.cpp.md#fn-cbba84ded03c0ac9)
- [`xv::drivers::Pty::Endpoint::PollFlow::reader`](pty.cpp.md#fn-7785737302fbdaa3)
- [`xv::drivers::Pty::Endpoint::PollFlow::snapshot`](pty.cpp.md#fn-4492ce825689fbec)
- [`xv::drivers::Pty::Endpoint::PollFlow::writer`](pty.cpp.md#fn-c30865e9f68bedea)
- [`xv::drivers::Pty::Endpoint::close`](pty.cpp.md#fn-2f858e49784ccac1)
- [`xv::drivers::Pty::Endpoint::ioctl`](pty.cpp.md#fn-6b6a9388f115d902)
- [`xv::drivers::Pty::Endpoint::poll`](pty.cpp.md#fn-e30ef808741bf1eb)
- [`xv::drivers::Pty::Endpoint::read`](pty.cpp.md#fn-2fe0922873329d2b)
- [`xv::drivers::Pty::Endpoint::write`](pty.cpp.md#fn-fce9e782c37be567)
- [`xv::drivers::Pty::Output::drain`](pty.cpp.md#fn-f8bb828f54046dc7)
- [`xv::drivers::Pty::Output::put_sync`](pty.cpp.md#fn-c576e0a8d12cb145)
- [`xv::drivers::Pty::Output::transmit`](pty.cpp.md#fn-ee0c89905868fc1c)
- [`xv::drivers::Pty::Output::writable`](pty.cpp.md#fn-b1d57a724fdf31ed)
- [`xv::drivers::Pty::ReadMasterFlow::again`](pty.cpp.md#fn-3ba8453f14f5d53b)
- [`xv::drivers::Pty::ReadMasterFlow::drain`](pty.cpp.md#fn-d831255dfffa6a10)
- [`xv::drivers::Pty::ReadMasterFlow::killed`](pty.cpp.md#fn-7bd2f3c03eb680a1)
- [`xv::drivers::Pty::ReadMasterFlow::nonblocking`](pty.cpp.md#fn-7153ecb09a8c7995)
- [`xv::drivers::Pty::ReadMasterFlow::wait`](pty.cpp.md#fn-e2cff60233df0c70)
- [`xv::drivers::Pty::ReadMasterFlow::waiting`](pty.cpp.md#fn-3a21d666a6580bb1)
- [`xv::drivers::Pty::ReleaseFlow::destroy`](pty.cpp.md#fn-b9f146d56859543b)
- [`xv::drivers::Pty::ReleaseFlow::free_page`](pty.cpp.md#fn-48347091a34c74a8)
- [`xv::drivers::Pty::ReleaseFlow::unlink`](pty.cpp.md#fn-b47cadf582744aa2)
- [`xv::drivers::Pty::ReleaseFlow::unlock`](pty.cpp.md#fn-402e61b70895ae2b)
- [`xv::drivers::Pty::TransmitFlow::again`](pty.cpp.md#fn-5d5600563141891a)
- [`xv::drivers::Pty::TransmitFlow::nonblocking`](pty.cpp.md#fn-b8b386182a0dd4d7)
- [`xv::drivers::Pty::TransmitFlow::publish`](pty.cpp.md#fn-a33be44a91cd0c49)
- [`xv::drivers::Pty::TransmitFlow::space`](pty.cpp.md#fn-7705f7ff73ad8a08)
- [`xv::drivers::Pty::TransmitFlow::stopped`](pty.cpp.md#fn-937c01c96e1fa1f7)
- [`xv::drivers::Pty::TransmitFlow::wait`](pty.cpp.md#fn-5f92a24b3ddf8366)
- [`xv::drivers::Pty::WriteMasterFlow::accepted`](pty.cpp.md#fn-b9ff70461b3a64af)
- [`xv::drivers::Pty::WriteMasterFlow::complete`](pty.cpp.md#fn-fbe7c2a66a03dfc6)
- [`xv::drivers::Pty::WriteMasterFlow::connection`](pty.cpp.md#fn-bf472ed232337226)
- [`xv::drivers::Pty::WriteMasterFlow::copy`](pty.cpp.md#fn-f449f32b9575c6f5)
- [`xv::drivers::Pty::WriteMasterFlow::failure`](pty.cpp.md#fn-c9f9584d4831c0d1)
- [`xv::drivers::Pty::WriteMasterFlow::inject`](pty.cpp.md#fn-295be3660a8d9c2e)
- [`xv::drivers::Pty::WriteMasterFlow::ok`](pty.cpp.md#fn-2a6775ff889fe14b)
- [`xv::drivers::Pty::WriteMasterFlow::remaining`](pty.cpp.md#fn-428433811a5e17ca)
- [`xv::drivers::Pty::WriteMasterFlow::short_write`](pty.cpp.md#fn-d718eaa67a033970)
- [`xv::drivers::Pty::WriteMasterFlow::stopped`](pty.cpp.md#fn-5ab195e563374f17)
- [`xv::drivers::Pty::WriteMasterFlow::success`](pty.cpp.md#fn-7ac82b18d50c5f67)
- [`xv::drivers::Pty::attach_slave`](pty.cpp.md#fn-71b2427af9997333)
- [`xv::drivers::Pty::close`](pty.cpp.md#fn-5c7a739984ad05b7)
- [`xv::drivers::Pty::initialize`](pty.cpp.md#fn-87ffb0376d0f4542)
- [`xv::drivers::Pty::read_master`](pty.cpp.md#fn-26ae6ac8d5631f7d)
- [`xv::drivers::Pty::release`](pty.cpp.md#fn-c67e15beca56bec0)
- [`xv::drivers::Pty::write_master`](pty.cpp.md#fn-f40800f626106299)
- [`xv::drivers::PtyMultiplexer::OpenFlow::allocate`](pty.cpp.md#fn-8f8757ac526b3c1e)
- [`xv::drivers::PtyMultiplexer::OpenFlow::allocated`](pty.cpp.md#fn-1d17de6ca341ec5f)
- [`xv::drivers::PtyMultiplexer::OpenFlow::available`](pty.cpp.md#fn-8cb489526758f559)
- [`xv::drivers::PtyMultiplexer::OpenFlow::construct`](pty.cpp.md#fn-02f32ce119b7d8cb)
- [`xv::drivers::PtyMultiplexer::OpenFlow::find`](pty.cpp.md#fn-93c0b044fd7d15eb)
- [`xv::drivers::PtyMultiplexer::OpenFlow::publish`](pty.cpp.md#fn-da1cab0045878fc4)
- [`xv::drivers::PtyMultiplexer::OpenFlow::rollback`](pty.cpp.md#fn-3701359af3df7d88)
- [`xv::drivers::PtyMultiplexer::initialize`](pty.cpp.md#fn-00ffdebf9a16a0e5)
- [`xv::drivers::PtyMultiplexer::name`](pty.cpp.md#fn-669cf64d505a317d)
- [`xv::drivers::PtyMultiplexer::open`](pty.cpp.md#fn-6c66c151d344922e)
- [`xv::drivers::PtyMultiplexer::put_sync`](pty.cpp.md#fn-8d65e03b3b216626)
- [`xv::drivers::PtyMultiplexer::read`](pty.cpp.md#fn-52ac1df58dda7779)
- [`xv::drivers::PtyMultiplexer::write`](pty.cpp.md#fn-bd78a775363ec033)
- [`xv::drivers::create_pty`](pty.cpp.md#fn-8801034b0305377a)
- [`xv::drivers::initialize_ptys`](pty.cpp.md#fn-0c7545d488e75354)
- [`xv::drivers::named_pty_ids`](pty.cpp.md#fn-e22085482cfe7284)
- [`xv::drivers::named_pty_owner`](pty.cpp.md#fn-e0fbfb0d92d6e5b5)
- [`xv::drivers::open_named_pty`](pty.cpp.md#fn-e64b849fb108aed9)
- [`xv::drivers::ptmx`](pty.cpp.md#fn-1ed26c2173290846)
- [`xv::drivers::pty_timer_tick`](pty.cpp.md#fn-b050f1554cf27075)
- [`xv::drivers::release_named_pty`](pty.cpp.md#fn-46f405daa2728273)
- [`xv::drivers::retain_named_pty`](pty.cpp.md#fn-9706e46ba2bd1bf4)

<a id="fn-0c7545d488e75354"></a>

## `xv::drivers::initialize_ptys`

```doxygen
@fn void xv::drivers::initialize_ptys()
@brief 设备对象、MMIO、设备中断与终端数据流：初始化ptys。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ptys_lock.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b050f1554cf27075"></a>

## `xv::drivers::pty_timer_tick`

```doxygen
@fn void xv::drivers::pty_timer_tick()
@brief 设备对象、MMIO、设备中断与终端数据流：pty定时器tick。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `pair->timer_tick`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8801034b0305377a"></a>

## `xv::drivers::create_pty`

```doxygen
@fn bool xv::drivers::create_pty(CharacterDevice **master, CharacterDevice **slave)
@brief 设备对象、MMIO、设备中断与终端数据流：分配并发布一对匿名 PTY 端点。
@param master 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param slave 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 CreatePtyFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-87ffb0376d0f4542"></a>

## `xv::drivers::Pty::initialize`

```doxygen
@fn void xv::drivers::Pty::initialize()
@brief 匿名伪终端对：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init` → `input_lock_.init` → `reader_lock_.init` → `terminal_.initialize`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c67e15beca56bec0"></a>

## `xv::drivers::Pty::release`

```doxygen
@fn void xv::drivers::Pty::release()
@brief 匿名伪终端对：释放当前引用或归还对象占用的资源。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5c7a739984ad05b7"></a>

## `xv::drivers::Pty::close`

```doxygen
@fn void xv::drivers::Pty::close(bool master)
@brief 关闭一个主端或从端会话并归还对象引用。
@details 注册锁保护发布状态与 slave_files，输出锁保护连接标志和等待通道。主端最终关闭撤下设备名并挂断终端；只有最后从端关闭才切断主端读连接。命名从端关闭不永久挂断 TTY，以允许主端存续期间重新打开；匿名 PTY 保留原挂断行为。
@details 流程由 CloseFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-ee0c89905868fc1c"></a>

## `xv::drivers::Pty::Output::transmit`

```doxygen
@fn int xv::drivers::Pty::Output::transmit(const char *data, int count, bool nonblocking)
@brief 终端输出后端：向输出后端完整提交一个转换单元或小块。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 以 PTY 输出锁保护 written/read 环形计数，整个转换块能装下才入队。满时在读游标通道等待；master 关闭、信号及 nonblocking 分别走错误或 EAGAIN 路径，成功后唤醒 master 读者。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：`-1`；`-EAGAIN`；`count`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 TransmitFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-c576e0a8d12cb145"></a>

## `xv::drivers::Pty::Output::put_sync`

```doxygen
@fn void xv::drivers::Pty::Output::put_sync(int character)
@brief 终端输出后端：通过不能睡眠的输出路径提交一个字符。
@param character 要输出的单个字符值。
@details 回显由持 TTY 自旋锁的输入路径调用，不能等待 master 排空。这里只在输出队列有位置时提交字符，满时仍沿用尽力回显策略；普通 write 的背压不能套用于此路径。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`p.data_[p.written_++ % Capacity] = static_cast<char>(character);`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`p.master_open_ && p.written_ - p.read_ < Capacity`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b1d57a724fdf31ed"></a>

## `xv::drivers::Pty::Output::writable`

```doxygen
@fn bool xv::drivers::Pty::Output::writable(xv::sync::PollTable *table)
@brief 终端输出后端：读取或计算 `pair_.master_open_ && pair_.written_ - pair_.read_ <= Capacity - 2`，返回当前查询结果。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `table->watch`。
@details 需要特别跟踪的条件：`table`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`pair_.master_open_ && pair_.written_ - pair_.read_ <= Capacity - 2`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f8bb828f54046dc7"></a>

## `xv::drivers::Pty::Output::drain`

```doxygen
@fn void xv::drivers::Pty::Output::drain()
@brief 终端输出后端：等待已提交输出排空。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 DrainFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-26ae6ac8d5631f7d"></a>

## `xv::drivers::Pty::read_master`

```doxygen
@fn int xv::drivers::Pty::read_master(IoBuffer destination, int count)
@brief 匿名伪终端对：从 PTY 输出队列读取 slave 产生的数据。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : (fault ? -1 : 0)`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReadMasterFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-f40800f626106299"></a>

## `xv::drivers::Pty::write_master`

```doxygen
@fn int xv::drivers::Pty::write_master(IoBuffer source, int count)
@brief 匿名伪终端对：把 master 输入按 slave 的行规程注入。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : -1`；`done ? done : result`；`done`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 WriteMasterFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2fe0922873329d2b"></a>

## `xv::drivers::Pty::Endpoint::read`

```doxygen
@fn int xv::drivers::Pty::Endpoint::read(IoBuffer destination, int count)
@brief PTY 文件端点：读取已有数据并按接口返回读取结果。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_.read_master` → `pair_.terminal_.read`。
@return 各分支实际返回：`master_ ? pair_.read_master(destination, count) : pair_.terminal_.read(destination, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fce9e782c37be567"></a>

## `xv::drivers::Pty::Endpoint::write`

```doxygen
@fn int xv::drivers::Pty::Endpoint::write(IoBuffer source, int count)
@brief PTY 文件端点：提交输入数据并报告传输结果。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_.write_master` → `pair_.terminal_.write`。
@return 各分支实际返回：`master_ ? pair_.write_master(source, count) : pair_.terminal_.write(source, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6b6a9388f115d902"></a>

## `xv::drivers::Pty::Endpoint::ioctl`

```doxygen
@fn int xv::drivers::Pty::Endpoint::ioctl(unsigned request, IoBuffer argument)
@brief 处理 PTY 编号和锁请求，其余终端控制转发 TTY。
@details TIOCGPTN 仅对命名主端返回编号，TIOCSPTLCK 复制用户参数后在注册锁内改变从端打开锁；错误用户指针失败。主端禁止成为控制终端，FIONREAD 返回主端输出队列字节数；其余请求沿用 TTY。
@details 流程由 IoctlFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-e30ef808741bf1eb"></a>

## `xv::drivers::Pty::Endpoint::poll`

```doxygen
@fn unsigned xv::drivers::Pty::Endpoint::poll(xv::sync::PollTable *table)
@brief PTY 文件端点：查询当前就绪状态，并在提供表时登记通知依赖。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@return 各分支实际返回：`pair_.terminal_.poll(table)`；`events | POLLIN | POLLHUP`；`events`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 PollFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2f858e49784ccac1"></a>

## `xv::drivers::Pty::Endpoint::close`

```doxygen
@fn void xv::drivers::Pty::Endpoint::close()
@brief PTY 文件端点：关闭当前端点或释放文件关联。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_.close`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
<a id="fn-1ed26c2173290846"></a>

## `xv::drivers::ptmx`

```doxygen
@fn CharacterDevice &xv::drivers::ptmx()
@brief 借用静态 PTY 多路复用工厂。
@details 板级把此对象登记为 /dev/ptmx；它本身不传输数据，每次 open 才分配独立主端及终端。
```

<a id="fn-9706e46ba2bd1bf4"></a>

## `xv::drivers::retain_named_pty`

```doxygen
@fn bool xv::drivers::retain_named_pty(unsigned number)
@brief 按已发布编号取得一个 PTY 节点固定引用。
@details 在 PTY 注册锁内查找仍有主端的命名对并增加 references，失败返回 false。devfs 动态 inode 在首个引用时固定该编号，避免仍被旧节点引用的编号提前复用。
```

<a id="fn-46f405daa2728273"></a>

## `xv::drivers::release_named_pty`

```doxygen
@fn void xv::drivers::release_named_pty(unsigned number)
@brief 归还一个 devfs 节点持有的 PTY 固定引用。
@details 调用者已经持有对应编号的固定引用，先在注册锁内定位稳定对象，释放注册锁后调用 release。最后引用删除注册记录并回收对象页；节点固定引用保证查找后对象不会提前释放。
```

<a id="fn-e64b849fb108aed9"></a>

## `xv::drivers::open_named_pty`

```doxygen
@fn CharacterDevice *xv::drivers::open_named_pty(unsigned number)
@brief 取得一个解锁且仍有主端的 PTY 从端会话。
@details 注册锁内验证编号、发布状态和从端锁，增加会话引用及 slave_files，再在输出锁内恢复连接状态。多个独立打开共用同一终端，每次成功都对应一次 close；失败不取得引用。
@details 流程由 OpenNamedPtyFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-e22085482cfe7284"></a>

## `xv::drivers::named_pty_ids`

```doxygen
@fn unsigned xv::drivers::named_pty_ids(unsigned *numbers, unsigned capacity)
@brief 按编号顺序复制当前已发布 PTY 列表。
@details 持注册锁扫描，最多写 capacity 个编号并返回数量；主端最终关闭后不再列出，旧从端或节点仍可保留对象和编号。输出不含裸对象指针。
```

<a id="fn-71b2427af9997333"></a>

## `xv::drivers::Pty::attach_slave`

```doxygen
@fn void xv::drivers::Pty::attach_slave()
@brief 在新从端会话建立时恢复连接状态。
@details 调用者持 PTY 注册锁；内部取得输出锁，设置 slave_open_ 并唤醒主端读等待。命名对最后从端关闭后可以再次打开，终端设置保持。
```

<a id="fn-669cf64d505a317d"></a>

## `xv::drivers::PtyMultiplexer::name`

```doxygen
@fn const char *xv::drivers::PtyMultiplexer::name() const override
@brief 返回 PTY 工厂设备名 ptmx。
@details 返回静态字符串，登记设备名称时不分配会话。
```

<a id="fn-00ffdebf9a16a0e5"></a>

## `xv::drivers::PtyMultiplexer::initialize`

```doxygen
@fn void xv::drivers::PtyMultiplexer::initialize() override
@brief 保留无额外初始化工作的设备接口。
@details 全局 PTY 列表锁由 initialize_ptys 提前初始化，会话初始化放在 open 中完成。
```

<a id="fn-8d65e03b3b216626"></a>

## `xv::drivers::PtyMultiplexer::put_sync`

```doxygen
@fn void xv::drivers::PtyMultiplexer::put_sync(int) override
@brief 忽略对工厂本身的同步输出。
@details 工厂不代表一个终端会话，实际主从端在 open 成功后返回。
```

<a id="fn-52ac1df58dda7779"></a>

## `xv::drivers::PtyMultiplexer::read`

```doxygen
@fn int xv::drivers::PtyMultiplexer::read(IoBuffer, int) override
@brief 拒绝直接读取工厂对象。
@details 返回 -1；File 从 open 取得具体主端后只对会话执行 I/O。
```

<a id="fn-bd78a775363ec033"></a>

## `xv::drivers::PtyMultiplexer::write`

```doxygen
@fn int xv::drivers::PtyMultiplexer::write(IoBuffer, int) override
@brief 拒绝直接写入工厂对象。
@details 返回 -1；数据传输由 open 返回的具体主端实现。
```

<a id="fn-6c66c151d344922e"></a>

## `xv::drivers::PtyMultiplexer::open`

```doxygen
@fn CharacterDevice *xv::drivers::PtyMultiplexer::open(bool)
@brief 打开新的 PTY 主端并记录会话拥有者。
@details 按页创建 Pty，保存调用者有效 uid/gid；发布编号前准备两端引用，任一分配失败归还页和端点。节点编号仍受活动会话及 inode 固定引用保护。
@details 流程由 OpenFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-e0fbfb0d92d6e5b5"></a>

## `xv::drivers::named_pty_owner`

```doxygen
@fn bool xv::drivers::named_pty_owner(unsigned number, unsigned &uid, unsigned &gid)
@brief 查询活动命名 PTY 的创建者。
@details 持 ptys_lock 查找编号并复制 uid/gid，找不到返回 false；不返回设备对象指针，不延长会话寿命。
```

<a id="fn-c18209482fe2e356"></a>

## `xv::drivers::Pty::CloseFlow::disconnect`

```doxygen
@fn static void xv::drivers::Pty::CloseFlow::disconnect(Context &c) noexcept
@brief 按原引用规则更新端点状态与发布标志并唤醒读写者。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c13e0e67f0b44287"></a>

## `xv::drivers::Pty::CloseFlow::hangup`

```doxygen
@fn static bool xv::drivers::Pty::CloseFlow::hangup(Context &c) noexcept
@brief 按主端或匿名最后从端规则判断是否挂断终端。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d007f6b16e6e7bfe"></a>

## `xv::drivers::Pty::CloseFlow::master`

```doxygen
@fn static bool xv::drivers::Pty::CloseFlow::master(Context &c) noexcept
@brief 检查本次关闭是否为主端。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a9cb03713786c4db"></a>

## `xv::drivers::Pty::CloseFlow::release`

```doxygen
@fn static void xv::drivers::Pty::CloseFlow::release(Context &c) noexcept
@brief 归还关闭端点的引用，最后引用可回收整个对。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6c37cfa75a436efc"></a>

## `xv::drivers::Pty::CloseFlow::session`

```doxygen
@fn static void xv::drivers::Pty::CloseFlow::session(Context &c) noexcept
@brief 在两自旋锁外处理原会话挂断协议。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-91e2ed6ff45ccb5f"></a>

## `xv::drivers::Pty::CloseFlow::terminal`

```doxygen
@fn static void xv::drivers::Pty::CloseFlow::terminal(Context &c) noexcept
@brief 执行原 TTY 挂断发布。
@details 按全局 PTY 锁再输出锁次序更新发布和端点状态，唤醒后释放两锁；随后才处理会话与终端挂断，最后 release 可结束对象生命周期。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5ce9e203ece17acb"></a>

## `xv::drivers::Pty::DrainFlow::pending`

```doxygen
@fn static bool xv::drivers::Pty::DrainFlow::pending(Context &c) noexcept
@brief 按原读取次序检查是否仍需要排空等待。
@details 调用者持输出锁；仅在仍有输出、主端存活且进程未中断时等待，等待登记在条件锁内，睡眠后迭代复查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-867cf6113fa32eb6"></a>

## `xv::drivers::Pty::DrainFlow::wait`

```doxygen
@fn static void xv::drivers::Pty::DrainFlow::wait(Context &c) noexcept
@brief 登记输出消费等待、解锁睡眠并复取输出锁。
@details 调用者持输出锁；仅在仍有输出、主端存活且进程未中断时等待，等待登记在条件锁内，睡眠后迭代复查。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9113e62d2a85b3e3"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::delegate`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::IoctlFlow::delegate(Context &c) noexcept
@brief 把剩余命令交给原 TTY ioctl。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-decfdf34bc961dcc"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::lock`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::IoctlFlow::lock(Context &c) noexcept
@brief 先复制锁定值，再持全局锁发布从端锁定状态。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-efa66fd4a7d4a43b"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::lock_request`

```doxygen
@fn static bool xv::drivers::Pty::Endpoint::IoctlFlow::lock_request(Context &c) noexcept
@brief 检查是否修改命名从端锁定状态。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4b2793e6576382f8"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::master_claim`

```doxygen
@fn static bool xv::drivers::Pty::Endpoint::IoctlFlow::master_claim(Context &c) noexcept
@brief 检查是否为主端禁止的控制终端认领请求。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3db5ecc93b11e834"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::master_readable`

```doxygen
@fn static bool xv::drivers::Pty::Endpoint::IoctlFlow::master_readable(Context &c) noexcept
@brief 检查是否为主端可读字节快照请求。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0cdf173f081047ca"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::number`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::IoctlFlow::number(Context &c) noexcept
@brief 验证命名主端身份并复制编号。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-81cfff93d301d426"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::number_request`

```doxygen
@fn static bool xv::drivers::Pty::Endpoint::IoctlFlow::number_request(Context &c) noexcept
@brief 检查是否查询命名 PTY 编号。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8832ff9cac5ff276"></a>

## `xv::drivers::Pty::Endpoint::IoctlFlow::readable`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::IoctlFlow::readable(Context &c) noexcept
@brief 在输出锁内快照字节数后解锁复制输出。
@details 请求由嵌套模板分支声明去向；命名端点限制、用户复制、全局 PTY 锁和输出快照锁次序保持原协议，其余请求仍委托 TTY。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8e0beb055af0e7fa"></a>

## `xv::drivers::Pty::Endpoint::PollFlow::connected`

```doxygen
@fn static bool xv::drivers::Pty::Endpoint::PollFlow::connected(Context &c) noexcept
@brief 检查快照中的从端是否仍连接。
@details 从端入口直接委托 TTY；主端先在输出锁内订阅与快照，解锁后才取序列锁或 TTY 锁，避免和回显反向嵌套。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cbba84ded03c0ac9"></a>

## `xv::drivers::Pty::Endpoint::PollFlow::hangup`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::PollFlow::hangup(Context &c) noexcept
@brief 保留原可读与 HUP 挂断事件。
@details 从端入口直接委托 TTY；主端先在输出锁内订阅与快照，解锁后才取序列锁或 TTY 锁，避免和回显反向嵌套。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7785737302fbdaa3"></a>

## `xv::drivers::Pty::Endpoint::PollFlow::reader`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::PollFlow::reader(Context &c) noexcept
@brief 按请求订阅读序列锁并修正可读事件。
@details 从端入口直接委托 TTY；主端先在输出锁内订阅与快照，解锁后才取序列锁或 TTY 锁，避免和回显反向嵌套。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4492ce825689fbec"></a>

## `xv::drivers::Pty::Endpoint::PollFlow::snapshot`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::PollFlow::snapshot(Context &c) noexcept
@brief 在输出锁内订阅主端数据通道并快照从端状态与可读事件。
@details 从端入口直接委托 TTY；主端先在输出锁内订阅与快照，解锁后才取序列锁或 TTY 锁，避免和回显反向嵌套。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c30865e9f68bedea"></a>

## `xv::drivers::Pty::Endpoint::PollFlow::writer`

```doxygen
@fn static void xv::drivers::Pty::Endpoint::PollFlow::writer(Context &c) noexcept
@brief 分别订阅输入序列锁和 TTY 输入空间，再判断可写。
@details 从端入口直接委托 TTY；主端先在输出锁内订阅与快照，解锁后才取序列锁或 TTY 锁，避免和回显反向嵌套。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3ba8453f14f5d53b"></a>

## `xv::drivers::Pty::ReadMasterFlow::again`

```doxygen
@fn static void xv::drivers::Pty::ReadMasterFlow::again(Context &c) noexcept
@brief 记录空队列的 EAGAIN。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d831255dfffa6a10"></a>

## `xv::drivers::Pty::ReadMasterFlow::drain`

```doxygen
@fn static void xv::drivers::Pty::ReadMasterFlow::drain(Context &c) noexcept
@brief 直接复制可用输出、成功才消费并唤醒空间等待者。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7bd2f3c03eb680a1"></a>

## `xv::drivers::Pty::ReadMasterFlow::killed`

```doxygen
@fn static bool xv::drivers::Pty::ReadMasterFlow::killed(Context &) noexcept
@brief 在空队列等待前检查进程中断。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7153ecb09a8c7995"></a>

## `xv::drivers::Pty::ReadMasterFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::Pty::ReadMasterFlow::nonblocking(Context &c) noexcept
@brief 检查主读取是否非阻塞。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e2cff60233df0c70"></a>

## `xv::drivers::Pty::ReadMasterFlow::wait`

```doxygen
@fn static void xv::drivers::Pty::ReadMasterFlow::wait(Context &c) noexcept
@brief 在输出锁内登记数据等待并解锁睡眠、复取。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3a21d666a6580bb1"></a>

## `xv::drivers::Pty::ReadMasterFlow::waiting`

```doxygen
@fn static bool xv::drivers::Pty::ReadMasterFlow::waiting(Context &c) noexcept
@brief 检查输出队列空且从端仍存活。
@details 读者 SleepLock 保持入口词法作用域；图运行持输出锁，空队列按原从端状态等待。复制成功后才消费，失败等待路径不额外唤醒写者。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b9f146d56859543b"></a>

## `xv::drivers::Pty::ReleaseFlow::destroy`

```doxygen
@fn static void xv::drivers::Pty::ReleaseFlow::destroy(Context &c) noexcept
@brief 销毁已经摘链的 PTY 对。
@details 入口持全局 PTY 锁且已确认最后引用；先摘链解锁，再销毁并归还页，销毁后只传递保存的页指针，不再访问对象字段。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-48347091a34c74a8"></a>

## `xv::drivers::Pty::ReleaseFlow::free_page`

```doxygen
@fn static void xv::drivers::Pty::ReleaseFlow::free_page(Context &c) noexcept
@brief 归还保存的终端页指针且不再读取对象。
@details 入口持全局 PTY 锁且已确认最后引用；先摘链解锁，再销毁并归还页，销毁后只传递保存的页指针，不再访问对象字段。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b47cadf582744aa2"></a>

## `xv::drivers::Pty::ReleaseFlow::unlink`

```doxygen
@fn static void xv::drivers::Pty::ReleaseFlow::unlink(Context &c) noexcept
@brief 从全局列表摘除最后引用的 PTY，未登记仍保留原诊断。
@details 入口持全局 PTY 锁且已确认最后引用；先摘链解锁，再销毁并归还页，销毁后只传递保存的页指针，不再访问对象字段。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-402e61b70895ae2b"></a>

## `xv::drivers::Pty::ReleaseFlow::unlock`

```doxygen
@fn static void xv::drivers::Pty::ReleaseFlow::unlock(Context &) noexcept
@brief 释放全局 PTY 锁。
@details 入口持全局 PTY 锁且已确认最后引用；先摘链解锁，再销毁并归还页，销毁后只传递保存的页指针，不再访问对象字段。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5d5600563141891a"></a>

## `xv::drivers::Pty::TransmitFlow::again`

```doxygen
@fn static void xv::drivers::Pty::TransmitFlow::again(Context &c) noexcept
@brief 记录完整块暂不可提交的 EAGAIN。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b8b386182a0dd4d7"></a>

## `xv::drivers::Pty::TransmitFlow::nonblocking`

```doxygen
@fn static bool xv::drivers::Pty::TransmitFlow::nonblocking(Context &c) noexcept
@brief 检查本次输出是否非阻塞。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a33be44a91cd0c49"></a>

## `xv::drivers::Pty::TransmitFlow::publish`

```doxygen
@fn static void xv::drivers::Pty::TransmitFlow::publish(Context &c) noexcept
@brief 直接写入完整输出块、唤醒主读者并记录字节数。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7705f7ff73ad8a08"></a>

## `xv::drivers::Pty::TransmitFlow::space`

```doxygen
@fn static bool xv::drivers::Pty::TransmitFlow::space(Context &c) noexcept
@brief 检查整个输出块是否都能放入环。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-937c01c96e1fa1f7"></a>

## `xv::drivers::Pty::TransmitFlow::stopped`

```doxygen
@fn static bool xv::drivers::Pty::TransmitFlow::stopped(Context &c) noexcept
@brief 按原顺序检查主端关闭或进程中断。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5f92a24b3ddf8366"></a>

## `xv::drivers::Pty::TransmitFlow::wait`

```doxygen
@fn static void xv::drivers::Pty::TransmitFlow::wait(Context &c) noexcept
@brief 登记空间等待后释放输出锁睡眠并复取。
@details 调用者持输出锁；完整变换块必须整体有空间，等待在锁内登记后解锁睡眠，成功一次提交整块，错误退出由入口统一解锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b9ff70461b3a64af"></a>

## `xv::drivers::Pty::WriteMasterFlow::accepted`

```doxygen
@fn static bool xv::drivers::Pty::WriteMasterFlow::accepted(Context &c) noexcept
@brief 检查软件字节注入是否成功。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fbe7c2a66a03dfc6"></a>

## `xv::drivers::Pty::WriteMasterFlow::complete`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::complete(Context &c) noexcept
@brief 增加完成字节数并在相关自旋锁外服务终端信号。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bf472ed232337226"></a>

## `xv::drivers::Pty::WriteMasterFlow::connection`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::connection(Context &c) noexcept
@brief 在输出锁内快照从端连接状态后解锁。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f449f32b9575c6f5"></a>

## `xv::drivers::Pty::WriteMasterFlow::copy`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::copy(Context &c) noexcept
@brief 复制下一个软件输入字节。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c9f9584d4831c0d1"></a>

## `xv::drivers::Pty::WriteMasterFlow::failure`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::failure(Context &c) noexcept
@brief 返回已完成源字节数或原失败值。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-295be3660a8d9c2e"></a>

## `xv::drivers::Pty::WriteMasterFlow::inject`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::inject(Context &c) noexcept
@brief 通过原 TTY 注入一个字节并保存结果。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2a6775ff889fe14b"></a>

## `xv::drivers::Pty::WriteMasterFlow::ok`

```doxygen
@fn static bool xv::drivers::Pty::WriteMasterFlow::ok(Context &c) noexcept
@brief 检查源字节是否复制成功。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-428433811a5e17ca"></a>

## `xv::drivers::Pty::WriteMasterFlow::remaining`

```doxygen
@fn static bool xv::drivers::Pty::WriteMasterFlow::remaining(Context &c) noexcept
@brief 检查主输入是否仍有未处理源字节。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d718eaa67a033970"></a>

## `xv::drivers::Pty::WriteMasterFlow::short_write`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::short_write(Context &c) noexcept
@brief 按部分成功优先规则保留 inject 的失败结果。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5ab195e563374f17"></a>

## `xv::drivers::Pty::WriteMasterFlow::stopped`

```doxygen
@fn static bool xv::drivers::Pty::WriteMasterFlow::stopped(Context &c) noexcept
@brief 按原连接与进程中断规则判断是否停止。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7ac82b18d50c5f67"></a>

## `xv::drivers::Pty::WriteMasterFlow::success`

```doxygen
@fn static void xv::drivers::Pty::WriteMasterFlow::success(Context &c) noexcept
@brief 记录整个源输入完成。
@details 输入 SleepLock 仍由入口词法 guard 持有；逐源字节检查连接与复制，inject 返回后已释放 TTY/组自旋锁，成功字节记账后才服务软件信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9fca282de39b5271"></a>

## `xv::drivers::CreatePtyFlow::allocate`

```doxygen
@fn static void xv::drivers::CreatePtyFlow::allocate(Context &c) noexcept
@brief 取得匿名 PTY 候选页。
@details 入口清空两端输出并验证对象可放一页；候选页构造并初始化后才在全局锁内登记，最后发布端点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-344b4c00f4005b69"></a>

## `xv::drivers::CreatePtyFlow::allocated`

```doxygen
@fn static bool xv::drivers::CreatePtyFlow::allocated(Context &c) noexcept
@brief 检查候选页是否分配成功。
@details 入口清空两端输出并验证对象可放一页；候选页构造并初始化后才在全局锁内登记，最后发布端点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b21951d44177cf08"></a>

## `xv::drivers::CreatePtyFlow::construct`

```doxygen
@fn static void xv::drivers::CreatePtyFlow::construct(Context &c) noexcept
@brief 原位构造并初始化整个 PTY 对。
@details 入口清空两端输出并验证对象可放一页；候选页构造并初始化后才在全局锁内登记，最后发布端点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3dfbe41e687d5f54"></a>

## `xv::drivers::CreatePtyFlow::publish`

```doxygen
@fn static void xv::drivers::CreatePtyFlow::publish(Context &c) noexcept
@brief 发布主从端点并记录成功。
@details 入口清空两端输出并验证对象可放一页；候选页构造并初始化后才在全局锁内登记，最后发布端点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5801084a71a447cd"></a>

## `xv::drivers::CreatePtyFlow::register_pair`

```doxygen
@fn static void xv::drivers::CreatePtyFlow::register_pair(Context &c) noexcept
@brief 在全局 PTY 锁内接入已初始化候选。
@details 入口清空两端输出并验证对象可放一页；候选页构造并初始化后才在全局锁内登记，最后发布端点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9b43999f37a48661"></a>

## `xv::drivers::OpenNamedPtyFlow::attach`

```doxygen
@fn static void xv::drivers::OpenNamedPtyFlow::attach(Context &c) noexcept
@brief 按原输出锁协议激活从端并唤醒等待。
@details 原入口 ScopedLock 全程持全局 PTY 锁；只接管已发布且未锁定的匹配对，先取得引用和从文件计数，再激活从端并转移端点引用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8df6da1be0582016"></a>

## `xv::drivers::OpenNamedPtyFlow::find`

```doxygen
@fn static void xv::drivers::OpenNamedPtyFlow::find(Context &c) noexcept
@brief 直接查找已发布且未锁定的命名对。
@details 原入口 ScopedLock 全程持全局 PTY 锁；只接管已发布且未锁定的匹配对，先取得引用和从文件计数，再激活从端并转移端点引用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f9bd84b466348a78"></a>

## `xv::drivers::OpenNamedPtyFlow::found`

```doxygen
@fn static bool xv::drivers::OpenNamedPtyFlow::found(Context &c) noexcept
@brief 检查是否找到可打开从端的对。
@details 原入口 ScopedLock 全程持全局 PTY 锁；只接管已发布且未锁定的匹配对，先取得引用和从文件计数，再激活从端并转移端点引用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-49eb6b864474f842"></a>

## `xv::drivers::OpenNamedPtyFlow::publish`

```doxygen
@fn static void xv::drivers::OpenNamedPtyFlow::publish(Context &c) noexcept
@brief 记录将转移给调用者的从端指针。
@details 原入口 ScopedLock 全程持全局 PTY 锁；只接管已发布且未锁定的匹配对，先取得引用和从文件计数，再激活从端并转移端点引用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-53cf4fee37e8a00b"></a>

## `xv::drivers::OpenNamedPtyFlow::retain`

```doxygen
@fn static void xv::drivers::OpenNamedPtyFlow::retain(Context &c) noexcept
@brief 增加对引用与从文件计数。
@details 原入口 ScopedLock 全程持全局 PTY 锁；只接管已发布且未锁定的匹配对，先取得引用和从文件计数，再激活从端并转移端点引用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8f8757ac526b3c1e"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::allocate`

```doxygen
@fn static void xv::drivers::PtyMultiplexer::OpenFlow::allocate(Context &c) noexcept
@brief 取得命名 PTY 候选页。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1d17de6ca341ec5f"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::allocated`

```doxygen
@fn static bool xv::drivers::PtyMultiplexer::OpenFlow::allocated(Context &c) noexcept
@brief 检查候选页是否分配成功。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8cb489526758f559"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::available`

```doxygen
@fn static bool xv::drivers::PtyMultiplexer::OpenFlow::available(Context &c) noexcept
@brief 检查是否仍有未使用编号。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-02f32ce119b7d8cb"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::construct`

```doxygen
@fn static void xv::drivers::PtyMultiplexer::OpenFlow::construct(Context &c) noexcept
@brief 构造并初始化候选对，按原次序保存有效 UID/GID。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-93c0b044fd7d15eb"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::find`

```doxygen
@fn static void xv::drivers::PtyMultiplexer::OpenFlow::find(Context &c) noexcept
@brief 持全局 PTY 锁直接查找最低未使用编号。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-da1cab0045878fc4"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::publish`

```doxygen
@fn static void xv::drivers::PtyMultiplexer::OpenFlow::publish(Context &c) noexcept
@brief 发布命名、编号、引用和列表状态，解锁后返回主端。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3701359af3df7d88"></a>

## `xv::drivers::PtyMultiplexer::OpenFlow::rollback`

```doxygen
@fn static void xv::drivers::PtyMultiplexer::OpenFlow::rollback(Context &c) noexcept
@brief 编号耗尽时解锁并仅销毁归还未发布候选。
@details 保持原命名工厂和静态实例；候选构造并保存 UID/GID 后才持全局锁选编号，耗尽时解锁销毁候选，成功发布全部命名与引用状态后才返回主端。 下一节点由图策略声明，业务结果与控制信号分离。
```
