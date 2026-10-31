# `kernel/fs/ext3/mutate.cpp`

<a id="fn-2bc8f865a8d95715"></a>

## `xv::ext3::Page::Page`

```doxygen
@fn explicit xv::ext3::Page::Page(Io &io):io_(io),data_(static_cast<u8 *>(io.allocate_page()))
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-6c4903705eeebd6a"></a>

## `xv::ext3::Page::~Page`

```doxygen
@fn xv::ext3::Page::~Page()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-c5ee4d7a7371063e"></a>

## `xv::ext3::Page::get`

```doxygen
@fn u8 *xv::ext3::Page::get() const
@brief 按inode号复用或取得受引用锁保护的缓存槽。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b0fba8d1fd7c3caf"></a>

## `xv::ext3::set_size`

```doxygen
@fn void xv::ext3::set_size(Inode &inode,u64 size)
@brief 实现 xv::ext3::set_size 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-477b959b42260ba7"></a>

## `xv::ext3::set_pointer`

```doxygen
@fn void xv::ext3::set_pointer(Inode &inode,unsigned index,u32 block)
@brief 实现 xv::ext3::set_pointer 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b3d4a3896a1ea8d5"></a>

## `xv::ext3::touch`

```doxygen
@fn void xv::ext3::touch(Inode &inode,u32 now,bool data)
@brief 实现 xv::ext3::touch 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-864aaab99cf643bf"></a>

## `xv::ext3::directory_type`

```doxygen
@fn u8 xv::ext3::directory_type(u16 mode)
@brief 实现 xv::ext3::directory_type 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-ca2a978fdb3761b8"></a>

## `xv::ext3::power`

```doxygen
@fn bool xv::ext3::power(u32 n,u32 base)
@brief 返回 `n==1`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-81c5506b2dd2d2e6"></a>

## `xv::ext3::name_size`

```doxygen
@fn unsigned xv::ext3::name_size(const char *name)
@brief 实现 xv::ext3::name_size 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-f1a46248da43f7b0"></a>

## `xv::ext3::special_name`

```doxygen
@fn bool xv::ext3::special_name(const char *name)
@brief 返回 `name[0]=='.' && (!name[1] || (name[1]=='.' && !name[2]))`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e246c2943a0afd5f"></a>

## `xv::ext3::Volume::Operation::Operation`

```doxygen
@fn explicit xv::ext3::Volume::Operation::Operation(Volume &v):volume(v),previous(v.geometry_)
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-3fab5cb7903b7a8e"></a>

## `xv::ext3::Volume::Operation::~Operation`

```doxygen
@fn xv::ext3::Volume::Operation::~Operation()
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-be19bd35a9e02f12"></a>

## `xv::ext3::Volume::Operation::finish`

```doxygen
@fn bool xv::ext3::Volume::Operation::finish()
@brief 实现 xv::ext3::Volume::Operation::finish 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-eb822b6975d16ed4"></a>

## `xv::ext3::Volume::edit`

```doxygen
@fn u8 *Volume::xv::ext3::Volume::edit(u32 block)
@brief 取得可修改暂存块，首次或savepoint后写入按需分配并检查credit。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-ceea3eea813dbe1c"></a>

## `xv::ext3::Volume::edit_super`

```doxygen
@fn u8 *Volume::xv::ext3::Volume::edit_super()
@brief 返回 `p ? p+1024%geometry_.block_size:nullptr`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-fb9f263383dac503"></a>

## `xv::ext3::Volume::edit_group`

```doxygen
@fn u8 *Volume::xv::ext3::Volume::edit_group(u32 number)
@brief 返回 `p ? p+offset%geometry_.block_size:nullptr`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-649b328bbf101e80"></a>

## `xv::ext3::Volume::put_inode`

```doxygen
@fn bool Volume::xv::ext3::Volume::put_inode(const Inode &inode,bool fresh)
@brief 实现 xv::ext3::Volume::put_inode 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-62e9585751b7a8a9"></a>

## `xv::ext3::Volume::metadata_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::metadata_block(u32 block,u32 number,u32 bitmap,u32 inode_bitmap,u32 table) const
@brief 实现 xv::ext3::Volume::metadata_block 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-cb22b8c21b6fe415"></a>

## `xv::ext3::Volume::allocate_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::allocate_block(u32 preferred,u32 &result)
@brief 在组位图中分配数据块，同时更新空闲计数。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e86dfd837a697d92"></a>

## `xv::ext3::Volume::free_block`

```doxygen
@fn bool Volume::xv::ext3::Volume::free_block(u32 block)
@brief 清除块位图并更新组和超级块计数，拒绝释放保留元数据。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-30e83e529ffbf388"></a>

## `xv::ext3::Volume::allocate_inode`

```doxygen
@fn bool Volume::xv::ext3::Volume::allocate_inode(u16 mode,u32 uid,u32 gid,u32 preferred,Inode &inode)
@brief 在inode位图分配节点并同步组计数。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-fd41b1cd1447632e"></a>

## `xv::ext3::Volume::map_allocated`

```doxygen
@fn bool Volume::xv::ext3::Volume::map_allocated(Inode &inode,u64 logical,u32 &physical)
@brief 为逻辑块补齐间接指针与数据块，受操作savepoint保护。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-da55a551708539bb"></a>

## `xv::ext3::Volume::write`

```doxygen
@fn long Volume::xv::ext3::Volume::write(Inode &inode,u64 offset,const void *source,unsigned count)
@brief 实现 xv::ext3::Volume::write 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-cd51fe0f0ab2c850"></a>

## `xv::ext3::Volume::free_tree`

```doxygen
@fn bool Volume::xv::ext3::Volume::free_tree(Inode &inode,u32 block,unsigned level,u64 base,u64 keep,u32 &replacement,unsigned &budget,bool &complete)
@brief 释放最多三级的间接子树，不把即将释放的指针块写入日志。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-d8cf153cf1fb61fd"></a>

## `xv::ext3::Volume::prune`

```doxygen
@fn bool Volume::xv::ext3::Volume::prune(Inode &inode,u64 keep,unsigned &budget,bool &complete)
@brief 修剪截断边界后的间接树，同时保留仍有效的前缀。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1f8c10d6557602c8"></a>

## `xv::ext3::Volume::resize_raw`

```doxygen
@fn bool Volume::xv::ext3::Volume::resize_raw(Inode &inode,u64 size)
@brief 在操作savepoint内调整数据树、大小与零填充尾部。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-30efaad9cbb4a6c7"></a>

## `xv::ext3::Volume::resize`

```doxygen
@fn bool Volume::xv::ext3::Volume::resize(Inode &inode,u64 size)
@brief 原子改变文件大小，失败返回-1并保留修改前状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-db375798f6bf7ab1"></a>

## `xv::ext3::Volume::set_attributes`

```doxygen
@fn bool Volume::xv::ext3::Volume::set_attributes(Inode &inode,u32 uid,u32 gid,u16 permissions)
@brief 在savepoint内更新属主、组及权限并刷新ctime。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b90b576e86e09ab5"></a>

## `xv::ext3::Volume::set_device`

```doxygen
@fn bool Volume::xv::ext3::Volume::set_device(Inode &inode,u32 major,u32 minor)
@brief 检查设备号范围后写入ext3设备编码。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b134760f1d63dea6"></a>

## `xv::ext3::Volume::find_entry`

```doxygen
@fn bool Volume::xv::ext3::Volume::find_entry(const Inode &directory,const char *name,u64 &offset,u32 &number)
@brief 逐条校验可变长目录记录并定位指定名字。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-bc48e4efd088311a"></a>

## `xv::ext3::Volume::add_entry`

```doxygen
@fn bool Volume::xv::ext3::Volume::add_entry(Inode &directory,const char *name,u32 number,u16 mode)
@brief 在现有记录间隙或新目录块中插入完整名字。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-07c6fa6a5f2bee78"></a>

## `xv::ext3::Volume::remove_entry`

```doxygen
@fn bool Volume::xv::ext3::Volume::remove_entry(Inode &directory,const char *name,u32 &number)
@brief 移除目录名字并合并可复用记录空间。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-049a56c76e7cf720"></a>

## `xv::ext3::Volume::empty_directory`

```doxygen
@fn bool Volume::xv::ext3::Volume::empty_directory(const Inode &directory)
@brief 除点和点点外检查目录是否为空。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-8fed6746c7aeaf2e"></a>

## `xv::ext3::Volume::orphan_head`

```doxygen
@fn bool Volume::xv::ext3::Volume::orphan_head(u32 &number)
@brief 读取并检查超级块中的持久孤儿链头。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-46ca4f81d6f810b8"></a>

## `xv::ext3::Volume::add_orphan`

```doxygen
@fn bool Volume::xv::ext3::Volume::add_orphan(Inode &inode)
@brief 将零链接inode登记到持久orphan链。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-bfacd58eb3a7ecbe"></a>

## `xv::ext3::Volume::remove_orphan`

```doxygen
@fn bool Volume::xv::ext3::Volume::remove_orphan(Inode &inode)
@brief 在持久链中摘除已回收inode并更新前驱。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-d8b13bcc2b215c13"></a>

## `xv::ext3::Volume::free_inode`

```doxygen
@fn bool Volume::xv::ext3::Volume::free_inode(Inode &inode)
@brief 释放inode位图并更新空闲inode及目录计数。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-175c1ffe8b63fddb"></a>

## `xv::ext3::Volume::create`

```doxygen
@fn bool Volume::xv::ext3::Volume::create(u32 parent,const char *name,u16 mode,u32 uid,u32 gid,Inode &created)
@brief 实现 xv::ext3::Volume::create 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-53fe6c028faa48d9"></a>

## `xv::ext3::Volume::link`

```doxygen
@fn bool Volume::xv::ext3::Volume::link(u32 parent,const char *name,u32 target)
@brief 实现 xv::ext3::Volume::link 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-762ee7759d3f30fd"></a>

## `xv::ext3::Volume::unlink`

```doxygen
@fn bool Volume::xv::ext3::Volume::unlink(u32 parent,const char *name,u32 &removed)
@brief 实现 xv::ext3::Volume::unlink 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-f1a1fe0929a9f193"></a>

## `xv::ext3::Volume::rename`

```doxygen
@fn bool Volume::xv::ext3::Volume::rename(u32 from,const char *old_name,u32 to,const char *new_name,u32 &replaced)
@brief 实现 xv::ext3::Volume::rename 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e5be415f5ea8ce38"></a>

## `xv::ext3::Volume::reclaim`

```doxygen
@fn bool Volume::xv::ext3::Volume::reclaim(u32 number)
@brief 在原子操作内释放零链接inode及其数据。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-4a8280b07b5116c6"></a>

## `xv::ext3::Volume::orphan_step`

```doxygen
@fn bool Volume::xv::ext3::Volume::orphan_step(Inode &inode,bool &complete)
@brief 在给定回收额度内释放间接树的一部分。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-053d87f5c5eb7695"></a>

## `xv::ext3::Volume::reclaim_step`

```doxygen
@fn bool Volume::xv::ext3::Volume::reclaim_step(u32 number,bool &complete)
@brief 每次最多释放四个物理块，事务内更新指针与位图；完成后摘除orphan并释放inode。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-6274bf8645e2f832"></a>

## `xv::ext3::Volume::recover_orphans`

```doxygen
@fn bool Volume::xv::ext3::Volume::recover_orphans()
@brief 挂载时按有界批次恢复孤儿回收，每批独立提交。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```
