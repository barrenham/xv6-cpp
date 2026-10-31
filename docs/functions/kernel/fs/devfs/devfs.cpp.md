# `kernel/fs/devfs/devfs.cpp`

## 函数导航

- [`xv::fs::DevFileSystem::allocate`](devfs.cpp.md#fn-c5abadcff72de9ca)
- [`xv::fs::DevFileSystem::create_at`](devfs.cpp.md#fn-fb5edfa93d9d9127)
- [`xv::fs::DevFileSystem::initialize`](devfs.cpp.md#fn-c49eba16f985de33)
- [`xv::fs::DevFileSystem::initialize_inodes`](devfs.cpp.md#fn-00243c5ab4dcd922)
- [`xv::fs::DevFileSystem::link_at`](devfs.cpp.md#fn-83c2550aaf305a21)
- [`xv::fs::DevFileSystem::lock`](devfs.cpp.md#fn-6590a1415f934484)
- [`xv::fs::DevFileSystem::lookup_entry`](devfs.cpp.md#fn-5b759172a8b7cbd0)
- [`xv::fs::DevFileSystem::open_device`](devfs.cpp.md#fn-77a253e4e89cb39a)
- [`xv::fs::DevFileSystem::read`](devfs.cpp.md#fn-b088477df66b3ab4)
- [`xv::fs::DevFileSystem::release`](devfs.cpp.md#fn-6dacf32c76c227d1)
- [`xv::fs::DevFileSystem::retain`](devfs.cpp.md#fn-e24bb911f90a0865)
- [`xv::fs::DevFileSystem::stat`](devfs.cpp.md#fn-7d2a6b1d10adebe5)
- [`xv::fs::DevFileSystem::truncate`](devfs.cpp.md#fn-ab46f212380aff11)
- [`xv::fs::DevFileSystem::unlink_at`](devfs.cpp.md#fn-467ccd07c5a2c8b8)
- [`xv::fs::DevFileSystem::unlock`](devfs.cpp.md#fn-3c033f6b9674e8b8)
- [`xv::fs::DevFileSystem::unlock_release`](devfs.cpp.md#fn-24503ee0dd07bbe5)
- [`xv::fs::DevFileSystem::update`](devfs.cpp.md#fn-8ac05bd9ae1dd23c)
- [`xv::fs::DevFileSystem::write`](devfs.cpp.md#fn-35d09c6791808ad8)
- [`xv::fs::MemoryDevice::initialize`](devfs.cpp.md#fn-7ea96491bdf7a542)
- [`xv::fs::MemoryDevice::name`](devfs.cpp.md#fn-807dbcfa2dee03aa)
- [`xv::fs::MemoryDevice::poll`](devfs.cpp.md#fn-1ee610cc543a4533)
- [`xv::fs::MemoryDevice::put_sync`](devfs.cpp.md#fn-2b92efdd15790e32)
- [`xv::fs::MemoryDevice::read`](devfs.cpp.md#fn-78dcb885ab7432e2)
- [`xv::fs::MemoryDevice::write`](devfs.cpp.md#fn-eecfc6d4b67d727b)

<a id="fn-807dbcfa2dee03aa"></a>

## `xv::fs::MemoryDevice::name`

```doxygen
@fn const char *xv::fs::MemoryDevice::name() const override
@brief 取得内置虚拟设备名称。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-7ea96491bdf7a542"></a>

## `xv::fs::MemoryDevice::initialize`

```doxygen
@fn void xv::fs::MemoryDevice::initialize() override
@brief 保留无额外初始化工作的字符设备接口。
@details null/zero 不依赖硬件和队列，构造时确定的行为可直接使用。
```

<a id="fn-2b92efdd15790e32"></a>

## `xv::fs::MemoryDevice::put_sync`

```doxygen
@fn void xv::fs::MemoryDevice::put_sync(int) override
@brief 丢弃同步输出的单个字符。
@details null 与 zero 的输出方向都没有存储和硬件副作用。
```

<a id="fn-78dcb885ab7432e2"></a>

## `xv::fs::MemoryDevice::read`

```doxygen
@fn int xv::fs::MemoryDevice::read(xv::drivers::IoBuffer destination, int count) override
@brief 按设备类型返回 EOF 或向目标复制零字节。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-eecfc6d4b67d727b"></a>

## `xv::fs::MemoryDevice::write`

```doxygen
@fn int xv::fs::MemoryDevice::write(xv::drivers::IoBuffer, int count) override
@brief 丢弃写入并返回消费的字节数。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-1ee610cc543a4533"></a>

## `xv::fs::MemoryDevice::poll`

```doxygen
@fn unsigned xv::fs::MemoryDevice::poll(xv::sync::PollTable *) override
@brief 报告内置设备可立即读写。
@details 始终返回 POLLIN|POLLOUT；null 的可读事件对应立即 EOF，不登记等待通道。
```

<a id="fn-00243c5ab4dcd922"></a>

## `xv::fs::DevFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::DevFileSystem::initialize_inodes()
@brief 初始化静态设备节点表的锁与归属。
@details 为 NDEV+1 个槽绑定本实例 owner_，初始化引用计数自旋锁及节点睡眠锁。启动时调用一次，尚不读取注册表或磁盘。
```

<a id="fn-c49eba16f985de33"></a>

## `xv::fs::DevFileSystem::initialize`

```doxygen
@fn void DevFileSystem::xv::fs::DevFileSystem::initialize(int device)
@brief 初始化只读设备命名空间及节点权限。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-e24bb911f90a0865"></a>

## `xv::fs::DevFileSystem::retain`

```doxygen
@fn Inode *xv::fs::DevFileSystem::retain(Inode *node)
@brief 在引用锁内增加静态节点的引用。
@details 允许从零引用重新取得仍存在的设备节点；静态节点不因无人打开而销毁。调用者保证节点属于本实例。
```

<a id="fn-6dacf32c76c227d1"></a>

## `xv::fs::DevFileSystem::release`

```doxygen
@fn void xv::fs::DevFileSystem::release(Inode *node)
@brief 释放设备节点引用并在必要时归还 PTY 固定引用。
@details 引用锁内减少 ref，动态从端节点降至零时在锁外归还对应 PTY 固定引用。静态节点不销毁；动态槽允许后续重新绑定同编号，但旧对象最后引用消失前该编号不会被新 PTY 取得。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-6590a1415f934484"></a>

## `xv::fs::DevFileSystem::lock`

```doxygen
@fn void xv::fs::DevFileSystem::lock(Inode *node)
@brief 取得 devfs 节点的睡眠锁。
@details 调用者持有节点引用；锁保护与通用 inode 后端共享的目录偏移操作和元数据访问约定，不得持有自旋锁进入。
```

<a id="fn-3c033f6b9674e8b8"></a>

## `xv::fs::DevFileSystem::unlock`

```doxygen
@fn void xv::fs::DevFileSystem::unlock(Inode *node)
@brief 释放 devfs 节点的睡眠锁。
@details 调用者必须拥有该锁；不改变节点引用计数。
```

<a id="fn-24503ee0dd07bbe5"></a>

## `xv::fs::DevFileSystem::unlock_release`

```doxygen
@fn void xv::fs::DevFileSystem::unlock_release(Inode *node)
@brief 先解锁再释放一个 devfs 节点引用。
@details 与 FileSystem 公共生命周期约定一致，最终引用消失仍保留静态节点。
```

<a id="fn-7d2a6b1d10adebe5"></a>

## `xv::fs::DevFileSystem::stat`

```doxygen
@fn void xv::fs::DevFileSystem::stat(Inode *node, Stat *result)
@brief 读取设备节点状态与安全属性。
@details 先清零输出，复制设备身份、类型和链接数，再通过 security_stat 输出 uid/gid/mode；调用者持节点锁。
```

<a id="fn-b088477df66b3ab4"></a>

## `xv::fs::DevFileSystem::read`

```doxygen
@fn int xv::fs::DevFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 按字节偏移合成 /dev 或 /dev/pts 目录记录。
@details 保持 16 字节 dirent ABI，支持非对齐读取、短读和复制失败。pts 以编号对应固定槽，未发布编号输出空目录项，避免枚举期间新增/删除造成后续条目偏移移动；普通根列出注册设备和 pts。
@details 流程由 ReadFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-5b759172a8b7cbd0"></a>

## `xv::fs::DevFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::DevFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
@brief 查找静态设备或活动命名 PTY。
@details 动态 pts 的拥有者来自创建者凭据，模式 0600。先查询 PTY 拥有者再取引用锁，避免锁序反转；新缓存槽在引用发布前写好属性，已有引用保持编号固定。
@details 流程由 LookupFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-fb5edfa93d9d9127"></a>

## `xv::fs::DevFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::DevFileSystem::create_at(Inode *directory, const char *name, short type, short, short)
@brief 让 O_CREATE 复用已存在的字符设备。
@details 只接受 T_FILE 的打开意图，自行锁定借用父目录并查找名称；目标为 T_DEVICE 时返回已锁定引用。新名称、目录和其他创建类型返回 nullptr，目录内容始终不变。
@details 流程由 CreateFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-c5abadcff72de9ca"></a>

## `xv::fs::DevFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::DevFileSystem::allocate(unsigned, short)
@brief 拒绝分配动态 devfs 节点。
@details 返回 nullptr；设备节点由启动注册表生成，运行期不提供 mknod 或热插拔分配。
```

<a id="fn-8ac05bd9ae1dd23c"></a>

## `xv::fs::DevFileSystem::update`

```doxygen
@fn void xv::fs::DevFileSystem::update(Inode *)
@brief 拒绝对合成元数据执行磁盘式更新。
@details 调用表示内核违反 devfs 不可变约定并触发 panic；正常设备打开与 I/O 不走此入口。
```

<a id="fn-ab46f212380aff11"></a>

## `xv::fs::DevFileSystem::truncate`

```doxygen
@fn void xv::fs::DevFileSystem::truncate(Inode *)
@brief 拒绝截断合成节点。
@details 调用触发 panic；sys_open 的 O_TRUNC 只作用于 T_FILE，因此不能截断设备或设备目录。
```

<a id="fn-35d09c6791808ad8"></a>

## `xv::fs::DevFileSystem::write`

```doxygen
@fn int xv::fs::DevFileSystem::write(Inode *, int, unsigned long, unsigned, unsigned)
@brief 拒绝向设备目录写目录数据。
@details 始终返回 -1；设备输出经 DeviceBackend 转到字符设备，不经过文件系统节点 write。
```

<a id="fn-83c2550aaf305a21"></a>

## `xv::fs::DevFileSystem::link_at`

```doxygen
@fn int xv::fs::DevFileSystem::link_at(Inode *, const char *, Inode *)
@brief 拒绝在 devfs 内添加硬链接。
@details 始终返回 -1，保持启动发布的设备名称集合不变。
```

<a id="fn-467ccd07c5a2c8b8"></a>

## `xv::fs::DevFileSystem::unlink_at`

```doxygen
@fn int xv::fs::DevFileSystem::unlink_at(Inode *, const char *)
@brief 拒绝删除 devfs 目录项。
@details 始终返回 -1；设备条目不会因 unlink 从注册表移除。
```

<a id="fn-77a253e4e89cb39a"></a>

## `xv::fs::DevFileSystem::open_device`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::DevFileSystem::open_device(Inode *node, bool nonblocking)
@brief 为静态设备或动态 PTY 从端取得会话。
@details 动态从端节点已经固定其 PTY 编号，委托驱动验证主端仍存在且从端已解锁；静态条目按 major/minor 查询设备工厂并调用 open。失败返回 nullptr，不转移节点引用。
```
<a id="fn-d216c823111a921f"></a>

## `xv::fs::MemoryDevice::MemoryDevice`

```doxygen
@fn constexpr explicit xv::fs::MemoryDevice::MemoryDevice(Kind kind) : kind_(kind)
@brief 选择 null 或 zero 的静态设备行为。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-01655ad066617f5c"></a>

## `xv::fs::DevFileSystem::open_block_device`

```doxygen
@fn xv::drivers::BlockDevice *DevFileSystem::xv::fs::DevFileSystem::open_block_device(Inode *node,bool writable)
@brief 返回 `open_registered_block(node,writable)`。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
