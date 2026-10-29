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
@brief 设置新设备和块号并使数据无效、引用计数为一。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-aef0663a46b9a706"></a>

## `xv::fs::LruBlockCache::GetFlow::find_cached`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::find_cached(Context &c) noexcept
@brief 直接按原顺序查找指定设备和块号。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-9a908b7f24ac34c4"></a>

## `xv::fs::LruBlockCache::GetFlow::find_unused`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::find_unused(Context &c) noexcept
@brief 从 LRU 尾部直接查找首个零引用缓冲。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
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
@fn static void xv::fs::LruBlockCache::GetFlow::missing(Context &) noexcept
@brief 诊断没有可回收缓存缓冲。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-e4f9ebd802ac274d"></a>

## `xv::fs::LruBlockCache::GetFlow::retain`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::retain(Context &c) noexcept
@brief 增加已缓存缓冲的引用计数。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-cb38bac0ddefb71a"></a>

## `xv::fs::LruBlockCache::GetFlow::unlock_registry`

```doxygen
@fn static void xv::fs::LruBlockCache::GetFlow::unlock_registry(Context &c) noexcept
@brief 释放缓存登记自旋锁。
@details 入口持登记锁；缓存命中或选择空闲 LRU 后先增加引用，再释放登记锁，最后取得可睡眠的缓冲锁。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3eddfdb897b66873"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::drop`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::drop(Context &c) noexcept
@brief 取得登记自旋锁并减少缓冲引用。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-12b469235df9ae8b"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::held`

```doxygen
@fn static bool xv::fs::LruBlockCache::ReleaseFlow::held(Context &c) noexcept
@brief 检查调用者是否持有缓冲锁。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-3c84379676e8cc59"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::invalid`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::invalid(Context &) noexcept
@brief 诊断释放未持锁缓冲。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-ebeaca335eefb03c"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::move_front`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::move_front(Context &c) noexcept
@brief 直接更新链表指针将缓冲移到 MRU 头部。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-cf6ba582715e3f19"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unlock_buffer`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::unlock_buffer(Context &c) noexcept
@brief 释放缓冲睡眠锁。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-f8f6571306a36325"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unlock_registry`

```doxygen
@fn static void xv::fs::LruBlockCache::ReleaseFlow::unlock_registry(Context &c) noexcept
@brief 释放缓存登记自旋锁。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-dcd1734401a1fbac"></a>

## `xv::fs::LruBlockCache::ReleaseFlow::unused`

```doxygen
@fn static bool xv::fs::LruBlockCache::ReleaseFlow::unused(Context &c) noexcept
@brief 检查是否已没有持有者或 pin。
@details 先释放缓冲睡眠锁，再取得登记锁减少引用；只有变成零引用的缓冲才移动到 MRU 头部。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```
