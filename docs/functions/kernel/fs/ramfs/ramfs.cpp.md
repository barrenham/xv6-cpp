# `kernel/fs/ramfs/ramfs.cpp`

## 函数导航

- [`xv::fs::RamFileSystem::create_at`](ramfs.cpp.md#fn-5b7e740118badcc7)
- [`xv::fs::RamFileSystem::empty`](ramfs.cpp.md#fn-8edb568294f3071c)
- [`xv::fs::RamFileSystem::find`](ramfs.cpp.md#fn-dd69ef647e56a43e)
- [`xv::fs::RamFileSystem::initialize`](ramfs.cpp.md#fn-9e40f661a8e79e17)
- [`xv::fs::RamFileSystem::initialize_inodes`](ramfs.cpp.md#fn-f50c5c9aa65cd651)
- [`xv::fs::RamFileSystem::insert`](ramfs.cpp.md#fn-3b8ec07f45144052)
- [`xv::fs::RamFileSystem::link_at`](ramfs.cpp.md#fn-284ead7688a027d6)
- [`xv::fs::RamFileSystem::lookup_entry`](ramfs.cpp.md#fn-47a1af054919c8af)
- [`xv::fs::RamFileSystem::make`](ramfs.cpp.md#fn-b4793ba9f01ef544)
- [`xv::fs::RamFileSystem::read`](ramfs.cpp.md#fn-24fe196855f00a9c)
- [`xv::fs::RamFileSystem::reclaim`](ramfs.cpp.md#fn-1a54a9d94f11be0a)
- [`xv::fs::RamFileSystem::release`](ramfs.cpp.md#fn-217486517d13a8eb)
- [`xv::fs::RamFileSystem::retain`](ramfs.cpp.md#fn-0339515fc48a1692)
- [`xv::fs::RamFileSystem::root`](ramfs.cpp.md#fn-5495fb0db3fbb09a)
- [`xv::fs::RamFileSystem::slot`](ramfs.cpp.md#fn-ca85354173f3327c)
- [`xv::fs::RamFileSystem::stat`](ramfs.cpp.md#fn-1c27602529c8e52e)
- [`xv::fs::RamFileSystem::truncate`](ramfs.cpp.md#fn-bda258709d76e544)
- [`xv::fs::RamFileSystem::unlink_at`](ramfs.cpp.md#fn-33548eacf725bea9)
- [`xv::fs::RamFileSystem::unlock_release`](ramfs.cpp.md#fn-652b84b0201f47dc)
- [`xv::fs::RamFileSystem::write`](ramfs.cpp.md#fn-9512b40f84b5ad5d)

<a id="fn-f50c5c9aa65cd651"></a>

## `xv::fs::RamFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::RamFileSystem::initialize_inodes()
@brief 初始化 RAM 命名空间睡眠锁和引用自旋锁。
@details 启动早期执行，不分配页面。所有节点操作共用 tree_ 以串行化目录和数据，短暂引用更新使用 references_。
```

<a id="fn-9e40f661a8e79e17"></a>

## `xv::fs::RamFileSystem::initialize`

```doxygen
@fn void xv::fs::RamFileSystem::initialize(int device)
@brief 创建带 sticky 权限的 RAM 根目录。
@details 在树锁内分配根并显式设为 root:root 01777，挂载 /tmp 后任何用户可创建，删除仍受属主约束。
```

<a id="fn-5495fb0db3fbb09a"></a>

## `xv::fs::RamFileSystem::root`

```doxygen
@fn Inode *xv::fs::RamFileSystem::root()
@brief 取得常驻 RAM 根目录的新引用。
@details 根节点已有系统生命周期引用，返回的引用由调用者归还。
```

<a id="fn-b4793ba9f01ef544"></a>

## `xv::fs::RamFileSystem::make`

```doxygen
@fn RamFileSystem::Node *xv::fs::RamFileSystem::make(short type, Node *parent)
@brief 按调用者凭据分配 RAM inode。
@details 树锁内申请页与编号，新文件/目录使用有效 uid/gid 及 0666/0777 扣除 umask。目录固定父引用；未发布节点可完整回滚。
```

<a id="fn-0339515fc48a1692"></a>

## `xv::fs::RamFileSystem::retain`

```doxygen
@fn Inode *xv::fs::RamFileSystem::retain(Inode *node)
@brief 在引用锁内增加 RAM 节点引用。
@details 目录项查找在树锁内取得链接节点，已有引用也可直接保留；引用与目录链接分开计数。
```

<a id="fn-217486517d13a8eb"></a>

## `xv::fs::RamFileSystem::release`

```doxygen
@fn void xv::fs::RamFileSystem::release(Inode *node)
@brief 释放引用，并安全处理最后引用的回收。
@details 多个引用时只短暂持引用锁；可能是最后引用时先保留该引用，再取得或复用当前持有的树锁，重新判断 ref/nlink。这样等待树锁期间节点不会被其他 unlink 路径提前释放。
```

<a id="fn-1a54a9d94f11be0a"></a>

## `xv::fs::RamFileSystem::reclaim`

```doxygen
@fn void xv::fs::RamFileSystem::reclaim(Node *node)
@brief 回收无链接且无人引用的节点及其页。
@details 调用者持树锁。释放目录页、PageBuffer 和节点对象页，归还 inode 编号；目录另归还父固定引用，若父也满足回收条件则迭代向上处理，避免深目录递归耗尽内核栈。
```

<a id="fn-652b84b0201f47dc"></a>

## `xv::fs::RamFileSystem::unlock_release`

```doxygen
@fn void xv::fs::RamFileSystem::unlock_release(Inode *node)
@brief 先解锁 RAM 节点，再归还一个引用。
@details 最后引用可能再次取得树锁检查回收条件，引用在等待期间仍保持有效。
```

<a id="fn-ca85354173f3327c"></a>

## `xv::fs::RamFileSystem::slot`

```doxygen
@fn RamFileSystem::Slot *xv::fs::RamFileSystem::slot(Node *node, unsigned index, bool create)
@brief 按下标定位目录槽并可按需增加目录页。
@details 调用者持树锁，每页容纳一组名称和节点指针。create 为 false 不分配，true 时逐页创建零初始化的槽页；分配失败返回 nullptr。
```

<a id="fn-dd69ef647e56a43e"></a>

## `xv::fs::RamFileSystem::find`

```doxygen
@fn RamFileSystem::Slot *xv::fs::RamFileSystem::find(Node *node, const char *name, unsigned *index)
@brief 在 RAM 目录已使用范围查找一个名称。
@details 跳过空槽，按 DIRSIZ 比较，成功可返回逻辑槽下标。只借用槽指针，树锁释放后不能保留使用。
```

<a id="fn-3b8ec07f45144052"></a>

## `xv::fs::RamFileSystem::insert`

```doxygen
@fn bool xv::fs::RamFileSystem::insert(Node *parent, const char *name, Node *target)
@brief 在空洞或目录末尾发布一个 RAM 名称。
@details 调用者持树锁并已验证名称不存在；按需分配槽页、复制名字，只有成功后才增加逻辑槽数和目录大小。目标链接数由调用者按创建或硬链接规则维护。
```

<a id="fn-8edb568294f3071c"></a>

## `xv::fs::RamFileSystem::empty`

```doxygen
@fn bool xv::fs::RamFileSystem::empty(Node *node)
@brief 判断 RAM 目录是否没有普通子项。
@details 合成的点目录不占槽；扫描有效槽，只要存在一个 target 就返回 false。调用者持树锁。
```

<a id="fn-47a1af054919c8af"></a>

## `xv::fs::RamFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::RamFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
@brief 在已锁定 RAM 目录查找普通或点目录名称。
@details 普通项成功时增加目标引用，点目录返回自身或被固定的父引用；可选 offset 按通用 dirent 字节位置输出。失败不修改目录。
```

<a id="fn-5b7e740118badcc7"></a>

## `xv::fs::RamFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::RamFileSystem::create_at(Inode *directory, const char *name, short type, short, short)
@brief 在 RAM 目录创建并返回锁定节点。
@details 共享树锁内检查父 W/X、链接状态和支持的类型；按凭据创建新节点，目录槽发布失败回收节点及父引用，成功保持树锁供打开继续使用。
```

<a id="fn-284ead7688a027d6"></a>

## `xv::fs::RamFileSystem::link_at`

```doxygen
@fn int xv::fs::RamFileSystem::link_at(Inode *directory, const char *name, Inode *target)
@brief 在 RAM 目录添加硬链接。
@details 树锁内检查后端一致、父 W/X、目标类型和链接上限；目录槽成功发布才增加 nlink，失败保持输入引用及原内容。
```

<a id="fn-33548eacf725bea9"></a>

## `xv::fs::RamFileSystem::unlink_at`

```doxygen
@fn int xv::fs::RamFileSystem::unlink_at(Inode *directory, const char *name)
@brief 删除 RAM 目录项并应用 sticky 策略。
@details 共享树锁内验证父 W/X，sticky 仅允许 root、目录属主或目标属主；拒绝点目录及非空目录。先固定目标再撤链接，最后引用归还时回收数据页和编号。
```

<a id="fn-bda258709d76e544"></a>

## `xv::fs::RamFileSystem::truncate`

```doxygen
@fn void xv::fs::RamFileSystem::truncate(Inode *node)
@brief 清空普通 RAM 文件的数据页和逻辑大小。
@details 调用者持树锁，非普通文件触发内部错误；清空内容不修改其他 File 的既有偏移，后续越过 EOF 写入由 PageBuffer 补零。
```

<a id="fn-24fe196855f00a9c"></a>

## `xv::fs::RamFileSystem::read`

```doxygen
@fn int xv::fs::RamFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 读取 RAM 文件内容或合成目录记录。
@details 调用者持树锁，普通文件委托跨页缓冲，目录按稳定槽位置生成点目录、有效项和空洞。支持非对齐读取、短读、复制失败和 EOF，未链接目录的父指针由固定引用保持有效。
```

<a id="fn-9512b40f84b5ad5d"></a>

## `xv::fs::RamFileSystem::write`

```doxygen
@fn int xv::fs::RamFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
@brief 把数据写入 RAM 普通文件并更新大小。
@details 调用者持树锁，非普通文件失败。页分配或用户复制部分成功时保留实际长度和返回值，后端无需日志和磁盘块。
```

<a id="fn-1c27602529c8e52e"></a>

## `xv::fs::RamFileSystem::stat`

```doxygen
@fn void xv::fs::RamFileSystem::stat(Inode *node, Stat *result)
@brief 复制 RAM 节点状态与权限。
@details 树锁保护类型、长度、链接及安全属性；文件大小为逻辑字节数，目录按合成 dirent 范围计算。
```
