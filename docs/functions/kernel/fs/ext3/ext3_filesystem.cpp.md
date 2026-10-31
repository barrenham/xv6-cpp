# `kernel/fs/ext3/ext3_filesystem.cpp`

<a id="fn-da44e6b3bead1e48"></a>

## `xv::fs::Ext3FileSystem::DiskIo::read`

```doxygen
@fn bool Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::read(xv::ext3::u64 offset,void *data,unsigned count)
@brief 实现 xv::fs::Ext3FileSystem::DiskIo::read 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-676a6bfdc1244d56"></a>

## `xv::fs::Ext3FileSystem::DiskIo::write`

```doxygen
@fn bool Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::write(xv::ext3::u64 offset,const void *data,unsigned count)
@brief 实现 xv::fs::Ext3FileSystem::DiskIo::write 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-eb20af1e8ecae214"></a>

## `xv::fs::Ext3FileSystem::DiskIo::flush`

```doxygen
@fn bool Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::flush()
@brief 返回 `true`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-dfa7186f138719b0"></a>

## `xv::fs::Ext3FileSystem::DiskIo::size_bytes`

```doxygen
@fn xv::ext3::u64 Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::size_bytes() const
@brief 按真实扇区容量计算字节数，超出缓存块号范围则拒绝。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-8ec89db63d0dba30"></a>

## `xv::fs::Ext3FileSystem::DiskIo::now_seconds`

```doxygen
@fn xv::ext3::u32 Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::now_seconds() const
@brief 读取实际RTC秒数作为inode时间。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-3db24b1623354620"></a>

## `xv::fs::Ext3FileSystem::DiskIo::prepare_reserve`

```doxygen
@fn bool Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::prepare_reserve()
@brief 提前申请三个提交暂存页，失败归还已申请页。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-a243ed0aa250989e"></a>

## `xv::fs::Ext3FileSystem::DiskIo::release_reserve`

```doxygen
@fn void Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::release_reserve()
@brief 确认没有借出提交页后归还三个保留页。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-852ac8af17e88182"></a>

## `xv::fs::Ext3FileSystem::DiskIo::commit`

```doxygen
@fn bool Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::commit(xv::ext3::Volume &volume)
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-ad5de7c5d8f5fe10"></a>

## `xv::fs::Ext3FileSystem::DiskIo::allocate_page`

```doxygen
@fn void *Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::allocate_page()
@brief 优先申请普通页，仅提交阶段OOM时借用保留页。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-c973b4b2c86540f6"></a>

## `xv::fs::Ext3FileSystem::DiskIo::free_page`

```doxygen
@fn void Ext3FileSystem::DiskIo::xv::fs::Ext3FileSystem::DiskIo::free_page(void *page)
@brief 归还普通页或撤销提交储备借用，拒绝重复归还。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-c3525fae60442c17"></a>

## `xv::fs::Ext3FileSystem::Log::initialize`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::initialize()
@brief 实现 xv::fs::Ext3FileSystem::Log::initialize 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-d4a34f9d0fb9d34b"></a>

## `xv::fs::Ext3FileSystem::Log::ready`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::ready(unsigned capacity)
@brief 实现 xv::fs::Ext3FileSystem::Log::ready 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-b20e819813377e2a"></a>

## `xv::fs::Ext3FileSystem::Log::changed`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::changed(unsigned used)
@brief 实现 xv::fs::Ext3FileSystem::Log::changed 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-ad6465385c48e803"></a>

## `xv::fs::Ext3FileSystem::Log::wait`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::wait()
@brief 实现 xv::fs::Ext3FileSystem::Log::wait 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-fbaeebb59ac09b47"></a>

## `xv::fs::Ext3FileSystem::Log::begin`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::begin()
@brief 实现 xv::fs::Ext3FileSystem::Log::begin 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-b9dc7f1dbddb1193"></a>

## `xv::fs::Ext3FileSystem::Log::commit`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::commit(bool flush)
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-af9755f73c5675ef"></a>

## `xv::fs::Ext3FileSystem::Log::end`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::end()
@brief 实现 xv::fs::Ext3FileSystem::Log::end 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-e6cd0201678e66c2"></a>

## `xv::fs::Ext3FileSystem::Log::record`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::record(Buffer &)
@brief 实现 xv::fs::Ext3FileSystem::Log::record 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-01d3c4443aee01d9"></a>

## `xv::fs::Ext3FileSystem::Log::sync`

```doxygen
@fn void Ext3FileSystem::Log::xv::fs::Ext3FileSystem::Log::sync()
@brief 实现 xv::fs::Ext3FileSystem::Log::sync 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-95816528f8a4fe2f"></a>

## `xv::fs::Ext3FileSystem::initialize_inodes`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::initialize_inodes()
@brief 实现 xv::fs::Ext3FileSystem::initialize_inodes 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-702837280a873644"></a>

## `xv::fs::Ext3FileSystem::initialize`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::initialize(int device)
@brief 实现 xv::fs::Ext3FileSystem::initialize 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-267a4ca53aa4eb93"></a>

## `xv::fs::Ext3FileSystem::changed`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::changed()
@brief 实现 xv::fs::Ext3FileSystem::changed 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-e6fea83b0e152392"></a>

## `xv::fs::Ext3FileSystem::busy`

```doxygen
@fn bool Ext3FileSystem::xv::fs::Ext3FileSystem::busy()
@brief 返回 `references!=1`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-c9c4c79acedb3b57"></a>

## `xv::fs::Ext3FileSystem::shutdown`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::shutdown()
@brief 实现 xv::fs::Ext3FileSystem::shutdown 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-ab3267c1b6083c7a"></a>

## `xv::fs::Ext3FileSystem::get`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::get(unsigned number)
@brief 按inode号复用或取得受引用锁保护的缓存槽。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-2219bf0c921e510f"></a>

## `xv::fs::Ext3FileSystem::root`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::root()
@brief 返回 `mounted_ ? get(2):nullptr`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-708f2a85a201de71"></a>

## `xv::fs::Ext3FileSystem::retain`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::retain(Inode *node)
@brief 返回 `node`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-75e032d40eea8be3"></a>

## `xv::fs::Ext3FileSystem::refresh`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::refresh(Inode *node,const WireInode &wire)
@brief 把磁盘inode的类型、权限、引用数、大小和设备号刷新到VFS缓存。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-453ed9272bac3517"></a>

## `xv::fs::Ext3FileSystem::wire`

```doxygen
@fn bool Ext3FileSystem::xv::fs::Ext3FileSystem::wire(Inode *node,WireInode &result)
@brief 从当前暂存视图重新读取inode，避免覆盖并发元数据。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-2feda166195a1851"></a>

## `xv::fs::Ext3FileSystem::lock`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::lock(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::lock 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-1004bcff94a10317"></a>

## `xv::fs::Ext3FileSystem::unlock`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::unlock(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::unlock 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-e0544df51a26b59f"></a>

## `xv::fs::Ext3FileSystem::unlock_release`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::unlock_release(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::unlock_release 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-cfc86476f85b27a1"></a>

## `xv::fs::Ext3FileSystem::release`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::release(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::release 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-3ad05b607c427be7"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::head`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::head(Context &c) noexcept
@brief 流程动作或条件 head；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-fb41dd682d25dba3"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::okay`

```doxygen
@fn static bool xv::fs::Ext3FileSystem::CollectFlow::okay(Context &c) noexcept
@brief 流程动作或条件 okay；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-1224e6cdbb099381"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::more`

```doxygen
@fn static bool xv::fs::Ext3FileSystem::CollectFlow::more(Context &c) noexcept
@brief 流程动作或条件 more；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-408a055382f8bacf"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::inspect`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::inspect(Context &c) noexcept
@brief 流程动作或条件 inspect；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-1d9cd11535c18d17"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::pinned`

```doxygen
@fn static bool xv::fs::Ext3FileSystem::CollectFlow::pinned(Context &c) noexcept
@brief 流程动作或条件 pinned；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-eaf00a3ec6b8db9d"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::retain`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::retain(Context &c) noexcept
@brief 流程动作或条件 retain；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-912a9cdf4d31b176"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::reclaim`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::reclaim(Context &c) noexcept
@brief 在原子操作内释放零链接inode及其数据。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-9e10724ae6b631a6"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::commit`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::commit(Context &c) noexcept
@brief 提交暂存事务，提交记录落盘后再checkpoint并清理日志。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-0c9daf66512c7cdf"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::complete`

```doxygen
@fn static bool xv::fs::Ext3FileSystem::CollectFlow::complete(Context &c) noexcept
@brief 流程动作或条件 complete；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-a99d7f74628bece5"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::advance`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::advance(Context &c) noexcept
@brief 流程动作或条件 advance；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-2b537f2c952baf00"></a>

## `xv::fs::Ext3FileSystem::CollectFlow::finish`

```doxygen
@fn static void xv::fs::Ext3FileSystem::CollectFlow::finish(Context &c) noexcept
@brief 流程动作或条件 finish；作用于当前Context，转移关系由所属图声明。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-5188eb545030b86a"></a>

## `xv::fs::Ext3FileSystem::collect_orphans`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::collect_orphans()
@brief 通过CollectFlow遍历持久orphan链，跳过有引用节点，逐批回收与提交。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-af18914c0e2b5dc9"></a>

## `xv::fs::Ext3FileSystem::component`

```doxygen
@fn bool Ext3FileSystem::xv::fs::Ext3FileSystem::component(const char *source,char (&name)[256])
@brief 复制至多255字节的组件并终止，超长则失败而不截断别名。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-51333fb3d846df32"></a>

## `xv::fs::Ext3FileSystem::lookup_entry`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::lookup_entry(Inode *directory,const char *source,unsigned *offset)
@brief 实现 xv::fs::Ext3FileSystem::lookup_entry 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-a4f52a1cf4cc44de"></a>

## `xv::fs::Ext3FileSystem::create_at`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::create_at(Inode *directory,const char *source,short type,short major,short minor)
@brief 在目录权限和inode锁保护下原子创建名字，成功交付带锁引用；设备设置失败执行回滚。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-d64aba2c0950e86c"></a>

## `xv::fs::Ext3FileSystem::link_at`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::link_at(Inode *directory,const char *source,Inode *target)
@brief 检查同后端与目录写/搜索权限后增加硬链接。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-047edaa14cbf21cd"></a>

## `xv::fs::Ext3FileSystem::unlink_at`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::unlink_at(Inode *directory,const char *source)
@brief 检查目录和sticky权限后删除名字，标记持久orphan待回收。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-0d761799def280d5"></a>

## `xv::fs::Ext3FileSystem::rename_at`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::rename_at(Inode *from,const char *old_source,Inode *to,const char *new_source)
@brief 按inode号排序锁住父目录，检查sticky权限后原子重命名，覆盖目标进入orphan回收。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-e2c3bb50fcd4a9b9"></a>

## `xv::fs::Ext3FileSystem::allocate`

```doxygen
@fn Inode *Ext3FileSystem::xv::fs::Ext3FileSystem::allocate(unsigned,short)
@brief 返回 `nullptr`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-1d590cd4ac89473d"></a>

## `xv::fs::Ext3FileSystem::update`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::update(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::update 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-10c6216358bba2cc"></a>

## `xv::fs::Ext3FileSystem::resize`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::resize(Inode *node,unsigned size)
@brief 原子改变文件大小，失败返回-1并保留修改前状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-83b644b03b0202bf"></a>

## `xv::fs::Ext3FileSystem::truncate`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::truncate(Inode *node)
@brief 实现 xv::fs::Ext3FileSystem::truncate 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-126a617dc2cdd982"></a>

## `xv::fs::Ext3FileSystem::next_directory`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::next_directory(Inode *node,unsigned long &cookie,DirectoryRecord &entry)
@brief 读取一条完整目录记录并推进后端cookie，EOF返回0，错误返回-1。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-b46bf5ed64088515"></a>

## `xv::fs::Ext3FileSystem::read`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::read(Inode *node,int user,unsigned long destination,unsigned offset,unsigned count)
@brief 实现 xv::fs::Ext3FileSystem::read 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-2c26647454530b5b"></a>

## `xv::fs::Ext3FileSystem::write`

```doxygen
@fn int Ext3FileSystem::xv::fs::Ext3FileSystem::write(Inode *node,int user,unsigned long source,unsigned offset,unsigned count)
@brief 实现 xv::fs::Ext3FileSystem::write 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-b5c4b6b2761705bc"></a>

## `xv::fs::Ext3FileSystem::stat`

```doxygen
@fn void Ext3FileSystem::xv::fs::Ext3FileSystem::stat(Inode *node,Stat *result)
@brief 实现 xv::fs::Ext3FileSystem::stat 的模块操作，按声明参数读写其封装状态。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-91abfb4955cbd6e2"></a>

## `xv::fs::Ext3FileSystem::open_block_device`

```doxygen
@fn xv::drivers::BlockDevice *Ext3FileSystem::xv::fs::Ext3FileSystem::open_block_device(Inode *node,bool writable)
@brief 返回 `open_registered_block(node,writable)`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```

<a id="fn-c6fb5ecf0f85b05e"></a>

## `xv::fs::Ext3FileSystem::open_device`

```doxygen
@fn xv::drivers::CharacterDevice *Ext3FileSystem::xv::fs::Ext3FileSystem::open_device(Inode *node,bool nonblocking)
@brief 返回 `open_registered_device(node,nonblocking)`。
@details ext3 的内核 FileSystem 适配器。inode锁先于core睡眠锁；自旋锁内不做磁盘I/O。事务最后参与者提交，orphan 回收跳过仍有引用的节点，OOM 可延迟重试。详见 docs/ext3.md。
```
