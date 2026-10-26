# `kernel/fs/file/open_file.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/open_file.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:operations`, `:pipe`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::DeviceRegistry::attach`](open_file.cpp.md#fn-bda8b34bb94e637a)
- [`xv::fs::DeviceRegistry::get`](open_file.cpp.md#fn-ad2454960d73ee76)
- [`xv::fs::File::nonblocking`](open_file.cpp.md#fn-6ae1d9a30631a953)
- [`xv::fs::OpenFileTable::allocate`](open_file.cpp.md#fn-319840ebccd17589)
- [`xv::fs::OpenFileTable::bind_endpoint`](open_file.cpp.md#fn-71d5734b8a7caee5)
- [`xv::fs::OpenFileTable::bind_inode`](open_file.cpp.md#fn-dd0a9f0a843218e4)
- [`xv::fs::OpenFileTable::bind_pipe`](open_file.cpp.md#fn-28ec255ef808a0a2)
- [`xv::fs::OpenFileTable::fcntl`](open_file.cpp.md#fn-3a55abe43cfd3ddc)
- [`xv::fs::OpenFileTable::initialize`](open_file.cpp.md#fn-6b5a5967a8ffe320)
- [`xv::fs::OpenFileTable::ioctl`](open_file.cpp.md#fn-227fb8963ea0dda0)
- [`xv::fs::OpenFileTable::poll`](open_file.cpp.md#fn-1565f5ef091ec77d)
- [`xv::fs::OpenFileTable::read`](open_file.cpp.md#fn-ca0e6e5ab12e2355)
- [`xv::fs::OpenFileTable::release`](open_file.cpp.md#fn-68d546d0ff9f6adb)
- [`xv::fs::OpenFileTable::retain`](open_file.cpp.md#fn-5dc21f043fc5419d)
- [`xv::fs::OpenFileTable::stat`](open_file.cpp.md#fn-bc87515094e34bf0)
- [`xv::fs::OpenFileTable::write`](open_file.cpp.md#fn-0115ff202f5e535d)

<a id="fn-6ae1d9a30631a953"></a>

## `xv::fs::File::nonblocking`

```doxygen
@fn bool xv::fs::File::nonblocking() const
@brief 打开文件描述：读取或计算 `__atomic_load_n(&status_flags_, __ATOMIC_ACQUIRE) & O_NONBLOCK`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&status_flags_, __ATOMIC_ACQUIRE) & O_NONBLOCK`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3a55abe43cfd3ddc"></a>

## `xv::fs::OpenFileTable::fcntl`

```doxygen
@fn int xv::fs::OpenFileTable::fcntl(File *file, int command, unsigned flags)
@brief 共享打开文件表：查询或修改打开文件的共享状态标志。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param command 本次操作使用的 command 值。
@param flags 接口标志位组合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `__atomic_compare_exchange_n`。
@details 需要特别跟踪的条件：`command == F_GETFL`；`command != F_SETFL || (flags & ~(O_ACCMODE | O_NONBLOCK))`；`__atomic_compare_exchange_n(&file->status_flags_, &old, value, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&file->status_flags_, __ATOMIC_ACQUIRE)`；`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6b5a5967a8ffe320"></a>

## `xv::fs::OpenFileTable::initialize`

```doxygen
@fn void xv::fs::OpenFileTable::initialize()
@brief 共享打开文件表：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-319840ebccd17589"></a>

## `xv::fs::OpenFileTable::allocate`

```doxygen
@fn File *xv::fs::OpenFileTable::allocate()
@brief 共享打开文件表：分配并取得对象或存储资源。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@details 本函数可见的状态更新包括：`file.ref = 1;`；`file.type = File::Kind::None;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`file.ref == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`&file`；`nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5dc21f043fc5419d"></a>

## `xv::fs::OpenFileTable::retain`

```doxygen
@fn File *xv::fs::OpenFileTable::retain(File *file)
@brief 共享打开文件表：增加对象引用，延长其存活期。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::fail`。
@details 需要特别跟踪的条件：`file->ref < 1`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`file`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-68d546d0ff9f6adb"></a>

## `xv::fs::OpenFileTable::release`

```doxygen
@fn void xv::fs::OpenFileTable::release(File *file)
@brief 共享打开文件表：释放当前引用或归还对象占用的资源。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.acquire` → `detail::fail` → `lock_.release` → `pipe_operations.close` → `inode_operations.close` → `device_operations.close` → `detached.endpoint->close`。
@details 本函数可见的状态更新包括：`detached.type = file->type;`；`detached.pipe = file->pipe;`；`detached.node = file->node;`；`detached.writable = file->writable;`；`detached.major = file->major;`；`detached.endpoint = file->endpoint;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`file->ref < 1`；`--file->ref > 0`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-dd0a9f0a843218e4"></a>

## `xv::fs::OpenFileTable::bind_inode`

```doxygen
@fn void xv::fs::OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable, bool nonblocking)
@brief 共享打开文件表：把打开文件绑定到 inode 及访问权限。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param node inode 或链表节点对象。
@param readable 本次操作使用的 可读数据 值。
@param writable 本次操作使用的 可写条件 值。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`node.kind` → `node.device_major` → `__atomic_store_n`。
@details 本函数可见的状态更新包括：`file.type = node.kind() == T_DEVICE ? File::Kind::Device : File::Kind::Inode;`；`file.major = node.device_major();`；`file.node = &node;`；`file.offset = 0;`；`file.readable = readable;`；`file.writable = writable;`。局部临时变量与对象字段应按作用域区分。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-71d5734b8a7caee5"></a>

## `xv::fs::OpenFileTable::bind_endpoint`

```doxygen
@fn void xv::fs::OpenFileTable::bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint)
@brief 共享打开文件表：把打开文件绑定到动态字符设备端点。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param endpoint 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n`。
@details 本函数可见的状态更新包括：`file.type = File::Kind::Endpoint;`；`file.endpoint = &endpoint;`；`file.readable = file.writable = true;`；`file.writable = true;`。局部临时变量与对象字段应按作用域区分。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-28ec255ef808a0a2"></a>

## `xv::fs::OpenFileTable::bind_pipe`

```doxygen
@fn void xv::fs::OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
@brief 共享打开文件表：把打开文件绑定到管道的一端。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param pipe 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param writable 本次操作使用的 可写条件 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n`。
@details 本函数可见的状态更新包括：`file.type = File::Kind::Pipe;`；`file.pipe = &pipe;`；`file.readable = !writable;`；`file.writable = writable;`。局部临时变量与对象字段应按作用域区分。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ca0e6e5ab12e2355"></a>

## `xv::fs::OpenFileTable::read`

```doxygen
@fn int xv::fs::OpenFileTable::read(File *file, unsigned long address, int count)
@brief 共享打开文件表：读取已有数据并按接口返回读取结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file->endpoint->read` → `file->nonblocking` → `detail::fail` → `operations->read`。
@details 需要特别跟踪的条件：`!file->readable || count < 0`；`file->type == File::Kind::Endpoint`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`file->endpoint->read({xv::drivers::AddressSpace::User, address, file->nonblocking()}, count)`；`operations->read(*file, address, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0115ff202f5e535d"></a>

## `xv::fs::OpenFileTable::write`

```doxygen
@fn int xv::fs::OpenFileTable::write(File *file, unsigned long address, int count)
@brief 共享打开文件表：提交输入数据并报告传输结果。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file->endpoint->write` → `file->nonblocking` → `detail::fail` → `operations->write`。
@details 需要特别跟踪的条件：`!file->writable || count < 0`；`file->type == File::Kind::Endpoint`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`file->endpoint->write({xv::drivers::AddressSpace::User, address, file->nonblocking()}, count)`；`operations->write(*file, address, count)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bc87515094e34bf0"></a>

## `xv::fs::OpenFileTable::stat`

```doxygen
@fn int xv::fs::OpenFileTable::stat(File *file, unsigned long address)
@brief 共享打开文件表：stat。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::copy_out` → `inode_operations.stat` → `device_operations.stat`。
@details 本函数可见的状态更新包括：`value.type = T_DEVICE;`；`value.nlink = 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`file->type == File::Kind::Endpoint`；`file->type == File::Kind::Inode`；`file->type == File::Kind::Device`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`detail::copy_out(true, address, &value, sizeof(value))`；`inode_operations.stat(*file, address)`；`device_operations.stat(*file, address)`；`-1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-227fb8963ea0dda0"></a>

## `xv::fs::OpenFileTable::ioctl`

```doxygen
@fn int xv::fs::OpenFileTable::ioctl(File *file, unsigned request, unsigned long argument)
@brief 共享打开文件表：分派设备控制请求并处理参数复制。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param request 本次操作使用的 请求 值。
@param argument 本次操作使用的 argument 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file->endpoint->ioctl` → `devices` → `get` → `device->ioctl`。
@details 需要特别跟踪的条件：`file->type == File::Kind::Endpoint`；`file->type != File::Kind::Device`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`file->endpoint->ioctl(request, {xv::drivers::AddressSpace::User, argument})`；`-1`；`device ? device->ioctl(request, {xv::drivers::AddressSpace::User, argument}) : -1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1565f5ef091ec77d"></a>

## `xv::fs::OpenFileTable::poll`

```doxygen
@fn unsigned xv::fs::OpenFileTable::poll(File *file, xv::sync::PollTable *table)
@brief 共享打开文件表：查询当前就绪状态，并在提供表时登记通知依赖。
@param file 打开文件对象，访问权限和共享偏移由该对象管理。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`file->pipe->poll` → `file->endpoint->poll` → `devices` → `get` → `device->poll`。
@details 需要特别跟踪的条件：`!file->readable`；`!file->writable`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`POLLNVAL`；`events`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bda8b34bb94e637a"></a>

## `xv::fs::DeviceRegistry::attach`

```doxygen
@fn void xv::fs::DeviceRegistry::attach(unsigned major, xv::drivers::CharacterDevice &device)
@brief 字符设备登记表：关联后端对象、接收器或组成员。
@param major 字符设备主设备号。
@param device 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail`。
@details 本函数可见的状态更新包括：`devices_[major] = &device;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`major >= NDEV`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ad2454960d73ee76"></a>

## `xv::fs::DeviceRegistry::get`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::DeviceRegistry::get(int major) const
@brief 字符设备登记表：读取或计算 `major >= 0 && major < NDEV ? devices_[major] : nullptr`，返回当前查询结果。
@param major 字符设备主设备号。
@return 各分支实际返回：`major >= 0 && major < NDEV ? devices_[major] : nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```
