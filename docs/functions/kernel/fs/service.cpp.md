# `kernel/fs/service.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../kernel/fs/service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:lru`, `:xv6`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::cache`](service.cpp.md#fn-8340fd0d2696b2e9)
- [`xv::fs::devices`](service.cpp.md#fn-94c869e5070c351f)
- [`xv::fs::files`](service.cpp.md#fn-f6456e38e2f03d00)
- [`xv::fs::filesystem`](service.cpp.md#fn-33ca000fe0c21f95)
- [`xv::fs::initialize_cache`](service.cpp.md#fn-6945466a2423e97b)
- [`xv::fs::journal`](service.cpp.md#fn-fde437052d31cdd9)

<a id="fn-8340fd0d2696b2e9"></a>

## `xv::fs::cache`

```doxygen
@fn BlockCache &xv::fs::cache()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_cache`，返回当前查询结果。
@return 各分支实际返回：`system_cache`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6945466a2423e97b"></a>

## `xv::fs::initialize_cache`

```doxygen
@fn void xv::fs::initialize_cache()
@brief 装配系统缓存和可选数据盘缓存。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-f6456e38e2f03d00"></a>

## `xv::fs::files`

```doxygen
@fn OpenFileTable &xv::fs::files()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_files`，返回当前查询结果。
@return 各分支实际返回：`system_files`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-94c869e5070c351f"></a>

## `xv::fs::devices`

```doxygen
@fn DeviceRegistry &xv::fs::devices()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_devices`，返回当前查询结果。
@return 各分支实际返回：`system_devices`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-33ca000fe0c21f95"></a>

## `xv::fs::filesystem`

```doxygen
@fn VirtualFileSystem &xv::fs::filesystem()
@brief 取得统一路径命名空间的 VFS 实例。
@details 返回类型为 VirtualFileSystem，具体磁盘、devfs 与 procfs 通过节点 owner 分发。VFS 不再继承文件系统后端接口，后端无需提供独立路径包装。
```

<a id="fn-fde437052d31cdd9"></a>

## `xv::fs::journal`

```doxygen
@fn Journal &xv::fs::journal()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_journal`，返回当前查询结果。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-2764ac52db897bd0"></a>

## `xv::fs::detail::registered_filesystem`

```doxygen
@fn bool detail::xv::fs::detail::registered_filesystem(unsigned index,detail::FileSystemType &record)
@brief 按索引读取已注册文件系统类型及nodev属性。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
