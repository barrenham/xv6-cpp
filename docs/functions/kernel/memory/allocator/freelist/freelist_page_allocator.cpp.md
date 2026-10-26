# `kernel/memory/allocator/freelist/freelist_page_allocator.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../../kernel/memory/allocator/freelist/freelist_page_allocator.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:freelist`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::FreeListPageAllocator::allocate`](freelist_page_allocator.cpp.md#fn-04a2ef715486f832)
- [`xv::memory::FreeListPageAllocator::deallocate`](freelist_page_allocator.cpp.md#fn-52f97fcca1c3f61d)
- [`xv::memory::FreeListPageAllocator::initialize_layout`](freelist_page_allocator.cpp.md#fn-cde7c1a4166fbfc3)
- [`xv::memory::FreeListPageAllocator::split_to_pages`](freelist_page_allocator.cpp.md#fn-d6e5d9e01e5480ed)
- [`xv::memory::FreeListPageAllocator::valid_page`](freelist_page_allocator.cpp.md#fn-6ab5756247d1a4c1)

<a id="fn-6ab5756247d1a4c1"></a>

## `xv::memory::FreeListPageAllocator::valid_page`

```doxygen
@fn bool xv::memory::FreeListPageAllocator::valid_page(PhysicalAddress pa) const
@brief 单页空闲链分配器：验证页面地址是否属于可管理区间。
@param pa 对应的物理地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pa.is_aligned` → `layout_.contains_usable_block`。
@return 各分支实际返回：`pa.is_aligned() && layout_.contains_usable_block(pa, PageSize)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cde7c1a4166fbfc3"></a>

## `xv::memory::FreeListPageAllocator::initialize_layout`

```doxygen
@fn void xv::memory::FreeListPageAllocator::initialize_layout()
@brief 单页空闲链分配器：按已规划内存布局建立分配器元数据与空闲集合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`layout_.metadata` → `begin` → `detail::fill` → `size` → `lock_.init` → `layout_.usable_count` → `layout_.usable` → `usable.page_count` → `usable.page_address` → `layout_.ram` → `page_index`。
@details 本函数可见的状态更新包括：`states_ = layout_.metadata().begin().pointer<unsigned char>();`；`run->next = head_;`；`head_ = run;`；`states_[layout_.ram().page_index(page)] = Free;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-04a2ef715486f832"></a>

## `xv::memory::FreeListPageAllocator::allocate`

```doxygen
@fn void *xv::memory::FreeListPageAllocator::allocate(unsigned order)
@brief 单页空闲链分配器：分配并取得对象或存储资源。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supports` → `lock_.acquire` → `layout_.ram` → `page_index` → `PhysicalAddress::from_pointer` → `lock_.release` → `detail::fill`。
@details 本函数可见的状态更新包括：`head_ = page->next;`；`states_[layout_.ram().page_index(PhysicalAddress::from_pointer(page))] = Allocated;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!supports(order)`；`page`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`nullptr`；`page`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-52f97fcca1c3f61d"></a>

## `xv::memory::FreeListPageAllocator::deallocate`

```doxygen
@fn void xv::memory::FreeListPageAllocator::deallocate(void *address, unsigned order)
@brief 单页空闲链分配器：把已分配的存储归还分配器。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress::from_pointer` → `valid_page` → `detail::fail` → `lock_.acquire` → `layout_.ram` → `page_index` → `detail::fill` → `lock_.release`。
@details 本函数可见的状态更新包括：`page->next = head_;`；`head_ = page;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`order != 0 || !valid_page(pa)`；`state != Allocated`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d6e5d9e01e5480ed"></a>

## `xv::memory::FreeListPageAllocator::split_to_pages`

```doxygen
@fn bool xv::memory::FreeListPageAllocator::split_to_pages(void *address, unsigned order)
@brief 单页空闲链分配器：把高阶物理块转换为独立页面的所有权状态。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supports` → `PhysicalAddress::from_pointer` → `valid_page` → `detail::fail` → `lock_.acquire` → `layout_.ram` → `page_index` → `lock_.release`。
@details 需要特别跟踪的条件：`!supports(order)`；`!valid_page(pa)`；`!owned`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```
