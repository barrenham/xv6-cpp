# `kernel/fs/ramfs/ramfs.cpp`

## 函数导航

- [`xv::fs::RamFileSystem::busy`](ramfs.cpp.md#fn-bf6f827e9769d7e9)
- [`xv::fs::RamFileSystem::create_at`](ramfs.cpp.md#fn-5b7e740118badcc7)
- [`xv::fs::RamFileSystem::empty`](ramfs.cpp.md#fn-8edb568294f3071c)
- [`xv::fs::RamFileSystem::find`](ramfs.cpp.md#fn-dd69ef647e56a43e)
- [`xv::fs::RamFileSystem::initialize`](ramfs.cpp.md#fn-9e40f661a8e79e17)
- [`xv::fs::RamFileSystem::initialize_inodes`](ramfs.cpp.md#fn-f50c5c9aa65cd651)
- [`xv::fs::RamFileSystem::initialize_mount`](ramfs.cpp.md#fn-1e5edc9d6003fd62)
- [`xv::fs::RamFileSystem::insert`](ramfs.cpp.md#fn-3b8ec07f45144052)
- [`xv::fs::RamFileSystem::link_at`](ramfs.cpp.md#fn-284ead7688a027d6)
- [`xv::fs::RamFileSystem::lookup_entry`](ramfs.cpp.md#fn-47a1af054919c8af)
- [`xv::fs::RamFileSystem::make`](ramfs.cpp.md#fn-b4793ba9f01ef544)
- [`xv::fs::RamFileSystem::read`](ramfs.cpp.md#fn-24fe196855f00a9c)
- [`xv::fs::RamFileSystem::reclaim`](ramfs.cpp.md#fn-1a54a9d94f11be0a)
- [`xv::fs::RamFileSystem::release`](ramfs.cpp.md#fn-217486517d13a8eb)
- [`xv::fs::RamFileSystem::rename_at`](ramfs.cpp.md#fn-af0a949d4f892d41)
- [`xv::fs::RamFileSystem::resize`](ramfs.cpp.md#fn-26a32874b953ad67)
- [`xv::fs::RamFileSystem::retain`](ramfs.cpp.md#fn-0339515fc48a1692)
- [`xv::fs::RamFileSystem::root`](ramfs.cpp.md#fn-5495fb0db3fbb09a)
- [`xv::fs::RamFileSystem::shutdown`](ramfs.cpp.md#fn-854bbfaca9e5aef5)
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
@brief 建立启动 RAM 根并将分配失败视为启动错误。
@details 调用可失败的 initialize_mount；根为 root:root 01777，用于 /tmp。动态挂载直接调用可失败版本而不触发 panic。
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
@brief 分配带归属、权限及生命周期登记的 RAM 节点。
@details 树锁内选择空闲编号并分配单页 Node。登记节点链表和初始外部引用，继承当前 UID/GID/umask；目录保留父节点的内部引用，此引用不计 live_references_。分配失败不占编号。
@return 初始引用为 1 的节点；编号或页面不足返回 nullptr。
@details 流程由 MakeFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-0339515fc48a1692"></a>

## `xv::fs::RamFileSystem::retain`

```doxygen
@fn Inode *xv::fs::RamFileSystem::retain(Inode *node)
@brief 增加节点引用及实例外部引用总数。
@details references_ 短自旋锁同时保护 ref 和 live_references_。目录父固定引用在建立时另行抵扣，不能把内部生命周期计数误报为外部使用者。
@return 传入的节点指针。
```

<a id="fn-217486517d13a8eb"></a>

## `xv::fs::RamFileSystem::release`

```doxygen
@fn void xv::fs::RamFileSystem::release(Inode *node)
@brief 归还外部节点引用并按需回收已删除节点。
@details 多引用时在 references_ 内同步减少 ref 和 live_references_；最后引用先取得 tree_ 后才减少外部计数，使卸载不能在释放者等待树锁时提前销毁实例。nlink 为零时执行迭代回收；已持树锁时避免递归获取。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-1a54a9d94f11be0a"></a>

## `xv::fs::RamFileSystem::reclaim`

```doxygen
@fn void xv::fs::RamFileSystem::reclaim(Node *node)
@brief 迭代回收零链接、零引用节点及失去最后固定引用的父目录。
@details 树锁内先从全实例节点链表和编号位图移除，再析构目录页、PageBuffer 和节点。父引用是内部引用，只扣父 ref 而不扣 live_references_；因此深目录回收不递归占用内核栈，孤儿目录仍能阻止卸载。
@details 流程由 ReclaimFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
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
@brief 在树锁内创建 RAM 节点。
@details 除普通文件和目录外允许 T_SYMLINK；权限、编号、父固定引用与失败回收仍沿用原有约定。
@details 流程由 CreateFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-284ead7688a027d6"></a>

## `xv::fs::RamFileSystem::link_at`

```doxygen
@fn int xv::fs::RamFileSystem::link_at(Inode *directory, const char *name, Inode *target)
@brief 在 RAM 目录添加硬链接。
@details 树锁内检查后端一致、父 W/X、目标类型和链接上限；目录槽成功发布才增加 nlink，失败保持输入引用及原内容。
@details 流程由 LinkFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-33548eacf725bea9"></a>

## `xv::fs::RamFileSystem::unlink_at`

```doxygen
@fn int xv::fs::RamFileSystem::unlink_at(Inode *directory, const char *name)
@brief 删除 RAM 目录项并应用 sticky 策略。
@details 共享树锁内验证父 W/X，sticky 仅允许 root、目录属主或目标属主；拒绝点目录及非空目录。先固定目标再撤链接，最后引用归还时回收数据页和编号。
@details 流程由 UnlinkFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
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
@brief 读取 RAM 文件、链接内容或目录记录。
@details 普通文件与符号链接共用 PageBuffer；目录仍按固定 dirent 布局逐段复制，失败不越过有效数据。
@details 流程由 ReadFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-9512b40f84b5ad5d"></a>

## `xv::fs::RamFileSystem::write`

```doxygen
@fn int xv::fs::RamFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
@brief 向 RAM 普通文件或符号链接写入内容。
@details 不允许目录写入，PageBuffer 处理补零和部分失败；回写实际逻辑长度。
```

<a id="fn-1c27602529c8e52e"></a>

## `xv::fs::RamFileSystem::stat`

```doxygen
@fn void xv::fs::RamFileSystem::stat(Inode *node, Stat *result)
@brief 复制 RAM 节点状态与权限。
@details 树锁保护类型、长度、链接及安全属性；文件大小为逻辑字节数，目录按合成 dirent 范围计算。
```

<a id="fn-1e5edc9d6003fd62"></a>

## `xv::fs::RamFileSystem::initialize_mount`

```doxygen
@fn bool xv::fs::RamFileSystem::initialize_mount(unsigned device)
@brief 尝试分配独立 RAM 根而不在 OOM 时触发 panic。
@details 树锁内分配根、设置自指父节点、root:root 01777 和初始链接。失败释放树锁返回 false；动态 VFS 随后将其根权限改为 0755，启动 /tmp 保留 01777。
@return true 表示根已初始化，false 表示页面分配失败。
@details 流程由 InitializeMountFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-bf6f827e9769d7e9"></a>

## `xv::fs::RamFileSystem::busy`

```doxygen
@fn bool xv::fs::RamFileSystem::busy()
@brief 检查外部引用是否仅剩根和挂载表的固定引用。
@details references_ 保护 live_references_；父节点内部固定引用不参与计数。打开文件、cwd、孤儿节点及子挂载引用都会阻止卸载。调用者持 VFS 命名空间锁，防止新路径查找进入。
@return 外部引用数不等于 2 时返回 true。
```

<a id="fn-854bbfaca9e5aef5"></a>

## `xv::fs::RamFileSystem::shutdown`

```doxygen
@fn void xv::fs::RamFileSystem::shutdown()
@brief 回收已解除挂载实例中的全部节点及数据页。
@details 取得 tree_，等待已扣减引用但仍在回收的操作完成。遍历独立节点链表，直接销毁目录页、PageBuffer 和节点，不沿内部父引用递归释放；清空编号位图及根/计数。仅可在 busy 返回 false 且挂载入口已移除时调用。
```

<a id="fn-af0a949d4f892d41"></a>

## `xv::fs::RamFileSystem::rename_at`

```doxygen
@fn int xv::fs::RamFileSystem::rename_at(Inode *from, const char *old_name, Inode *to, const char *new_name)
@brief 在树锁内移动或替换 RAM 目录项。
@details 检查源/目标父目录授权、sticky、类型、目标目录是否为空及链接数边界。先完成目标槽安装，再清除源槽；替换 inode 减少链接并在最后引用释放时回收。跨父目录移动转换内部父节点引用，同时维护外部引用计数。调用前由 VFS 拒绝目录环及挂载移动。
@return 0 表示成功，-1 表示检查或槽分配失败。
@details 流程由 RenameFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-26a32874b953ad67"></a>

## `xv::fs::RamFileSystem::resize`

```doxygen
@fn int xv::fs::RamFileSystem::resize(Inode *node, unsigned size)
@brief 调整 RAM 普通文件的页缓冲。
@details 调用时树锁已持有；成功设置 inode 长度，失败返回 -1，不允许通过此接口调整目录或链接。
```
