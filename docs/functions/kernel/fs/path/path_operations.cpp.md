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
@brief 在借用的父目录中创建磁盘 inode。
@details 外层须有磁盘事务，父目录未锁定且有引用。拒绝孤儿目录和链接数溢出，已有普通文件或设备可按 T_FILE 打开；新目录建立 . 和 ..。失败回滚新 inode，成功返回锁定的子引用，不释放父引用。
```

<a id="fn-603e65641638664d"></a>

## `xv::fs::Xv6FileSystem::link_at`

```doxygen
@fn int xv::fs::Xv6FileSystem::link_at(Inode *dp, const char *name, Inode *ip)
@brief 在同一磁盘目录中增加目标硬链接。
@details 借用两个未锁定引用，要求外层事务。拒绝目录、无链接节点和 nlink 溢出；先保护目标链接数再加锁修改父目录，目录写入失败时回滚目标链接数。父目录已删除或设备不同则失败。
```

<a id="fn-5b4b6b7276311573"></a>

## `xv::fs::Xv6FileSystem::unlink_at`

```doxygen
@fn int xv::fs::Xv6FileSystem::unlink_at(Inode *dp, const char *name)
@brief 从借用的磁盘目录中删除一个目录项。
@details 要求外层事务及未锁定父引用。加锁后拒绝 .、.. 和非空目录，清空目录项并更新父子链接数，释放查得的子引用；父引用由调用者释放。
```
