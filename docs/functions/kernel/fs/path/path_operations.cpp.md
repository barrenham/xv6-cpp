# `kernel/fs/path/path_operations.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/path/path_operations.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:xv6`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Xv6FileSystem::compare_names`](path_operations.cpp.md#fn-2582fcda97170617)
- [`xv::fs::Xv6FileSystem::create_at`](path_operations.cpp.md#fn-c7e8c6cda59e8d12)
- [`xv::fs::Xv6FileSystem::directory_empty`](path_operations.cpp.md#fn-349493cbf4320e0a)
- [`xv::fs::Xv6FileSystem::link_at`](path_operations.cpp.md#fn-603e65641638664d)
- [`xv::fs::Xv6FileSystem::link_entry`](path_operations.cpp.md#fn-2c549cede9fc1d13)
- [`xv::fs::Xv6FileSystem::lookup_entry`](path_operations.cpp.md#fn-004094c7c5387803)
- [`xv::fs::Xv6FileSystem::rename_at`](path_operations.cpp.md#fn-df5473361440e15d)
- [`xv::fs::Xv6FileSystem::unlink_at`](path_operations.cpp.md#fn-5b4b6b7276311573)

<a id="fn-2582fcda97170617"></a>

## `xv::fs::Xv6FileSystem::compare_names`

```doxygen
@fn int xv::fs::Xv6FileSystem::compare_names(const char *s, const char *t)
@brief xv6 inode 文件系统：读取或计算 `detail::compare(s, t, DIRSIZ)`，返回当前查询结果。
@param s 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param t 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::compare`。
@return 各分支实际返回：`detail::compare(s, t, DIRSIZ)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-004094c7c5387803"></a>

## `xv::fs::Xv6FileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::lookup_entry(Inode *dp, const char *name, uint *poff)
@brief xv6 inode 文件系统：在目录数据中查找名称对应的目录项。
@param dp 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param name 名称键或调试名称。
@param poff 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `read` → `compare_names` → `get`。
@details 需要特别跟踪的条件：`dp->type != T_DIR`；`read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de)`；`de.inum == 0`；`compare_names(name, de.name) == 0`；`poff`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`get(dp->dev, inum)`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2c549cede9fc1d13"></a>

## `xv::fs::Xv6FileSystem::link_entry`

```doxygen
@fn int xv::fs::Xv6FileSystem::link_entry(Inode *dp, const char *name, uint inum)
@brief xv6 inode 文件系统：把名字与 inode 号写入目录。
@param dp 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param name 名称键或调试名称。
@param inum 本次操作使用的 inum 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lookup_entry` → `release` → `read` → `detail::fail` → `detail::copy_fixed` → `write`。
@details 本函数可见的状态更新包括：`de.inum = inum;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`(ip = lookup_entry(dp, name, 0)) != 0`；`read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de)`；`de.inum == 0`；`write(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-349493cbf4320e0a"></a>

## `xv::fs::Xv6FileSystem::directory_empty`

```doxygen
@fn int xv::fs::Xv6FileSystem::directory_empty(Inode *dp)
@brief xv6 inode 文件系统：目录空状态。
@param dp 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`read` → `detail::fail`。
@details 需要特别跟踪的条件：`read(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de)`；`de.inum != 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c7e8c6cda59e8d12"></a>

## `xv::fs::Xv6FileSystem::create_at`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::create_at(Inode *dp, const char *name, short type, short major, short minor)
@brief 在磁盘父目录创建并返回锁定节点。
@details 父锁内验证目录仍有链接及 W/X；设备创建仅 root。新节点使用有效 uid/gid，文件 0666、目录 0777、设备 0600 再扣除 umask；失败回滚引用和链接。
@details 流程由 CreateFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-603e65641638664d"></a>

## `xv::fs::Xv6FileSystem::link_at`

```doxygen
@fn int xv::fs::Xv6FileSystem::link_at(Inode *dp, const char *name, Inode *ip)
@brief 在磁盘目录添加已有文件的硬链接。
@details 拒绝跨后端和目录目标；父目录锁内验证 W/X 与链接状态，发布目录项失败时撤销增加的链接数。调用者维持磁盘事务。
@details 流程由 LinkFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-5b4b6b7276311573"></a>

## `xv::fs::Xv6FileSystem::unlink_at`

```doxygen
@fn int xv::fs::Xv6FileSystem::unlink_at(Inode *dp, const char *name)
@brief 在磁盘目录删除目录项。
@details 父子节点锁内检查父 W/X 及 sticky：root、目录属主或目标属主可删；仍拒绝点目录及非空目录。日志化目录项和链接数，最后引用回收由 inode 生命周期管理。
@details 流程由 UnlinkFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-df5473361440e15d"></a>

## `xv::fs::Xv6FileSystem::rename_at`

```doxygen
@fn int xv::fs::Xv6FileSystem::rename_at(Inode *from, const char *old_name, Inode *to, const char *new_name)
@brief 在单个日志事务内重命名磁盘文件或目录。
@details 父目录按 inode 号排序加锁；源/目标与父节点别名时先拒绝，避免递归取得同一 inode 锁，然后锁定源和不同的目标。检查写/执行权限、sticky 属主、类型和空目录约束；先安装目标再清除源项，目录跨父移动时改写 .. 和父链接计数。相同 inode 的两个名字保持不变。解锁后释放节点引用；调用者持有 VFS 命名空间锁和日志事务。
@return 0 表示成功，-1 表示校验或安装失败。
@details 流程由 RenameFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```
