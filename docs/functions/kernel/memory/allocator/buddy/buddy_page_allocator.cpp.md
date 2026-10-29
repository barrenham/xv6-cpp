# `kernel/memory/allocator/buddy/buddy_page_allocator.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../../kernel/memory/allocator/buddy/buddy_page_allocator.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:buddy`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::BuddyPageAllocator::AllocateFlow::found`](buddy_page_allocator.cpp.md#fn-6621d3592652a6ab)
- [`xv::memory::BuddyPageAllocator::AllocateFlow::publish`](buddy_page_allocator.cpp.md#fn-d04dcd307e092f89)
- [`xv::memory::BuddyPageAllocator::AllocateFlow::search`](buddy_page_allocator.cpp.md#fn-7d061109ef9ff2c1)
- [`xv::memory::BuddyPageAllocator::AllocateFlow::split`](buddy_page_allocator.cpp.md#fn-7e0814732a008ef0)
- [`xv::memory::BuddyPageAllocator::AllocateFlow::splitting`](buddy_page_allocator.cpp.md#fn-a1a84dc2c04b1899)
- [`xv::memory::BuddyPageAllocator::DeallocateFlow::advance`](buddy_page_allocator.cpp.md#fn-367785b01708036e)
- [`xv::memory::BuddyPageAllocator::DeallocateFlow::claim`](buddy_page_allocator.cpp.md#fn-776a366830e51901)
- [`xv::memory::BuddyPageAllocator::DeallocateFlow::inspect`](buddy_page_allocator.cpp.md#fn-e43f37f44d8625ac)
- [`xv::memory::BuddyPageAllocator::DeallocateFlow::merging`](buddy_page_allocator.cpp.md#fn-c1f7f589f676cca9)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::inside`](buddy_page_allocator.cpp.md#fn-d01b02ec4c85a36d)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::metadata`](buddy_page_allocator.cpp.md#fn-83ee44a402c5a5ab)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::next`](buddy_page_allocator.cpp.md#fn-d781b423906f17bd)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::range`](buddy_page_allocator.cpp.md#fn-b26c5b00f455ca60)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::remaining`](buddy_page_allocator.cpp.md#fn-9164583dc8c24001)
- [`xv::memory::BuddyPageAllocator::InitializeFlow::seed`](buddy_page_allocator.cpp.md#fn-11d6fc452207d1fc)
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
@details 流程由 AllocateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 DeallocateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-2d9e53492f4ac4fd"></a>

## `xv::memory::BuddyPageAllocator::initialize_layout`

```doxygen
@fn void xv::memory::BuddyPageAllocator::initialize_layout()
@brief 伙伴物理页分配器：按已规划内存布局建立分配器元数据与空闲集合。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 InitializeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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

<a id="fn-6621d3592652a6ab"></a>

## `xv::memory::BuddyPageAllocator::AllocateFlow::found`

```doxygen
@fn static bool xv::memory::BuddyPageAllocator::AllocateFlow::found(Context &c) noexcept
@brief 检查是否取得私有候选块。
@details 非法阶数在入口返回；查找和拆分每次只持一个阶的锁，取出的块保持私有，最终按原顺序记账、发布分配标签并填充返回块。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d04dcd307e092f89"></a>

## `xv::memory::BuddyPageAllocator::AllocateFlow::publish`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::AllocateFlow::publish(Context &c) noexcept
@brief 扣减空闲页、发布分配标签并填充候选后返回。
@details 非法阶数在入口返回；查找和拆分每次只持一个阶的锁，取出的块保持私有，最终按原顺序记账、发布分配标签并填充返回块。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7d061109ef9ff2c1"></a>

## `xv::memory::BuddyPageAllocator::AllocateFlow::search`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::AllocateFlow::search(Context &c) noexcept
@brief 逐阶加锁查找并摘除首个可用块，离开该阶前释放锁。
@details 非法阶数在入口返回；查找和拆分每次只持一个阶的锁，取出的块保持私有，最终按原顺序记账、发布分配标签并填充返回块。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7e0814732a008ef0"></a>

## `xv::memory::BuddyPageAllocator::AllocateFlow::split`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::AllocateFlow::split(Context &c) noexcept
@brief 下降一阶并在该阶锁下发布右半块。
@details 非法阶数在入口返回；查找和拆分每次只持一个阶的锁，取出的块保持私有，最终按原顺序记账、发布分配标签并填充返回块。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a1a84dc2c04b1899"></a>

## `xv::memory::BuddyPageAllocator::AllocateFlow::splitting`

```doxygen
@fn static bool xv::memory::BuddyPageAllocator::AllocateFlow::splitting(Context &c) noexcept
@brief 检查候选是否仍高于请求阶数。
@details 非法阶数在入口返回；查找和拆分每次只持一个阶的锁，取出的块保持私有，最终按原顺序记账、发布分配标签并填充返回块。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-367785b01708036e"></a>

## `xv::memory::BuddyPageAllocator::DeallocateFlow::advance`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::DeallocateFlow::advance(Context &c) noexcept
@brief 两半块已私有后选择低地址并推进到更高阶。
@details 入口保留范围对齐验证；先 CAS 认领并按原顺序记账与毒化，再逐阶合并，不同时持不同阶锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-776a366830e51901"></a>

## `xv::memory::BuddyPageAllocator::DeallocateFlow::claim`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::DeallocateFlow::claim(Context &c) noexcept
@brief 认领原块、增加空闲页计数并毒化数据。
@details 入口保留范围对齐验证；先 CAS 认领并按原顺序记账与毒化，再逐阶合并，不同时持不同阶锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e43f37f44d8625ac"></a>

## `xv::memory::BuddyPageAllocator::DeallocateFlow::inspect`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::DeallocateFlow::inspect(Context &c) noexcept
@brief 持当前阶锁判断伙伴，合并时摘伙伴，否则发布最终空闲块。
@details 入口保留范围对齐验证；先 CAS 认领并按原顺序记账与毒化，再逐阶合并，不同时持不同阶锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c1f7f589f676cca9"></a>

## `xv::memory::BuddyPageAllocator::DeallocateFlow::merging`

```doxygen
@fn static bool xv::memory::BuddyPageAllocator::DeallocateFlow::merging(Context &c) noexcept
@brief 检查当前阶是否取得可合并伙伴。
@details 入口保留范围对齐验证；先 CAS 认领并按原顺序记账与毒化，再逐阶合并，不同时持不同阶锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d01b02ec4c85a36d"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::inside`

```doxygen
@fn static bool xv::memory::BuddyPageAllocator::InitializeFlow::inside(Context &c) noexcept
@brief 检查当前种子地址是否仍在区间内。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-83ee44a402c5a5ab"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::metadata`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::InitializeFlow::metadata(Context &c) noexcept
@brief 设置并清零标签区，初始化各阶空闲链锁。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d781b423906f17bd"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::next`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::InitializeFlow::next(Context &c) noexcept
@brief 切换到下一个可用区间。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b26c5b00f455ca60"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::range`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::InitializeFlow::range(Context &c) noexcept
@brief 选取当前可用区间的起始地址。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9164583dc8c24001"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::remaining`

```doxygen
@fn static bool xv::memory::BuddyPageAllocator::InitializeFlow::remaining(Context &c) noexcept
@brief 检查是否仍有可用物理区间。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-11d6fc452207d1fc"></a>

## `xv::memory::BuddyPageAllocator::InitializeFlow::seed`

```doxygen
@fn static void xv::memory::BuddyPageAllocator::InitializeFlow::seed(Context &c) noexcept
@brief 直接选择最大合法阶、发布种子块并推进地址。
@details 先清元数据并初始化所有阶锁；按可用区间选择最大对齐块初始化，未拥有 RAM 不提前逐字节毒化。 下一节点由图策略声明，业务结果与控制信号分离。
```
