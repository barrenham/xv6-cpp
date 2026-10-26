# `kernel/fs/procfs/procfs.cppm`

## 函数导航

- [`xv::fs::ProcFileSystem::ProcFileSystem`](procfs.cppm.md#fn-9a4171e62a306030)
- [`xv::fs::ProcFileSystem::allocate`](procfs.cppm.md#fn-d9a64c9410865df0)
- [`xv::fs::ProcFileSystem::create_at`](procfs.cppm.md#fn-40eee34b1c697597)
- [`xv::fs::ProcFileSystem::get`](procfs.cpp.md#fn-90753126637513eb)
- [`xv::fs::ProcFileSystem::initialize`](procfs.cpp.md#fn-74b43972b16a8a28)
- [`xv::fs::ProcFileSystem::initialize_inodes`](procfs.cpp.md#fn-731ba950b1042b0b)
- [`xv::fs::ProcFileSystem::link_at`](procfs.cppm.md#fn-276c525c157f69f8)
- [`xv::fs::ProcFileSystem::lock`](procfs.cpp.md#fn-9b990852215e1514)
- [`xv::fs::ProcFileSystem::lookup_entry`](procfs.cpp.md#fn-0512bfc7f1743007)
- [`xv::fs::ProcFileSystem::name`](procfs.cppm.md#fn-29187708d3c725e4)
- [`xv::fs::ProcFileSystem::persistent`](procfs.cppm.md#fn-1e82863fde209311)
- [`xv::fs::ProcFileSystem::read`](procfs.cpp.md#fn-603705cd3aba4822)
- [`xv::fs::ProcFileSystem::read_only`](procfs.cppm.md#fn-14200cf92fcd8328)
- [`xv::fs::ProcFileSystem::release`](procfs.cpp.md#fn-0072b177352873a8)
- [`xv::fs::ProcFileSystem::retain`](procfs.cpp.md#fn-d20670821de275d2)
- [`xv::fs::ProcFileSystem::root`](procfs.cppm.md#fn-f02b9beecdca08d3)
- [`xv::fs::ProcFileSystem::snapshot`](procfs.cpp.md#fn-a48454ff4df79560)
- [`xv::fs::ProcFileSystem::snapshot_reads`](procfs.cppm.md#fn-28dbddfa27b2963f)
- [`xv::fs::ProcFileSystem::stat`](procfs.cpp.md#fn-1eba63efb443f71d)
- [`xv::fs::ProcFileSystem::truncate`](procfs.cpp.md#fn-188d230713358c4d)
- [`xv::fs::ProcFileSystem::unlink_at`](procfs.cppm.md#fn-f668fff6f55ab2b6)
- [`xv::fs::ProcFileSystem::unlock`](procfs.cpp.md#fn-441bd17e918e61e8)
- [`xv::fs::ProcFileSystem::unlock_release`](procfs.cpp.md#fn-df886b5ac3282c5b)
- [`xv::fs::ProcFileSystem::update`](procfs.cpp.md#fn-0f8f2177ab9f8594)
- [`xv::fs::ProcFileSystem::write`](procfs.cppm.md#fn-866b01c03ee5091e)

<a id="fn-9a4171e62a306030"></a>

## `xv::fs::ProcFileSystem::ProcFileSystem`

```doxygen
@fn constexpr xv::fs::ProcFileSystem::ProcFileSystem() = default
@brief 建立只读 procfs 缓存的初始状态。
@details 节点表静态分配，锁与 owner 由 initialize_inodes 建立，启动时不申请用户内容快照。
```

<a id="fn-29187708d3c725e4"></a>

## `xv::fs::ProcFileSystem::name`

```doxygen
@fn const char *xv::fs::ProcFileSystem::name() const override
@brief 返回 procfs 后端名称。
@details 返回静态字符串；具体节点归属由 inode.owner 记录。
```

<a id="fn-1e82863fde209311"></a>

## `xv::fs::ProcFileSystem::persistent`

```doxygen
@fn bool xv::fs::ProcFileSystem::persistent() const override
@brief 声明 procfs 节点不参与磁盘日志。
@details 始终返回 false，节点引用和生成内容均在内存中。
```

<a id="fn-14200cf92fcd8328"></a>

## `xv::fs::ProcFileSystem::read_only`

```doxygen
@fn bool xv::fs::ProcFileSystem::read_only() const override
@brief 声明 procfs 普通文件为只读。
@details sys_open 据此拒绝写打开和 O_TRUNC；节点级写入及名称修改也独立拒绝。
```

<a id="fn-28dbddfa27b2963f"></a>

## `xv::fs::ProcFileSystem::snapshot_reads`

```doxygen
@fn bool xv::fs::ProcFileSystem::snapshot_reads() const override
@brief 为 procfs 的文件和目录启用每次打开快照。
@details File 取得独立页面，生成后不随进程退出或后续系统状态变化而改变；dup/fork 共享快照与偏移。
```

<a id="fn-f02b9beecdca08d3"></a>

## `xv::fs::ProcFileSystem::root`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::root() override
@brief 取得 procfs 根节点的未锁定引用。
@details 固定节点零号槽在 initialize 完成元数据，调用者负责释放引用。
```

<a id="fn-40eee34b1c697597"></a>

## `xv::fs::ProcFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::create_at(Inode *, const char *, short, short, short) override
@brief 拒绝修改 procfs 名称或内容。
@details procfs 由内核状态生成，只读操作之外的创建、链接、删除、分配和写入返回 nullptr 或 -1；不写磁盘，不改变进程状态。
```

<a id="fn-276c525c157f69f8"></a>

## `xv::fs::ProcFileSystem::link_at`

```doxygen
@fn int xv::fs::ProcFileSystem::link_at(Inode *, const char *, Inode *) override
@brief 拒绝修改 procfs 名称或内容。
@details procfs 由内核状态生成，只读操作之外的创建、链接、删除、分配和写入返回 nullptr 或 -1；不写磁盘，不改变进程状态。
```

<a id="fn-f668fff6f55ab2b6"></a>

## `xv::fs::ProcFileSystem::unlink_at`

```doxygen
@fn int xv::fs::ProcFileSystem::unlink_at(Inode *, const char *) override
@brief 拒绝修改 procfs 名称或内容。
@details procfs 由内核状态生成，只读操作之外的创建、链接、删除、分配和写入返回 nullptr 或 -1；不写磁盘，不改变进程状态。
```

<a id="fn-d9a64c9410865df0"></a>

## `xv::fs::ProcFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::ProcFileSystem::allocate(unsigned, short) override
@brief 拒绝修改 procfs 名称或内容。
@details procfs 由内核状态生成，只读操作之外的创建、链接、删除、分配和写入返回 nullptr 或 -1；不写磁盘，不改变进程状态。
```

<a id="fn-866b01c03ee5091e"></a>

## `xv::fs::ProcFileSystem::write`

```doxygen
@fn int xv::fs::ProcFileSystem::write(Inode *, int, unsigned long, unsigned, unsigned) override
@brief 拒绝修改 procfs 名称或内容。
@details procfs 由内核状态生成，只读操作之外的创建、链接、删除、分配和写入返回 nullptr 或 -1；不写磁盘，不改变进程状态。
```
