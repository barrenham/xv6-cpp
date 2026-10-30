# `kernel/memory/slab/slab_service.cpp`

- [`xv::memory::configured_slab_kind`](slab_service.cpp.md#fn-426c49f6ae22702c)
- [`xv::memory::allocate_object`](slab_service.cpp.md#fn-8c9c7394dfc70a4f)
- [`xv::memory::deallocate_object`](slab_service.cpp.md#fn-b3ffd68f7c669dcc)
- [`xv::memory::trim_object_caches`](slab_service.cpp.md#fn-62cad3f2729a908b)
- [`xv::memory::cache_at`](slab_service.cpp.md#fn-814d5d13fc341da6)
- [`xv::memory::prepare_caches`](slab_service.cpp.md#fn-554fa3704ea7e301)

<a id="fn-426c49f6ae22702c"></a>

## `xv::memory::configured_slab_kind`

```doxygen
@fn SlabKind xv::memory::configured_slab_kind()
@brief 返回构建配置选中的空闲链表或位图 slab 实现，独立于物理页分配器选择。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-8c9c7394dfc70a4f"></a>

## `xv::memory::allocate_object`

```doxygen
@fn void *xv::memory::allocate_object(unsigned long bytes, unsigned long alignment)
@brief 验证大小与对齐并选择 16 到 2048 字节尺寸类，通过选中实现分配原始对象存储。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-b3ffd68f7c669dcc"></a>

## `xv::memory::deallocate_object`

```doxygen
@fn bool xv::memory::deallocate_object(void *object)
@brief 验证 RAM 地址及已知尺寸类缓存归属，再调用所属 slab 的归还接口。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-62cad3f2729a908b"></a>

## `xv::memory::trim_object_caches`

```doxygen
@fn unsigned long xv::memory::trim_object_caches()
@brief 逐个回收生产尺寸类中的空页并累计实际归还数量。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-814d5d13fc341da6"></a>

## `xv::memory::cache_at`

```doxygen
@fn SlabCache &xv::memory::cache_at(unsigned index)
@brief 按尺寸类索引访问构建时选中的多态缓存实例。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```

<a id="fn-554fa3704ea7e301"></a>

## `xv::memory::prepare_caches`

```doxygen
@fn void xv::memory::prepare_caches()
@brief 通过原子首次初始化协议设置八个尺寸类，使用 acquire/release 发布完整状态，不申请对象页。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```
