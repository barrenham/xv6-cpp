# `kernel/fs/path/path_operations.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/path/path_operations.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:xv6`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Xv6FileSystem::compare_names`](path_operations.cpp.md#fn-2582fcda97170617)
- [`xv::fs::Xv6FileSystem::create`](path_operations.cpp.md#fn-a0d31dedbcf7f673)
- [`xv::fs::Xv6FileSystem::directory_empty`](path_operations.cpp.md#fn-349493cbf4320e0a)
- [`xv::fs::Xv6FileSystem::link`](path_operations.cpp.md#fn-13afb769dcabb57b)
- [`xv::fs::Xv6FileSystem::link_entry`](path_operations.cpp.md#fn-2c549cede9fc1d13)
- [`xv::fs::Xv6FileSystem::lookup`](path_operations.cpp.md#fn-0b09ef09763d844a)
- [`xv::fs::Xv6FileSystem::lookup_entry`](path_operations.cpp.md#fn-004094c7c5387803)
- [`xv::fs::Xv6FileSystem::lookup_parent`](path_operations.cpp.md#fn-3c571ed40b048be1)
- [`xv::fs::Xv6FileSystem::unlink`](path_operations.cpp.md#fn-885ae8493e8a4843)
- [`xv::fs::Xv6FileSystem::walk_path`](path_operations.cpp.md#fn-a49b0d1e488e894d)
- [`xv::fs::skip_element`](path_operations.cpp.md#fn-5fe1a12a0d3cbb98)

<a id="fn-0b09ef09763d844a"></a>

## `xv::fs::Xv6FileSystem::lookup`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::lookup(const char *path)
@brief xv6 inode 文件系统：按路径或键查找对象。
@param path 待解析的零结尾路径字符串。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walk_path`。
@return 各分支实际返回：`walk_path(path, 0, name)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3c571ed40b048be1"></a>

## `xv::fs::Xv6FileSystem::lookup_parent`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::lookup_parent(const char *path, char *name)
@brief xv6 inode 文件系统：查找路径末尾元素的父目录。
@param path 待解析的零结尾路径字符串。
@param name 名称键或调试名称。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walk_path`。
@return 各分支实际返回：`walk_path(path, 1, name)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a0d31dedbcf7f673"></a>

## `xv::fs::Xv6FileSystem::create`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::create(char *path, short type, short major, short minor)
@brief xv6 inode 文件系统：建立新的文件系统对象。
@param path 待解析的零结尾路径字符串。
@param type 本次操作使用的 type 值。
@param major 字符设备主设备号。
@param minor 本次操作使用的 minor 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lookup_parent` → `lock` → `unlock_release` → `lookup_entry` → `allocate` → `update` → `link_entry`。
@details 本函数可见的状态更新包括：`ip->major = major;`；`ip->minor = minor;`；`ip->nlink = 1;`；`ip->nlink = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`(dp = lookup_parent(path, name)) == 0`；`dp->nlink == 0`；`type == T_DIR && dp->nlink >= NLINK_MAX`；`(ip = lookup_entry(dp, name, 0)) != 0`；`type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`ip`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-13afb769dcabb57b"></a>

## `xv::fs::Xv6FileSystem::link`

```doxygen
@fn int xv::fs::Xv6FileSystem::link(char *old, char *new_path)
@brief xv6 inode 文件系统：建立新的目录链接并维护链接计数。
@param old 原有状态或源页表。
@param new_path 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`transaction` → `lookup` → `lock` → `unlock_release` → `update` → `unlock` → `lookup_parent` → `link_entry` → `release`。
@details 需要特别跟踪的条件：`(ip = lookup(old)) == 0`；`ip->type == T_DIR`；`ip->nlink >= NLINK_MAX`；`(dp = lookup_parent(new_path, name)) == 0`；`dp->nlink == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-885ae8493e8a4843"></a>

## `xv::fs::Xv6FileSystem::unlink`

```doxygen
@fn int xv::fs::Xv6FileSystem::unlink(char *path)
@brief xv6 inode 文件系统：解除目录链接并处理对应对象的最后引用。
@param path 待解析的零结尾路径字符串。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`transaction` → `lookup_parent` → `lock` → `compare_names` → `lookup_entry` → `detail::fail` → `directory_empty` → `unlock_release` → `detail::fill` → `write` → `update`。
@details 需要特别跟踪的条件：`(dp = lookup_parent(path, name)) == 0`；`compare_names(name, ".") == 0 || compare_names(name, "..") == 0`；`(ip = lookup_entry(dp, name, &off)) == 0`；`ip->nlink < 1`；`ip->type == T_DIR && !directory_empty(ip)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

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

<a id="fn-a49b0d1e488e894d"></a>

## `xv::fs::Xv6FileSystem::walk_path`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::walk_path(const char *path, int nameiparent, char *name)
@brief xv6 inode 文件系统：逐段解析路径并推进当前 inode 引用。
@param path 待解析的零结尾路径字符串。
@param nameiparent 本次操作使用的 nameiparent 值。
@param name 名称键或调试名称。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`get` → `detail::current_directory` → `skip_element` → `lock` → `unlock_release` → `unlock` → `lookup_entry` → `release`。
@details 需要特别跟踪的条件：`*path == '/'`；`(path = skip_element(path, name)) != 0`；`ip->type != T_DIR`；`ip->nlink == 0`；`nameiparent && *path == '\0'`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`ip`。应结合上面的条件检查区分正常值、空值和错误值。
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

<a id="fn-5fe1a12a0d3cbb98"></a>

## `xv::fs::skip_element`

```doxygen
@fn static const char *xv::fs::skip_element(const char *path, char *name)
@brief 文件描述、inode、路径、块缓存与日志事务：跳过路径分隔符并取出下一段名字。
@param path 待解析的零结尾路径字符串。
@param name 名称键或调试名称。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::move`。
@details 本函数可见的状态更新包括：`name[len] = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`*path == '/'`；`*path == 0`；`*path != '/' && *path != 0`；`len >= DIRSIZ`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`path`。应结合上面的条件检查区分正常值、空值和错误值。
```
