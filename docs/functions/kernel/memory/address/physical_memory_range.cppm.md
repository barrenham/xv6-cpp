# `kernel/memory/address/physical_memory_range.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/address/physical_memory_range.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:range`。

直接模块依赖：`:address`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::PhysicalMemoryRange::PhysicalMemoryRange`](physical_memory_range.cppm.md#fn-12466768f83bd14e)
- [`xv::memory::PhysicalMemoryRange::PhysicalMemoryRange`](physical_memory_range.cppm.md#fn-aacae382595153bb)
- [`xv::memory::PhysicalMemoryRange::begin`](physical_memory_range.cppm.md#fn-2a74b95dcf286e96)
- [`xv::memory::PhysicalMemoryRange::contains`](physical_memory_range.cppm.md#fn-59caa1e17a0f6d3c)
- [`xv::memory::PhysicalMemoryRange::contains`](physical_memory_range.cppm.md#fn-b6040dfaf0e25b2f)
- [`xv::memory::PhysicalMemoryRange::contains_block`](physical_memory_range.cppm.md#fn-b5c9862fc259f5db)
- [`xv::memory::PhysicalMemoryRange::empty`](physical_memory_range.cppm.md#fn-bf63cb186fdc905f)
- [`xv::memory::PhysicalMemoryRange::end`](physical_memory_range.cppm.md#fn-4314506b5b69af6f)
- [`xv::memory::PhysicalMemoryRange::page_address`](physical_memory_range.cppm.md#fn-6bcb14243515ba47)
- [`xv::memory::PhysicalMemoryRange::page_aligned`](physical_memory_range.cppm.md#fn-c39d8592ea87fccb)
- [`xv::memory::PhysicalMemoryRange::page_count`](physical_memory_range.cppm.md#fn-5ac78cacc6dba1c2)
- [`xv::memory::PhysicalMemoryRange::page_index`](physical_memory_range.cppm.md#fn-1e7d832056b2569d)
- [`xv::memory::PhysicalMemoryRange::size`](physical_memory_range.cppm.md#fn-ad15e71d7376ff3d)

<a id="fn-12466768f83bd14e"></a>

## `xv::memory::PhysicalMemoryRange::PhysicalMemoryRange`

```doxygen
@fn constexpr xv::memory::PhysicalMemoryRange::PhysicalMemoryRange()
@brief 构造物理地址区间，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-aacae382595153bb"></a>

## `xv::memory::PhysicalMemoryRange::PhysicalMemoryRange`

```doxygen
@fn constexpr xv::memory::PhysicalMemoryRange::PhysicalMemoryRange(PhysicalAddress begin, PhysicalAddress end)
@brief 构造物理地址区间，建立本重载要求的成员初值和借用关系。
@param begin 本次操作使用的 起始边界 值。
@param end 本次操作使用的 结束边界 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`begin_` → `end_` → `detail::fail`。
@details 需要特别跟踪的条件：`end < begin`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2a74b95dcf286e96"></a>

## `xv::memory::PhysicalMemoryRange::begin`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalMemoryRange::begin() const
@brief 物理地址区间：取得区间的起始物理地址。
@return 各分支实际返回：`begin_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4314506b5b69af6f"></a>

## `xv::memory::PhysicalMemoryRange::end`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalMemoryRange::end() const
@brief 物理地址区间：取得不包含在区间内的结束边界。
@return 各分支实际返回：`end_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ad15e71d7376ff3d"></a>

## `xv::memory::PhysicalMemoryRange::size`

```doxygen
@fn constexpr MemorySize xv::memory::PhysicalMemoryRange::size() const
@brief 物理地址区间：读取或计算 `end_.offset_from(begin_)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`end_.offset_from`。
@return 各分支实际返回：`end_.offset_from(begin_)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bf63cb186fdc905f"></a>

## `xv::memory::PhysicalMemoryRange::empty`

```doxygen
@fn constexpr bool xv::memory::PhysicalMemoryRange::empty() const
@brief 物理地址区间：读取或计算 `begin_ == end_`，返回当前查询结果。
@return 各分支实际返回：`begin_ == end_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-59caa1e17a0f6d3c"></a>

## `xv::memory::PhysicalMemoryRange::contains`

```doxygen
@fn constexpr bool xv::memory::PhysicalMemoryRange::contains(PhysicalAddress address) const
@brief 物理地址区间：读取或计算 `begin_ <= address && address < end_`，返回当前查询结果。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@return 各分支实际返回：`begin_ <= address && address < end_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b6040dfaf0e25b2f"></a>

## `xv::memory::PhysicalMemoryRange::contains`

```doxygen
@fn constexpr bool xv::memory::PhysicalMemoryRange::contains(PhysicalMemoryRange range) const
@brief 物理地址区间：读取或计算 `begin_ <= range.begin_ && range.end_ <= end_`，返回当前查询结果。
@param range 本次操作使用的 区间 值。
@return 各分支实际返回：`begin_ <= range.begin_ && range.end_ <= end_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b5c9862fc259f5db"></a>

## `xv::memory::PhysicalMemoryRange::contains_block`

```doxygen
@fn constexpr bool xv::memory::PhysicalMemoryRange::contains_block(PhysicalAddress address, MemorySize bytes) const
@brief 物理地址区间：读取或计算 `bytes != 0 && contains(address) && bytes <= end_.offset_from(address)`，返回当前查询结果。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param bytes 本次操作使用的 字节区域 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`contains` → `end_.offset_from`。
@return 各分支实际返回：`bytes != 0 && contains(address) && bytes <= end_.offset_from(address)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c39d8592ea87fccb"></a>

## `xv::memory::PhysicalMemoryRange::page_aligned`

```doxygen
@fn constexpr PhysicalMemoryRange xv::memory::PhysicalMemoryRange::page_aligned() const
@brief 物理地址区间：页对齐状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`end_.align_down` → `PhysicalMemoryRange` → `begin_.align_up`。
@details 需要特别跟踪的条件：`begin_ >= last`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`PhysicalMemoryRange(last, last)`；`PhysicalMemoryRange(begin_.align_up(), last)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5ac78cacc6dba1c2"></a>

## `xv::memory::PhysicalMemoryRange::page_count`

```doxygen
@fn constexpr PageIndex xv::memory::PhysicalMemoryRange::page_count() const
@brief 物理地址区间：读取或计算 `size() / PageSize`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`begin_.is_aligned` → `end_.is_aligned` → `detail::fail` → `size`。
@details 需要特别跟踪的条件：`!begin_.is_aligned() || !end_.is_aligned()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`size() / PageSize`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1e7d832056b2569d"></a>

## `xv::memory::PhysicalMemoryRange::page_index`

```doxygen
@fn constexpr PageIndex xv::memory::PhysicalMemoryRange::page_index(PhysicalAddress address) const
@brief 物理地址区间：页index。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`page_count` → `address.is_aligned` → `contains` → `detail::fail` → `address.offset_from`。
@details 需要特别跟踪的条件：`!address.is_aligned() || !contains(address)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`address.offset_from(begin_) / PageSize`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6bcb14243515ba47"></a>

## `xv::memory::PhysicalMemoryRange::page_address`

```doxygen
@fn constexpr PhysicalAddress xv::memory::PhysicalMemoryRange::page_address(PageIndex index) const
@brief 物理地址区间：读取或计算 `begin_.advanced(index * PageSize)`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`page_count` → `detail::fail` → `begin_.advanced`。
@details 需要特别跟踪的条件：`index >= page_count()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`begin_.advanced(index * PageSize)`。应结合上面的条件检查区分正常值、空值和错误值。
```
