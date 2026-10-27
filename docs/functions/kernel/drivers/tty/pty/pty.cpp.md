# `kernel/drivers/tty/pty/pty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/pty/pty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:pty`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Pty::Endpoint::close`](pty.cpp.md#fn-2f858e49784ccac1)
- [`xv::drivers::Pty::Endpoint::ioctl`](pty.cpp.md#fn-6b6a9388f115d902)
- [`xv::drivers::Pty::Endpoint::poll`](pty.cpp.md#fn-e30ef808741bf1eb)
- [`xv::drivers::Pty::Endpoint::read`](pty.cpp.md#fn-2fe0922873329d2b)
- [`xv::drivers::Pty::Endpoint::write`](pty.cpp.md#fn-fce9e782c37be567)
- [`xv::drivers::Pty::Output::drain`](pty.cpp.md#fn-f8bb828f54046dc7)
- [`xv::drivers::Pty::Output::put_sync`](pty.cpp.md#fn-c576e0a8d12cb145)
- [`xv::drivers::Pty::Output::transmit`](pty.cpp.md#fn-ee0c89905868fc1c)
- [`xv::drivers::Pty::Output::writable`](pty.cpp.md#fn-b1d57a724fdf31ed)
- [`xv::drivers::Pty::attach_slave`](pty.cpp.md#fn-71b2427af9997333)
- [`xv::drivers::Pty::close`](pty.cpp.md#fn-5c7a739984ad05b7)
- [`xv::drivers::Pty::initialize`](pty.cpp.md#fn-87ffb0376d0f4542)
- [`xv::drivers::Pty::read_master`](pty.cpp.md#fn-26ae6ac8d5631f7d)
- [`xv::drivers::Pty::release`](pty.cpp.md#fn-c67e15beca56bec0)
- [`xv::drivers::Pty::write_master`](pty.cpp.md#fn-f40800f626106299)
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::allocate_dma_page` → `new` → `pair->initialize` → `guard` → `pair->master` → `pair->slave`。
@details 本函数可见的状态更新包括：`pair->next = ptys;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!page`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ptys_lock.acquire` → `ptys_lock.release` → `detail::fail` → `Pty` → `detail::free_terminal_page`。
@details 需要特别跟踪的条件：`--references`；`*link && *link != this`；`!*link`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`ptys_lock.acquire` → `ptys_lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5c7a739984ad05b7"></a>

## `xv::drivers::Pty::close`

```doxygen
@fn void xv::drivers::Pty::close(bool master)
@brief 关闭一个主端或从端会话并归还对象引用。
@details 注册锁保护发布状态与 slave_files，输出锁保护连接标志和等待通道。主端最终关闭撤下设备名并挂断终端；只有最后从端关闭才切断主端读连接。命名从端关闭不永久挂断 TTY，以允许主端存续期间重新打开；匿名 PTY 保留原挂断行为。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`p.lock_.acquire` → `detail::process_killed` → `p.lock_.release` → `detail::prepare_interruptible_sleep` → `detail::sleep_process` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`p.data_[p.written_++ % Capacity] = data[i];`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`count < 0 || count > Capacity`；`Capacity - (p.written_ - p.read_) < static_cast<unsigned>(count)`；`!p.master_open_ || detail::process_killed()`；`nonblocking`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`p.lock_.acquire` → `p.lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：`-1`；`-EAGAIN`；`count`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`p.lock_.acquire` → `detail::process_killed` → `detail::prepare_interruptible_sleep` → `p.lock_.release` → `detail::sleep_process`。
@details 需要特别跟踪的条件：`p.read_ != p.written_ && p.master_open_ && !detail::process_killed()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`p.lock_.acquire` → `p.lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-26ae6ac8d5631f7d"></a>

## `xv::drivers::Pty::read_master`

```doxygen
@fn int xv::drivers::Pty::read_master(IoBuffer destination, int count)
@brief 匿名伪终端对：从 PTY 输出队列读取 slave 产生的数据。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::process_killed` → `reader_lock_.try_acquire` → `reader_lock_.acquire_interruptible` → `reader` → `lock_.acquire` → `lock_.release` → `detail::prepare_interruptible_sleep` → `detail::sleep_process` → `detail::copy_out` → `detail::wake_processes`。
@details 需要特别跟踪的条件：`count <= 0`；`detail::process_killed()`；`destination.nonblocking`；`!reader_lock_.try_acquire()`；`!reader_lock_.acquire_interruptible()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`reader_lock_.try_acquire` → `reader_lock_.acquire_interruptible` → `lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : (fault ? -1 : 0)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f40800f626106299"></a>

## `xv::drivers::Pty::write_master`

```doxygen
@fn int xv::drivers::Pty::write_master(IoBuffer source, int count)
@brief 匿名伪终端对：把 master 输入按 slave 的行规程注入。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::process_killed` → `input_lock_.try_acquire` → `input_lock_.acquire_interruptible` → `input` → `lock_.acquire` → `lock_.release` → `detail::copy_in` → `terminal_.inject` → `detail::service_terminal_signals`。
@details 需要特别跟踪的条件：`count <= 0`；`detail::process_killed()`；`source.nonblocking`；`!input_lock_.try_acquire()`；`!input_lock_.acquire_interruptible()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`input_lock_.try_acquire` → `input_lock_.acquire_interruptible` → `lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`count == 0 ? 0 : -1`；`-1`；`-EAGAIN`；`done ? done : -1`；`done ? done : result`；`done`。应结合上面的条件检查区分正常值、空值和错误值。
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
```

<a id="fn-e30ef808741bf1eb"></a>

## `xv::drivers::Pty::Endpoint::poll`

```doxygen
@fn unsigned xv::drivers::Pty::Endpoint::poll(xv::sync::PollTable *table)
@brief PTY 文件端点：查询当前就绪状态，并在提供表时登记通知依赖。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pair_.terminal_.poll` → `pair_.lock_.acquire` → `table->watch` → `pair_.lock_.release` → `table->events` → `pair_.reader_lock_.available` → `pair_.input_lock_.available` → `pair_.terminal_.input_writable`。
@details 需要特别跟踪的条件：`!master_`；`table`；`!connected`；`(!table || (table->events() & POLLIN)) && !pair_.reader_lock_.available(table)`；`!table || (table->events() & POLLOUT)`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`pair_.lock_.acquire` → `pair_.lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`pair_.terminal_.poll(table)`；`events | POLLIN | POLLHUP`；`events`。应结合上面的条件检查区分正常值、空值和错误值。
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
@fn CharacterDevice *xv::drivers::PtyMultiplexer::open(bool) override
@brief 打开新的 PTY 主端并记录会话拥有者。
@details 按页创建 Pty，保存调用者有效 uid/gid；发布编号前准备两端引用，任一分配失败归还页和端点。节点编号仍受活动会话及 inode 固定引用保护。
```

<a id="fn-e0fbfb0d92d6e5b5"></a>

## `xv::drivers::named_pty_owner`

```doxygen
@fn bool xv::drivers::named_pty_owner(unsigned number, unsigned &uid, unsigned &gid)
@brief 查询活动命名 PTY 的创建者。
@details 持 ptys_lock 查找编号并复制 uid/gid，找不到返回 false；不返回设备对象指针，不延长会话寿命。
```
