# `kernel/fs/inode/xv6_filesystem.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/inode/xv6_filesystem.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:xv6`。

直接模块依赖：`:filesystem`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Xv6FileSystem::Xv6FileSystem`](xv6_filesystem.cppm.md#fn-fbb2438a657db6f0)
- [`xv::fs::Xv6FileSystem::allocate`](xv6_filesystem.cpp.md#fn-071e023c7d9e28d2)
- [`xv::fs::Xv6FileSystem::allocate_block`](xv6_filesystem.cpp.md#fn-5a13ce655a471c2a)
- [`xv::fs::Xv6FileSystem::compare_names`](../path/path_operations.cpp.md#fn-2582fcda97170617)
- [`xv::fs::Xv6FileSystem::create`](../path/path_operations.cpp.md#fn-a0d31dedbcf7f673)
- [`xv::fs::Xv6FileSystem::directory_empty`](../path/path_operations.cpp.md#fn-349493cbf4320e0a)
- [`xv::fs::Xv6FileSystem::free_block`](xv6_filesystem.cpp.md#fn-9f690bb8450d7917)
- [`xv::fs::Xv6FileSystem::free_inode`](xv6_filesystem.cpp.md#fn-55c1f3a16e6a8da9)
- [`xv::fs::Xv6FileSystem::get`](xv6_filesystem.cpp.md#fn-9e44fd39e00d972e)
- [`xv::fs::Xv6FileSystem::initialize`](xv6_filesystem.cpp.md#fn-e1029df290612731)
- [`xv::fs::Xv6FileSystem::initialize_inodes`](xv6_filesystem.cpp.md#fn-8e9ee55da6c91ffd)
- [`xv::fs::Xv6FileSystem::link`](../path/path_operations.cpp.md#fn-13afb769dcabb57b)
- [`xv::fs::Xv6FileSystem::link_entry`](../path/path_operations.cpp.md#fn-2c549cede9fc1d13)
- [`xv::fs::Xv6FileSystem::lock`](xv6_filesystem.cpp.md#fn-6d014e00edddc19a)
- [`xv::fs::Xv6FileSystem::lookup`](../path/path_operations.cpp.md#fn-0b09ef09763d844a)
- [`xv::fs::Xv6FileSystem::lookup_entry`](../path/path_operations.cpp.md#fn-004094c7c5387803)
- [`xv::fs::Xv6FileSystem::lookup_parent`](../path/path_operations.cpp.md#fn-3c571ed40b048be1)
- [`xv::fs::Xv6FileSystem::map_block`](xv6_filesystem.cpp.md#fn-e115e4280d884517)
- [`xv::fs::Xv6FileSystem::name`](xv6_filesystem.cppm.md#fn-76aed8d1c1501404)
- [`xv::fs::Xv6FileSystem::read`](xv6_filesystem.cpp.md#fn-dd71262a05693ac2)
- [`xv::fs::Xv6FileSystem::read_super`](xv6_filesystem.cpp.md#fn-8886c17d9bd7f74a)
- [`xv::fs::Xv6FileSystem::reclaim`](xv6_filesystem.cpp.md#fn-63ef700d59c2ecf4)
- [`xv::fs::Xv6FileSystem::release`](xv6_filesystem.cpp.md#fn-488eb6e0a79dde7a)
- [`xv::fs::Xv6FileSystem::retain`](xv6_filesystem.cpp.md#fn-cf818bc3fcef07af)
- [`xv::fs::Xv6FileSystem::stat`](xv6_filesystem.cpp.md#fn-380640571e8b5b9b)
- [`xv::fs::Xv6FileSystem::truncate`](xv6_filesystem.cpp.md#fn-c1850e2e3bb309e8)
- [`xv::fs::Xv6FileSystem::unlink`](../path/path_operations.cpp.md#fn-885ae8493e8a4843)
- [`xv::fs::Xv6FileSystem::unlock`](xv6_filesystem.cpp.md#fn-93ed9249a65b47aa)
- [`xv::fs::Xv6FileSystem::unlock_release`](xv6_filesystem.cpp.md#fn-45dcd30b8fa7a526)
- [`xv::fs::Xv6FileSystem::update`](xv6_filesystem.cpp.md#fn-d23dade71242312c)
- [`xv::fs::Xv6FileSystem::walk_path`](../path/path_operations.cpp.md#fn-a49b0d1e488e894d)
- [`xv::fs::Xv6FileSystem::write`](xv6_filesystem.cpp.md#fn-b5be87921570227b)
- [`xv::fs::Xv6FileSystem::zero_block`](xv6_filesystem.cpp.md#fn-e9142d5fbb52fb87)

<a id="fn-fbb2438a657db6f0"></a>

## `xv::fs::Xv6FileSystem::Xv6FileSystem`

```doxygen
@fn constexpr xv::fs::Xv6FileSystem::Xv6FileSystem(BlockCache &cache, Journal &journal)
@brief 构造xv6 inode 文件系统，建立本重载要求的成员初值和借用关系。
@param cache 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param journal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_` → `journal_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-76aed8d1c1501404"></a>

## `xv::fs::Xv6FileSystem::name`

```doxygen
@fn const char *xv::fs::Xv6FileSystem::name() const override
@brief xv6 inode 文件系统：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
