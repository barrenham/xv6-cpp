# `kernel/fs/inode/xv6_filesystem.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/inode/xv6_filesystem.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:xv6`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Xv6FileSystem::allocate`](xv6_filesystem.cpp.md#fn-071e023c7d9e28d2)
- [`xv::fs::Xv6FileSystem::allocate_block`](xv6_filesystem.cpp.md#fn-5a13ce655a471c2a)
- [`xv::fs::Xv6FileSystem::busy`](xv6_filesystem.cpp.md#fn-2af87439a305e904)
- [`xv::fs::Xv6FileSystem::free_block`](xv6_filesystem.cpp.md#fn-9f690bb8450d7917)
- [`xv::fs::Xv6FileSystem::free_inode`](xv6_filesystem.cpp.md#fn-55c1f3a16e6a8da9)
- [`xv::fs::Xv6FileSystem::get`](xv6_filesystem.cpp.md#fn-9e44fd39e00d972e)
- [`xv::fs::Xv6FileSystem::initialize`](xv6_filesystem.cpp.md#fn-e1029df290612731)
- [`xv::fs::Xv6FileSystem::initialize_inodes`](xv6_filesystem.cpp.md#fn-8e9ee55da6c91ffd)
- [`xv::fs::Xv6FileSystem::lock`](xv6_filesystem.cpp.md#fn-6d014e00edddc19a)
- [`xv::fs::Xv6FileSystem::map_block`](xv6_filesystem.cpp.md#fn-e115e4280d884517)
- [`xv::fs::Xv6FileSystem::open_device`](xv6_filesystem.cpp.md#fn-9f79fb79bd4090b4)
- [`xv::fs::Xv6FileSystem::read`](xv6_filesystem.cpp.md#fn-dd71262a05693ac2)
- [`xv::fs::Xv6FileSystem::read_super`](xv6_filesystem.cpp.md#fn-8886c17d9bd7f74a)
- [`xv::fs::Xv6FileSystem::reclaim`](xv6_filesystem.cpp.md#fn-63ef700d59c2ecf4)
- [`xv::fs::Xv6FileSystem::release`](xv6_filesystem.cpp.md#fn-488eb6e0a79dde7a)
- [`xv::fs::Xv6FileSystem::resize`](xv6_filesystem.cpp.md#fn-13385ef79a786dd7)
- [`xv::fs::Xv6FileSystem::retain`](xv6_filesystem.cpp.md#fn-cf818bc3fcef07af)
- [`xv::fs::Xv6FileSystem::stat`](xv6_filesystem.cpp.md#fn-380640571e8b5b9b)
- [`xv::fs::Xv6FileSystem::truncate`](xv6_filesystem.cpp.md#fn-c1850e2e3bb309e8)
- [`xv::fs::Xv6FileSystem::unlock`](xv6_filesystem.cpp.md#fn-93ed9249a65b47aa)
- [`xv::fs::Xv6FileSystem::unlock_release`](xv6_filesystem.cpp.md#fn-45dcd30b8fa7a526)
- [`xv::fs::Xv6FileSystem::update`](xv6_filesystem.cpp.md#fn-d23dade71242312c)
- [`xv::fs::Xv6FileSystem::write`](xv6_filesystem.cpp.md#fn-b5be87921570227b)
- [`xv::fs::Xv6FileSystem::zero_block`](xv6_filesystem.cpp.md#fn-e9142d5fbb52fb87)

<a id="fn-8886c17d9bd7f74a"></a>

## `xv::fs::Xv6FileSystem::read_super`

```doxygen
@fn void xv::fs::Xv6FileSystem::read_super(int dev, superblock *layout)
@brief xv6 inode 文件系统：读取大页。
@param dev 块设备编号。
@param layout 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `detail::move` → `bp->bytes` → `bp.reset`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e1029df290612731"></a>

## `xv::fs::Xv6FileSystem::initialize`

```doxygen
@fn void xv::fs::Xv6FileSystem::initialize(int dev)
@brief 初始化指定磁盘的布局、日志恢复与孤儿回收。
@details 保存根节点使用的 device_，读取并验证超级块，然后初始化 Journal 并清理无链接 inode。必须在可睡眠的首个进程上下文调用。
@details 流程由 InitializeFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-e9142d5fbb52fb87"></a>

## `xv::fs::Xv6FileSystem::zero_block`

```doxygen
@fn void xv::fs::Xv6FileSystem::zero_block(int dev, int bno)
@brief xv6 inode 文件系统：清零指定磁盘块的数据。
@param dev 块设备编号。
@param bno 本次操作使用的 bno 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `detail::fill` → `bp->bytes` → `journal_.record` → `bp.reset`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5a13ce655a471c2a"></a>

## `xv::fs::Xv6FileSystem::allocate_block`

```doxygen
@fn uint xv::fs::Xv6FileSystem::allocate_block(uint dev)
@brief xv6 inode 文件系统：从位图中分配磁盘数据块。
@param dev 块设备编号。
@return 各分支实际返回：`b + bi`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 AllocateBlockFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-9f690bb8450d7917"></a>

## `xv::fs::Xv6FileSystem::free_block`

```doxygen
@fn void xv::fs::Xv6FileSystem::free_block(int dev, uint b)
@brief xv6 inode 文件系统：把磁盘块归还位图。
@param dev 块设备编号。
@param b 本次操作使用的 b 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `BBLOCK` → `bp->bytes` → `detail::fail` → `journal_.record` → `bp.reset`。
@details 需要特别跟踪的条件：`(bp->bytes()[bi / 8] & m) == 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8e9ee55da6c91ffd"></a>

## `xv::fs::Xv6FileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::Xv6FileSystem::initialize_inodes()
@brief 初始化磁盘 inode 表及其归属和锁。
@details 为全部缓存槽绑定本实例 owner_，初始化 inode_lock_ 与逐节点睡眠锁。启动时执行一次，随后初始进程可以先取得尚未加载磁盘内容的根引用。
```

<a id="fn-071e023c7d9e28d2"></a>

## `xv::fs::Xv6FileSystem::allocate`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::allocate(uint dev, short type)
@brief xv6 inode 文件系统：分配并取得对象或存储资源。
@param dev 块设备编号。
@param type 本次操作使用的 type 值。
@return 各分支实际返回：`get(dev, inum)`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 AllocateFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-d23dade71242312c"></a>

## `xv::fs::Xv6FileSystem::update`

```doxygen
@fn void xv::fs::Xv6FileSystem::update(Inode *ip)
@brief 将 inode 元数据写入日志缓冲。
@details 持节点锁及磁盘事务，按 128 字节 dinode 布局写入 uid/gid/mode 及既有块映射，最后登记日志块。格式版本与旧 64 字节 inode 不兼容。
```

<a id="fn-9e44fd39e00d972e"></a>

## `xv::fs::Xv6FileSystem::get`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::get(uint dev, uint inum)
@brief xv6 inode 文件系统：取得。
@param dev 块设备编号。
@param inum 本次操作使用的 inum 值。
@note 源码中出现的锁操作：`inode_lock_.acquire` → `inode_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`ip`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 GetFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-cf818bc3fcef07af"></a>

## `xv::fs::Xv6FileSystem::retain`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::retain(Inode *ip)
@brief xv6 inode 文件系统：增加对象引用，延长其存活期。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`inode_lock_.acquire` → `inode_lock_.release`。
@note 源码中出现的锁操作：`inode_lock_.acquire` → `inode_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`ip`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6d014e00edddc19a"></a>

## `xv::fs::Xv6FileSystem::lock`

```doxygen
@fn void xv::fs::Xv6FileSystem::lock(Inode *ip)
@brief 锁定磁盘 inode 并按需加载缓存。
@details 持有效引用取得睡眠锁，首次从 128 字节 dinode 读取类型、块地址与 uid/gid/mode；无效类型触发内核错误，后续访问复用缓存。
@details 流程由 LockFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-93ed9249a65b47aa"></a>

## `xv::fs::Xv6FileSystem::unlock`

```doxygen
@fn void xv::fs::Xv6FileSystem::unlock(Inode *ip)
@brief xv6 inode 文件系统：释放已持有的对象互斥保护。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ip->lock.holding` → `detail::fail` → `ip->lock.release`。
@details 需要特别跟踪的条件：`ip == 0 || !ip->lock.holding() || ip->ref < 1`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`ip->lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-55c1f3a16e6a8da9"></a>

## `xv::fs::Xv6FileSystem::free_inode`

```doxygen
@fn void xv::fs::Xv6FileSystem::free_inode(uint dev, uint inum)
@brief xv6 inode 文件系统：释放inode。
@param dev 块设备编号。
@param inum 本次操作使用的 inum 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `IBLOCK` → `bp->bytes` → `journal_.record` → `bp.reset`。
@details 本函数可见的状态更新包括：`dip->type = 0;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-488eb6e0a79dde7a"></a>

## `xv::fs::Xv6FileSystem::release`

```doxygen
@fn void xv::fs::Xv6FileSystem::release(Inode *ip)
@brief 归还 inode 引用，并在最后无链接引用时回收磁盘资源。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
@details 普通引用在 inode 表锁内直接递减并解锁，不进行图分派；只有 ref 为一、缓存有效且链接数为零时运行 ReleaseFlow。进入图前保存设备与 inode 号，释放缓存引用后不再读取可能已被重用的缓存身份。
```

<a id="fn-45dcd30b8fa7a526"></a>

## `xv::fs::Xv6FileSystem::unlock_release`

```doxygen
@fn void xv::fs::Xv6FileSystem::unlock_release(Inode *ip)
@brief xv6 inode 文件系统：释放 inode 锁并归还 inode 引用。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`unlock` → `release`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-63ef700d59c2ecf4"></a>

## `xv::fs::Xv6FileSystem::reclaim`

```doxygen
@fn void xv::fs::Xv6FileSystem::reclaim(int dev)
@brief xv6 inode 文件系统：回收不再有外部使用者的 inode 状态。
@param dev 块设备编号。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 ReclaimFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-e115e4280d884517"></a>

## `xv::fs::Xv6FileSystem::map_block`

```doxygen
@fn uint xv::fs::Xv6FileSystem::map_block(Inode *ip, uint bn, bool create)
@brief 查找或按需建立磁盘数据块映射。
@details create 为 false 时缺失的直接或间接映射返回零，绝不分配；create 为 true 保留原有分配与日志记录行为。
@details 流程由 MapBlockFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-c1850e2e3bb309e8"></a>

## `xv::fs::Xv6FileSystem::truncate`

```doxygen
@fn void xv::fs::Xv6FileSystem::truncate(Inode *ip)
@brief xv6 inode 文件系统：释放 inode 数据块并重置文件长度。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 TruncateFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-380640571e8b5b9b"></a>

## `xv::fs::Xv6FileSystem::stat`

```doxygen
@fn void xv::fs::Xv6FileSystem::stat(Inode *ip, Stat *st)
@brief 复制磁盘 inode 的用户可见状态。
@details 调用者持节点锁；输出原有类型、链接、大小及安全属性，权限检查由打开或路径系统调用执行。
```

<a id="fn-dd71262a05693ac2"></a>

## `xv::fs::Xv6FileSystem::read`

```doxygen
@fn int xv::fs::Xv6FileSystem::read(Inode *ip, int user_dst, uint64 dst, uint off, uint n)
@brief 读取文件内容并对稀疏空洞补零。
@details 只查找现有块映射，缺块用只读零缓冲复制，不因读取分配磁盘空间；仍遵守 EOF 与用户复制失败处理。
@details 流程由 ReadFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-b5be87921570227b"></a>

## `xv::fs::Xv6FileSystem::write`

```doxygen
@fn int xv::fs::Xv6FileSystem::write(Inode *ip, int user_src, uint64 src, uint off, uint n)
@brief 在文件范围内写入数据并允许 EOF 后定位。
@details 拒绝溢出和 MAXFILE 之外的请求，仅为实际写入范围分配块；未分配间隔读取为零，成功后更新长度和 inode 映射。
@details 流程由 WriteFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```
<a id="fn-9f79fb79bd4090b4"></a>

## `xv::fs::Xv6FileSystem::open_device`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::Xv6FileSystem::open_device(Inode *node, bool nonblocking)
@brief 为磁盘中的设备节点打开登记会话。
@details 委托 open_registered_device 按完整设备号查找工厂，支持静态字符设备及会话工厂；节点引用由调用者继续持有，失败返回 nullptr。
```

<a id="fn-13385ef79a786dd7"></a>

## `xv::fs::Xv6FileSystem::resize`

```doxygen
@fn int xv::fs::Xv6FileSystem::resize(Inode *node, unsigned size)
@brief 在日志事务内调整磁盘普通文件长度。
@details 缩短时释放超出新 EOF 的直接/间接块，清空残留尾部；增长仅修改长度，不预分配空洞。拒绝非普通文件和超出 MAXFILE 的长度。
@details 流程由 ResizeFlow::Graph 的编译期策略连接；具名阶段保留原返回值、锁范围和引用转移次序。
```

<a id="fn-2af87439a305e904"></a>

## `xv::fs::Xv6FileSystem::busy`

```doxygen
@fn bool xv::fs::Xv6FileSystem::busy()
@brief 检查磁盘卷是否仍被挂载根以外的引用使用。
@details inode 引用锁内统计缓存引用；仅剩挂载表持有的一个根引用时允许卸载，打开文件和 cwd 等引用使其忙碌。
```
