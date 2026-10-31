# `kernel/fs/ext3/ext3_filesystem.cppm`

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
