# `kernel/fs/cache/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::LruBlockCache::GetFlow::assign`](flow.cppm.md#fn-f3e6213c6761d624)
- [`xv::fs::LruBlockCache::GetFlow::find_cached`](flow.cppm.md#fn-aef0663a46b9a706)
- [`xv::fs::LruBlockCache::GetFlow::find_unused`](flow.cppm.md#fn-9a908b7f24ac34c4)
- [`xv::fs::LruBlockCache::GetFlow::found`](flow.cppm.md#fn-e16bec18fd5129a4)
- [`xv::fs::LruBlockCache::GetFlow::lock_buffer`](flow.cppm.md#fn-92358b61f0305f12)
- [`xv::fs::LruBlockCache::GetFlow::missing`](flow.cppm.md#fn-3a6b8df2e3965fd1)
- [`xv::fs::LruBlockCache::GetFlow::retain`](flow.cppm.md#fn-e4f9ebd802ac274d)
- [`xv::fs::LruBlockCache::GetFlow::unlock_registry`](flow.cppm.md#fn-cb38bac0ddefb71a)
- [`xv::fs::LruBlockCache::ReleaseFlow::drop`](flow.cppm.md#fn-3eddfdb897b66873)
- [`xv::fs::LruBlockCache::ReleaseFlow::held`](flow.cppm.md#fn-12b469235df9ae8b)
- [`xv::fs::LruBlockCache::ReleaseFlow::invalid`](flow.cppm.md#fn-3c84379676e8cc59)
- [`xv::fs::LruBlockCache::ReleaseFlow::move_front`](flow.cppm.md#fn-ebeaca335eefb03c)
- [`xv::fs::LruBlockCache::ReleaseFlow::unlock_buffer`](flow.cppm.md#fn-cf6ba582715e3f19)
- [`xv::fs::LruBlockCache::ReleaseFlow::unlock_registry`](flow.cppm.md#fn-f8f6571306a36325)
- [`xv::fs::LruBlockCache::ReleaseFlow::unused`](flow.cppm.md#fn-dcd1734401a1fbac)

<a id="fn-f3e6213c6761d624"></a>

## `xv::fs::LruBlockCache::GetFlow::assign`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::assign(Context &c) noexcept
@brief 迁移零引用候选的身份并发布到目标桶。
@details 迁移零引用候选的身份并发布到目标桶。
```

<a id="fn-aef0663a46b9a706"></a>

## `xv::fs::LruBlockCache::GetFlow::find_cached`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::find_cached(Context &c) noexcept
@brief 定位目标哈希桶并在桶锁下查找键。
@details 定位目标哈希桶并在桶锁下查找键。
```

<a id="fn-9a908b7f24ac34c4"></a>

## `xv::fs::LruBlockCache::GetFlow::find_unused`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::find_unused(Context &c) noexcept
@brief 开始挑选未引用的淘汰候选。
@details 开始挑选未引用的淘汰候选。
```

<a id="fn-e16bec18fd5129a4"></a>

## `xv::fs::LruBlockCache::GetFlow::found`

```doxygen
@fn static bool xv::fs::LruBlockCache::GetFlow::found(Context &c) noexcept
@brief 检查本阶段是否选中了缓冲。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-92358b61f0305f12"></a>

## `xv::fs::LruBlockCache::GetFlow::lock_buffer`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::lock_buffer(Context &c) noexcept
@brief 取得已保留缓冲的睡眠锁。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3a6b8df2e3965fd1"></a>

## `xv::fs::LruBlockCache::GetFlow::missing`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::missing(Context &c) noexcept
@brief 释放持有的管理锁并报告缓存容量耗尽。
@details 释放持有的管理锁并报告缓存容量耗尽。
```

<a id="fn-e4f9ebd802ac274d"></a>

## `xv::fs::LruBlockCache::GetFlow::retain`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::retain(Context &c) noexcept
@brief 记录第一次查询命中并增加引用。
@details 记录第一次查询命中并增加引用。
```

<a id="fn-cb38bac0ddefb71a"></a>

## `xv::fs::LruBlockCache::GetFlow::unlock_registry`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::unlock_registry(Context &c) noexcept
@brief 释放目标桶锁。
@details 释放目标桶锁。
```

<a id="fn-3eddfdb897b66873"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::drop`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::drop(Context &c) noexcept
@brief 持所属桶锁减少引用；已有引用保证身份在归还前固定。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-12b469235df9ae8b"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::held`

```doxygen
@fn static bool xv::fs::LruBlockCache::ReleaseFlow::held(Context &c) noexcept
@brief 检查调用者是否持有块内容 SleepLock。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-3c84379676e8cc59"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::invalid`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::invalid(Context &) noexcept
@brief 报告归还未持内容锁缓冲块的错误。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-ebeaca335eefb03c"></a>

此旧全局链表阶段已由哈希索引和独立淘汰管理替代，保留历史锚点；当前实现见 [块缓存](../../../../block-cache.md)。

<a id="fn-cf6ba582715e3f19"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unlock_buffer`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::unlock_buffer(Context &c) noexcept
@brief 释放块内容 SleepLock，随后才能进入所属桶的管理锁。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-f8f6571306a36325"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unlock_registry`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::unlock_registry(Context &c) noexcept
@brief 释放所属桶锁，结束归还。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-dcd1734401a1fbac"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unused`

```doxygen
@fn static bool xv::fs::LruBlockCache::ReleaseFlow::unused(Context &c) noexcept
@brief 检查归还后引用数是否为零。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

<a id="fn-585f62105a41c254"></a>

## `xv::fs::LruBlockCache::GetFlow::lock_victim`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::lock_victim(Context &c) noexcept
@brief 在淘汰锁内获取候选原身份桶的锁。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-1efba38a56221817"></a>

## `xv::fs::LruBlockCache::GetFlow::lookup`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::lookup(Context &c) noexcept
@brief 遍历目标桶并累计本轮比较次数。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-1ff00c73b2766d83"></a>

## `xv::fs::LruBlockCache::GetFlow::miss`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::miss(Context &c) noexcept
@brief 记录未命中，释放桶锁后获取独立淘汰锁。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-647bf56813a080ef"></a>

## `xv::fs::LruBlockCache::GetFlow::recheck`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::recheck(Context &c) noexcept
@brief 取得替换所有权后重新检查同键身份。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-ef9b4a64cf65fcf4"></a>

## `xv::fs::LruBlockCache::GetFlow::reference`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::reference(Context &c) noexcept
@brief 在已持桶锁下增加引用以固定身份。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-9fb0efe57e164b9e"></a>

## `xv::fs::LruBlockCache::GetFlow::retry`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::retry(Context &c) noexcept
@brief 释放原桶锁并重选候选。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-45024515386c6560"></a>

## `xv::fs::LruBlockCache::GetFlow::select`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::select(Context &c) noexcept
@brief 批量排序归还时间戳并丢弃已失效候选。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-85e3c9514f5469bc"></a>

## `xv::fs::LruBlockCache::GetFlow::unlock_eviction`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::unlock_eviction(Context &c) noexcept
@brief 释放独立淘汰锁。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-ab7dfc3f63b66782"></a>

## `xv::fs::LruBlockCache::GetFlow::unused`

```doxygen
@fn static bool xv::fs::LruBlockCache::GetFlow::unused(Context &c) noexcept
@brief 复核候选仍为零引用且时间戳未改变。
@details 哈希命中增加引用后释放桶锁，再等待块 SleepLock；未命中先释放目标桶再获取淘汰锁并重查，合并同键并发请求。替换只在淘汰锁下同时持目标/原桶锁，复核零引用和候选时间戳后迁移身份。候选批次按归还时间排序，近似 LRU；全部被引用时保留容量耗尽错误。归还不维护全局链表。
```

<a id="fn-a5e6392e37b35cb8"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::stamp`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::stamp(Context &c) noexcept
@brief 在最后归还时保存时间戳，供淘汰候选重新验证。
@details 归还先释放 SleepLock，再获取所属桶锁减少引用；零引用时更新时间戳，不移动全局链表。引用与时间戳由桶锁串行；淘汰方同时持淘汰锁并复核二者，禁止复用活跃或固定块。
```

- [`xv::fs::LruBlockCache::GetFlow::lock_victim`](flow.cppm.md#fn-585f62105a41c254)

- [`xv::fs::LruBlockCache::GetFlow::lookup`](flow.cppm.md#fn-1efba38a56221817)

- [`xv::fs::LruBlockCache::GetFlow::miss`](flow.cppm.md#fn-1ff00c73b2766d83)

- [`xv::fs::LruBlockCache::GetFlow::recheck`](flow.cppm.md#fn-647bf56813a080ef)

- [`xv::fs::LruBlockCache::GetFlow::reference`](flow.cppm.md#fn-ef9b4a64cf65fcf4)

- [`xv::fs::LruBlockCache::GetFlow::retry`](flow.cppm.md#fn-9fb0efe57e164b9e)

- [`xv::fs::LruBlockCache::GetFlow::select`](flow.cppm.md#fn-45024515386c6560)

- [`xv::fs::LruBlockCache::GetFlow::unlock_eviction`](flow.cppm.md#fn-85e3c9514f5469bc)

- [`xv::fs::LruBlockCache::GetFlow::unused`](flow.cppm.md#fn-ab7dfc3f63b66782)

- [`xv::fs::LruBlockCache::ReleaseFlow::stamp`](flow.cppm.md#fn-a5e6392e37b35cb8)
