# `kernel/fs/file/open_file.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/open_file.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:operations`, `:pipe`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::DeviceRegistry::attach`](open_file.cpp.md#fn-3619f8a51976d854)
- [`xv::fs::DeviceRegistry::attach`](open_file.cpp.md#fn-bda8b34bb94e637a)
- [`xv::fs::DeviceRegistry::get`](open_file.cpp.md#fn-ad2454960d73ee76)
- [`xv::fs::File::nonblocking`](open_file.cpp.md#fn-6ae1d9a30631a953)
- [`xv::fs::OpenFileTable::allocate`](open_file.cpp.md#fn-319840ebccd17589)
- [`xv::fs::OpenFileTable::bind_endpoint`](open_file.cpp.md#fn-71d5734b8a7caee5)
- [`xv::fs::OpenFileTable::bind_inode`](open_file.cpp.md#fn-dd0a9f0a843218e4)
- [`xv::fs::OpenFileTable::bind_object`](open_file.cpp.md#fn-5b6c8b6fa35ede20)
- [`xv::fs::OpenFileTable::bind_pipe`](open_file.cpp.md#fn-28ec255ef808a0a2)
- [`xv::fs::OpenFileTable::configure`](open_file.cpp.md#fn-34475d43f15a5471)
- [`xv::fs::OpenFileTable::describe`](open_file.cpp.md#fn-66c95719b54955f5)
- [`xv::fs::OpenFileTable::fcntl`](open_file.cpp.md#fn-3a55abe43cfd3ddc)
- [`xv::fs::OpenFileTable::initialize`](open_file.cpp.md#fn-6b5a5967a8ffe320)
- [`xv::fs::OpenFileTable::ioctl`](open_file.cpp.md#fn-227fb8963ea0dda0)
- [`xv::fs::OpenFileTable::object`](open_file.cpp.md#fn-2e5a0e8ff1089c2a)
- [`xv::fs::OpenFileTable::poll`](open_file.cpp.md#fn-1565f5ef091ec77d)
- [`xv::fs::OpenFileTable::read`](open_file.cpp.md#fn-ca0e6e5ab12e2355)
- [`xv::fs::OpenFileTable::release`](open_file.cpp.md#fn-68d546d0ff9f6adb)
- [`xv::fs::OpenFileTable::resize`](open_file.cpp.md#fn-2cc8e03aa2bb7cde)
- [`xv::fs::OpenFileTable::retain`](open_file.cpp.md#fn-5dc21f043fc5419d)
- [`xv::fs::OpenFileTable::seek`](open_file.cpp.md#fn-d97473d82762fb27)
- [`xv::fs::OpenFileTable::stat`](open_file.cpp.md#fn-bc87515094e34bf0)
- [`xv::fs::OpenFileTable::sync`](open_file.cpp.md#fn-c82ab0b98fb2ba90)
- [`xv::fs::OpenFileTable::write`](open_file.cpp.md#fn-0115ff202f5e535d)
- [`xv::fs::open_registered_device`](open_file.cpp.md#fn-9dc9a9219a36d4e9)

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
@brief 查询或原子修改共享状态标志。
@details F_SETFL 允许 NONBLOCK 和 APPEND，保留访问模式；dup/fork 别名共享同一个状态字。
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
@brief 取得一个打开文件槽并分配稳定的打开实例标识。
@details 在表锁内寻找 ref 为零的槽，清空发布标记与旧身份，分配递增 identity，建立未绑定引用。观察者在 configure 完成前不能取得其描述信息。
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
@brief 表锁内减少引用，非最后引用立即返回。
@details 表锁内减少引用，非最后引用立即返回。最后引用把槽设为 ref=-1，解锁后虚调用后端 close，等待其可能睡眠的资源清理完成，再重新加锁清空指针并设 ref=0。内置适配器在 close 期间仍位于保留槽中；不会与新打开的对象重叠。未绑定 File 也可安全回滚。
```

<a id="fn-dd0a9f0a843218e4"></a>

## `xv::fs::OpenFileTable::bind_inode`

```doxygen
@fn bool xv::fs::OpenFileTable::bind_inode(File &file, Inode &node, bool readable, bool writable, bool nonblocking, bool append)
@brief 建立每次打开的 inode 或设备后端。
@details 沿用独立快照和设备会话所有权；初始化后以原子操作发布 APPEND 标志，inode/设备后端仍受内置存储大小检查约束。
```

<a id="fn-71d5734b8a7caee5"></a>

## `xv::fs::OpenFileTable::bind_endpoint`

```doxygen
@fn void xv::fs::OpenFileTable::bind_endpoint(File &file, xv::drivers::CharacterDevice &endpoint)
@brief 接管匿名字符端点并发布 terminal 种类。
@details 在 File 内置存储构造 EndpointBackend，端点最终 close 责任不变；观测身份不包含内核对象地址。
```

<a id="fn-28ec255ef808a0a2"></a>

## `xv::fs::OpenFileTable::bind_pipe`

```doxygen
@fn void xv::fs::OpenFileTable::bind_pipe(File &file, Pipe &pipe, bool writable)
@brief 接管一个管道端并发布 pipe 种类。
@details 保存原有读写方向与关闭责任，描述信息使用打开实例 identity，不向 procfs 暴露 Pipe 指针。
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
@brief 检查可写性并向后端传递当前状态。
@details 将共享 NONBLOCK/APPEND 状态加入 I/O Buffer，后端在实际写入锁内决定追加位置；拒绝负长度和无后端描述符。
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

<a id="fn-bda8b34bb94e637a"></a>

## `xv::fs::DeviceRegistry::attach`

```doxygen
@fn void xv::fs::DeviceRegistry::attach(unsigned major, xv::drivers::CharacterDevice &device)
@brief 在启动阶段登记一个静态字符设备及其名称和设备号。
@details 旧形式登记未命名的 minor=0 设备；命名形式复制最多 13 字节名称，拒绝斜杠、点目录、重复名称、重复 major/minor、容量超限及封存后的修改。设备实例必须保持系统级生命周期，注册阶段不允许并发使用。
```

<a id="fn-5b6c8b6fa35ede20"></a>

## `xv::fs::OpenFileTable::bind_object`

```doxygen
@fn void xv::fs::OpenFileTable::bind_object(File &file, xv::io::IoObject &object, bool nonblocking)
@brief 绑定已拥有的通用 I/O 对象并发布资源种类。
@details 通过 Socket 能力识别 socket，其余对象为 unknown。不调用可能睡眠的 stat，先准备元数据再发布后端；真实对象由其 close 管理。
```

<a id="fn-2e5a0e8ff1089c2a"></a>

## `xv::fs::OpenFileTable::object`

```doxygen
@fn xv::io::IoObject *xv::fs::OpenFileTable::object(File *file) const
@brief 返回 File 所持有的统一 IoObject 借用指针，空 File 指针或尚未绑定时返回 nullptr。
@details 返回 File 所持有的统一 IoObject 借用指针，空 File 指针或尚未绑定时返回 nullptr。调用者必须保持有效 File 引用，不因查询而增加拥有权。
```
<a id="fn-3619f8a51976d854"></a>

## `xv::fs::DeviceRegistry::attach`

```doxygen
@fn void xv::fs::DeviceRegistry::attach(const char *name, unsigned major, unsigned minor, xv::drivers::CharacterDevice &device)
@brief 在启动阶段登记一个静态字符设备及其名称和设备号。
@details 旧形式登记未命名的 minor=0 设备；命名形式复制最多 13 字节名称，拒绝斜杠、点目录、重复名称、重复 major/minor、容量超限及封存后的修改。设备实例必须保持系统级生命周期，注册阶段不允许并发使用。
```

<a id="fn-ad2454960d73ee76"></a>

## `xv::fs::DeviceRegistry::get`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::DeviceRegistry::get(int major, int minor) const
@brief 按 major 和 minor 精确查询字符设备。
@details 负值或未登记的组合返回 nullptr；minor 缺省为零。封存后注册表只读，I/O 路径无需注册锁；返回的指针只借用静态设备。
```
<a id="fn-9dc9a9219a36d4e9"></a>

## `xv::fs::open_registered_device`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::open_registered_device(Inode *node, bool nonblocking)
@brief 按 major/minor 查询工厂并打开字符设备会话。
@details 找不到设备或工厂分配失败返回 nullptr。静态设备可返回自身，ptmx 工厂返回新的 PTY 主端；返回引用由 File 最后关闭负责。
```
<a id="fn-34475d43f15a5471"></a>

## `xv::fs::OpenFileTable::configure`

```doxygen
@fn void xv::fs::OpenFileTable::configure(File &file, xv::io::IoObject &object, bool readable, bool writable, bool nonblocking, FileKind kind, const Inode *node)
@brief 发布完整的后端、访问标志和不可变文件身份。
@details 绑定阶段先记录资源种类及可选 inode 类型、设备号和编号，再设置后端和共享标志，最后以 release 存储 ready_。describe 以 acquire 检查，避免读取尚未初始化的 fd。
```

<a id="fn-66c95719b54955f5"></a>

## `xv::fs::OpenFileTable::describe`

```doxygen
@fn bool xv::fs::OpenFileTable::describe(File *file, FileInfo &result) const
@brief 复制已发布 File 的不可变身份与当前共享标志。
@details 调用者持有 File 引用，或持描述符表锁阻止本槽被取走。acquire 检查 ready_ 后复制 identity/kind/type/device/inode，原子读取 flags；不加节点锁、不调用后端、不分配和不睡眠。
```

<a id="fn-d97473d82762fb27"></a>

## `xv::fs::OpenFileTable::seek`

```doxygen
@fn long xv::fs::OpenFileTable::seek(File *file, long offset, int origin)
@brief 转发打开对象的偏移操作。
@details 无对象或后端不支持时返回 -1；inode 后端负责同一次打开偏移的串行化。
```

<a id="fn-2cc8e03aa2bb7cde"></a>

## `xv::fs::OpenFileTable::resize`

```doxygen
@fn int xv::fs::OpenFileTable::resize(File *file, unsigned size)
@brief 仅允许可写打开对象请求截断。
@details 打开访问权限检查先于后端调用；管道、设备和只读后端通过默认或专用拒绝路径返回 -1。
```

<a id="fn-c82ab0b98fb2ba90"></a>

## `xv::fs::OpenFileTable::sync`

```doxygen
@fn int xv::fs::OpenFileTable::sync(File *file)
@brief 转发文件同步请求。
@details 缺少后端时返回 -1，实际持久化能力由对象契约决定。
```
