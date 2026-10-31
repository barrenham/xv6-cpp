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
