# `kernel/memory/allocator/buddy/buddy_page_allocator.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../../kernel/memory/allocator/buddy/buddy_page_allocator.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:buddy`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::BuddyPageAllocator::allocate`](buddy_page_allocator.cpp.md#fn-0e4aa39ee90982b8)
- [`xv::memory::BuddyPageAllocator::claim`](buddy_page_allocator.cpp.md#fn-6f33683dd35acc36)
- [`xv::memory::BuddyPageAllocator::deallocate`](buddy_page_allocator.cpp.md#fn-3eb32f523d5d265d)
- [`xv::memory::BuddyPageAllocator::initialize_layout`](buddy_page_allocator.cpp.md#fn-2d9e53492f4ac4fd)
- [`xv::memory::BuddyPageAllocator::insert`](buddy_page_allocator.cpp.md#fn-4e685c08c7401b4d)
- [`xv::memory::BuddyPageAllocator::page_tag`](buddy_page_allocator.cpp.md#fn-c14f515bed1c0679)
- [`xv::memory::BuddyPageAllocator::remove_block`](buddy_page_allocator.cpp.md#fn-de65a654acaf47be)
- [`xv::memory::BuddyPageAllocator::set_tag`](buddy_page_allocator.cpp.md#fn-7257ac4207c21591)
- [`xv::memory::BuddyPageAllocator::split_to_pages`](buddy_page_allocator.cpp.md#fn-98b0a2320cb39511)
- [`xv::memory::BuddyPageAllocator::valid_block`](buddy_page_allocator.cpp.md#fn-904352b78a66a7f9)

<a id="fn-c14f515bed1c0679"></a>

## `xv::memory::BuddyPageAllocator::page_tag`

```doxygen
@fn int xv::memory::BuddyPageAllocator::page_tag(PageIndex index) const
@brief 伙伴物理页分配器：读取或计算 `__atomic_load_n(&tags_[index], __ATOMIC_ACQUIRE)`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&tags_[index], __ATOMIC_ACQUIRE)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7257ac4207c21591"></a>

## `xv::memory::BuddyPageAllocator::set_tag`

```doxygen
@fn void xv::memory::BuddyPageAllocator::set_tag(PageIndex index, int tag)
@brief 伙伴物理页分配器：设置页标记。
@param index 数组、CPU 或页面的索引。
@param tag 本次操作使用的 页标记 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-904352b78a66a7f9"></a>

## `xv::memory::BuddyPageAllocator::valid_block`

```doxygen
@fn bool xv::memory::BuddyPageAllocator::valid_block(PhysicalAddress pa, unsigned order) const
@brief 伙伴物理页分配器：验证物理块范围、对齐和所有权状态。
@param pa 对应的物理地址。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::block_size` → `pa.is_aligned` → `layout_.contains_usable_block`。
@details 需要特别跟踪的条件：`order > SuperpageOrder`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`pa.is_aligned(size) && layout_.contains_usable_block(pa, size)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4e685c08c7401b4d"></a>

## `xv::memory::BuddyPageAllocator::insert`

```doxygen
@fn void xv::memory::BuddyPageAllocator::insert(PhysicalAddress pa, unsigned order)
@brief 伙伴物理页分配器：把空闲块插入对应阶链表。
@param pa 对应的物理地址。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`set_tag` → `layout_.ram` → `page_index`。
@details 本函数可见的状态更新包括：`r->prev = 0;`；`r->next = area_[order].head;`；`r->next->prev = r;`；`area_[order].head = r;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`r->next`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-de65a654acaf47be"></a>

## `xv::memory::BuddyPageAllocator::remove_block`

```doxygen
@fn void xv::memory::BuddyPageAllocator::remove_block(PhysicalAddress pa, unsigned order)
@brief 伙伴物理页分配器：从相应空闲阶链表摘除一个块。
@param pa 对应的物理地址。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`set_tag` → `layout_.ram` → `page_index`。
@details 本函数可见的状态更新包括：`r->prev->next = r->next;`；`area_[order].head = r->next;`；`r->next->prev = r->prev;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`r->prev`；`r->next`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0e4aa39ee90982b8"></a>

## `xv::memory::BuddyPageAllocator::allocate`

```doxygen
@fn void *xv::memory::BuddyPageAllocator::allocate(unsigned order)
@brief 伙伴物理页分配器：分配并取得对象或存储资源。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock.acquire` → `PhysicalAddress::from_pointer` → `remove_block` → `lock.release` → `pa.is_null` → `insert` → `pa.advanced` → `detail::block_size` → `set_tag` → `layout_.ram` → `page_index` → `detail::fill`。 成功分配按原始阶扣减空闲页，成功归还加回原始页数；仅统计变化，不改变原有分裂、合并、poison 与所有权校验。
```

<a id="fn-6f33683dd35acc36"></a>

## `xv::memory::BuddyPageAllocator::claim`

```doxygen
@fn void xv::memory::BuddyPageAllocator::claim(PhysicalAddress pa, unsigned order)
@brief 伙伴物理页分配器：领取待处理外部中断或认领空闲存储块。
@param pa 对应的物理地址。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_compare_exchange_n` → `layout_.ram` → `page_index` → `detail::fail`。
@details 需要特别跟踪的条件：`!__atomic_compare_exchange_n(&tags_[layout_.ram().page_index(pa)], &expected, Busy, 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3eb32f523d5d265d"></a>

## `xv::memory::BuddyPageAllocator::deallocate`

```doxygen
@fn void xv::memory::BuddyPageAllocator::deallocate(void *mem, unsigned order)
@brief 伙伴物理页分配器：把已分配的存储归还分配器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress::from_pointer` → `valid_block` → `detail::fail` → `claim` → `detail::fill` → `detail::block_size` → `lock.acquire` → `layout_.ram` → `page_index` → `page_count` → `page_tag` → `page_address`。 成功分配按原始阶扣减空闲页，成功归还加回原始页数；仅统计变化，不改变原有分裂、合并、poison 与所有权校验。
```

<a id="fn-2d9e53492f4ac4fd"></a>

## `xv::memory::BuddyPageAllocator::initialize_layout`

```doxygen
@fn void xv::memory::BuddyPageAllocator::initialize_layout()
@brief 伙伴物理页分配器：按已规划内存布局建立分配器元数据与空闲集合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`layout_.metadata` → `begin` → `detail::fill` → `size` → `lock.init` → `layout_.usable_count` → `layout_.usable` → `usable.begin` → `usable.end` → `pa.is_aligned` → `detail::block_size` → `usable.contains_block`。
@details 本函数可见的状态更新包括：`tags_ = layout_.metadata().begin().pointer<int>();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!pa.is_aligned(detail::block_size(order)) || !usable.contains_block(pa, detail::block_size(order))`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-98b0a2320cb39511"></a>

## `xv::memory::BuddyPageAllocator::split_to_pages`

```doxygen
@fn bool xv::memory::BuddyPageAllocator::split_to_pages(void *mem, unsigned order)
@brief 伙伴物理页分配器：把高阶物理块转换为独立页面的所有权状态。
@param mem 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supports` → `PhysicalAddress::from_pointer` → `valid_block` → `detail::fail` → `claim` → `layout_.ram` → `page_index` → `set_tag`。
@details 需要特别跟踪的条件：`!supports(order)`；`!valid_block(pa, order)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```
