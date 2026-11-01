# `kernel/memory/slab/slab_cache.cppm`

- [`xv::memory::SlabCache::SlabCache`](slab_cache.cppm.md#fn-4457a1f9a2782e59)
- [`xv::memory::SlabCache::SlabCache`](slab_cache.cppm.md#fn-2f809b9d0100cc66)
- [`xv::memory::SlabCache::operator=`](slab_cache.cppm.md#fn-b204341310fabed8)
- [`xv::memory::SlabCache::name`](slab_cache.cppm.md#fn-b1a1198e084bc177)
- [`xv::memory::SlabCache::~SlabCache`](slab_cache.cppm.md#fn-4dfd7d3e7224c5d8)
- [`xv::memory::SlabCache::prepare_page`](slab_cache.cppm.md#fn-9ee0991b24d49fcb)
- [`xv::memory::SlabCache::take_slot`](slab_cache.cppm.md#fn-49233115439dc0b1)
- [`xv::memory::SlabCache::return_slot`](slab_cache.cppm.md#fn-a35e6d8cb1d16070)
- [`xv::memory::FreeListSlabCache::FreeListSlabCache`](slab_cache.cppm.md#fn-a688ed727874d1ec)
- [`xv::memory::FreeListSlabCache::name`](slab_cache.cppm.md#fn-b8dba7ca8bc8b83d)
- [`xv::memory::BitmapSlabCache::BitmapSlabCache`](slab_cache.cppm.md#fn-7903977f9674b4a4)
- [`xv::memory::BitmapSlabCache::name`](slab_cache.cppm.md#fn-2a131ce7e7fc18a5)

<a id="fn-4457a1f9a2782e59"></a>

## `xv::memory::SlabCache::SlabCache`

```doxygen
@fn constexpr xv::memory::SlabCache::SlabCache() = default
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-2f809b9d0100cc66"></a>

## `xv::memory::SlabCache::SlabCache`

```doxygen
@fn xv::memory::SlabCache::SlabCache(const SlabCache &) = delete
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-b204341310fabed8"></a>

## `xv::memory::SlabCache::operator=`

```doxygen
@fn SlabCache &xv::memory::SlabCache::operator=(const SlabCache &) = delete
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-b1a1198e084bc177"></a>

## `xv::memory::SlabCache::name`

```doxygen
@fn virtual const char *xv::memory::SlabCache::name() const = 0
@brief 返回具体 slab 策略的稳定名称。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-4dfd7d3e7224c5d8"></a>

## `xv::memory::SlabCache::~SlabCache`

```doxygen
@fn xv::memory::SlabCache::~SlabCache() = default
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-9ee0991b24d49fcb"></a>

## `xv::memory::SlabCache::prepare_page`

```doxygen
@fn virtual void xv::memory::SlabCache::prepare_page(Page &page) = 0
@brief 由具体实现定义新 slab 的槽位初始化。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-49233115439dc0b1"></a>

## `xv::memory::SlabCache::take_slot`

```doxygen
@fn virtual unsigned xv::memory::SlabCache::take_slot(Page &page) = 0
@brief 由具体实现选择一个可分配槽位。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-a35e6d8cb1d16070"></a>

## `xv::memory::SlabCache::return_slot`

```doxygen
@fn virtual void xv::memory::SlabCache::return_slot(Page &page, unsigned slot) = 0
@brief 由具体实现维护归还后的空槽状态。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-a688ed727874d1ec"></a>

## `xv::memory::FreeListSlabCache::FreeListSlabCache`

```doxygen
@fn constexpr xv::memory::FreeListSlabCache::FreeListSlabCache() = default
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-b8dba7ca8bc8b83d"></a>

## `xv::memory::FreeListSlabCache::name`

```doxygen
@fn const char *xv::memory::FreeListSlabCache::name() const override
@brief 返回具体 slab 策略的稳定名称。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-7903977f9674b4a4"></a>

## `xv::memory::BitmapSlabCache::BitmapSlabCache`

```doxygen
@fn constexpr xv::memory::BitmapSlabCache::BitmapSlabCache() = default
@brief 声明可替换的固定尺寸对象缓存策略或禁止共享缓存所有权。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-2a131ce7e7fc18a5"></a>

## `xv::memory::BitmapSlabCache::name`

```doxygen
@fn const char *xv::memory::BitmapSlabCache::name() const override
@brief 返回具体 slab 策略的稳定名称。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

- [`xv::memory::allocate_object`](slab_service.cpp.md#fn-8c9c7394dfc70a4f)

- [`xv::memory::configured_slab_kind`](slab_service.cpp.md#fn-426c49f6ae22702c)

- [`xv::memory::deallocate_object`](slab_service.cpp.md#fn-b3ffd68f7c669dcc)

- [`xv::memory::trim_object_caches`](slab_service.cpp.md#fn-62cad3f2729a908b)

- [`xv::memory::BitmapSlabCache::prepare_page`](slab_cache.cpp.md#fn-d1ade02bcba292a6)

- [`xv::memory::BitmapSlabCache::return_slot`](slab_cache.cpp.md#fn-f2d27006b444c953)

- [`xv::memory::BitmapSlabCache::take_slot`](slab_cache.cpp.md#fn-7675ee16018ca775)

- [`xv::memory::FreeListSlabCache::prepare_page`](slab_cache.cpp.md#fn-10b848041e2743c4)

- [`xv::memory::FreeListSlabCache::return_slot`](slab_cache.cpp.md#fn-3c2cbdb56af3999a)

- [`xv::memory::FreeListSlabCache::take_slot`](slab_cache.cpp.md#fn-354b953875e14a3c)

- [`xv::memory::SlabCache::allocate`](slab_cache.cpp.md#fn-48e214e4c232ec57)

- [`xv::memory::SlabCache::deallocate`](slab_cache.cpp.md#fn-116cd4cb490e600a)

- [`xv::memory::SlabCache::initialize`](slab_cache.cpp.md#fn-cdeed7dab456794b)

- [`xv::memory::SlabCache::link_available`](slab_cache.cpp.md#fn-35b94690adfd82cc)

- [`xv::memory::SlabCache::owner_of`](slab_cache.cpp.md#fn-0f4890c255694b3a)

- [`xv::memory::SlabCache::slot_address`](slab_cache.cpp.md#fn-5eab8b2fbd20d3cf)

- [`xv::memory::SlabCache::statistics`](slab_cache.cpp.md#fn-80e1643f01271a2f)

- [`xv::memory::SlabCache::trim`](slab_cache.cpp.md#fn-f536c841ff918335)

- [`xv::memory::SlabCache::unlink_available`](slab_cache.cpp.md#fn-25336609199f53f0)

- [`xv::memory::SlabCache::unlink_empty`](slab_cache.cpp.md#fn-49dbe6b89fb3307f)
