# `kernel/fs/vfs/vfs.cpp`

## 函数导航

- [`xv::fs::VirtualFileSystem::allocate`](vfs.cpp.md#fn-b14436f73059aab1)
- [`xv::fs::VirtualFileSystem::allowed`](vfs.cpp.md#fn-abf62fef3e7f21c5)
- [`xv::fs::VirtualFileSystem::ancestor`](vfs.cpp.md#fn-35b6020510aa9401)
- [`xv::fs::VirtualFileSystem::create`](vfs.cpp.md#fn-6591de3da7091bf9)
- [`xv::fs::VirtualFileSystem::create_at`](vfs.cpp.md#fn-6139d98544fd4958)
- [`xv::fs::VirtualFileSystem::enter_mount`](vfs.cpp.md#fn-4860e29021bbf700)
- [`xv::fs::VirtualFileSystem::initialize`](vfs.cpp.md#fn-e6a15b643dd8c735)
- [`xv::fs::VirtualFileSystem::initialize_inodes`](vfs.cpp.md#fn-3975fe035d893dce)
- [`xv::fs::VirtualFileSystem::link`](vfs.cpp.md#fn-365a4e7acb8561db)
- [`xv::fs::VirtualFileSystem::link_at`](vfs.cpp.md#fn-71de116d242b4ba0)
- [`xv::fs::VirtualFileSystem::lock`](vfs.cpp.md#fn-5967b2c0b5f42168)
- [`xv::fs::VirtualFileSystem::lookup`](vfs.cpp.md#fn-f4cf807217fcb7f0)
- [`xv::fs::VirtualFileSystem::lookup_entry`](vfs.cpp.md#fn-5d4339ff52e6c374)
- [`xv::fs::VirtualFileSystem::mount`](vfs.cpp.md#fn-fd77401a8aa09874)
- [`xv::fs::VirtualFileSystem::mount_ram`](vfs.cpp.md#fn-09eb092ae4d24474)
- [`xv::fs::VirtualFileSystem::read`](vfs.cpp.md#fn-a35df3a283d2bfaf)
- [`xv::fs::VirtualFileSystem::release`](vfs.cpp.md#fn-e5559898f89824ab)
- [`xv::fs::VirtualFileSystem::rename`](vfs.cpp.md#fn-3ce7ce542ff58054)
- [`xv::fs::VirtualFileSystem::retain`](vfs.cpp.md#fn-8e0bbe0879ea9c9e)
- [`xv::fs::VirtualFileSystem::same`](vfs.cpp.md#fn-7cbd893bc133ef55)
- [`xv::fs::VirtualFileSystem::stat`](vfs.cpp.md#fn-8e48167e5fe78464)
- [`xv::fs::VirtualFileSystem::symlink`](vfs.cpp.md#fn-67eba603f2e485ac)
- [`xv::fs::VirtualFileSystem::truncate`](vfs.cpp.md#fn-c409ab8df43d2e55)
- [`xv::fs::VirtualFileSystem::unlink`](vfs.cpp.md#fn-5b042e585b21f4bf)
- [`xv::fs::VirtualFileSystem::unlink_at`](vfs.cpp.md#fn-b4095b4ec764cda2)
- [`xv::fs::VirtualFileSystem::unlock`](vfs.cpp.md#fn-a638e65a0686494e)
- [`xv::fs::VirtualFileSystem::unlock_release`](vfs.cpp.md#fn-ad637485c5f0cda9)
- [`xv::fs::VirtualFileSystem::unmount`](vfs.cpp.md#fn-5396bbf57ffd12ef)
- [`xv::fs::VirtualFileSystem::update`](vfs.cpp.md#fn-39fbbef1274fce08)
- [`xv::fs::VirtualFileSystem::write`](vfs.cpp.md#fn-80c4191a47122858)

<a id="fn-3975fe035d893dce"></a>

## `xv::fs::VirtualFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::VirtualFileSystem::initialize_inodes()
@brief 初始化固定、动态及可选磁盘后端同步。
@details 额外初始化借用的数据卷 inode 缓存；根的恢复和分配继续留给首个进程上下文。
@details 流程由 InitializeFlow::InodesGraph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-e6a15b643dd8c735"></a>

## `xv::fs::VirtualFileSystem::initialize`

```doxygen
@fn void VirtualFileSystem::xv::fs::VirtualFileSystem::initialize(int device)
@brief 恢复根卷并建立三个系统挂载。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-7cbd893bc133ef55"></a>

## `xv::fs::VirtualFileSystem::same`

```doxygen
@fn bool xv::fs::VirtualFileSystem::same(Inode *first, Inode *second)
@brief 比较节点的文件系统归属与稳定身份。
@details 同时比较 owner、device 和 inode 号，空指针返回 false，防止不同文件系统的相同 inode 号误匹配挂载。
```

<a id="fn-4860e29021bbf700"></a>

## `xv::fs::VirtualFileSystem::enter_mount`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::enter_mount(Inode *node)
@brief 把覆盖目录引用转换为对应挂载根引用。
@details 遍历启动后只读的挂载记录，以 owner/device/inode 身份匹配；命中时释放覆盖引用并取得根引用，否则原样返回。
```

<a id="fn-6591de3da7091bf9"></a>

## `xv::fs::VirtualFileSystem::create`

```doxygen
@fn Inode *VirtualFileSystem::xv::fs::VirtualFileSystem::create(char *path, short type, short major, short minor, bool follow)
@brief 按节点类型创建或取得文件。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-365a4e7acb8561db"></a>

## `xv::fs::VirtualFileSystem::link`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::link(char *old_path, char *new_path)
@brief 为最终节点本身建立同后端硬链接。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-5b042e585b21f4bf"></a>

## `xv::fs::VirtualFileSystem::unlink`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::unlink(char *path)
@brief 在事务内解析父路径并请求删除名称。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-b14436f73059aab1"></a>

## `xv::fs::VirtualFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::allocate(unsigned device, short type)
@brief 将显式磁盘 inode 分配请求交给根文件系统。
@details 保留现有低层磁盘分配接口；普通路径创建应调用 create/create_at，devfs 不通过此入口分配设备。
```

<a id="fn-8e0bbe0879ea9c9e"></a>

## `xv::fs::VirtualFileSystem::retain`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::retain(Inode *node)
@brief 把 retain 分发给 inode 所属文件系统。
@details 增加引用并返回同一节点，引用锁由所属实现取得。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-e5559898f89824ab"></a>

## `xv::fs::VirtualFileSystem::release`

```doxygen
@fn void xv::fs::VirtualFileSystem::release(Inode *node)
@brief 把 release 分发给 inode 所属文件系统。
@details 释放一个引用；持久节点可能回收无链接 inode，调用者须处于磁盘事务。devfs 仅减少内存引用。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-5967b2c0b5f42168"></a>

## `xv::fs::VirtualFileSystem::lock`

```doxygen
@fn void xv::fs::VirtualFileSystem::lock(Inode *node)
@brief 把 lock 分发给 inode 所属文件系统。
@details 取得节点睡眠锁，调用者持有引用且不能持有自旋锁。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-a638e65a0686494e"></a>

## `xv::fs::VirtualFileSystem::unlock`

```doxygen
@fn void xv::fs::VirtualFileSystem::unlock(Inode *node)
@brief 把 unlock 分发给 inode 所属文件系统。
@details 释放调用者持有的节点锁，保持引用。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-ad637485c5f0cda9"></a>

## `xv::fs::VirtualFileSystem::unlock_release`

```doxygen
@fn void xv::fs::VirtualFileSystem::unlock_release(Inode *node)
@brief 把 unlock_release 分发给 inode 所属文件系统。
@details 先释放节点锁再释放一个引用，磁盘释放仍遵守事务约定。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-39fbbef1274fce08"></a>

## `xv::fs::VirtualFileSystem::update`

```doxygen
@fn void xv::fs::VirtualFileSystem::update(Inode *node)
@brief 把 update 分发给 inode 所属文件系统。
@details 写回持久 inode 元数据，调用者须有节点锁和磁盘事务；不能用于 devfs。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-c409ab8df43d2e55"></a>

## `xv::fs::VirtualFileSystem::truncate`

```doxygen
@fn void xv::fs::VirtualFileSystem::truncate(Inode *node)
@brief 把 truncate 分发给 inode 所属文件系统。
@details 释放持久文件数据块并清空大小，要求节点锁与磁盘事务；不能用于 devfs。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-a35df3a283d2bfaf"></a>

## `xv::fs::VirtualFileSystem::read`

```doxygen
@fn int xv::fs::VirtualFileSystem::read(Inode *node, int user, unsigned long destination, unsigned offset, unsigned count)
@brief 把 read 分发给 inode 所属文件系统。
@details 在已锁定节点上按 offset 读取并返回字节数，user 指定目标地址空间；devfs 根生成目录记录。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-80c4191a47122858"></a>

## `xv::fs::VirtualFileSystem::write`

```doxygen
@fn int xv::fs::VirtualFileSystem::write(Inode *node, int user, unsigned long source, unsigned offset, unsigned count)
@brief 把 write 分发给 inode 所属文件系统。
@details 在已锁定节点上按 offset 写入并返回字节数，持久写入由外层事务保护；devfs 目录拒绝写入。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-8e48167e5fe78464"></a>

## `xv::fs::VirtualFileSystem::stat`

```doxygen
@fn void xv::fs::VirtualFileSystem::stat(Inode *node, Stat *result)
@brief 把 stat 分发给 inode 所属文件系统。
@details 把已锁定节点元数据写入内核输出，不执行用户复制。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-5d4339ff52e6c374"></a>

## `xv::fs::VirtualFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
@brief 把 lookup_entry 分发给 inode 所属文件系统。
@details 查找已锁定目录中的一个名称，成功返回新的未锁定节点引用；本层不执行挂载跳转。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-6139d98544fd4958"></a>

## `xv::fs::VirtualFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::create_at(Inode *directory, const char *name, short type, short major, short minor)
@brief 在命名空间锁内请求所属后端建立目录项。
@details 与 rename/mount/unmount 串行化，后端自行检查权限和已有节点；成功仍按原契约返回已锁定 inode，调用者负责解锁和释放。
@return 已锁定 inode 引用，失败返回 nullptr。
```

<a id="fn-71de116d242b4ba0"></a>

## `xv::fs::VirtualFileSystem::link_at`

```doxygen
@fn int xv::fs::VirtualFileSystem::link_at(Inode *directory, const char *name, Inode *target)
@brief 串行化并转发同后端硬链接。
@details 取得命名空间锁，拒绝 directory 和 target 归属不同的后端，再由后端检查授权、名称和链接计数。
@return 0 表示成功，-1 表示检查或后端失败。
```

<a id="fn-b4095b4ec764cda2"></a>

## `xv::fs::VirtualFileSystem::unlink_at`

```doxygen
@fn int xv::fs::VirtualFileSystem::unlink_at(Inode *directory, const char *name)
@brief 在稳定挂载关系下删除目录项。
@details 命名空间锁串行化检查和实际删除。短暂锁住目录并查找目标，拒绝覆盖点或挂载根，释放临时引用后转交后端执行目录/sticky 权限与空目录检查。
@return 0 表示成功，-1 表示挂载保护或后端拒绝。
@details 流程由 UnlinkAtFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-fd77401a8aa09874"></a>

## `xv::fs::VirtualFileSystem::mount`

```doxygen
@fn bool VirtualFileSystem::xv::fs::VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
@brief 装配系统生命周期的启动后端。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-f4cf807217fcb7f0"></a>

## `xv::fs::VirtualFileSystem::lookup`

```doxygen
@fn Inode *VirtualFileSystem::xv::fs::VirtualFileSystem::lookup(const char *path, bool real, bool follow)
@brief 按跟随策略取得稳定节点引用。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-abf62fef3e7f21c5"></a>

## `xv::fs::VirtualFileSystem::allowed`

```doxygen
@fn bool xv::fs::VirtualFileSystem::allowed(Inode *node, unsigned requested, bool real)
@brief 委托公共节点访问策略。
@details 调用者持节点锁；requested 是 R/W/X 位，real 决定真实或有效身份，不在包装层重复获取节点锁。
```

<a id="fn-35b6020510aa9401"></a>

## `xv::fs::VirtualFileSystem::ancestor`

```doxygen
@fn bool xv::fs::VirtualFileSystem::ancestor(Inode *possible, Inode *node)
@brief 沿父目录关系判断一个节点是否包含另一个节点。
@details 命名空间锁内保留待检查节点；遇到挂载根使用挂载表中的父节点，否则读取后端 ..。每一步归还旧引用；最多走 128 步，超出时保守返回 true，供环与挂载祖先保护使用。
@return 匹配或达到遍历上限返回 true；到达根且不匹配返回 false。
@details 流程由 AncestorFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-09eb092ae4d24474"></a>

## `xv::fs::VirtualFileSystem::mount_ram`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::mount_ram(const char *path)
@brief 发布独立动态 RAM 实例。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-5396bbf57ffd12ef"></a>

## `xv::fs::VirtualFileSystem::unmount`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::unmount(const char *path)
@brief 安全撤销 RAM 或持久卷挂载。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-3ce7ce542ff58054"></a>

## `xv::fs::VirtualFileSystem::rename`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::rename(const char *old_path, const char *new_path)
@brief 在稳定挂载拓扑下校验并转发原子重命名。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-67eba603f2e485ac"></a>

## `xv::fs::VirtualFileSystem::symlink`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::symlink(const char *target, const char *path)
@brief 原子创建保存目标字节的链接节点。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-f77a31b05fc26086"></a>

## `xv::fs::VirtualFileSystem::walk`

```doxygen
@fn Inode *VirtualFileSystem::xv::fs::VirtualFileSystem::walk(const char *path, bool parent, char *name, bool real, bool follow, bool creating,unsigned capacity,bool exact_names)
@brief 按后端名称上限解析路径和符号链接，保持节点引用交接。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-6d0bcfd9ae87ee67"></a>

## `xv::fs::VirtualFileSystem::lookup_parent`

```doxygen
@fn Inode *VirtualFileSystem::xv::fs::VirtualFileSystem::lookup_parent(const char *path, char *name,unsigned capacity)
@brief 解析父目录并按显式缓冲容量交付末级名字，旧默认14字节ABI保留。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-4b81eaa3029bfdad"></a>

## `xv::fs::VirtualFileSystem::publish_mount_snapshot`

```doxygen
@fn void VirtualFileSystem::xv::fs::VirtualFileSystem::publish_mount_snapshot()
@brief 在挂载变更后通过独立锁发布只读挂载信息。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-49863f7ed2744744"></a>

## `xv::fs::VirtualFileSystem::snapshot_mounts`

```doxygen
@fn unsigned VirtualFileSystem::xv::fs::VirtualFileSystem::snapshot_mounts(MountSnapshot *records,unsigned capacity) const
@brief 从已发布快照复制挂载信息，不取得VFS拓扑锁。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-5c98bc2f3956ca9e"></a>

## `xv::fs::VirtualFileSystem::mounts`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::mounts(unsigned index, char *path, char *type,bool canonical,bool *readonly)
@brief 输出独立发布的挂载快照，避免与VFS拓扑锁递归。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-d081694eae1d34f0"></a>

## `xv::fs::VirtualFileSystem::mount_disk`

```doxygen
@fn int VirtualFileSystem::xv::fs::VirtualFileSystem::mount_disk(const char *path,const char *type)
@brief 使用MountDiskFlow挂载已选择磁盘后端，保留事务和失败回滚。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
