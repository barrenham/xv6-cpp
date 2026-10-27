# `kernel/fs/ramfs/ramfs.cppm`

## 函数导航

- [`xv::fs::RamFileSystem::RamFileSystem`](ramfs.cppm.md#fn-4dd039bb9ddd786b)
- [`xv::fs::RamFileSystem::allocate`](ramfs.cppm.md#fn-a86f7315dbdb81ec)
- [`xv::fs::RamFileSystem::create_at`](ramfs.cpp.md#fn-5b7e740118badcc7)
- [`xv::fs::RamFileSystem::empty`](ramfs.cpp.md#fn-8edb568294f3071c)
- [`xv::fs::RamFileSystem::find`](ramfs.cpp.md#fn-dd69ef647e56a43e)
- [`xv::fs::RamFileSystem::initialize`](ramfs.cpp.md#fn-9e40f661a8e79e17)
- [`xv::fs::RamFileSystem::initialize_inodes`](ramfs.cpp.md#fn-f50c5c9aa65cd651)
- [`xv::fs::RamFileSystem::insert`](ramfs.cpp.md#fn-3b8ec07f45144052)
- [`xv::fs::RamFileSystem::link_at`](ramfs.cpp.md#fn-284ead7688a027d6)
- [`xv::fs::RamFileSystem::lock`](ramfs.cppm.md#fn-4faf4b31c38e085a)
- [`xv::fs::RamFileSystem::lookup_entry`](ramfs.cpp.md#fn-47a1af054919c8af)
- [`xv::fs::RamFileSystem::make`](ramfs.cpp.md#fn-b4793ba9f01ef544)
- [`xv::fs::RamFileSystem::name`](ramfs.cppm.md#fn-4e3451d1290ebea9)
- [`xv::fs::RamFileSystem::persistent`](ramfs.cppm.md#fn-04c3d55db3f03821)
- [`xv::fs::RamFileSystem::read`](ramfs.cpp.md#fn-24fe196855f00a9c)
- [`xv::fs::RamFileSystem::reclaim`](ramfs.cpp.md#fn-1a54a9d94f11be0a)
- [`xv::fs::RamFileSystem::release`](ramfs.cpp.md#fn-217486517d13a8eb)
- [`xv::fs::RamFileSystem::retain`](ramfs.cpp.md#fn-0339515fc48a1692)
- [`xv::fs::RamFileSystem::root`](ramfs.cpp.md#fn-5495fb0db3fbb09a)
- [`xv::fs::RamFileSystem::slot`](ramfs.cpp.md#fn-ca85354173f3327c)
- [`xv::fs::RamFileSystem::stat`](ramfs.cpp.md#fn-1c27602529c8e52e)
- [`xv::fs::RamFileSystem::state`](ramfs.cppm.md#fn-c79b50650687707f)
- [`xv::fs::RamFileSystem::truncate`](ramfs.cpp.md#fn-bda258709d76e544)
- [`xv::fs::RamFileSystem::unlink_at`](ramfs.cpp.md#fn-33548eacf725bea9)
- [`xv::fs::RamFileSystem::unlock`](ramfs.cppm.md#fn-0712f897e6188fb7)
- [`xv::fs::RamFileSystem::unlock_release`](ramfs.cpp.md#fn-652b84b0201f47dc)
- [`xv::fs::RamFileSystem::update`](ramfs.cppm.md#fn-af580044ed0c54ef)
- [`xv::fs::RamFileSystem::write`](ramfs.cpp.md#fn-9512b40f84b5ad5d)

<a id="fn-4dd039bb9ddd786b"></a>

## `xv::fs::RamFileSystem::RamFileSystem`

```doxygen
@fn constexpr xv::fs::RamFileSystem::RamFileSystem() = default
@brief 建立 RAM 文件系统的静态控制状态。
@details 构造不分配节点或数据页；位图、根指针和设备号以默认成员初值建立，锁与根分阶段初始化。
```

<a id="fn-4e3451d1290ebea9"></a>

## `xv::fs::RamFileSystem::name`

```doxygen
@fn const char *xv::fs::RamFileSystem::name() const override
@brief 返回 RAM 文件系统后端名称。
@details 名称为 ramfs，内容由物理页驻留，不使用磁盘数据块。
```

<a id="fn-04c3d55db3f03821"></a>

## `xv::fs::RamFileSystem::persistent`

```doxygen
@fn bool xv::fs::RamFileSystem::persistent() const override
@brief 声明 RAM 节点不需要磁盘日志事务。
@details 返回 false，使文件读写及最后关闭只管理内存资源。路径穿越磁盘目录时仍沿用外层事务规则。
```

<a id="fn-a86f7315dbdb81ec"></a>

## `xv::fs::RamFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::RamFileSystem::allocate(unsigned, short) override
@brief 拒绝绕过目录生命周期的低层分配请求。
@details RAM 节点由 create_at 内部的 make 分配和回滚；公开低层 allocate 返回 nullptr。
```

<a id="fn-4faf4b31c38e085a"></a>

## `xv::fs::RamFileSystem::lock`

```doxygen
@fn void xv::fs::RamFileSystem::lock(Inode *) override
@brief 取得 RAM 后端共享的树睡眠锁。
@details 所有节点锁映射到同一 tree_，串行化名字、数据和共享文件偏移。调用者持有节点引用，不能重复取得同一 RAM 后端的节点锁。
```

<a id="fn-0712f897e6188fb7"></a>

## `xv::fs::RamFileSystem::unlock`

```doxygen
@fn void xv::fs::RamFileSystem::unlock(Inode *) override
@brief 释放 RAM 后端树睡眠锁。
@details 不改变节点引用；调用者必须持有本后端树锁。
```

<a id="fn-af580044ed0c54ef"></a>

## `xv::fs::RamFileSystem::update`

```doxygen
@fn void xv::fs::RamFileSystem::update(Inode *) override
@brief 完成无需持久写回的 RAM 元数据更新。
@details 内存字段在树锁内直接维护，此兼容节点操作无额外动作，不写磁盘。
```

<a id="fn-c79b50650687707f"></a>

## `xv::fs::RamFileSystem::state`

```doxygen
@fn static Node *xv::fs::RamFileSystem::state(Inode *node)
@brief 取得 inode 对应的 RAM 节点控制对象。
@details 通过私有 context_ 记录定位，不依赖跨类型布局强转。调用者保证节点归属为本实例且持有有效引用。
```
