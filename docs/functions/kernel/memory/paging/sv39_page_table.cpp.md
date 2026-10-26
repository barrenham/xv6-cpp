# `kernel/memory/paging/sv39_page_table.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/sv39_page_table.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:sv39`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::LargestPagePolicy::choose`](sv39_page_table.cpp.md#fn-2171228f92751c34)
- [`xv::memory::OwnedSv39PageTable::OwnedSv39PageTable`](sv39_page_table.cpp.md#fn-2e410d014f53be24)
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`index_at` → `child_table` → `candidate` → `detail::fill` → `candidate.get` → `PhysicalAddress::from_pointer` → `address.is_aligned` → `encode` → `candidate.release`。
@details 需要特别跟踪的条件：`entry & Valid`；`entry & Leaf`；`!candidate`；`!address.is_aligned()`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`candidate.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`MappingResult::Collision`；`MappingResult::OutOfMemory`；`MappingResult::InvalidRange`；`MappingResult::Ok`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pa.is_aligned` → `pa.raw` → `policy_.choose` → `leaf_size` → `map_leaf` → `pa.advanced`。
@details 需要特别跟踪的条件：`!root_`；`(permissions & ~Allowed) || !(permissions & (Read | Execute)) || ((permissions & Write) && !(permissions & Read))`；`bytes`；`static_cast<unsigned>(level) > 2`；`step > bytes || va % step || !pa.is_aligned(step)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`MappingResult::OutOfMemory`；`MappingResult::InvalidRange`；`MappingResult::InvalidPermissions`；`result`；`MappingResult::Ok`。应结合上面的条件检查区分正常值、空值和错误值。
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
@brief Sv39 页表对象：数量表。
@param table 页表或 poll 登记接口；其具体类型见函数签名。
@param level 本次操作使用的 层级 值。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`count_table` → `child_table`。
@details 需要特别跟踪的条件：`!(entry & Valid)`；`entry & Leaf`；`level == 2`；`level == 1`；`level`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
