# `kernel/memory/paging/sv39_page_table.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/sv39_page_table.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:sv39`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::LargestPagePolicy::choose`](sv39_page_table.cpp.md#fn-2171228f92751c34)
- [`xv::memory::OwnedSv39PageTable::OwnedSv39PageTable`](sv39_page_table.cpp.md#fn-2e410d014f53be24)
- [`xv::memory::Sv39PageTable::LeafFlow::allocate`](sv39_page_table.cpp.md#fn-6efdb92b67cd1b63)
- [`xv::memory::Sv39PageTable::LeafFlow::collision`](sv39_page_table.cpp.md#fn-410b32ad82d8f959)
- [`xv::memory::Sv39PageTable::LeafFlow::descend`](sv39_page_table.cpp.md#fn-004e1e313029978d)
- [`xv::memory::Sv39PageTable::LeafFlow::descending`](sv39_page_table.cpp.md#fn-98d8b11a3f9638ab)
- [`xv::memory::Sv39PageTable::LeafFlow::leaf`](sv39_page_table.cpp.md#fn-dbddd5ba803a369e)
- [`xv::memory::Sv39PageTable::LeafFlow::lookup`](sv39_page_table.cpp.md#fn-e5bf41efd0a99721)
- [`xv::memory::Sv39PageTable::LeafFlow::next`](sv39_page_table.cpp.md#fn-30208f7620a2f6e7)
- [`xv::memory::Sv39PageTable::LeafFlow::ok`](sv39_page_table.cpp.md#fn-b10fc811a2b57339)
- [`xv::memory::Sv39PageTable::LeafFlow::publish`](sv39_page_table.cpp.md#fn-6e20ada01e294839)
- [`xv::memory::Sv39PageTable::LeafFlow::valid`](sv39_page_table.cpp.md#fn-6f16260f48acf534)
- [`xv::memory::Sv39PageTable::RangeFlow::advance`](sv39_page_table.cpp.md#fn-1bf3def884511140)
- [`xv::memory::Sv39PageTable::RangeFlow::aligned`](sv39_page_table.cpp.md#fn-95646d4191c1cc9d)
- [`xv::memory::Sv39PageTable::RangeFlow::choose`](sv39_page_table.cpp.md#fn-b1d02fa565861f51)
- [`xv::memory::Sv39PageTable::RangeFlow::invalid_permissions`](sv39_page_table.cpp.md#fn-6135cca80fae6468)
- [`xv::memory::Sv39PageTable::RangeFlow::invalid_range`](sv39_page_table.cpp.md#fn-945f669a2932bf2e)
- [`xv::memory::Sv39PageTable::RangeFlow::level`](sv39_page_table.cpp.md#fn-d9d9148e9c90c0dd)
- [`xv::memory::Sv39PageTable::RangeFlow::map`](sv39_page_table.cpp.md#fn-a7559a0d9435359b)
- [`xv::memory::Sv39PageTable::RangeFlow::ok`](sv39_page_table.cpp.md#fn-61ba912f2ae49ccc)
- [`xv::memory::Sv39PageTable::RangeFlow::out_of_memory`](sv39_page_table.cpp.md#fn-367a3da4ff2064cb)
- [`xv::memory::Sv39PageTable::RangeFlow::permissions`](sv39_page_table.cpp.md#fn-a83028355af4abf7)
- [`xv::memory::Sv39PageTable::RangeFlow::range`](sv39_page_table.cpp.md#fn-4786cb85cccad6d4)
- [`xv::memory::Sv39PageTable::RangeFlow::remaining`](sv39_page_table.cpp.md#fn-3ce27057fe65cf4e)
- [`xv::memory::Sv39PageTable::RangeFlow::root`](sv39_page_table.cpp.md#fn-594e563d18e14616)
- [`xv::memory::Sv39PageTable::RangeFlow::step`](sv39_page_table.cpp.md#fn-ad867b861c4c3a1e)
- [`xv::memory::Sv39PageTable::RangeFlow::success`](sv39_page_table.cpp.md#fn-824f5edf064fc850)
- [`xv::memory::Sv39PageTable::clear`](sv39_page_table.cpp.md#fn-2f1f8c9aab0dc295)
- [`xv::memory::Sv39PageTable::clear_table`](sv39_page_table.cpp.md#fn-b168aaa56651a459)
- [`xv::memory::Sv39PageTable::count_table`](sv39_page_table.cpp.md#fn-4eab1f09025d1b08)
- [`xv::memory::Sv39PageTable::map_leaf`](sv39_page_table.cpp.md#fn-ffee637b26705f43)
- [`xv::memory::Sv39PageTable::map_range`](sv39_page_table.cpp.md#fn-1b298dc585488ae2)
- [`xv::memory::Sv39PageTable::statistics`](sv39_page_table.cpp.md#fn-abb0ead8402f759b)
- [`xv::memory::Sv39PageTable::translate`](sv39_page_table.cpp.md#fn-b693975e5af85c1d)
- [`xv::memory::child_table`](sv39_page_table.cpp.md#fn-7de99c24f4adcb5e)
- [`xv::memory::encode`](sv39_page_table.cpp.md#fn-a53c70023e08785e)
- [`xv::memory::index_at`](sv39_page_table.cpp.md#fn-fa39a8b2a658830b)

<a id="fn-7de99c24f4adcb5e"></a>

## `xv::memory::child_table`

```doxygen
@fn MemorySize *xv::memory::child_table(MemorySize entry)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `PhysicalAddress((entry >> 10) << 12).pointer<MemorySize>()`，返回当前查询结果。
@param entry 需要链接或解析的条目。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress`。
@return 各分支实际返回：`PhysicalAddress((entry >> 10) << 12).pointer<MemorySize>()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a53c70023e08785e"></a>

## `xv::memory::encode`

```doxygen
@fn MemorySize xv::memory::encode(PhysicalAddress address)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `(address.raw() >> 12) << 10`，返回当前查询结果。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`address.raw`。
@return 各分支实际返回：`(address.raw() >> 12) << 10`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fa39a8b2a658830b"></a>

## `xv::memory::index_at`

```doxygen
@fn unsigned xv::memory::index_at(VirtualAddress va, unsigned level)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `(va >> (12 + 9 * level)) & 511`，返回当前查询结果。
@param va 待处理的虚拟地址。
@param level 本次操作使用的 层级 值。
@return 各分支实际返回：`(va >> (12 + 9 * level)) & 511`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2171228f92751c34"></a>

## `xv::memory::LargestPagePolicy::choose`

```doxygen
@fn Sv39LeafLevel xv::memory::LargestPagePolicy::choose(VirtualAddress va, PhysicalAddress pa, MemorySize remaining) const
@brief 物理页所有权、地址范围、页表和用户内存复制：choose。
@param va 待处理的虚拟地址。
@param pa 对应的物理地址。
@param remaining 本次操作使用的 remaining 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Sv39PageTable::leaf_size` → `pa.is_aligned`。
@details 需要特别跟踪的条件：`remaining >= size && va % size == 0 && pa.is_aligned(size)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`static_cast<Sv39LeafLevel>(level)`；`Sv39LeafLevel::Page4KiB`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2e410d014f53be24"></a>

## `xv::memory::OwnedSv39PageTable::OwnedSv39PageTable`

```doxygen
@fn xv::memory::OwnedSv39PageTable::OwnedSv39PageTable(PhysicalPageAllocator &memory, const PageMappingPolicy &policy)
@brief 构造页表所有权句柄，建立本重载要求的成员初值和借用关系。
@param memory 启动内存信息或地址空间对象。
@param policy 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`root_` → `view_` → `root_.get` → `detail::fill`。
@details 需要特别跟踪的条件：`root_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ffee637b26705f43"></a>

## `xv::memory::Sv39PageTable::map_leaf`

```doxygen
@fn MappingResult xv::memory::Sv39PageTable::map_leaf(VirtualAddress va, PhysicalAddress pa, unsigned target, MemorySize permissions)
@brief Sv39 页表对象：在指定层级建立页表叶子条目。
@param va 待处理的虚拟地址。
@param pa 对应的物理地址。
@param target 本次操作使用的 target 值。
@param permissions 本次操作使用的 permissions 值。
@return 各分支实际返回：`MappingResult::Collision`；`MappingResult::OutOfMemory`；`MappingResult::InvalidRange`；`MappingResult::Ok`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 LeafFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-1b298dc585488ae2"></a>

## `xv::memory::Sv39PageTable::map_range`

```doxygen
@fn MappingResult xv::memory::Sv39PageTable::map_range(VirtualAddress va, PhysicalAddress pa, MemorySize bytes, MemorySize permissions)
@brief Sv39 页表对象：按页面策略映射完整地址区间。
@param va 待处理的虚拟地址。
@param pa 对应的物理地址。
@param bytes 本次操作使用的 字节区域 值。
@param permissions 本次操作使用的 permissions 值。
@return 各分支实际返回：`MappingResult::OutOfMemory`；`MappingResult::InvalidRange`；`MappingResult::InvalidPermissions`；`result`；`MappingResult::Ok`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 RangeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-b693975e5af85c1d"></a>

## `xv::memory::Sv39PageTable::translate`

```doxygen
@fn bool xv::memory::Sv39PageTable::translate(VirtualAddress va, PageTranslation &result) const
@brief Sv39 页表对象：解析当前页表或地址空间中的地址。
@param va 待处理的虚拟地址。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`index_at` → `PhysicalAddress` → `leaf_size` → `physical.is_aligned` → `physical.advanced` → `child_table`。
@details 需要特别跟踪的条件：`!root_ || va >= VirtualLimit`；`!(entry & Valid)`；`entry & Leaf`；`!physical.is_aligned(bytes) || ((entry & Write) && !(entry & Read))`；`level == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4eab1f09025d1b08"></a>

## `xv::memory::Sv39PageTable::count_table`

```doxygen
@fn void xv::memory::Sv39PageTable::count_table(const MemorySize *table, unsigned level, PageTableStatistics &result)
@brief 递归统计不同层级的页表及叶映射。
@details 用 acquire 原子读取每项 PTE，区分 1 GiB、2 MiB 和 4 KiB 叶子。调用者须保护下级页表节点不被回收；KernelPageTable::statistics 持区域锁提供该生命周期约束。
```

<a id="fn-abb0ead8402f759b"></a>

## `xv::memory::Sv39PageTable::statistics`

```doxygen
@fn PageTableStatistics xv::memory::Sv39PageTable::statistics() const
@brief Sv39 页表对象：统计。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`count_table`。
@details 需要特别跟踪的条件：`root_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`result`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b168aaa56651a459"></a>

## `xv::memory::Sv39PageTable::clear_table`

```doxygen
@fn void xv::memory::Sv39PageTable::clear_table(MemorySize *table, unsigned level)
@brief Sv39 页表对象：清除表。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@param level 本次操作使用的 层级 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`child_table` → `clear_table` → `memory_.deallocate`。
@details 本函数可见的状态更新包括：`table[i] = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`(entry & Valid) && !(entry & Leaf) && level`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2f1f8c9aab0dc295"></a>

## `xv::memory::Sv39PageTable::clear`

```doxygen
@fn void xv::memory::Sv39PageTable::clear()
@brief Sv39 页表对象：清空当前集合、指针或映射状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`clear_table`。
@details 需要特别跟踪的条件：`root_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6efdb92b67cd1b63"></a>

## `xv::memory::Sv39PageTable::LeafFlow::allocate`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::allocate(Context &c) noexcept
@brief 取得并清零候选子表、验证对齐后发布并转移所有权。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-410b32ad82d8f959"></a>

## `xv::memory::Sv39PageTable::LeafFlow::collision`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::collision(Context &c) noexcept
@brief 记录现有叶造成的映射冲突。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-004e1e313029978d"></a>

## `xv::memory::Sv39PageTable::LeafFlow::descend`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::descend(Context &c) noexcept
@brief 通过已有目录项进入子页表。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-98d8b11a3f9638ab"></a>

## `xv::memory::Sv39PageTable::LeafFlow::descending`

```doxygen
@fn static bool xv::memory::Sv39PageTable::LeafFlow::descending(Context &c) noexcept
@brief 检查是否仍需下降到更低页表层。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dbddd5ba803a369e"></a>

## `xv::memory::Sv39PageTable::LeafFlow::leaf`

```doxygen
@fn static bool xv::memory::Sv39PageTable::LeafFlow::leaf(Context &c) noexcept
@brief 检查已有项是否为不可覆盖的叶。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e5bf41efd0a99721"></a>

## `xv::memory::Sv39PageTable::LeafFlow::lookup`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::lookup(Context &c) noexcept
@brief 选择当前层的页表项。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-30208f7620a2f6e7"></a>

## `xv::memory::Sv39PageTable::LeafFlow::next`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::next(Context &c) noexcept
@brief 下降一层以继续迭代。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b10fc811a2b57339"></a>

## `xv::memory::Sv39PageTable::LeafFlow::ok`

```doxygen
@fn static bool xv::memory::Sv39PageTable::LeafFlow::ok(Context &c) noexcept
@brief 检查子表分配与验证结果是否成功。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6e20ada01e294839"></a>

## `xv::memory::Sv39PageTable::LeafFlow::publish`

```doxygen
@fn static void xv::memory::Sv39PageTable::LeafFlow::publish(Context &c) noexcept
@brief 在目标层拒绝碰撞或发布新的叶映射。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6f16260f48acf534"></a>

## `xv::memory::Sv39PageTable::LeafFlow::valid`

```doxygen
@fn static bool xv::memory::Sv39PageTable::LeafFlow::valid(Context &c) noexcept
@brief 检查当前项是否已存在。
@details 保留最多两层中间表遍历；缺表时局部 OwnedPage 负责未发布候选，验证后才 release，后续失败保留已发布前缀且不替换已有叶。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1bf3def884511140"></a>

## `xv::memory::Sv39PageTable::RangeFlow::advance`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::advance(Context &c) noexcept
@brief 推进虚实地址并扣除本轮映射字节。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-95646d4191c1cc9d"></a>

## `xv::memory::Sv39PageTable::RangeFlow::aligned`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::aligned(Context &c) noexcept
@brief 验证叶跨度适合剩余长度且虚实地址均对齐。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b1d02fa565861f51"></a>

## `xv::memory::Sv39PageTable::RangeFlow::choose`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::choose(Context &c) noexcept
@brief 调用原映射策略选择下一叶大小。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6135cca80fae6468"></a>

## `xv::memory::Sv39PageTable::RangeFlow::invalid_permissions`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::invalid_permissions(Context &c) noexcept
@brief 记录无效权限结果。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-945f669a2932bf2e"></a>

## `xv::memory::Sv39PageTable::RangeFlow::invalid_range`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::invalid_range(Context &c) noexcept
@brief 记录无效范围或策略输出结果。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d9d9148e9c90c0dd"></a>

## `xv::memory::Sv39PageTable::RangeFlow::level`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::level(Context &c) noexcept
@brief 检查策略返回的叶级别受支持。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a7559a0d9435359b"></a>

## `xv::memory::Sv39PageTable::RangeFlow::map`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::map(Context &c) noexcept
@brief 通过叶映射协议映射本轮范围并保存结果。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-61ba912f2ae49ccc"></a>

## `xv::memory::Sv39PageTable::RangeFlow::ok`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::ok(Context &c) noexcept
@brief 检查最近叶映射是否成功。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-367a3da4ff2064cb"></a>

## `xv::memory::Sv39PageTable::RangeFlow::out_of_memory`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::out_of_memory(Context &c) noexcept
@brief 记录缺少根表的内存不足结果。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a83028355af4abf7"></a>

## `xv::memory::Sv39PageTable::RangeFlow::permissions`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::permissions(Context &c) noexcept
@brief 验证受支持权限与 Sv39 写必须可读规则。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4786cb85cccad6d4"></a>

## `xv::memory::Sv39PageTable::RangeFlow::range`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::range(Context &c) noexcept
@brief 验证非空长度、页对齐与虚实地址上界及溢出。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3ce27057fe65cf4e"></a>

## `xv::memory::Sv39PageTable::RangeFlow::remaining`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::remaining(Context &c) noexcept
@brief 检查范围是否仍有未映射字节。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-594e563d18e14616"></a>

## `xv::memory::Sv39PageTable::RangeFlow::root`

```doxygen
@fn static bool xv::memory::Sv39PageTable::RangeFlow::root(Context &c) noexcept
@brief 检查视图是否具有根表。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ad867b861c4c3a1e"></a>

## `xv::memory::Sv39PageTable::RangeFlow::step`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::step(Context &c) noexcept
@brief 取得本轮叶跨度。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-824f5edf064fc850"></a>

## `xv::memory::Sv39PageTable::RangeFlow::success`

```doxygen
@fn static void xv::memory::Sv39PageTable::RangeFlow::success(Context &c) noexcept
@brief 记录整个范围映射完成。
@details 原页大小策略仍为业务扩展点；按原次序验证范围与权限，逐叶选择和映射，失败返回准确原因并允许保留成功前缀，由外层持有者负责最终树回收。 下一节点由图策略声明，业务结果与控制信号分离。
```
