# `kernel/fs/inode/disk_filesystem.cppm`

<a id="fn-13dd63566efb0eec"></a>

## `xv::fs::DiskFileSystem::DiskFileSystem`

```doxygen
@fn constexpr xv::fs::DiskFileSystem::DiskFileSystem(FileSystem &legacy,FileSystem &ext3,BlockCache &cache) :legacy_(legacy),ext3_(ext3),cache_(cache),selected_(&legacy)
@brief 磁盘格式路由：DiskFileSystem 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-507a905aead5d92e"></a>

## `xv::fs::DiskFileSystem::name`

```doxygen
@fn const char *xv::fs::DiskFileSystem::name() const override
@brief 磁盘格式路由：name 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-151077a6cbad7e21"></a>

## `xv::fs::DiskFileSystem::type_name`

```doxygen
@fn const char *xv::fs::DiskFileSystem::type_name() const override
@brief 磁盘格式路由：type_name 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-4179a1eb483ab615"></a>

## `xv::fs::DiskFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::DiskFileSystem::initialize_inodes() override
@brief 磁盘格式路由：initialize_inodes 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-c5784a0c776c3d1d"></a>

## `xv::fs::DiskFileSystem::busy`

```doxygen
@fn bool xv::fs::DiskFileSystem::busy() override
@brief 磁盘格式路由：busy 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-f257be5848e1f8b0"></a>

## `xv::fs::DiskFileSystem::shutdown`

```doxygen
@fn void xv::fs::DiskFileSystem::shutdown() override
@brief 磁盘格式路由：shutdown 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-b57a2302745f0331"></a>

## `xv::fs::DiskFileSystem::root`

```doxygen
@fn Inode *xv::fs::DiskFileSystem::root() override
@brief 磁盘格式路由：root 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-af16af5162689d8d"></a>

## `xv::fs::DiskFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::DiskFileSystem::lookup_entry(Inode *d,const char *n,unsigned *o) override
@brief 磁盘格式路由：lookup_entry 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-fddc8db9188743d6"></a>

## `xv::fs::DiskFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::DiskFileSystem::create_at(Inode *d,const char *n,short t,short a,short b) override
@brief 磁盘格式路由：create_at 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-f5a902a649a5ef11"></a>

## `xv::fs::DiskFileSystem::link_at`

```doxygen
@fn int xv::fs::DiskFileSystem::link_at(Inode *d,const char *n,Inode *t) override
@brief 磁盘格式路由：link_at 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-55f4910026c31999"></a>

## `xv::fs::DiskFileSystem::unlink_at`

```doxygen
@fn int xv::fs::DiskFileSystem::unlink_at(Inode *d,const char *n) override
@brief 磁盘格式路由：unlink_at 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-3aa7e804e57277f8"></a>

## `xv::fs::DiskFileSystem::rename_at`

```doxygen
@fn int xv::fs::DiskFileSystem::rename_at(Inode *a,const char *n,Inode *b,const char *m) override
@brief 磁盘格式路由：rename_at 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-cbc39a9de8c578f8"></a>

## `xv::fs::DiskFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::DiskFileSystem::allocate(unsigned d,short t) override
@brief 磁盘格式路由：allocate 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-7364ee959e448480"></a>

## `xv::fs::DiskFileSystem::retain`

```doxygen
@fn Inode *xv::fs::DiskFileSystem::retain(Inode *n) override
@brief 磁盘格式路由：retain 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-d0b7382a10a3783d"></a>

## `xv::fs::DiskFileSystem::release`

```doxygen
@fn void xv::fs::DiskFileSystem::release(Inode *n) override
@brief 磁盘格式路由：release 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-3bb0592df199b4b6"></a>

## `xv::fs::DiskFileSystem::lock`

```doxygen
@fn void xv::fs::DiskFileSystem::lock(Inode *n) override
@brief 磁盘格式路由：lock 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-b2ba4713191e6369"></a>

## `xv::fs::DiskFileSystem::unlock`

```doxygen
@fn void xv::fs::DiskFileSystem::unlock(Inode *n) override
@brief 磁盘格式路由：unlock 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-5cc837448f5d11e8"></a>

## `xv::fs::DiskFileSystem::unlock_release`

```doxygen
@fn void xv::fs::DiskFileSystem::unlock_release(Inode *n) override
@brief 磁盘格式路由：unlock_release 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-12448c3daff8d33d"></a>

## `xv::fs::DiskFileSystem::update`

```doxygen
@fn void xv::fs::DiskFileSystem::update(Inode *n) override
@brief 磁盘格式路由：update 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-19f5e0196d7f8e30"></a>

## `xv::fs::DiskFileSystem::next_directory`

```doxygen
@fn int xv::fs::DiskFileSystem::next_directory(Inode *n,unsigned long &cookie,DirectoryRecord &entry) override
@brief 磁盘格式路由：next_directory 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-a2c2f42548bf2294"></a>

## `xv::fs::DiskFileSystem::name_max`

```doxygen
@fn unsigned xv::fs::DiskFileSystem::name_max() const override
@brief 磁盘格式路由：name_max 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-4a76d72f85e318f5"></a>

## `xv::fs::DiskFileSystem::truncate`

```doxygen
@fn void xv::fs::DiskFileSystem::truncate(Inode *n) override
@brief 磁盘格式路由：truncate 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-20568617c60e8c9a"></a>

## `xv::fs::DiskFileSystem::resize`

```doxygen
@fn int xv::fs::DiskFileSystem::resize(Inode *n,unsigned s) override
@brief 磁盘格式路由：resize 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-f7d668e61a50c2b6"></a>

## `xv::fs::DiskFileSystem::flush`

```doxygen
@fn void xv::fs::DiskFileSystem::flush() override
@brief 磁盘格式路由：flush 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-fead50c6a5eecbd1"></a>

## `xv::fs::DiskFileSystem::transaction_log`

```doxygen
@fn Journal *xv::fs::DiskFileSystem::transaction_log() override
@brief 磁盘格式路由：transaction_log 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-dbf7c3fba94f9ae1"></a>

## `xv::fs::DiskFileSystem::read`

```doxygen
@fn int xv::fs::DiskFileSystem::read(Inode *n,int u,unsigned long p,unsigned o,unsigned c) override
@brief 磁盘格式路由：read 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-3013e9fe4615eb37"></a>

## `xv::fs::DiskFileSystem::write`

```doxygen
@fn int xv::fs::DiskFileSystem::write(Inode *n,int u,unsigned long p,unsigned o,unsigned c) override
@brief 磁盘格式路由：write 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-10ae192b0a002230"></a>

## `xv::fs::DiskFileSystem::stat`

```doxygen
@fn void xv::fs::DiskFileSystem::stat(Inode *n,Stat *s) override
@brief 磁盘格式路由：stat 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

<a id="fn-1566ee10aabe56b1"></a>

## `xv::fs::DiskFileSystem::open_device`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::DiskFileSystem::open_device(Inode *n,bool b) override
@brief 磁盘格式路由：open_device 委托已选择的后端，inode所有权保留在具体后端。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```
