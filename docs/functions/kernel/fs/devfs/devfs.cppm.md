# `kernel/fs/devfs/devfs.cppm`

## 函数导航

- [`xv::fs::DevFileSystem::DevFileSystem`](devfs.cppm.md#fn-6236a1917c8e409c)
- [`xv::fs::DevFileSystem::allocate`](devfs.cpp.md#fn-c5abadcff72de9ca)
- [`xv::fs::DevFileSystem::create_at`](devfs.cpp.md#fn-fb5edfa93d9d9127)
- [`xv::fs::DevFileSystem::initialize`](devfs.cpp.md#fn-c49eba16f985de33)
- [`xv::fs::DevFileSystem::initialize_inodes`](devfs.cpp.md#fn-00243c5ab4dcd922)
- [`xv::fs::DevFileSystem::link_at`](devfs.cpp.md#fn-83c2550aaf305a21)
- [`xv::fs::DevFileSystem::lock`](devfs.cpp.md#fn-6590a1415f934484)
- [`xv::fs::DevFileSystem::lookup_entry`](devfs.cpp.md#fn-5b759172a8b7cbd0)
- [`xv::fs::DevFileSystem::name`](devfs.cppm.md#fn-8db52e74df64a315)
- [`xv::fs::DevFileSystem::open_device`](devfs.cpp.md#fn-77a253e4e89cb39a)
- [`xv::fs::DevFileSystem::persistent`](devfs.cppm.md#fn-764f161c37df7fdb)
- [`xv::fs::DevFileSystem::read`](devfs.cpp.md#fn-b088477df66b3ab4)
- [`xv::fs::DevFileSystem::release`](devfs.cpp.md#fn-6dacf32c76c227d1)
- [`xv::fs::DevFileSystem::retain`](devfs.cpp.md#fn-e24bb911f90a0865)
- [`xv::fs::DevFileSystem::root`](devfs.cppm.md#fn-81706a03ed59f77d)
- [`xv::fs::DevFileSystem::stat`](devfs.cpp.md#fn-7d2a6b1d10adebe5)
- [`xv::fs::DevFileSystem::truncate`](devfs.cpp.md#fn-ab46f212380aff11)
- [`xv::fs::DevFileSystem::unlink_at`](devfs.cpp.md#fn-467ccd07c5a2c8b8)
- [`xv::fs::DevFileSystem::unlock`](devfs.cpp.md#fn-3c033f6b9674e8b8)
- [`xv::fs::DevFileSystem::unlock_release`](devfs.cpp.md#fn-24503ee0dd07bbe5)
- [`xv::fs::DevFileSystem::update`](devfs.cpp.md#fn-8ac05bd9ae1dd23c)
- [`xv::fs::DevFileSystem::write`](devfs.cpp.md#fn-35d09c6791808ad8)

<a id="fn-6236a1917c8e409c"></a>

## `xv::fs::DevFileSystem::DevFileSystem`

```doxygen
@fn constexpr xv::fs::DevFileSystem::DevFileSystem() = default
@brief 建立静态 devfs 节点表的初始状态。
@details 成员默认初始化，不分配页面。initialize_inodes 负责锁和归属，initialize 根据注册表构建目录。
```

<a id="fn-8db52e74df64a315"></a>

## `xv::fs::DevFileSystem::name`

```doxygen
@fn const char *xv::fs::DevFileSystem::name() const override
@brief 返回 devfs 文件系统名称。
@details 返回静态字符串，供内核标识文件系统实例。
```

<a id="fn-764f161c37df7fdb"></a>

## `xv::fs::DevFileSystem::persistent`

```doxygen
@fn bool xv::fs::DevFileSystem::persistent() const override
@brief 声明 devfs 节点不需要磁盘事务。
@details 始终返回 false；节点数据、引用和元数据全部驻留内存，最后关闭不进入磁盘日志。
```

<a id="fn-81706a03ed59f77d"></a>

## `xv::fs::DevFileSystem::root`

```doxygen
@fn Inode *xv::fs::DevFileSystem::root() override
@brief 取得 devfs 根目录的未锁定引用。
@details 根为固定节点表的第零项，引用计数经 references_ 保护；调用者负责释放。
```
