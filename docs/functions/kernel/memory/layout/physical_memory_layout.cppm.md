# `kernel/memory/layout/physical_memory_layout.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/layout/physical_memory_layout.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:layout`。

直接模块依赖：`:range`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::PhysicalMemoryLayout::can_subtract`](physical_memory_layout.cppm.md#fn-1b1a39f66896173e)
- [`xv::memory::PhysicalMemoryLayout::configure`](physical_memory_layout.cppm.md#fn-c1dc25373b045896)
- [`xv::memory::PhysicalMemoryLayout::contains_usable_block`](physical_memory_layout.cppm.md#fn-d4b8c635f6f366b0)
- [`xv::memory::PhysicalMemoryLayout::direct`](physical_memory_layout.cppm.md#fn-66ee57b736e46c63)
- [`xv::memory::PhysicalMemoryLayout::direct_count`](physical_memory_layout.cppm.md#fn-1a834097ac37e64e)
- [`xv::memory::PhysicalMemoryLayout::metadata`](physical_memory_layout.cppm.md#fn-985cc5c042e8d04b)
- [`xv::memory::PhysicalMemoryLayout::ram`](physical_memory_layout.cppm.md#fn-4f0adfd8c12a87ef)
- [`xv::memory::PhysicalMemoryLayout::reserve`](physical_memory_layout.cppm.md#fn-f1acb57c735d22a4)
- [`xv::memory::PhysicalMemoryLayout::reserve_metadata`](physical_memory_layout.cppm.md#fn-c058c194f22f4c9f)
- [`xv::memory::PhysicalMemoryLayout::subtract`](physical_memory_layout.cppm.md#fn-f2be976df46f3b72)
- [`xv::memory::PhysicalMemoryLayout::usable`](physical_memory_layout.cppm.md#fn-483cdd34c7ca4cfc)
- [`xv::memory::PhysicalMemoryLayout::usable_count`](physical_memory_layout.cppm.md#fn-16f4a05c83671a8e)
- [`xv::memory::PhysicalMemoryLayout::usable_pages`](physical_memory_layout.cppm.md#fn-2117303a15314d71)

<a id="fn-c1dc25373b045896"></a>

## `xv::memory::PhysicalMemoryLayout::configure`

```doxygen
@fn bool xv::memory::PhysicalMemoryLayout::configure(PhysicalMemoryRange ram, PhysicalAddress first_usable)
@brief 物理内存布局规划：校验并规划内存区间布局。
@param ram 本次操作使用的 RAM 范围 值。
@param first_usable 允许分配器使用的首个物理地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ram.empty` → `ram.begin` → `is_aligned` → `ram.end` → `first_usable.align_up` → `PhysicalMemoryRange` → `empty`。
@details 本函数可见的状态更新包括：`ram_ = ram;`；`metadata_ = {};`；`usable_count_ = direct_count_ = 1;`；`direct_count_ = 1;`；`usable_[0] = PhysicalMemoryRange(begin, ram.end());`；`direct_[0] = ram;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`ram.empty() || !ram.begin().is_aligned() || !ram.end().is_aligned() || first_usable < ram.begin() || first_usable >= ram.end()`；`begin >= ram.end()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`!usable_[0].empty()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f1acb57c735d22a4"></a>

## `xv::memory::PhysicalMemoryLayout::reserve`

```doxygen
@fn bool xv::memory::PhysicalMemoryLayout::reserve(PhysicalMemoryRange reservation, bool no_map = false)
@brief 物理内存布局规划：从可分配区间中排除保留内存。
@param reservation 本次操作使用的 保留区间 值。
@param no_map 本次操作使用的 nomap 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`reservation.empty` → `reservation.end` → `ram_.begin` → `reservation.begin` → `ram_.end` → `pages` → `begin.align_down` → `end.align_up` → `can_subtract` → `subtract`。
@details 需要特别跟踪的条件：`reservation.empty() || reservation.end() <= ram_.begin() || reservation.begin() >= ram_.end()`；`!can_subtract(usable_, usable_count_, pages) || (no_map && !can_subtract(direct_, direct_count_, pages))`；`!subtract(usable_, usable_count_, pages)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`；`!no_map || subtract(direct_, direct_count_, pages)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c058c194f22f4c9f"></a>

## `xv::memory::PhysicalMemoryLayout::reserve_metadata`

```doxygen
@fn bool xv::memory::PhysicalMemoryLayout::reserve_metadata(MemorySize bytes)
@brief 物理内存布局规划：为分配器元数据保留对齐后的物理区间。
@param bytes 本次操作使用的 字节区域 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`metadata_.empty` → `MemorySize` → `size` → `begin` → `chosen` → `begin.advanced` → `reserve`。
@details 本函数可见的状态更新包括：`metadata_ = chosen;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!metadata_.empty() || bytes == 0 || bytes > ~MemorySize(0) - (PageSize - 1)`；`usable_[i].size() >= bytes`；`!reserve(chosen)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4f0adfd8c12a87ef"></a>

## `xv::memory::PhysicalMemoryLayout::ram`

```doxygen
@fn const PhysicalMemoryRange &xv::memory::PhysicalMemoryLayout::ram() const
@brief 物理内存布局规划：读取或计算 `ram_`，返回当前查询结果。
@return 各分支实际返回：`ram_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-985cc5c042e8d04b"></a>

## `xv::memory::PhysicalMemoryLayout::metadata`

```doxygen
@fn const PhysicalMemoryRange &xv::memory::PhysicalMemoryLayout::metadata() const
@brief 物理内存布局规划：读取或计算 `metadata_`，返回当前查询结果。
@return 各分支实际返回：`metadata_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-16f4a05c83671a8e"></a>

## `xv::memory::PhysicalMemoryLayout::usable_count`

```doxygen
@fn unsigned xv::memory::PhysicalMemoryLayout::usable_count() const
@brief 物理内存布局规划：读取或计算 `usable_count_`，返回当前查询结果。
@return 各分支实际返回：`usable_count_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1a834097ac37e64e"></a>

## `xv::memory::PhysicalMemoryLayout::direct_count`

```doxygen
@fn unsigned xv::memory::PhysicalMemoryLayout::direct_count() const
@brief 物理内存布局规划：读取或计算 `direct_count_`，返回当前查询结果。
@return 各分支实际返回：`direct_count_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-483cdd34c7ca4cfc"></a>

## `xv::memory::PhysicalMemoryLayout::usable`

```doxygen
@fn const PhysicalMemoryRange &xv::memory::PhysicalMemoryLayout::usable(unsigned index) const
@brief 物理内存布局规划：读取或计算 `usable_[index]`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail`。
@details 需要特别跟踪的条件：`index >= usable_count_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`usable_[index]`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-66ee57b736e46c63"></a>

## `xv::memory::PhysicalMemoryLayout::direct`

```doxygen
@fn const PhysicalMemoryRange &xv::memory::PhysicalMemoryLayout::direct(unsigned index) const
@brief 物理内存布局规划：读取或计算 `direct_[index]`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail`。
@details 需要特别跟踪的条件：`index >= direct_count_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`direct_[index]`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2117303a15314d71"></a>

## `xv::memory::PhysicalMemoryLayout::usable_pages`

```doxygen
@fn PageIndex xv::memory::PhysicalMemoryLayout::usable_pages() const
@brief 物理内存布局规划：可用内存页集合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`page_count`。
@return 各分支实际返回：`count`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d4b8c635f6f366b0"></a>

## `xv::memory::PhysicalMemoryLayout::contains_usable_block`

```doxygen
@fn bool xv::memory::PhysicalMemoryLayout::contains_usable_block(PhysicalAddress address, MemorySize bytes) const
@brief 物理内存布局规划：包含关系可用内存块。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param bytes 本次操作使用的 字节区域 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`contains_block`。
@details 需要特别跟踪的条件：`usable_[i].contains_block(address, bytes)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1b1a39f66896173e"></a>

## `xv::memory::PhysicalMemoryLayout::can_subtract`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::can_subtract(const PhysicalMemoryRange *ranges, unsigned count, PhysicalMemoryRange excluded)
@brief 物理内存布局规划：是否允许区间扣除。
@param ranges 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@param excluded 本次操作使用的 excluded 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`excluded.begin` → `begin` → `excluded.end` → `end`。
@details 需要特别跟踪的条件：`count < MaxRanges`；`excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f2be976df46f3b72"></a>

## `xv::memory::PhysicalMemoryLayout::subtract`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::subtract(PhysicalMemoryRange *ranges, unsigned &count, PhysicalMemoryRange excluded)
@brief 物理内存布局规划：区间扣除。
@param ranges 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@param excluded 本次操作使用的 excluded 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`excluded.begin` → `begin` → `excluded.end` → `end` → `range.begin` → `range.end` → `PhysicalMemoryRange`。
@details 本函数可见的状态更新包括：`ranges[j - 1] = ranges[j];`；`ranges[i++] = PhysicalMemoryRange(right, range.end());`；`ranges[i++] = PhysicalMemoryRange(range.begin(), left);`；`ranges[j] = ranges[j - 1];`；`ranges[i] = PhysicalMemoryRange(range.begin(), left);`；`ranges[i + 1] = PhysicalMemoryRange(right, range.end());`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`excluded.begin() > ranges[i].begin() && excluded.end() < ranges[i].end() && count == MaxRanges`；`excluded.end() <= range.begin() || excluded.begin() >= range.end()`；`left == range.begin() && right == range.end()`；`left == range.begin()`；`right == range.end()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```
