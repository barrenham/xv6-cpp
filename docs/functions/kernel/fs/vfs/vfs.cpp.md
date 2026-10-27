# `kernel/fs/vfs/vfs.cpp`

## 函数导航

- [`xv::fs::VirtualFileSystem::allocate`](vfs.cpp.md#fn-b14436f73059aab1)
- [`xv::fs::VirtualFileSystem::allowed`](vfs.cpp.md#fn-abf62fef3e7f21c5)
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
- [`xv::fs::VirtualFileSystem::lookup_parent`](vfs.cpp.md#fn-8bfb0d7b5ac233f1)
- [`xv::fs::VirtualFileSystem::mount`](vfs.cpp.md#fn-fd77401a8aa09874)
- [`xv::fs::VirtualFileSystem::read`](vfs.cpp.md#fn-a35df3a283d2bfaf)
- [`xv::fs::VirtualFileSystem::release`](vfs.cpp.md#fn-e5559898f89824ab)
- [`xv::fs::VirtualFileSystem::retain`](vfs.cpp.md#fn-8e0bbe0879ea9c9e)
- [`xv::fs::VirtualFileSystem::same`](vfs.cpp.md#fn-7cbd893bc133ef55)
- [`xv::fs::VirtualFileSystem::stat`](vfs.cpp.md#fn-8e48167e5fe78464)
- [`xv::fs::VirtualFileSystem::truncate`](vfs.cpp.md#fn-c409ab8df43d2e55)
- [`xv::fs::VirtualFileSystem::unlink`](vfs.cpp.md#fn-5b042e585b21f4bf)
- [`xv::fs::VirtualFileSystem::unlink_at`](vfs.cpp.md#fn-b4095b4ec764cda2)
- [`xv::fs::VirtualFileSystem::unlock`](vfs.cpp.md#fn-a638e65a0686494e)
- [`xv::fs::VirtualFileSystem::unlock_release`](vfs.cpp.md#fn-ad637485c5f0cda9)
- [`xv::fs::VirtualFileSystem::update`](vfs.cpp.md#fn-39fbbef1274fce08)
- [`xv::fs::VirtualFileSystem::walk`](vfs.cpp.md#fn-81e896ba99fe170a)
- [`xv::fs::VirtualFileSystem::write`](vfs.cpp.md#fn-80c4191a47122858)

<a id="fn-3975fe035d893dce"></a>

## `xv::fs::VirtualFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::VirtualFileSystem::initialize_inodes()
@brief 初始化所有已装配后端的节点锁和归属。
@details 为磁盘/devfs 及可选 procfs/ramfs 建立基础同步；RAM 根的实际分配推迟到首个进程中的 initialize。
```

<a id="fn-e6a15b643dd8c735"></a>

## `xv::fs::VirtualFileSystem::initialize`

```doxygen
@fn void xv::fs::VirtualFileSystem::initialize(int device)
@brief 恢复磁盘并依次挂载 /dev、/proc 和 /tmp。
@details 启动事务只负责覆盖目录和挂载关系；RAM 文件内容不写磁盘。全部装配后冻结三个记录，运行期没有新增 mount/unmount 系统调用。
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

<a id="fn-8bfb0d7b5ac233f1"></a>

## `xv::fs::VirtualFileSystem::lookup_parent`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::lookup_parent(const char *path, char *name)
@brief 解析父目录并复制最后一个路径分量。
@details name 至少有 DIRSIZ 字节空间；成功返回带引用、未锁定的父节点，根路径无末级分量时失败。沿用 xv6 超长分量截断行为。
```

<a id="fn-6591de3da7091bf9"></a>

## `xv::fs::VirtualFileSystem::create`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::create(char *path, short type, short major, short minor)
@brief 解析跨挂载父路径并向其文件系统请求创建。
@details 借用外层事务，父节点引用在委托后释放；成功节点保持已锁定并带引用。磁盘实现可分配 inode，devfs 仅复用已有设备，禁止添加名称。
```

<a id="fn-365a4e7acb8561db"></a>

## `xv::fs::VirtualFileSystem::link`

```doxygen
@fn int xv::fs::VirtualFileSystem::link(char *old_path, char *new_path)
@brief 在事务内解析两个路径并请求硬链接。
@details 目标和父目录必须属于同一文件系统，委托 link_at 检查并执行。无论成功与否都释放临时引用，跨挂载链接失败而不修改任一文件系统。
```

<a id="fn-5b042e585b21f4bf"></a>

## `xv::fs::VirtualFileSystem::unlink`

```doxygen
@fn int xv::fs::VirtualFileSystem::unlink(char *path)
@brief 在事务内解析父路径并请求删除名称。
@details 委托 unlink_at 保护挂载覆盖节点与设备根，再调用实际文件系统删除。释放父引用，返回 0 或 -1。
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
@brief 把 create_at 分发给 inode 所属文件系统。
@details 借用未锁定父目录，成功返回锁定的新引用；持久创建要求外层事务，devfs 只复用已有设备。 不根据路径前缀或设备号选择实现，也不改变调用者的锁与引用责任。
```

<a id="fn-71de116d242b4ba0"></a>

## `xv::fs::VirtualFileSystem::link_at`

```doxygen
@fn int xv::fs::VirtualFileSystem::link_at(Inode *directory, const char *name, Inode *target)
@brief 校验节点归属后请求同一文件系统创建硬链接。
@details 借用父目录和目标引用，不转移所有权；跨 owner 直接返回 -1，同一 owner 的实现负责锁、链接数与只读规则。磁盘修改要求外层事务。
```

<a id="fn-b4095b4ec764cda2"></a>

## `xv::fs::VirtualFileSystem::unlink_at`

```doxygen
@fn int xv::fs::VirtualFileSystem::unlink_at(Inode *directory, const char *name)
@brief 保护全部启动挂载的根与覆盖目录再委托删除。
@details 借用父目录引用，锁内查找子节点并检查挂载身份，释放临时引用后解锁；保护对象返回 -1，否则委托所属文件系统。devfs/procfs 自身的只读规则由后端执行。
```

<a id="fn-fd77401a8aa09874"></a>

## `xv::fs::VirtualFileSystem::mount`

```doxygen
@fn bool xv::fs::VirtualFileSystem::mount(const char *path, FileSystem &backend, unsigned device)
@brief 在启动阶段把一个节点后端挂载到目录。
@details 调用者已初始化后端节点锁并持磁盘事务。复用或创建覆盖目录，保存覆盖节点、挂载根与父目录的长期引用；拒绝非目录、重复挂载、冻结后修改及超过三个记录。后端 initialize 只在验证路径之后执行。
```
<a id="fn-81e896ba99fe170a"></a>

## `xv::fs::VirtualFileSystem::walk`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::walk(const char *path, bool parent, char *name, bool real)
@brief 逐组件解析挂载路径并检查目录搜索权限。
@details 每次锁定目录后验证 X，处理点目录和挂载边界。real 模式将真实身份贯穿全部组件供 access 使用；返回新的未锁定引用或失败，临时引用始终归还。
```

<a id="fn-f4cf807217fcb7f0"></a>

## `xv::fs::VirtualFileSystem::lookup`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::lookup(const char *path, bool real)
@brief 以指定凭据模式查找完整路径。
@details 默认有效身份；real=true 用于 access 的真实身份路径搜索。委托 walk，成功返回调用者负责归还的未锁定引用。
```

<a id="fn-abf62fef3e7f21c5"></a>

## `xv::fs::VirtualFileSystem::allowed`

```doxygen
@fn bool xv::fs::VirtualFileSystem::allowed(Inode *node, unsigned requested, bool real)
@brief 委托公共节点访问策略。
@details 调用者持节点锁；requested 是 R/W/X 位，real 决定真实或有效身份，不在包装层重复获取节点锁。
```
