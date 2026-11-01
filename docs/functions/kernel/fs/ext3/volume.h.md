# `kernel/fs/ext3/volume.h`

<a id="fn-8bb9790d15071e4c"></a>

## `xv::ext3::Io::read`

```doxygen
@fn virtual bool xv::ext3::Io::read(u64 offset,void *data,unsigned count)=0
@brief 实现 xv::ext3::Io::read 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-914410c43cfffbf9"></a>

## `xv::ext3::Io::write`

```doxygen
@fn virtual bool xv::ext3::Io::write(u64 offset,const void *data,unsigned count)=0
@brief 实现 xv::ext3::Io::write 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-455f328227a4b9c9"></a>

## `xv::ext3::Io::flush`

```doxygen
@fn virtual bool xv::ext3::Io::flush()=0
@brief 实现 xv::ext3::Io::flush 的模块操作，按声明参数读写其封装状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-a1ac50f2bd70d81b"></a>

## `xv::ext3::Io::size_bytes`

```doxygen
@fn virtual u64 xv::ext3::Io::size_bytes() const=0
@brief 按真实扇区容量计算字节数，超出缓存块号范围则拒绝。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1a7476dff824e727"></a>

## `xv::ext3::Io::now_seconds`

```doxygen
@fn virtual u32 xv::ext3::Io::now_seconds() const=0
@brief 读取实际RTC秒数作为inode时间。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e7b7a2d67e99d100"></a>

## `xv::ext3::Io::allocate_page`

```doxygen
@fn virtual void *xv::ext3::Io::allocate_page()=0
@brief 优先申请普通页，仅提交阶段OOM时借用保留页。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-9c1f7297568b55e1"></a>

## `xv::ext3::Io::free_page`

```doxygen
@fn virtual void xv::ext3::Io::free_page(void *page)=0
@brief 归还普通页或撤销提交储备借用，拒绝重复归还。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-902f0b4a6eb1cae7"></a>

## `xv::ext3::Io::~Io`

```doxygen
@fn xv::ext3::Io::~Io()=default
@brief 释放该对象持有的资源；借用对象仍归原所有者。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-01c4cdd0fe48458c"></a>

## `xv::ext3::Inode::mode`

```doxygen
@fn u16 xv::ext3::Inode::mode() const
@brief 返回 `le16(raw)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-8448f96afc37ea22"></a>

## `xv::ext3::Inode::uid`

```doxygen
@fn u32 xv::ext3::Inode::uid() const
@brief 返回 `le16(raw+2)|(u32(le16(raw+120))<<16)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-fbd2b5684465f4f6"></a>

## `xv::ext3::Inode::gid`

```doxygen
@fn u32 xv::ext3::Inode::gid() const
@brief 返回 `le16(raw+24)|(u32(le16(raw+122))<<16)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-2d32705098338acf"></a>

## `xv::ext3::Inode::links`

```doxygen
@fn u16 xv::ext3::Inode::links() const
@brief 返回 `le16(raw+26)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-1c7d36244fe8d896"></a>

## `xv::ext3::Inode::sectors`

```doxygen
@fn u32 xv::ext3::Inode::sectors() const
@brief 返回 `le32(raw+28)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-d2053a7f63d9892c"></a>

## `xv::ext3::Inode::flags`

```doxygen
@fn u32 xv::ext3::Inode::flags() const
@brief 返回 `le32(raw+32)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b68106a3ff028125"></a>

## `xv::ext3::Inode::pointer`

```doxygen
@fn u32 xv::ext3::Inode::pointer(unsigned index) const
@brief 读取间接块中的一个指针并验证目标。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-e05fd5afc837cbbb"></a>

## `xv::ext3::Inode::size`

```doxygen
@fn u64 xv::ext3::Inode::size() const
@brief 返回 `le32(raw+4)|((mode()&0xf000)==Regular ? u64(le32(raw+108))<<32:0)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-c714dcfc89b3dc88"></a>

## `xv::ext3::Inode::inline_link`

```doxygen
@fn bool xv::ext3::Inode::inline_link() const
@brief 返回 `(mode()&0xf000)==Symlink && !sectors() && size()<60`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-2342c5526e7c9d2b"></a>

## `xv::ext3::Inode::device_major`

```doxygen
@fn u32 xv::ext3::Inode::device_major() const
@brief 返回 `pointer(0) ? (pointer(0)>>8)&255:(pointer(1)>>8)&4095`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-a0207e8cda074962"></a>

## `xv::ext3::Inode::device_minor`

```doxygen
@fn u32 xv::ext3::Inode::device_minor() const
@brief 返回 `pointer(0) ? pointer(0)&255:(pointer(1)&255)|((pointer(1)>>12)&0xfff00)`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-d90b7940e7838eb6"></a>

## `xv::ext3::Volume::Volume`

```doxygen
@fn constexpr explicit xv::ext3::Volume::Volume(Io &io):io_(io),journal_io_(*this),journal_(journal_io_)
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-4eb2e7a03e593560"></a>

## `xv::ext3::Volume::Volume`

```doxygen
@fn xv::ext3::Volume::Volume(const Volume&)=delete
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-3d93f18687d63122"></a>

## `xv::ext3::Volume::operator=`

```doxygen
@fn Volume &xv::ext3::Volume::operator=(const Volume&)=delete
@brief 规定对象赋值/复制约束，防止资源所有权被隐式复制。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-b11d32e4a4a39fb6"></a>

## `xv::ext3::Volume::geometry`

```doxygen
@fn const Geometry &xv::ext3::Volume::geometry() const
@brief 返回 `geometry_`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-135bad6216f7b299"></a>

## `xv::ext3::Volume::error`

```doxygen
@fn Error xv::ext3::Volume::error() const
@brief 查询最近一次操作错误。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-d4d529803fd95596"></a>

## `xv::ext3::Volume::journal`

```doxygen
@fn Jbd &xv::ext3::Volume::journal()
@brief 返回 `journal_`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-feb870fa1d765894"></a>

## `xv::ext3::Volume::JournalDevice::JournalDevice`

```doxygen
@fn constexpr explicit xv::ext3::Volume::JournalDevice::JournalDevice(Volume &volume):volume_(volume)
@brief 构造对象并建立成员初值及借用关系。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-5dcdde3f7ae1c365"></a>

## `xv::ext3::Volume::JournalDevice::block_size`

```doxygen
@fn unsigned xv::ext3::Volume::JournalDevice::block_size() const override
@brief 返回 `volume_.geometry_.block_size`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-fd1fb602929942c6"></a>

## `xv::ext3::Volume::JournalDevice::journal_capacity`

```doxygen
@fn u32 xv::ext3::Volume::JournalDevice::journal_capacity() const override
@brief 返回 `volume_.journal_blocks_`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-559cc95886310ad0"></a>

## `xv::ext3::Volume::JournalDevice::flush`

```doxygen
@fn bool xv::ext3::Volume::JournalDevice::flush() override
@brief 返回 `volume_.io_.flush()`。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-63a41c7300c42816"></a>

## `xv::ext3::Volume::JournalDevice::recovery_flag`

```doxygen
@fn bool xv::ext3::Volume::JournalDevice::recovery_flag(bool enabled) override
@brief 读写文件系统需要恢复标志并保证所需持久化顺序。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-23c8859c502500d5"></a>

## `xv::ext3::Volume::JournalDevice::allocate_page`

```doxygen
@fn void *xv::ext3::Volume::JournalDevice::allocate_page() override
@brief 优先申请普通页，仅提交阶段OOM时借用保留页。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-9df7d0177f8a2813"></a>

## `xv::ext3::Volume::JournalDevice::free_page`

```doxygen
@fn void xv::ext3::Volume::JournalDevice::free_page(void *page) override
@brief 归还普通页或撤销提交储备借用，拒绝重复归还。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-c631d78e508d5fe9"></a>

## `xv::ext3::Volume::clear_error`

```doxygen
@fn void xv::ext3::Volume::clear_error()
@brief 清除上一操作的错误状态。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

<a id="fn-a438ee297af1e09d"></a>

## `xv::ext3::Volume::fail`

```doxygen
@fn bool xv::ext3::Volume::fail(Error error)
@brief 记录错误并返回失败。
@details ext3共享磁盘引擎。目录名最多255字节，支持稀疏及三级间接，操作savepoint失败回滚；不支持的特性拒绝挂载。底层Io由内核模块或宿主工具注入，未包含进程或VFS依赖。
```

- [`xv::ext3::Inode::device_major`](volume.h.md#fn-2342c5526e7c9d2b)

- [`xv::ext3::Inode::device_minor`](volume.h.md#fn-a0207e8cda074962)

- [`xv::ext3::Inode::flags`](volume.h.md#fn-d2053a7f63d9892c)

- [`xv::ext3::Inode::gid`](volume.h.md#fn-fbd2b5684465f4f6)

- [`xv::ext3::Inode::inline_link`](volume.h.md#fn-c714dcfc89b3dc88)

- [`xv::ext3::Inode::links`](volume.h.md#fn-2d32705098338acf)

- [`xv::ext3::Inode::mode`](volume.h.md#fn-01c4cdd0fe48458c)

- [`xv::ext3::Inode::pointer`](volume.h.md#fn-b68106a3ff028125)

- [`xv::ext3::Inode::sectors`](volume.h.md#fn-1c7d36244fe8d896)

- [`xv::ext3::Inode::size`](volume.h.md#fn-e05fd5afc837cbbb)

- [`xv::ext3::Inode::uid`](volume.h.md#fn-8448f96afc37ea22)

- [`xv::ext3::Io::allocate_page`](volume.h.md#fn-e7b7a2d67e99d100)

- [`xv::ext3::Io::flush`](volume.h.md#fn-455f328227a4b9c9)

- [`xv::ext3::Io::free_page`](volume.h.md#fn-9c1f7297568b55e1)

- [`xv::ext3::Io::now_seconds`](volume.h.md#fn-1a7476dff824e727)

- [`xv::ext3::Io::read`](volume.h.md#fn-8bb9790d15071e4c)

- [`xv::ext3::Io::size_bytes`](volume.h.md#fn-a1ac50f2bd70d81b)

- [`xv::ext3::Io::write`](volume.h.md#fn-914410c43cfffbf9)

- [`xv::ext3::Io::~Io`](volume.h.md#fn-902f0b4a6eb1cae7)

- [`xv::ext3::Volume::Volume`](volume.h.md#fn-d90b7940e7838eb6)

- [`xv::ext3::Volume::Volume`](volume.h.md#fn-4eb2e7a03e593560)

- [`xv::ext3::Volume::add_entry`](mutate.cpp.md#fn-bc48e4efd088311a)

- [`xv::ext3::Volume::add_orphan`](mutate.cpp.md#fn-46ca4f81d6f810b8)

- [`xv::ext3::Volume::allocate_block`](mutate.cpp.md#fn-cb22b8c21b6fe415)

- [`xv::ext3::Volume::allocate_inode`](mutate.cpp.md#fn-30e83e529ffbf388)

- [`xv::ext3::Volume::clear_error`](volume.h.md#fn-c631d78e508d5fe9)

- [`xv::ext3::Volume::create`](mutate.cpp.md#fn-175c1ffe8b63fddb)

- [`xv::ext3::Volume::edit`](mutate.cpp.md#fn-eb822b6975d16ed4)

- [`xv::ext3::Volume::edit_group`](mutate.cpp.md#fn-fb9f263383dac503)

- [`xv::ext3::Volume::edit_super`](mutate.cpp.md#fn-ceea3eea813dbe1c)

- [`xv::ext3::Volume::empty_directory`](mutate.cpp.md#fn-049a56c76e7cf720)

- [`xv::ext3::Volume::error`](volume.h.md#fn-135bad6216f7b299)

- [`xv::ext3::Volume::fail`](volume.h.md#fn-a438ee297af1e09d)

- [`xv::ext3::Volume::find_entry`](mutate.cpp.md#fn-b134760f1d63dea6)

- [`xv::ext3::Volume::free_block`](mutate.cpp.md#fn-e86dfd837a697d92)

- [`xv::ext3::Volume::free_inode`](mutate.cpp.md#fn-d8b13bcc2b215c13)

- [`xv::ext3::Volume::free_tree`](mutate.cpp.md#fn-cd51fe0f0ab2c850)

- [`xv::ext3::Volume::geometry`](volume.h.md#fn-b11d32e4a4a39fb6)

- [`xv::ext3::Volume::group`](volume.cpp.md#fn-26a322198cc7e318)

- [`xv::ext3::Volume::inode_location`](volume.cpp.md#fn-3b67f9d94d7f003f)

- [`xv::ext3::Volume::journal`](volume.h.md#fn-d4d529803fd95596)

- [`xv::ext3::Volume::journal_block`](volume.cpp.md#fn-3a66a7d2df1e0f9d)

- [`xv::ext3::Volume::journal_mapping`](volume.cpp.md#fn-f2c4033416c64d19)

- [`xv::ext3::Volume::link`](mutate.cpp.md#fn-53fe6c028faa48d9)

- [`xv::ext3::Volume::lookup`](volume.cpp.md#fn-c41c4c139c472207)

- [`xv::ext3::Volume::map_allocated`](mutate.cpp.md#fn-fd41b1cd1447632e)

- [`xv::ext3::Volume::map_block`](volume.cpp.md#fn-29f09b6e7aee4234)

- [`xv::ext3::Volume::metadata_block`](mutate.cpp.md#fn-62e9585751b7a8a9)

- [`xv::ext3::Volume::mount`](volume.cpp.md#fn-86a45bd16f7ac115)

- [`xv::ext3::Volume::next_entry`](volume.cpp.md#fn-5632ad9397bf2f0b)

- [`xv::ext3::Volume::operator=`](volume.h.md#fn-3d93f18687d63122)

- [`xv::ext3::Volume::orphan_head`](mutate.cpp.md#fn-8fed6746c7aeaf2e)

- [`xv::ext3::Volume::orphan_step`](mutate.cpp.md#fn-4a8280b07b5116c6)

- [`xv::ext3::Volume::pointer`](volume.cpp.md#fn-ffc2d006e838fcc7)

- [`xv::ext3::Volume::prune`](mutate.cpp.md#fn-d8cf153cf1fb61fd)

- [`xv::ext3::Volume::put_inode`](mutate.cpp.md#fn-649b328bbf101e80)

- [`xv::ext3::Volume::read`](volume.cpp.md#fn-5dcfa6d563ecc56d)

- [`xv::ext3::Volume::read_block`](volume.cpp.md#fn-8853974a145c2ce7)

- [`xv::ext3::Volume::read_inode`](volume.cpp.md#fn-84723da571b15285)

- [`xv::ext3::Volume::reclaim`](mutate.cpp.md#fn-e5be415f5ea8ce38)

- [`xv::ext3::Volume::reclaim_step`](mutate.cpp.md#fn-053d87f5c5eb7695)

- [`xv::ext3::Volume::recover_orphans`](mutate.cpp.md#fn-6274bf8645e2f832)

- [`xv::ext3::Volume::remove_entry`](mutate.cpp.md#fn-07c6fa6a5f2bee78)

- [`xv::ext3::Volume::remove_orphan`](mutate.cpp.md#fn-bfacd58eb3a7ecbe)

- [`xv::ext3::Volume::rename`](mutate.cpp.md#fn-f1a1fe0929a9f193)

- [`xv::ext3::Volume::reserve_block`](volume.cpp.md#fn-51b38431be659e50)

- [`xv::ext3::Volume::reserved_block`](volume.cpp.md#fn-dd46e9ed5c2383f3)

- [`xv::ext3::Volume::reset`](volume.cpp.md#fn-4f13e60f4c2125a4)

- [`xv::ext3::Volume::resize`](mutate.cpp.md#fn-30efaad9cbb4a6c7)

- [`xv::ext3::Volume::resize_raw`](mutate.cpp.md#fn-1f8c10d6557602c8)

- [`xv::ext3::Volume::set_attributes`](mutate.cpp.md#fn-db375798f6bf7ab1)

- [`xv::ext3::Volume::set_device`](mutate.cpp.md#fn-b90b576e86e09ab5)

- [`xv::ext3::Volume::set_recovery`](volume.cpp.md#fn-089515a810f5593c)

- [`xv::ext3::Volume::superblock`](volume.cpp.md#fn-59e35d3d5c14c2a3)

- [`xv::ext3::Volume::unlink`](mutate.cpp.md#fn-762ee7759d3f30fd)

- [`xv::ext3::Volume::write`](mutate.cpp.md#fn-da55a551708539bb)

- [`xv::ext3::Volume::JournalDevice::JournalDevice`](volume.h.md#fn-feb870fa1d765894)

- [`xv::ext3::Volume::JournalDevice::allocate_page`](volume.h.md#fn-23c8859c502500d5)

- [`xv::ext3::Volume::JournalDevice::block_size`](volume.h.md#fn-5dcdde3f7ae1c365)

- [`xv::ext3::Volume::JournalDevice::flush`](volume.h.md#fn-559cc95886310ad0)

- [`xv::ext3::Volume::JournalDevice::free_page`](volume.h.md#fn-9df7d0177f8a2813)

- [`xv::ext3::Volume::JournalDevice::journal_capacity`](volume.h.md#fn-fd1fb602929942c6)

- [`xv::ext3::Volume::JournalDevice::read_home`](volume.cpp.md#fn-561184945411f403)

- [`xv::ext3::Volume::JournalDevice::read_journal`](volume.cpp.md#fn-1e42f013d9856434)

- [`xv::ext3::Volume::JournalDevice::recovery_flag`](volume.h.md#fn-63a41c7300c42816)

- [`xv::ext3::Volume::JournalDevice::valid_target`](volume.cpp.md#fn-74b70e78d0ff709b)

- [`xv::ext3::Volume::JournalDevice::write_home`](volume.cpp.md#fn-0167e667e72f95aa)

- [`xv::ext3::Volume::JournalDevice::write_journal`](volume.cpp.md#fn-15096dab85f2a092)
