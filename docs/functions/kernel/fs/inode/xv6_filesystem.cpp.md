# `kernel/fs/inode/xv6_filesystem.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/inode/xv6_filesystem.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:xv6`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Xv6FileSystem::allocate`](xv6_filesystem.cpp.md#fn-071e023c7d9e28d2)
- [`xv::fs::Xv6FileSystem::allocate_block`](xv6_filesystem.cpp.md#fn-5a13ce655a471c2a)
- [`xv::fs::Xv6FileSystem::free_block`](xv6_filesystem.cpp.md#fn-9f690bb8450d7917)
- [`xv::fs::Xv6FileSystem::free_inode`](xv6_filesystem.cpp.md#fn-55c1f3a16e6a8da9)
- [`xv::fs::Xv6FileSystem::get`](xv6_filesystem.cpp.md#fn-9e44fd39e00d972e)
- [`xv::fs::Xv6FileSystem::initialize`](xv6_filesystem.cpp.md#fn-e1029df290612731)
- [`xv::fs::Xv6FileSystem::initialize_inodes`](xv6_filesystem.cpp.md#fn-8e9ee55da6c91ffd)
- [`xv::fs::Xv6FileSystem::lock`](xv6_filesystem.cpp.md#fn-6d014e00edddc19a)
- [`xv::fs::Xv6FileSystem::map_block`](xv6_filesystem.cpp.md#fn-e115e4280d884517)
- [`xv::fs::Xv6FileSystem::read`](xv6_filesystem.cpp.md#fn-dd71262a05693ac2)
- [`xv::fs::Xv6FileSystem::read_super`](xv6_filesystem.cpp.md#fn-8886c17d9bd7f74a)
- [`xv::fs::Xv6FileSystem::reclaim`](xv6_filesystem.cpp.md#fn-63ef700d59c2ecf4)
- [`xv::fs::Xv6FileSystem::release`](xv6_filesystem.cpp.md#fn-488eb6e0a79dde7a)
- [`xv::fs::Xv6FileSystem::retain`](xv6_filesystem.cpp.md#fn-cf818bc3fcef07af)
- [`xv::fs::Xv6FileSystem::stat`](xv6_filesystem.cpp.md#fn-380640571e8b5b9b)
- [`xv::fs::Xv6FileSystem::truncate`](xv6_filesystem.cpp.md#fn-c1850e2e3bb309e8)
- [`xv::fs::Xv6FileSystem::unlock`](xv6_filesystem.cpp.md#fn-93ed9249a65b47aa)
- [`xv::fs::Xv6FileSystem::unlock_release`](xv6_filesystem.cpp.md#fn-45dcd30b8fa7a526)
- [`xv::fs::Xv6FileSystem::update`](xv6_filesystem.cpp.md#fn-d23dade71242312c)
- [`xv::fs::Xv6FileSystem::write`](xv6_filesystem.cpp.md#fn-b5be87921570227b)
- [`xv::fs::Xv6FileSystem::zero_block`](xv6_filesystem.cpp.md#fn-e9142d5fbb52fb87)
- [`xv::fs::min`](xv6_filesystem.cpp.md#fn-8b8b180ad809a797)

<a id="fn-8b8b180ad809a797"></a>

## `xv::fs::min`

```doxygen
@fn static unsigned xv::fs::min(unsigned first, unsigned second)
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `first < second ? first : second`，返回当前查询结果。
@param first 本次操作使用的 首项 值。
@param second 本次操作使用的 second 值。
@return 各分支实际返回：`first < second ? first : second`。应结合上面的条件检查区分正常值、空值和错误值。
```

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
@brief xv6 inode 文件系统：建立该对象开始工作所需的初始状态。
@param dev 块设备编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`read_super` → `detail::fail` → `journal_.initialize` → `reclaim`。
@details 需要特别跟踪的条件：`layout_.magic != FSMAGIC`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `BBLOCK` → `bp->bytes` → `journal_.record` → `bp.reset` → `zero_block` → `detail::report`。
@details 需要特别跟踪的条件：`(bp->bytes()[bi / 8] & m) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`b + bi`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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
@brief xv6 inode 文件系统：初始化inodes。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`inode_lock_.init` → `lock.init`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-071e023c7d9e28d2"></a>

## `xv::fs::Xv6FileSystem::allocate`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::allocate(uint dev, short type)
@brief xv6 inode 文件系统：分配并取得对象或存储资源。
@param dev 块设备编号。
@param type 本次操作使用的 type 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `IBLOCK` → `bp->bytes` → `detail::fill` → `journal_.record` → `bp.reset` → `get` → `detail::report`。
@details 本函数可见的状态更新包括：`dip->type = type;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`dip->type == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`get(dev, inum)`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d23dade71242312c"></a>

## `xv::fs::Xv6FileSystem::update`

```doxygen
@fn void xv::fs::Xv6FileSystem::update(Inode *ip)
@brief xv6 inode 文件系统：把内存中的 inode 元数据写回缓存块。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `IBLOCK` → `bp->bytes` → `detail::move` → `journal_.record` → `bp.reset`。
@details 本函数可见的状态更新包括：`dip->type = ip->type;`；`dip->major = ip->major;`；`dip->minor = ip->minor;`；`dip->nlink = ip->nlink;`；`dip->size = ip->size;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9e44fd39e00d972e"></a>

## `xv::fs::Xv6FileSystem::get`

```doxygen
@fn Inode *xv::fs::Xv6FileSystem::get(uint dev, uint inum)
@brief xv6 inode 文件系统：取得。
@param dev 块设备编号。
@param inum 本次操作使用的 inum 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`inode_lock_.acquire` → `inode_lock_.release` → `detail::fail`。
@details 本函数可见的状态更新包括：`ip->dev = dev;`；`ip->inum = inum;`；`ip->ref = 1;`；`ip->valid = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`ip->ref > 0 && ip->dev == dev && ip->inum == inum`；`empty == 0 && ip->ref == 0`；`empty == 0`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`inode_lock_.acquire` → `inode_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`ip`。应结合上面的条件检查区分正常值、空值和错误值。
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
@brief xv6 inode 文件系统：取得对象状态所需的互斥保护。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `ip->lock.acquire` → `cache_.read` → `IBLOCK` → `bp->bytes` → `detail::move` → `bp.reset`。
@details 本函数可见的状态更新包括：`ip->type = dip->type;`；`ip->major = dip->major;`；`ip->minor = dip->minor;`；`ip->nlink = dip->nlink;`；`ip->size = dip->size;`；`ip->valid = 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`ip == 0 || ip->ref < 1`；`ip->valid == 0`；`ip->type == 0`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`ip->lock.acquire`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@brief xv6 inode 文件系统：释放当前引用或归还对象占用的资源。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`inode_lock_.acquire` → `ip->lock.acquire` → `inode_lock_.release` → `truncate` → `ip->lock.release` → `free_inode`。
@details 本函数可见的状态更新包括：`ip->valid = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`last`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`inode_lock_.acquire` → `ip->lock.acquire` → `inode_lock_.release` → `ip->lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_.read` → `IBLOCK` → `bp->bytes` → `detail::report` → `get` → `bp.reset` → `transaction` → `lock` → `unlock` → `release`。
@details 需要特别跟踪的条件：`dip->type != 0 && dip->nlink == 0`；`ip`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e115e4280d884517"></a>

## `xv::fs::Xv6FileSystem::map_block`

```doxygen
@fn uint xv::fs::Xv6FileSystem::map_block(Inode *ip, uint bn)
@brief xv6 inode 文件系统：取得或建立文件逻辑块对应的磁盘块。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param bn 本次操作使用的 bn 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocate_block` → `cache_.read` → `bp->bytes` → `journal_.record` → `bp.reset` → `detail::fail`。
@details 本函数可见的状态更新包括：`ip->addrs[bn] = addr;`；`ip->addrs[NDIRECT] = addr;`；`a[bn] = addr;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`bn < NDIRECT`；`(addr = ip->addrs[bn]) == 0`；`addr == 0`；`bn < NINDIRECT`；`(addr = ip->addrs[NDIRECT]) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`addr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c1850e2e3bb309e8"></a>

## `xv::fs::Xv6FileSystem::truncate`

```doxygen
@fn void xv::fs::Xv6FileSystem::truncate(Inode *ip)
@brief xv6 inode 文件系统：释放 inode 数据块并重置文件长度。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`free_block` → `cache_.read` → `bp->bytes` → `bp.reset` → `update`。
@details 本函数可见的状态更新包括：`ip->addrs[i] = 0;`；`ip->addrs[NDIRECT] = 0;`；`ip->size = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`ip->addrs[i]`；`ip->addrs[NDIRECT]`；`a[j]`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-380640571e8b5b9b"></a>

## `xv::fs::Xv6FileSystem::stat`

```doxygen
@fn void xv::fs::Xv6FileSystem::stat(Inode *ip, Stat *st)
@brief xv6 inode 文件系统：stat。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param st 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 本函数可见的状态更新包括：`st->dev = ip->dev;`；`st->ino = ip->inum;`；`st->type = ip->type;`；`st->nlink = ip->nlink;`；`st->size = ip->size;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-dd71262a05693ac2"></a>

## `xv::fs::Xv6FileSystem::read`

```doxygen
@fn int xv::fs::Xv6FileSystem::read(Inode *ip, int user_dst, uint64 dst, uint off, uint n)
@brief xv6 inode 文件系统：读取已有数据并按接口返回读取结果。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param user_dst 指明目标地址是否属于用户态。
@param dst 结果缓冲起点。
@param off 本次操作使用的 关闭 值。
@param n 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`map_block` → `cache_.read` → `min` → `detail::copy_out` → `bp->bytes` → `bp.reset`。
@details 需要特别跟踪的条件：`off > ip->size || off + n < off`；`off + n > ip->size`；`addr == 0`；`detail::copy_out(user_dst, dst, bp->bytes() + (off % BSIZE), m) == -1`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`0`；`tot`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b5be87921570227b"></a>

## `xv::fs::Xv6FileSystem::write`

```doxygen
@fn int xv::fs::Xv6FileSystem::write(Inode *ip, int user_src, uint64 src, uint off, uint n)
@brief xv6 inode 文件系统：提交输入数据并报告传输结果。
@param ip 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param user_src 指明源地址是否属于用户态。
@param src 输入数据起点。
@param off 本次操作使用的 关闭 值。
@param n 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`map_block` → `cache_.read` → `min` → `detail::copy_in` → `bp->bytes` → `journal_.record` → `bp.reset` → `update`。
@details 本函数可见的状态更新包括：`ip->size = off;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`off > ip->size || off + n < off`；`off + n > MAXFILE * BSIZE`；`addr == 0`；`detail::copy_in(bp->bytes() + (off % BSIZE), user_src, src, m) == -1`；`off > ip->size`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`tot`。应结合上面的条件检查区分正常值、空值和错误值。
```
