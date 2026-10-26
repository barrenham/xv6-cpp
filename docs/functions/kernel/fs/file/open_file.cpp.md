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
- [`xv::fs::OpenFileTable::bind_object`](open_file.cpp.md#fn-5b6c8b6fa35ede20)
- [`xv::fs::OpenFileTable::bind_pipe`](open_file.cpp.md#fn-28ec255ef808a0a2)
- [`xv::fs::OpenFileTable::configure`](open_file.cpp.md#fn-34475d43f15a5471)
- [`xv::fs::OpenFileTable::fcntl`](open_file.cpp.md#fn-3a55abe43cfd3ddc)
- [`xv::fs::OpenFileTable::initialize`](open_file.cpp.md#fn-6b5a5967a8ffe320)
- [`xv::fs::OpenFileTable::ioctl`](open_file.cpp.md#fn-227fb8963ea0dda0)
- [`xv::fs::OpenFileTable::object`](open_file.cpp.md#fn-2e5a0e8ff1089c2a)
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

<a id="fn-319840ebccd17589"></a>

## `xv::fs::OpenFileTable::allocate`

```doxygen
@fn File *xv::fs::OpenFileTable::allocate()
@brief 在全局表锁下寻找 ref 为零的空槽，设为一个拥有引用并清空后端。
@details 在全局表锁下寻找 ref 为零的空槽，设为一个拥有引用并清空后端。ref=-1 的关闭中槽仍占用，不能提前重用其内置适配器存储。失败返回 nullptr。
```

<a id="fn-68d546d0ff9f6adb"></a>

## `xv::fs::OpenFileTable::release`

```doxygen
@fn void xv::fs::OpenFileTable::release(File *file)
@brief 表锁内减少引用，非最后引用立即返回。
@details 表锁内减少引用，非最后引用立即返回。最后引用把槽设为 ref=-1，解锁后虚调用后端 close，等待其可能睡眠的资源清理完成，再重新加锁清空指针并设 ref=0。内置适配器在 close 期间仍位于保留槽中；不会与新打开的对象重叠。未绑定 File 也可安全回滚。
```

<a id="fn-dd0a9f0a843218e4"></a>

## `xv::fs::OpenFileTable::bind_inode`

```doxygen
@fn void xv::fs::OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable, bool nonblocking)
@brief 按 inode 的设备属性在 File 的64字节对齐存储内 placement 构造 InodeBackend 或 DeviceBackend，再设置统一对象指针和访问模式。
@details 按 inode 的设备属性在 File 的64字节对齐存储内 placement 构造 InodeBackend 或 DeviceBackend，再设置统一对象指针和访问模式。不分配额外物理页；接收调用者拥有的 inode 引用。每次打开的新 InodeBackend 偏移从零开始。
```

<a id="fn-71d5734b8a7caee5"></a>

## `xv::fs::OpenFileTable::bind_endpoint`

```doxygen
@fn void xv::fs::OpenFileTable::bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint)
@brief 在 File 内置存储构造 EndpointBackend，接收调用者的字符端点关闭责任，设可读写；PTY 最后引用释放时才关闭端点。
@details 在 File 内置存储构造 EndpointBackend，接收调用者的字符端点关闭责任，设可读写；PTY 最后引用释放时才关闭端点。
```

<a id="fn-28ec255ef808a0a2"></a>

## `xv::fs::OpenFileTable::bind_pipe`

```doxygen
@fn void xv::fs::OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
@brief 在 File 内置存储构造带方向的 PipeBackend，读端只读、写端只写。
@details 在 File 内置存储构造带方向的 PipeBackend，读端只读、写端只写。两个 File 共享 Pipe 缓冲对象，分别在最后引用关闭对应方向。
```

<a id="fn-ca0e6e5ab12e2355"></a>

## `xv::fs::OpenFileTable::read`

```doxygen
@fn int xv::fs::OpenFileTable::read(File *file, unsigned long address, int count)
@brief 检查读权限、非负长度和后端存在，然后只调用 IoObject::read，传入用户地址及当前共享 O_NONBLOCK。
@details 检查读权限、非负长度和后端存在，然后只调用 IoObject::read，传入用户地址及当前共享 O_NONBLOCK。没有资源类型 switch 或特定后端字段读取。
```

<a id="fn-0115ff202f5e535d"></a>

## `xv::fs::OpenFileTable::write`

```doxygen
@fn int xv::fs::OpenFileTable::write(File *file, unsigned long address, int count)
@brief 检查写权限、非负长度和后端存在，虚调用 IoObject::write。
@details 检查写权限、非负长度和后端存在，虚调用 IoObject::write。字节流部分写、数据报边界及 inode 事务切分均由对应后端处理。
```

<a id="fn-bc87515094e34bf0"></a>

## `xv::fs::OpenFileTable::stat`

```doxygen
@fn int xv::fs::OpenFileTable::stat(File *file, unsigned long address)
@brief 向后端请求内核 Metadata，成功后通过统一用户复制返回保持兼容的 stat ABI。
@details 向后端请求内核 Metadata，成功后通过统一用户复制返回保持兼容的 stat ABI。Pipe 默认不支持，Socket 自己提供 T_SOCKET，File 不按类型伪造元数据。
```

<a id="fn-227fb8963ea0dda0"></a>

## `xv::fs::OpenFileTable::ioctl`

```doxygen
@fn int xv::fs::OpenFileTable::ioctl(File *file, unsigned request, unsigned long argument)
@brief 通过 io::query<DeviceControl> 查询独立设备控制能力，存在时传递请求及用户参数 Buffer，不存在返回 -1；不判断 File 类型或直接查字符设备。
@details 通过 io::query<DeviceControl> 查询独立设备控制能力，存在时传递请求及用户参数 Buffer，不存在返回 -1；不判断 File 类型或直接查字符设备。
```

<a id="fn-1565f5ef091ec77d"></a>

## `xv::fs::OpenFileTable::poll`

```doxygen
@fn unsigned xv::fs::OpenFileTable::poll(File *file, xv::sync::PollTable *table)
@brief 调用后端虚 poll 登记通道及取得就绪位，随后只按 File 的读写权限过滤 POLLIN/POLLOUT；无后端为 POLLNVAL，不再按资源类型分派。
@details 调用后端虚 poll 登记通道及取得就绪位，随后只按 File 的读写权限过滤 POLLIN/POLLOUT；无后端为 POLLNVAL，不再按资源类型分派。
```

<a id="fn-34475d43f15a5471"></a>

## `xv::fs::OpenFileTable::configure`

```doxygen
@fn void xv::fs::OpenFileTable::configure(File &file, xv::io::IoObject &object, bool readable, bool writable, bool nonblocking)
@brief 在 File 尚未发布到描述符表时安装唯一 IoObject 指针、访问权限与共享打开标志。
@details 在 File 尚未发布到描述符表时安装唯一 IoObject 指针、访问权限与共享打开标志。后端 close 责任转交 File；此函数不额外分配或 retain 后端。
```

<a id="fn-5b6c8b6fa35ede20"></a>

## `xv::fs::OpenFileTable::bind_object`

```doxygen
@fn void xv::fs::OpenFileTable::bind_object(File &file, xv::io::IoObject &object, bool nonblocking)
@brief 把外部构造的 IoObject（例如 socket）绑定到可读写 File，传递非阻塞状态；File 最后引用仅调用对象 close，由对象决定立即释放或异步清理。
@details 把外部构造的 IoObject（例如 socket）绑定到可读写 File，传递非阻塞状态；File 最后引用仅调用对象 close，由对象决定立即释放或异步清理。
```

<a id="fn-2e5a0e8ff1089c2a"></a>

## `xv::fs::OpenFileTable::object`

```doxygen
@fn xv::io::IoObject *xv::fs::OpenFileTable::object(File *file) const
@brief 返回 File 所持有的统一 IoObject 借用指针，空 File 指针或尚未绑定时返回 nullptr。
@details 返回 File 所持有的统一 IoObject 借用指针，空 File 指针或尚未绑定时返回 nullptr。调用者必须保持有效 File 引用，不因查询而增加拥有权。
```
