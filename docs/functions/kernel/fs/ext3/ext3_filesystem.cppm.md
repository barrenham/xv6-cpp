# `kernel/fs/ext3/ext3_filesystem.cppm`

> 主机参考实现：此文件仅用于主机合同测试，不链接生产 `kernel.elf`。运行时后端见[文件系统服务迁移](../../../../fs-service-migration.md)。

<a id="fn-c31fdaaf2152796e"></a>

## `xv::fs::Ext3FileSystem::DiskIo::DiskIo`

```doxygen
@fn constexpr explicit xv::fs::Ext3FileSystem::DiskIo::DiskIo(BlockCache &cache):cache_(cache)
@brief 构造对象并建立成员初值及借用关系。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-548e1cbb1d78e56b"></a>

## `xv::fs::Ext3FileSystem::Log::Log`

```doxygen
@fn constexpr explicit xv::fs::Ext3FileSystem::Log::Log(Ext3FileSystem &owner):owner_(owner)
@brief 构造对象并建立成员初值及借用关系。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-a5daa3d7434f3d69"></a>

## `xv::fs::Ext3FileSystem::Ext3FileSystem`

```doxygen
@fn constexpr explicit xv::fs::Ext3FileSystem::Ext3FileSystem(BlockCache &cache):io_(cache),volume_(io_),log_(*this)
@brief 构造对象并建立成员初值及借用关系。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-0be3d52c3a59cb67"></a>

## `xv::fs::Ext3FileSystem::attach`

```doxygen
@fn void xv::fs::Ext3FileSystem::attach(xv::drivers::BlockDevice &device)
@brief 实现 xv::fs::Ext3FileSystem::attach 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-69d733afdd8e073d"></a>

## `xv::fs::Ext3FileSystem::name_max`

```doxygen
@fn unsigned xv::fs::Ext3FileSystem::name_max() const override
@brief 返回 `255`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-8428c09e2bd93045"></a>

## `xv::fs::Ext3FileSystem::name`

```doxygen
@fn const char *xv::fs::Ext3FileSystem::name() const override
@brief 返回 `"ext3"`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-c63092088222a107"></a>

## `xv::fs::Ext3FileSystem::flush`

```doxygen
@fn void xv::fs::Ext3FileSystem::flush() override
@brief 实现 xv::fs::Ext3FileSystem::flush 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-31fa0fee2d4f1bd8"></a>

## `xv::fs::Ext3FileSystem::transaction_log`

```doxygen
@fn Journal *xv::fs::Ext3FileSystem::transaction_log() override
@brief 返回 `&log_`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

- [`xv::fs::Ext3FileSystem::Ext3FileSystem`](ext3_filesystem.cppm.md#fn-a5daa3d7434f3d69)

- [`xv::fs::Ext3FileSystem::allocate`](ext3_filesystem.cpp.md#fn-e2c3bb50fcd4a9b9)

- [`xv::fs::Ext3FileSystem::attach`](ext3_filesystem.cppm.md#fn-0be3d52c3a59cb67)

- [`xv::fs::Ext3FileSystem::busy`](ext3_filesystem.cpp.md#fn-e6fea83b0e152392)

- [`xv::fs::Ext3FileSystem::changed`](ext3_filesystem.cpp.md#fn-267a4ca53aa4eb93)

- [`xv::fs::Ext3FileSystem::collect_orphans`](ext3_filesystem.cpp.md#fn-5188eb545030b86a)

- [`xv::fs::Ext3FileSystem::component`](ext3_filesystem.cpp.md#fn-af18914c0e2b5dc9)

- [`xv::fs::Ext3FileSystem::create_at`](ext3_filesystem.cpp.md#fn-a4f52a1cf4cc44de)

- [`xv::fs::Ext3FileSystem::flush`](ext3_filesystem.cppm.md#fn-c63092088222a107)

- [`xv::fs::Ext3FileSystem::get`](ext3_filesystem.cpp.md#fn-ab3267c1b6083c7a)

- [`xv::fs::Ext3FileSystem::initialize`](ext3_filesystem.cpp.md#fn-702837280a873644)

- [`xv::fs::Ext3FileSystem::initialize_inodes`](ext3_filesystem.cpp.md#fn-95816528f8a4fe2f)

- [`xv::fs::Ext3FileSystem::link_at`](ext3_filesystem.cpp.md#fn-d64aba2c0950e86c)

- [`xv::fs::Ext3FileSystem::lock`](ext3_filesystem.cpp.md#fn-2feda166195a1851)

- [`xv::fs::Ext3FileSystem::lookup_entry`](ext3_filesystem.cpp.md#fn-51333fb3d846df32)

- [`xv::fs::Ext3FileSystem::name`](ext3_filesystem.cppm.md#fn-8428c09e2bd93045)

- [`xv::fs::Ext3FileSystem::name_max`](ext3_filesystem.cppm.md#fn-69d733afdd8e073d)

- [`xv::fs::Ext3FileSystem::next_directory`](ext3_filesystem.cpp.md#fn-126a617dc2cdd982)

- [`xv::fs::Ext3FileSystem::open_block_device`](ext3_filesystem.cpp.md#fn-91abfb4955cbd6e2)

- [`xv::fs::Ext3FileSystem::open_device`](ext3_filesystem.cpp.md#fn-c6fb5ecf0f85b05e)

- [`xv::fs::Ext3FileSystem::read`](ext3_filesystem.cpp.md#fn-b46bf5ed64088515)

- [`xv::fs::Ext3FileSystem::refresh`](ext3_filesystem.cpp.md#fn-75e032d40eea8be3)

- [`xv::fs::Ext3FileSystem::release`](ext3_filesystem.cpp.md#fn-cfc86476f85b27a1)

- [`xv::fs::Ext3FileSystem::rename_at`](ext3_filesystem.cpp.md#fn-0d761799def280d5)

- [`xv::fs::Ext3FileSystem::resize`](ext3_filesystem.cpp.md#fn-10c6216358bba2cc)

- [`xv::fs::Ext3FileSystem::retain`](ext3_filesystem.cpp.md#fn-708f2a85a201de71)

- [`xv::fs::Ext3FileSystem::root`](ext3_filesystem.cpp.md#fn-2219bf0c921e510f)

- [`xv::fs::Ext3FileSystem::shutdown`](ext3_filesystem.cpp.md#fn-c9c4c79acedb3b57)

- [`xv::fs::Ext3FileSystem::stat`](ext3_filesystem.cpp.md#fn-b5c4b6b2761705bc)

- [`xv::fs::Ext3FileSystem::transaction_log`](ext3_filesystem.cppm.md#fn-31fa0fee2d4f1bd8)

- [`xv::fs::Ext3FileSystem::truncate`](ext3_filesystem.cpp.md#fn-83b644b03b0202bf)

- [`xv::fs::Ext3FileSystem::unlink_at`](ext3_filesystem.cpp.md#fn-047edaa14cbf21cd)

- [`xv::fs::Ext3FileSystem::unlock`](ext3_filesystem.cpp.md#fn-1004bcff94a10317)

- [`xv::fs::Ext3FileSystem::unlock_release`](ext3_filesystem.cpp.md#fn-e0544df51a26b59f)

- [`xv::fs::Ext3FileSystem::update`](ext3_filesystem.cpp.md#fn-1d590cd4ac89473d)

- [`xv::fs::Ext3FileSystem::wire`](ext3_filesystem.cpp.md#fn-453ed9272bac3517)

- [`xv::fs::Ext3FileSystem::write`](ext3_filesystem.cpp.md#fn-2c26647454530b5b)

- [`xv::fs::Ext3FileSystem::DiskIo::DiskIo`](ext3_filesystem.cppm.md#fn-c31fdaaf2152796e)

- [`xv::fs::Ext3FileSystem::DiskIo::allocate_page`](ext3_filesystem.cpp.md#fn-ad5de7c5d8f5fe10)

- [`xv::fs::Ext3FileSystem::DiskIo::commit`](ext3_filesystem.cpp.md#fn-852ac8af17e88182)

- [`xv::fs::Ext3FileSystem::DiskIo::flush`](ext3_filesystem.cpp.md#fn-eb20af1e8ecae214)

- [`xv::fs::Ext3FileSystem::DiskIo::free_page`](ext3_filesystem.cpp.md#fn-c973b4b2c86540f6)

- [`xv::fs::Ext3FileSystem::DiskIo::now_seconds`](ext3_filesystem.cpp.md#fn-8ec89db63d0dba30)

- [`xv::fs::Ext3FileSystem::DiskIo::prepare_reserve`](ext3_filesystem.cpp.md#fn-3db24b1623354620)

- [`xv::fs::Ext3FileSystem::DiskIo::read`](ext3_filesystem.cpp.md#fn-da44e6b3bead1e48)

- [`xv::fs::Ext3FileSystem::DiskIo::release_reserve`](ext3_filesystem.cpp.md#fn-a243ed0aa250989e)

- [`xv::fs::Ext3FileSystem::DiskIo::size_bytes`](ext3_filesystem.cpp.md#fn-dfa7186f138719b0)

- [`xv::fs::Ext3FileSystem::DiskIo::write`](ext3_filesystem.cpp.md#fn-676a6bfdc1244d56)

- [`xv::fs::Ext3FileSystem::Log::Log`](ext3_filesystem.cppm.md#fn-548e1cbb1d78e56b)

- [`xv::fs::Ext3FileSystem::Log::begin`](ext3_filesystem.cpp.md#fn-fbaeebb59ac09b47)

- [`xv::fs::Ext3FileSystem::Log::changed`](ext3_filesystem.cpp.md#fn-b20e819813377e2a)

- [`xv::fs::Ext3FileSystem::Log::commit`](ext3_filesystem.cpp.md#fn-b9dc7f1dbddb1193)

- [`xv::fs::Ext3FileSystem::Log::end`](ext3_filesystem.cpp.md#fn-af9755f73c5675ef)

- [`xv::fs::Ext3FileSystem::Log::initialize`](ext3_filesystem.cpp.md#fn-c3525fae60442c17)

- [`xv::fs::Ext3FileSystem::Log::ready`](ext3_filesystem.cpp.md#fn-d4a34f9d0fb9d34b)

- [`xv::fs::Ext3FileSystem::Log::record`](ext3_filesystem.cpp.md#fn-e6cd0201678e66c2)

- [`xv::fs::Ext3FileSystem::Log::sync`](ext3_filesystem.cpp.md#fn-01d3c4443aee01d9)

- [`xv::fs::Ext3FileSystem::Log::wait`](ext3_filesystem.cpp.md#fn-ad6465385c48e803)
