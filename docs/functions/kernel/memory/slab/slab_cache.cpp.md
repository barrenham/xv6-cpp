# `kernel/memory/slab/slab_cache.cpp`

- [`xv::memory::SlabCache::initialize`](slab_cache.cpp.md#fn-cdeed7dab456794b)
- [`xv::memory::SlabCache::slot_address`](slab_cache.cpp.md#fn-5eab8b2fbd20d3cf)
- [`xv::memory::SlabCache::link_available`](slab_cache.cpp.md#fn-35b94690adfd82cc)
- [`xv::memory::SlabCache::unlink_available`](slab_cache.cpp.md#fn-25336609199f53f0)
- [`xv::memory::SlabCache::unlink_empty`](slab_cache.cpp.md#fn-49dbe6b89fb3307f)
- [`xv::memory::SlabCache::allocate`](slab_cache.cpp.md#fn-48e214e4c232ec57)
- [`xv::memory::SlabCache::owner_of`](slab_cache.cpp.md#fn-0f4890c255694b3a)
- [`xv::memory::SlabCache::deallocate`](slab_cache.cpp.md#fn-116cd4cb490e600a)
- [`xv::memory::SlabCache::trim`](slab_cache.cpp.md#fn-f536c841ff918335)
- [`xv::memory::SlabCache::statistics`](slab_cache.cpp.md#fn-80e1643f01271a2f)
- [`xv::memory::FreeListSlabCache::prepare_page`](slab_cache.cpp.md#fn-10b848041e2743c4)
- [`xv::memory::FreeListSlabCache::take_slot`](slab_cache.cpp.md#fn-354b953875e14a3c)
- [`xv::memory::FreeListSlabCache::return_slot`](slab_cache.cpp.md#fn-3c2cbdb56af3999a)
- [`xv::memory::BitmapSlabCache::prepare_page`](slab_cache.cpp.md#fn-d1ade02bcba292a6)
- [`xv::memory::BitmapSlabCache::take_slot`](slab_cache.cpp.md#fn-7675ee16018ca775)
- [`xv::memory::BitmapSlabCache::return_slot`](slab_cache.cpp.md#fn-f2d27006b444c953)

<a id="fn-cdeed7dab456794b"></a>

## `xv::memory::SlabCache::initialize`

```doxygen
@fn bool xv::memory::SlabCache::initialize(PhysicalPageAllocator &pages, unsigned long bytes, unsigned long alignment, unsigned empty_limit)
@brief 在发布前验证对象大小、对齐及页内容量，绑定物理分配器和空页上限，不提前申请 slab 页。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-5eab8b2fbd20d3cf"></a>

## `xv::memory::SlabCache::slot_address`

```doxygen
@fn void *xv::memory::SlabCache::slot_address(Page &page, unsigned slot) const
@brief 根据已验证的页头偏移、槽步长和槽号计算页内对象地址。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-35b94690adfd82cc"></a>

## `xv::memory::SlabCache::link_available`

```doxygen
@fn void xv::memory::SlabCache::link_available(Page &page)
@brief 将非满页链接到可分配页链表头部并维护双向链接。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-25336609199f53f0"></a>

## `xv::memory::SlabCache::unlink_available`

```doxygen
@fn void xv::memory::SlabCache::unlink_available(Page &page)
@brief 常数时间移除可分配页链表中的页面并清理链接。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-49dbe6b89fb3307f"></a>

## `xv::memory::SlabCache::unlink_empty`

```doxygen
@fn void xv::memory::SlabCache::unlink_empty(Page &page)
@brief 常数时间摘除空页并减少空页计数。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-48e214e4c232ec57"></a>

## `xv::memory::SlabCache::allocate`

```doxygen
@fn void *xv::memory::SlabCache::allocate()
@brief 锁内选取非满页，必要时申请并初始化一页；通过策略虚方法取得槽位，更新活跃位、计数与链表。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-0f4890c255694b3a"></a>

## `xv::memory::SlabCache::owner_of`

```doxygen
@fn SlabCache *xv::memory::SlabCache::owner_of(void *object)
@brief 从已映射 slab 存储的页头读取所属缓存；不是任意地址探测接口。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-116cd4cb490e600a"></a>

## `xv::memory::SlabCache::deallocate`

```doxygen
@fn bool xv::memory::SlabCache::deallocate(void *object)
@brief 校验所属缓存、槽边界和活跃位，归还槽位；按空页上限保留或即时归还最后一个对象释放后的页面。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-f536c841ff918335"></a>

## `xv::memory::SlabCache::trim`

```doxygen
@fn unsigned long xv::memory::SlabCache::trim()
@brief 仅遍历空页链表并归还全部空页，不移动或释放活跃对象。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-80e1643f01271a2f"></a>

## `xv::memory::SlabCache::statistics`

```doxygen
@fn SlabStatistics xv::memory::SlabCache::statistics()
@brief 取得缓存锁内的对象、底层页申请、回收及保留空页统计快照。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-10b848041e2743c4"></a>

## `xv::memory::FreeListSlabCache::prepare_page`

```doxygen
@fn void xv::memory::FreeListSlabCache::prepare_page(Page &page)
@brief 初始化页内空闲索引链表。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-354b953875e14a3c"></a>

## `xv::memory::FreeListSlabCache::take_slot`

```doxygen
@fn unsigned xv::memory::FreeListSlabCache::take_slot(Page &page)
@brief 从页内空闲索引链表弹出一个槽号。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-3c2cbdb56af3999a"></a>

## `xv::memory::FreeListSlabCache::return_slot`

```doxygen
@fn void xv::memory::FreeListSlabCache::return_slot(Page &page, unsigned slot)
@brief 将槽号压回页内空闲索引链表。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-d1ade02bcba292a6"></a>

## `xv::memory::BitmapSlabCache::prepare_page`

```doxygen
@fn void xv::memory::BitmapSlabCache::prepare_page(Page &)
@brief 页头位图已由页面构造清零，无需逐对象初始化。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-7675ee16018ca775"></a>

## `xv::memory::BitmapSlabCache::take_slot`

```doxygen
@fn unsigned xv::memory::BitmapSlabCache::take_slot(Page &page)
@brief 在最多四个机器字中通过 ctz 选择空槽，不扫描每个对象。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-f2d27006b444c953"></a>

## `xv::memory::BitmapSlabCache::return_slot`

```doxygen
@fn void xv::memory::BitmapSlabCache::return_slot(Page &, unsigned)
@brief 公共层已经清除活跃位，位图策略无需额外归还操作。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```
