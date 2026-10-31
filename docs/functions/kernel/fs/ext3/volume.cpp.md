# `kernel/fs/ext3/volume.cpp`

<a id="fn-e16f68b2be2bdce3"></a>

## `xv::ext3::Page::Page`

```doxygen
@fn explicit xv::ext3::Page::Page(Io &io):io_(io),bytes_(static_cast<u8 *>(io.allocate_page()))
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-a79b9adde708ad3c"></a>

## `xv::ext3::Page::~Page`

```doxygen
@fn xv::ext3::Page::~Page()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e08d42013080121e"></a>

## `xv::ext3::Page::get`

```doxygen
@fn u8 *xv::ext3::Page::get() const
@brief 按inode号复用或取得受引用锁保护的缓存槽。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-59e35d3d5c14c2a3"></a>

## `xv::ext3::Volume::superblock`

```doxygen
@fn bool Volume::xv::ext3::Volume::superblock()
@brief 校验ext3超级块几何、特性与边界。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-8853974a145c2ce7"></a>

## `xv::ext3::Volume::read_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::read_block(u32 block,void *data,bool raw)
@brief 读取指定文件系统块并检查边界。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-26a322198cc7e318"></a>

## `xv::ext3::Volume::group`

```doxygen
@fn bool Volume::xv::ext3::Volume::group(u32 number,u32 &block_bitmap,u32 &inode_bitmap,u32 &inode_table)
@brief 读取并校验块组描述符。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-3b67f9d94d7f003f"></a>

## `xv::ext3::Volume::inode_location`

```doxygen
@fn bool Volume::xv::ext3::Volume::inode_location(u32 number,u32 &block,unsigned &offset)
@brief 计算并校验inode所在组、表块和字节偏移。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-84723da571b15285"></a>

## `xv::ext3::Volume::read_inode`

```doxygen
@fn bool Volume::xv::ext3::Volume::read_inode(u32 number,Inode &result)
@brief 校验inode号与位置，从暂存视图读取磁盘inode。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-ffc2d006e838fcc7"></a>

## `xv::ext3::Volume::pointer`

```doxygen
@fn bool Volume::xv::ext3::Volume::pointer(u32 block,unsigned index,u32 &result,bool raw)
@brief 读取间接块中的一个指针并验证目标。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-29f09b6e7aee4234"></a>

## `xv::ext3::Volume::map_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::map_block(const Inode &inode,u64 logical,u32 &physical,bool raw)
@brief 按直接、一级、二级、三级间接关系定位逻辑块，保留稀疏洞。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-51b38431be659e50"></a>

## `xv::ext3::Volume::reserve_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::reserve_block(u32 block)
@brief 将元数据块登记为不可分配。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-dd46e9ed5c2383f3"></a>

## `xv::ext3::Volume::reserved_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::reserved_block(u32 block) const
@brief 查询块是否属于已登记元数据保留区。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-f2c4033416c64d19"></a>

## `xv::ext3::Volume::journal_mapping`

```doxygen
@fn bool Volume::xv::ext3::Volume::journal_mapping()
@brief 检查日志inode映射与元数据保留区关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-3a66a7d2df1e0f9d"></a>

## `xv::ext3::Volume::journal_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::journal_block(u32 logical,u32 &physical) const
@brief 将内部日志位置转换为已验证物理块。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1e42f013d9856434"></a>

## `xv::ext3::Volume::JournalDevice::read_journal`

```doxygen
@fn bool Volume::JournalDevice::xv::ext3::Volume::JournalDevice::read_journal(u32 logical,void *data)
@brief 实现 xv::ext3::Volume::JournalDevice::read_journal 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-15096dab85f2a092"></a>

## `xv::ext3::Volume::JournalDevice::write_journal`

```doxygen
@fn bool Volume::JournalDevice::xv::ext3::Volume::JournalDevice::write_journal(u32 logical,const void *data)
@brief 实现 xv::ext3::Volume::JournalDevice::write_journal 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-561184945411f403"></a>

## `xv::ext3::Volume::JournalDevice::read_home`

```doxygen
@fn bool Volume::JournalDevice::xv::ext3::Volume::JournalDevice::read_home(u32 block,void *data)
@brief 返回 `volume_.read_block(block,data,true)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-0167e667e72f95aa"></a>

## `xv::ext3::Volume::JournalDevice::write_home`

```doxygen
@fn bool Volume::JournalDevice::xv::ext3::Volume::JournalDevice::write_home(u32 block,const void *data)
@brief 返回 `valid_target(block) && volume_.io_.write(u64(block)*block_size(),data,block_size())`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-74b70e78d0ff709b"></a>

## `xv::ext3::Volume::JournalDevice::valid_target`

```doxygen
@fn bool Volume::JournalDevice::xv::ext3::Volume::JournalDevice::valid_target(u32 block) const
@brief 返回 `block<volume_.geometry_.blocks && !volume_.reserved_block(block)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-089515a810f5593c"></a>

## `xv::ext3::Volume::set_recovery`

```doxygen
@fn bool Volume::xv::ext3::Volume::set_recovery(bool enabled)
@brief 更新ext3恢复特性位。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-86a45bd16f7ac115"></a>

## `xv::ext3::Volume::mount`

```doxygen
@fn bool Volume::xv::ext3::Volume::mount()
@brief 实现 xv::ext3::Volume::mount 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-4f13e60f4c2125a4"></a>

## `xv::ext3::Volume::reset`

```doxygen
@fn void Volume::xv::ext3::Volume::reset()
@brief 释放持有的快照/暂存资源，最后引用才归还页面。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-5dcfa6d563ecc56d"></a>

## `xv::ext3::Volume::read`

```doxygen
@fn long Volume::xv::ext3::Volume::read(const Inode &inode,u64 offset,void *destination,unsigned count)
@brief 实现 xv::ext3::Volume::read 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-5632ad9397bf2f0b"></a>

## `xv::ext3::Volume::next_entry`

```doxygen
@fn bool Volume::xv::ext3::Volume::next_entry(const Inode &directory,u64 &offset,DirectoryEntry &entry)
@brief 实现 xv::ext3::Volume::next_entry 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-c41c4c139c472207"></a>

## `xv::ext3::Volume::lookup`

```doxygen
@fn bool Volume::xv::ext3::Volume::lookup(const Inode &directory,const char *name,Inode &result)
@brief 实现 xv::ext3::Volume::lookup 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```
