# `kernel/fs/inode/filesystem.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/inode/filesystem.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:filesystem`。

直接模块依赖：`:inode`, `:journal`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::FileSystem::FileSystem`](filesystem.cppm.md#fn-b4ac6c4cbbce1f37)
- [`xv::fs::FileSystem::FileSystem`](filesystem.cppm.md#fn-fe4eb86d8c242275)
- [`xv::fs::FileSystem::allocate`](filesystem.cppm.md#fn-d9a7a26c2a809ad7)
- [`xv::fs::FileSystem::create_at`](filesystem.cppm.md#fn-e1fd6bfced25fe7e)
- [`xv::fs::FileSystem::initialize`](filesystem.cppm.md#fn-4c4ed942fc0d3cde)
- [`xv::fs::FileSystem::initialize_inodes`](filesystem.cppm.md#fn-07de6482d4dc8016)
- [`xv::fs::FileSystem::link_at`](filesystem.cppm.md#fn-6a28e4907d41efec)
- [`xv::fs::FileSystem::lock`](filesystem.cppm.md#fn-acc988a2c2299c7c)
- [`xv::fs::FileSystem::lookup_entry`](filesystem.cppm.md#fn-0826404bad88e5ba)
- [`xv::fs::FileSystem::name`](filesystem.cppm.md#fn-6c060336c02c1cf3)
- [`xv::fs::FileSystem::open_device`](filesystem.cppm.md#fn-5657367df56d279c)
- [`xv::fs::FileSystem::operator=`](filesystem.cppm.md#fn-f26e574b17bd4595)
- [`xv::fs::FileSystem::persistent`](filesystem.cppm.md#fn-4801219016c47066)
- [`xv::fs::FileSystem::read`](filesystem.cppm.md#fn-ee61db9886d3a952)
- [`xv::fs::FileSystem::read_only`](filesystem.cppm.md#fn-8653de95e35e7992)
- [`xv::fs::FileSystem::release`](filesystem.cppm.md#fn-d62ac7ac3d78ce23)
- [`xv::fs::FileSystem::retain`](filesystem.cppm.md#fn-180c2d3920d9b05a)
- [`xv::fs::FileSystem::root`](filesystem.cppm.md#fn-8f1be8d595dcc8b6)
- [`xv::fs::FileSystem::snapshot`](filesystem.cppm.md#fn-2c57dd6d3a76d87f)
- [`xv::fs::FileSystem::snapshot_reads`](filesystem.cppm.md#fn-1b2a3dd3b9e80c06)
- [`xv::fs::FileSystem::stat`](filesystem.cppm.md#fn-c6de975c21634106)
- [`xv::fs::FileSystem::truncate`](filesystem.cppm.md#fn-089efe799f78685b)
- [`xv::fs::FileSystem::unlink_at`](filesystem.cppm.md#fn-12ad13608c3030b0)
- [`xv::fs::FileSystem::unlock`](filesystem.cppm.md#fn-564c162686ac6d0c)
- [`xv::fs::FileSystem::unlock_release`](filesystem.cppm.md#fn-8d43fd92e5f7f57f)
- [`xv::fs::FileSystem::update`](filesystem.cppm.md#fn-52004d1ea813d05a)
- [`xv::fs::FileSystem::write`](filesystem.cppm.md#fn-d83567e51d8a8f80)
- [`xv::fs::FileSystem::~FileSystem`](filesystem.cppm.md#fn-d58e988d2e01c6d2)
- [`xv::fs::LockedInode::LockedInode`](filesystem.cppm.md#fn-99b1c6da155292e1)
- [`xv::fs::LockedInode::LockedInode`](filesystem.cppm.md#fn-ae9da48627340ea7)
- [`xv::fs::LockedInode::operator=`](filesystem.cppm.md#fn-bcb3b820aebea19d)
- [`xv::fs::LockedInode::~LockedInode`](filesystem.cppm.md#fn-1599cd04541c7f48)

<a id="fn-fe4eb86d8c242275"></a>

## `xv::fs::FileSystem::FileSystem`

```doxygen
@fn xv::fs::FileSystem::FileSystem(const FileSystem &)
@brief 文件描述、inode、路径、块缓存与日志事务的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-f26e574b17bd4595"></a>

## `xv::fs::FileSystem::operator=`

```doxygen
@fn FileSystem &xv::fs::FileSystem::operator=(const FileSystem &)
@brief 文件描述、inode、路径、块缓存与日志事务的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-6c060336c02c1cf3"></a>

## `xv::fs::FileSystem::name`

```doxygen
@fn virtual const char *xv::fs::FileSystem::name() const
@brief 文件描述、inode、路径、块缓存与日志事务：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-4c4ed942fc0d3cde"></a>

## `xv::fs::FileSystem::initialize`

```doxygen
@fn virtual void xv::fs::FileSystem::initialize(int device)
@brief 文件描述、inode、路径、块缓存与日志事务：建立该对象开始工作所需的初始状态。
@param device 本次操作使用的 设备 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-07de6482d4dc8016"></a>

## `xv::fs::FileSystem::initialize_inodes`

```doxygen
@fn virtual void xv::fs::FileSystem::initialize_inodes()
@brief 文件描述、inode、路径、块缓存与日志事务：初始化inodes。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d9a7a26c2a809ad7"></a>

## `xv::fs::FileSystem::allocate`

```doxygen
@fn virtual Inode *xv::fs::FileSystem::allocate(unsigned device, short type)
@brief 文件描述、inode、路径、块缓存与日志事务：分配并取得对象或存储资源。
@param device 本次操作使用的 设备 值。
@param type 本次操作使用的 type 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `Inode *(unsigned int, short)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-180c2d3920d9b05a"></a>

## `xv::fs::FileSystem::retain`

```doxygen
@fn virtual Inode *xv::fs::FileSystem::retain(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：增加对象引用，延长其存活期。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `Inode *(Inode *)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-d62ac7ac3d78ce23"></a>

## `xv::fs::FileSystem::release`

```doxygen
@fn virtual void xv::fs::FileSystem::release(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：释放当前引用或归还对象占用的资源。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-acc988a2c2299c7c"></a>

## `xv::fs::FileSystem::lock`

```doxygen
@fn virtual void xv::fs::FileSystem::lock(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：取得对象状态所需的互斥保护。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-564c162686ac6d0c"></a>

## `xv::fs::FileSystem::unlock`

```doxygen
@fn virtual void xv::fs::FileSystem::unlock(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：释放已持有的对象互斥保护。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8d43fd92e5f7f57f"></a>

## `xv::fs::FileSystem::unlock_release`

```doxygen
@fn virtual void xv::fs::FileSystem::unlock_release(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：释放 inode 锁并归还 inode 引用。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-52004d1ea813d05a"></a>

## `xv::fs::FileSystem::update`

```doxygen
@fn virtual void xv::fs::FileSystem::update(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：把内存中的 inode 元数据写回缓存块。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-089efe799f78685b"></a>

## `xv::fs::FileSystem::truncate`

```doxygen
@fn virtual void xv::fs::FileSystem::truncate(Inode *node)
@brief 文件描述、inode、路径、块缓存与日志事务：释放 inode 数据块并重置文件长度。
@param node inode 或链表节点对象。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ee61db9886d3a952"></a>

## `xv::fs::FileSystem::read`

```doxygen
@fn virtual int xv::fs::FileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：读取已有数据并按接口返回读取结果。
@param node inode 或链表节点对象。
@param user 指明地址是否属于用户地址空间。
@param destination 接收结果的缓冲或目标对象。
@param offset 本次操作使用的 偏移 值。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (Inode *, int, unsigned long, unsigned int, unsigned int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-d83567e51d8a8f80"></a>

## `xv::fs::FileSystem::write`

```doxygen
@fn virtual int xv::fs::FileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
@brief 文件描述、inode、路径、块缓存与日志事务：提交输入数据并报告传输结果。
@param node inode 或链表节点对象。
@param user 指明地址是否属于用户地址空间。
@param source 输入数据位置或源对象。
@param offset 本次操作使用的 偏移 值。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (Inode *, int, unsigned long, unsigned int, unsigned int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-c6de975c21634106"></a>

## `xv::fs::FileSystem::stat`

```doxygen
@fn virtual void xv::fs::FileSystem::stat(Inode *node, Stat *result)
@brief 文件描述、inode、路径、块缓存与日志事务：stat。
@param node inode 或链表节点对象。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b4ac6c4cbbce1f37"></a>

## `xv::fs::FileSystem::FileSystem`

```doxygen
@fn constexpr xv::fs::FileSystem::FileSystem()
@brief 构造文件描述、inode、路径、块缓存与日志事务，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d58e988d2e01c6d2"></a>

## `xv::fs::FileSystem::~FileSystem`

```doxygen
@fn xv::fs::FileSystem::~FileSystem()
@brief 结束文件描述、inode、路径、块缓存与日志事务的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-99b1c6da155292e1"></a>

## `xv::fs::LockedInode::LockedInode`

```doxygen
@fn xv::fs::LockedInode::LockedInode(FileSystem &fs, Inode &node)
@brief 构造inode 锁作用域守卫，建立本重载要求的成员初值和借用关系。
@param fs 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param node inode 或链表节点对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`fs_` → `node_` → `fs_.lock`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1599cd04541c7f48"></a>

## `xv::fs::LockedInode::~LockedInode`

```doxygen
@fn xv::fs::LockedInode::~LockedInode()
@brief 结束inode 锁作用域守卫的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`fs_.unlock`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ae9da48627340ea7"></a>

## `xv::fs::LockedInode::LockedInode`

```doxygen
@fn xv::fs::LockedInode::LockedInode(const LockedInode &)
@brief inode 锁作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-bcb3b820aebea19d"></a>

## `xv::fs::LockedInode::operator=`

```doxygen
@fn LockedInode &xv::fs::LockedInode::operator=(const LockedInode &)
@brief inode 锁作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
<a id="fn-4801219016c47066"></a>

## `xv::fs::FileSystem::persistent`

```doxygen
@fn virtual bool xv::fs::FileSystem::persistent() const
@brief 说明节点释放是否需要磁盘事务。
@details 磁盘实现沿用 true；devfs 返回 false。文件后端最后关闭时据此建立条件 Transaction，避免内存节点释放进入磁盘日志。
```

<a id="fn-8f1be8d595dcc8b6"></a>

## `xv::fs::FileSystem::root`

```doxygen
@fn virtual Inode *xv::fs::FileSystem::root() = 0
@brief 返回一个带引用、未锁定的文件系统根节点。
@details 调用者必须释放该引用；磁盘根节点允许在首个进程恢复日志之前取得，不能在此提前读取磁盘。VFS 使用根引用连接挂载边界。
```

<a id="fn-0826404bad88e5ba"></a>

## `xv::fs::FileSystem::lookup_entry`

```doxygen
@fn virtual Inode *xv::fs::FileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset) = 0
@brief 在已锁定目录中查找一个名字。
@details 目录必须属于本实例，name 沿用 DIRSIZ 字节规则。成功返回带引用、未锁定的子节点，可选 offset 返回目录项字节位置；失败返回 nullptr。挂载跳转由 VFS 完成。
```

<a id="fn-e1fd6bfced25fe7e"></a>

## `xv::fs::FileSystem::create_at`

```doxygen
@fn virtual Inode *xv::fs::FileSystem::create_at(Inode *directory, const char *name, short type, short major, short minor) = 0
@brief 在指定目录中创建节点或打开已有普通文件/设备。
@details 借用调用者持有的未锁定父目录引用，函数自行加锁。成功返回带引用且已锁定的节点；失败返回 nullptr。磁盘实现要求外层事务，devfs 只允许复用已有设备。
```

<a id="fn-6a28e4907d41efec"></a>

## `xv::fs::FileSystem::link_at`

```doxygen
@fn virtual int xv::fs::FileSystem::link_at(Inode *directory, const char *name, Inode *target) = 0
@brief 把一个已有节点链接到指定目录。
@details 父目录与目标均为借用、未锁定引用。实现自行获取锁并维护 nlink；失败返回 -1。VFS 先拒绝跨文件系统链接，磁盘调用者必须持有事务。
```

<a id="fn-12ad13608c3030b0"></a>

## `xv::fs::FileSystem::unlink_at`

```doxygen
@fn virtual int xv::fs::FileSystem::unlink_at(Inode *directory, const char *name) = 0
@brief 删除指定目录中的一个名字。
@details 借用未锁定的父目录引用；禁止删除 . 和 ..，检查非空目录并维护链接计数。返回 0 或 -1；VFS 负责额外保护挂载点。磁盘实现需要外层事务。
```
<a id="fn-5657367df56d279c"></a>

## `xv::fs::FileSystem::open_device`

```doxygen
@fn virtual xv::drivers::CharacterDevice *xv::fs::FileSystem::open_device(Inode *, bool)
@brief 为设备 inode 请求一次打开会话。
@details 默认不支持并返回 nullptr。设备文件系统和磁盘设备节点实现负责工厂查找与动态会话引用；失败不消费传入 inode 引用。
```

<a id="fn-1b2a3dd3b9e80c06"></a>

## `xv::fs::FileSystem::snapshot_reads`

```doxygen
@fn virtual bool xv::fs::FileSystem::snapshot_reads() const
@brief 说明文件和目录是否采用每次打开的只读快照。
@details 默认 false，procfs 返回 true。OpenFileTable 为每次打开生成一页内容；独立 open 刷新，dup/fork 共享已有内容和偏移。
```

<a id="fn-8653de95e35e7992"></a>

## `xv::fs::FileSystem::read_only`

```doxygen
@fn virtual bool xv::fs::FileSystem::read_only() const
@brief 报告普通文件是否禁止写打开与截断。
@details 默认 false，procfs 返回 true。sys_open 根据此契约拒绝写访问与 O_TRUNC；目录创建、链接和删除仍由各节点操作返回失败。
```

<a id="fn-2c57dd6d3a76d87f"></a>

## `xv::fs::FileSystem::snapshot`

```doxygen
@fn virtual int xv::fs::FileSystem::snapshot(Inode *, char *, unsigned)
@brief 把节点的打开时内容复制到内核快照缓冲。
@details 调用者锁定节点，buffer/capacity 为已分配的内核缓冲。成功返回字节数，容量不足或对象已退出返回 -1，不能发布半份内容。默认不支持；实现不得把活动内核对象指针交给用户。
```
