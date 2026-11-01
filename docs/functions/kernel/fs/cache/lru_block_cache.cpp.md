# `kernel/fs/cache/lru_block_cache.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/cache/lru_block_cache.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:lru`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::LruBlockCache::get`](lru_block_cache.cpp.md#fn-af4d8f5d14b2fff0)
- [`xv::fs::LruBlockCache::initialize`](lru_block_cache.cpp.md#fn-3cd05c2218bac5b8)
- [`xv::fs::LruBlockCache::pin`](lru_block_cache.cpp.md#fn-752c3a0a65094c6c)
- [`xv::fs::LruBlockCache::read`](lru_block_cache.cpp.md#fn-6f7b9e574580bc22)
- [`xv::fs::LruBlockCache::release`](lru_block_cache.cpp.md#fn-c5ecc882a204c8ff)
- [`xv::fs::LruBlockCache::unpin`](lru_block_cache.cpp.md#fn-1bfa0103c3bfaf36)
- [`xv::fs::LruBlockCache::write`](lru_block_cache.cpp.md#fn-82145096d9d76c5e)

<a id="fn-3cd05c2218bac5b8"></a>

## `xv::fs::LruBlockCache::initialize`

```doxygen
@fn void xv::fs::LruBlockCache::initialize()
@brief 在无并发借用的初始化阶段建立哈希桶、候选批次和未分配缓冲块。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-af4d8f5d14b2fff0"></a>

## `xv::fs::LruBlockCache::get`

```doxygen
@fn Buffer *xv::fs::LruBlockCache::get(unsigned device, unsigned block)
@brief 运行 GetFlow，返回已持内容锁且身份被引用固定的缓冲块。
@param device 块设备编号。
@param block 本次操作使用的 blockno 值。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-6f7b9e574580bc22"></a>

## `xv::fs::LruBlockCache::read`

```doxygen
@fn BlockHandle xv::fs::LruBlockCache::read(unsigned dev, unsigned blockno)
@brief 取得缓存块，仅在内容无效时读取设备，并返回持锁句柄。
@param dev 块设备编号。
@param blockno 本次操作使用的 blockno 值。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-82145096d9d76c5e"></a>

## `xv::fs::LruBlockCache::write`

```doxygen
@fn void xv::fs::LruBlockCache::write(Buffer &buffer)
@brief 要求调用者持内容锁，计数并向后端写出整个块。
@param buffer 接收或提交的数据缓冲。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-c5ecc882a204c8ff"></a>

## `xv::fs::LruBlockCache::release`

```doxygen
@fn void xv::fs::LruBlockCache::release(Buffer &buffer)
@brief 运行 ReleaseFlow，释放内容锁并归还引用；最后归还更新时间戳。
@param buffer 接收或提交的数据缓冲。
@details 先释放 SleepLock，再持所属桶锁减少引用；引用归零时记录时间戳供批量候选复核，不修改全局 LRU 链表。
```

<a id="fn-752c3a0a65094c6c"></a>

## `xv::fs::LruBlockCache::pin`

```doxygen
@fn void xv::fs::LruBlockCache::pin(Buffer &buffer)
@brief 在已有引用固定身份的前提下增加 pin 引用，阻止淘汰。
@param buffer 接收或提交的数据缓冲。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-1bfa0103c3bfaf36"></a>

## `xv::fs::LruBlockCache::unpin`

```doxygen
@fn void xv::fs::LruBlockCache::unpin(Buffer &buffer)
@brief 撤销已有 pin 引用，最后一个引用消失时更新时间戳。
@param buffer 接收或提交的数据缓冲。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-53ecf27400f3e0ad"></a>

## `xv::fs::LruBlockCache::acquire`

```doxygen
@fn void xv::fs::LruBlockCache::acquire(xv::sync::SpinLock &lock, CacheStatistics &statistics)
@brief 获取缓存锁，仅在首次竞争后测量等待与自旋。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

<a id="fn-5694eb576f5299cc"></a>

## `xv::fs::LruBlockCache::statistics`

```doxygen
@fn CacheStatistics xv::fs::LruBlockCache::statistics() const
@brief 按桶和淘汰管理器汇总累计统计，不冻结其他 CPU。
@details 初始化只在无并发借用时进行；命中使用桶锁，替换由独立淘汰锁串行。refcnt 非零固定身份，pin/unpin 需要已有引用。最后归还或 unpin 更新原子时间戳；SleepLock 保护内容与设备 I/O，等待期间不持缓存自旋锁。统计逐桶汇总而非原子事务。
```

- [`xv::fs::LruBlockCache::acquire`](lru_block_cache.cpp.md#fn-53ecf27400f3e0ad)

- [`xv::fs::LruBlockCache::statistics`](lru_block_cache.cpp.md#fn-5694eb576f5299cc)
