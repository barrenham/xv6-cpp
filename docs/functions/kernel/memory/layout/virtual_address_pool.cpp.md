# `kernel/memory/layout/virtual_address_pool.cpp`

源码：[打开实际文件](../../../../../kernel/memory/layout/virtual_address_pool.cpp)。

该文件的红黑树或地址池职责见 [内核虚拟地址池](../../../../virtual-address-pool.md)。

## 函数导航

- [`xv::memory::VirtualAddressPool::allocation`](virtual_address_pool.cpp.md#fn-d6a9e9d61906627d)
- [`xv::memory::VirtualAddressPool::initialize`](virtual_address_pool.cpp.md#fn-0009ac3368aaa857)
- [`xv::memory::VirtualAddressPool::largest_free`](virtual_address_pool.cpp.md#fn-ba3cdfa791efba66)
- [`xv::memory::VirtualAddressPool::put_node`](virtual_address_pool.cpp.md#fn-017f88ea27af8b28)
- [`xv::memory::VirtualAddressPool::release`](virtual_address_pool.cpp.md#fn-9806246ea6f6e910)
- [`xv::memory::VirtualAddressPool::reset`](virtual_address_pool.cpp.md#fn-11eceb419e804590)
- [`xv::memory::VirtualAddressPool::take_node`](virtual_address_pool.cpp.md#fn-70afa19e0dd61a33)
- [`xv::memory::VirtualAddressPool::valid`](virtual_address_pool.cpp.md#fn-fc0fb0f7e9a3c257)
- [`xv::memory::VirtualAddressPool::attributes`](virtual_address_pool.cpp.md#fn-9c1ea98aae4262f7)
- [`xv::memory::VirtualAddressPool::containing`](virtual_address_pool.cpp.md#fn-0c09f4324cc78221)
- [`xv::memory::VirtualAddressPool::find`](virtual_address_pool.cpp.md#fn-2515eb287d0d4264)
- [`xv::memory::VirtualAddressPool::reserve`](virtual_address_pool.cpp.md#fn-d5e884582fd0f65b)
- [`xv::memory::VirtualAddressPool::fit`](virtual_address_pool.cpp.md#fn-7cd88255501b3ed7)
- [`xv::memory::VirtualAddressPool::allocate`](virtual_address_pool.cpp.md#fn-f591d715372f863d)
- [`xv::memory::VirtualAddressPool::allocate_in`](virtual_address_pool.cpp.md#fn-e689223bfa6884ad)
- [`xv::memory::VirtualAddressPool::protect`](virtual_address_pool.cpp.md#fn-99c9de4880b135f1)
- [`xv::memory::VirtualAddressPool::return_free`](virtual_address_pool.cpp.md#fn-26ab46b6280b246c)
- [`xv::memory::VirtualAddressPool::erase`](virtual_address_pool.cpp.md#fn-f443c68878d0d459)
- [`xv::memory::VirtualAddressPool::copy_to`](virtual_address_pool.cpp.md#fn-f51d1468efd29191)
- [`xv::memory::VirtualAddressPool::swap`](virtual_address_pool.cpp.md#fn-76152a5847515bfc)

- [`xv::memory::prepare_pool_nodes`](virtual_address_pool.cpp.md#fn-7520c7b62288f91d)

<a id="fn-70afa19e0dd61a33"></a>

## `xv::memory::VirtualAddressPool::take_node`

```doxygen
@fn VirtualAddressPool::Node *xv::memory::VirtualAddressPool::take_node()
@brief 从所有地址池共享的精确尺寸 slab 缓存构造一个区间节点，首次使用时原子初始化缓存。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-017f88ea27af8b28"></a>

## `xv::memory::VirtualAddressPool::put_node`

```doxygen
@fn void xv::memory::VirtualAddressPool::put_node(Node *node)
@brief 销毁已摘树的区间节点并交回共享 slab，最后一个活跃槽位释放后归还物理页。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-0009ac3368aaa857"></a>

## `xv::memory::VirtualAddressPool::initialize`

```doxygen
@fn bool xv::memory::VirtualAddressPool::initialize(unsigned long begin, unsigned long end, unsigned long granule)
@brief 初始化完整地址窗口及一个空闲节点，支持地址 0；非法参数、重复初始化和元数据不足均不改变原状态。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-9806246ea6f6e910"></a>

## `xv::memory::VirtualAddressPool::release`

```doxygen
@fn bool xv::memory::VirtualAddressPool::release(unsigned long begin)
@brief 只接受已占用区间的准确起点，复用原节点归还空间并合并邻居，不申请物理内存。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-d6a9e9d61906627d"></a>

## `xv::memory::VirtualAddressPool::allocation`

```doxygen
@fn bool xv::memory::VirtualAddressPool::allocation(unsigned long begin, unsigned long &end) const
@brief 按准确起点查询独立已占用区间的终点，拒绝内部地址。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-ba3cdfa791efba66"></a>

## `xv::memory::VirtualAddressPool::largest_free`

```doxygen
@fn unsigned long xv::memory::VirtualAddressPool::largest_free() const
@brief 返回空闲树根的最大区间聚合值，空树返回零。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```

<a id="fn-11eceb419e804590"></a>

## `xv::memory::VirtualAddressPool::reset`

```doxygen
@fn void xv::memory::VirtualAddressPool::reset()
@brief 清空两棵树并释放全部元数据，恢复未初始化状态；映射和物理数据帧仍由调用者管理。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```


<a id="fn-fc0fb0f7e9a3c257"></a>

## `xv::memory::VirtualAddressPool::valid`

```doxygen
@fn bool xv::memory::VirtualAddressPool::valid(unsigned long begin, unsigned long end) const
@brief 校验初始化状态、地址窗口、半开边界及粒度约束。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-9c1ea98aae4262f7"></a>

## `xv::memory::VirtualAddressPool::attributes`

```doxygen
@fn bool xv::memory::VirtualAddressPool::attributes(unsigned permissions, RegionKind kind, bool demand)
@brief 校验 R/W/X/U 权限、写需可读以及保护区不可访问且不可按需补页的约束。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-0c09f4324cc78221"></a>

## `xv::memory::VirtualAddressPool::containing`

```doxygen
@fn VirtualAddressPool::Node *xv::memory::VirtualAddressPool::containing(const Tree &tree, unsigned long address)
@brief 通过 lower_bound 和前驱查找包含地址的半开区间。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-2515eb287d0d4264"></a>

## `xv::memory::VirtualAddressPool::find`

```doxygen
@fn const VirtualRegion *xv::memory::VirtualAddressPool::find(unsigned long address) const
@brief 通过已占用红黑树按包含关系查询区域及权限、用途和按需策略。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-d5e884582fd0f65b"></a>

## `xv::memory::VirtualAddressPool::reserve`

```doxygen
@fn bool xv::memory::VirtualAddressPool::reserve(unsigned long begin, unsigned long end, unsigned permissions, RegionKind kind, bool demand)
@brief 在指定地址预留带属性区间；重叠、非法权限或元数据不足时不修改区间树。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-7cd88255501b3ed7"></a>

## `xv::memory::VirtualAddressPool::fit`

```doxygen
@fn VirtualAddressPool::Node *xv::memory::VirtualAddressPool::fit(Node *node, unsigned long bytes, unsigned long minimum, unsigned long maximum, unsigned long alignment, unsigned long &address)
@brief 按地址顺序搜索满足窗口、长度及对齐的区间，利用子树最大长度剪枝，递归深度受树高约束。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-f591d715372f863d"></a>

## `xv::memory::VirtualAddressPool::allocate`

```doxygen
@fn unsigned long xv::memory::VirtualAddressPool::allocate(unsigned long bytes, unsigned permissions, RegionKind kind, bool demand)
@brief 在统一空闲树中首适应分配并记录权限、用途和按需策略；自动分配跳过地址 0，失败返回 0。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-e689223bfa6884ad"></a>

## `xv::memory::VirtualAddressPool::allocate_in`

```doxygen
@fn unsigned long xv::memory::VirtualAddressPool::allocate_in(unsigned long bytes, unsigned long minimum, unsigned long maximum, unsigned long alignment, unsigned permissions, RegionKind kind, bool demand)
@brief 在指定窗口及对齐约束下寻找首个可用区间，准备全部分裂元数据后提交占用及属性。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-99c9de4880b135f1"></a>

## `xv::memory::VirtualAddressPool::protect`

```doxygen
@fn bool xv::memory::VirtualAddressPool::protect(unsigned long begin, unsigned long end, unsigned permissions, RegionKind kind, bool demand)
@brief 先准备最多两个边界节点，再原子拆分一个已占用区间并修改目标片段属性。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-26ab46b6280b246c"></a>

## `xv::memory::VirtualAddressPool::return_free`

```doxygen
@fn void xv::memory::VirtualAddressPool::return_free(Node *node)
@brief 复用归还节点并合并相邻空闲区间，清除业务属性并即时回收空元数据页。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-f443c68878d0d459"></a>

## `xv::memory::VirtualAddressPool::erase`

```doxygen
@fn bool xv::memory::VirtualAddressPool::erase(unsigned long begin, unsigned long end)
@brief 预先准备边界分裂节点，删除范围内的已占用片段并与空闲邻居合并；失败保留原布局。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-f51d1468efd29191"></a>

## `xv::memory::VirtualAddressPool::copy_to`

```doxygen
@fn bool xv::memory::VirtualAddressPool::copy_to(VirtualAddressPool &other) const
@brief 构造带完整窗口及属性的候选池，成功才交换目标；失败保留目标，自复制直接成功。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-76152a5847515bfc"></a>

## `xv::memory::VirtualAddressPool::swap`

```doxygen
@fn void xv::memory::VirtualAddressPool::swap(VirtualAddressPool &other)
@brief 交换两棵树的根和窗口参数，转移节点所有权；共享 slab 无需迁移或交换页链表。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-7520c7b62288f91d"></a>

## `xv::memory::prepare_pool_nodes`

```doxygen
@fn void xv::memory::prepare_pool_nodes(unsigned long bytes, unsigned long alignment)
@brief 通过原子初始化协议准备所有地址池共享的精确尺寸节点缓存，不预分配物理页。
@details 原始对象的构造和析构由调用者负责；初始化在发布前完成，已发布缓存操作由自旋锁串行。布局、回收和测量范围见 docs/slab.md。
```
