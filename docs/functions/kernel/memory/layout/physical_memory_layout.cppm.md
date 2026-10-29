# `kernel/memory/layout/physical_memory_layout.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/layout/physical_memory_layout.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:layout`。

直接模块依赖：`:range`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::PhysicalMemoryLayout::MetadataFlow::choose`](physical_memory_layout.cppm.md#fn-b0d5509dd8eb7c1c)
- [`xv::memory::PhysicalMemoryLayout::MetadataFlow::found`](physical_memory_layout.cppm.md#fn-23e19e93faecae8f)
- [`xv::memory::PhysicalMemoryLayout::MetadataFlow::ok`](physical_memory_layout.cppm.md#fn-94664c4ab3db26e9)
- [`xv::memory::PhysicalMemoryLayout::MetadataFlow::publish`](physical_memory_layout.cppm.md#fn-28e173024043e840)
- [`xv::memory::PhysicalMemoryLayout::MetadataFlow::reserve`](physical_memory_layout.cppm.md#fn-14e88053dacb16ce)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::capacity`](physical_memory_layout.cppm.md#fn-bb6899a3fa637705)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::direct`](physical_memory_layout.cppm.md#fn-55fc4a30e43d8809)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::no_map`](physical_memory_layout.cppm.md#fn-1090d2d78d765e79)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::ok`](physical_memory_layout.cppm.md#fn-7865675b86c5b0b9)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::success`](physical_memory_layout.cppm.md#fn-f926171da6808553)
- [`xv::memory::PhysicalMemoryLayout::ReserveFlow::usable`](physical_memory_layout.cppm.md#fn-4928700ae5c4e032)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::found`](physical_memory_layout.cppm.md#fn-add0098b59ef770a)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::left`](physical_memory_layout.cppm.md#fn-c877ee2aa1de84a8)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::remove`](physical_memory_layout.cppm.md#fn-d4291436ed9832af)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::right`](physical_memory_layout.cppm.md#fn-1ede6b774f131f55)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::scan`](physical_memory_layout.cppm.md#fn-92774b1192a6362a)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::split`](physical_memory_layout.cppm.md#fn-c07afcf881bc1576)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::success`](physical_memory_layout.cppm.md#fn-854848cc3f4fd679)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_left`](physical_memory_layout.cppm.md#fn-63ef5e7e09f01a63)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_right`](physical_memory_layout.cppm.md#fn-f153e8658b155b53)
- [`xv::memory::PhysicalMemoryLayout::SubtractFlow::whole`](physical_memory_layout.cppm.md#fn-113af34f7c19d193)
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
@fn inline bool xv::memory::PhysicalMemoryLayout::reserve(PhysicalMemoryRange reservation, bool no_map)
@brief 物理内存布局规划：从可分配区间中排除保留内存。
@param reservation 本次操作使用的 保留区间 值。
@param no_map 本次操作使用的 nomap 值。
@return 各分支实际返回：`true`；`false`；`!no_map || subtract(direct_, direct_count_, pages)`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReserveFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-c058c194f22f4c9f"></a>

## `xv::memory::PhysicalMemoryLayout::reserve_metadata`

```doxygen
@fn inline bool xv::memory::PhysicalMemoryLayout::reserve_metadata(MemorySize bytes)
@brief 物理内存布局规划：为分配器元数据保留对齐后的物理区间。
@param bytes 本次操作使用的 字节区域 值。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 MetadataFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@fn inline bool xv::memory::PhysicalMemoryLayout::subtract(PhysicalMemoryRange *ranges, unsigned &count, PhysicalMemoryRange excluded)
@brief 物理内存布局规划：区间扣除。
@param ranges 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@param excluded 本次操作使用的 excluded 值。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SubtractFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-b0d5509dd8eb7c1c"></a>

## `xv::memory::PhysicalMemoryLayout::MetadataFlow::choose`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::MetadataFlow::choose(Context &c) noexcept
@brief 直接寻找首个容纳元数据的可用区间并保存候选。
@details 入口拒绝重复、零长度或舍入溢出；按页舍入后选择第一个足够大的可用区间，预留成功才发布元数据范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-23e19e93faecae8f"></a>

## `xv::memory::PhysicalMemoryLayout::MetadataFlow::found`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::MetadataFlow::found(Context &c) noexcept
@brief 检查元数据候选区间是否存在。
@details 入口拒绝重复、零长度或舍入溢出；按页舍入后选择第一个足够大的可用区间，预留成功才发布元数据范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-94664c4ab3db26e9"></a>

## `xv::memory::PhysicalMemoryLayout::MetadataFlow::ok`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::MetadataFlow::ok(Context &c) noexcept
@brief 检查元数据候选是否预留成功。
@details 入口拒绝重复、零长度或舍入溢出；按页舍入后选择第一个足够大的可用区间，预留成功才发布元数据范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-28e173024043e840"></a>

## `xv::memory::PhysicalMemoryLayout::MetadataFlow::publish`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::MetadataFlow::publish(Context &c) noexcept
@brief 发布唯一元数据范围并记录成功。
@details 入口拒绝重复、零长度或舍入溢出；按页舍入后选择第一个足够大的可用区间，预留成功才发布元数据范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-14e88053dacb16ce"></a>

## `xv::memory::PhysicalMemoryLayout::MetadataFlow::reserve`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::MetadataFlow::reserve(Context &c) noexcept
@brief 通过原预留接口排除元数据候选。
@details 入口拒绝重复、零长度或舍入溢出；按页舍入后选择第一个足够大的可用区间，预留成功才发布元数据范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bb6899a3fa637705"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::capacity`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::ReserveFlow::capacity(Context &c) noexcept
@brief 先检查 usable 及按需 direct 的区间拆分容量。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-55fc4a30e43d8809"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::direct`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::ReserveFlow::direct(Context &c) noexcept
@brief 从直接映射视图扣除相同预留页。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1090d2d78d765e79"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::no_map`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::ReserveFlow::no_map(Context &c) noexcept
@brief 检查预留是否还要求从直接映射视图扣除。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7865675b86c5b0b9"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::ok`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::ReserveFlow::ok(Context &c) noexcept
@brief 检查最近一次视图扣除是否成功。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f926171da6808553"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::success`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::ReserveFlow::success(Context &c) noexcept
@brief 记录整个预留成功。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4928700ae5c4e032"></a>

## `xv::memory::PhysicalMemoryLayout::ReserveFlow::usable`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::ReserveFlow::usable(Context &c) noexcept
@brief 从可用物理区间视图扣除预留页。
@details 入口保留空范围和 RAM 外预留直接成功；按页外扩后先同时预检两张数组容量，再修改 usable，按需修改 direct，失败不提前修改另一视图。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-add0098b59ef770a"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::found`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::SubtractFlow::found(Context &c) noexcept
@brief 检查是否仍有重叠项需处理。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c877ee2aa1de84a8"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::left`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::SubtractFlow::left(Context &c) noexcept
@brief 检查预留是否覆盖当前区间左边界。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d4291436ed9832af"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::remove`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::remove(Context &c) noexcept
@brief 删除整项并压紧数组，保持当前索引复查后继。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1ede6b774f131f55"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::right`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::SubtractFlow::right(Context &c) noexcept
@brief 检查预留是否覆盖当前区间右边界。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-92774b1192a6362a"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::scan`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::scan(Context &c) noexcept
@brief 直接跳过不重叠项并保存下一重叠区间和裁剪边界。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c07afcf881bc1576"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::split`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::split(Context &c) noexcept
@brief 右移后继、保留左右两段并更新计数和索引。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-854848cc3f4fd679"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::success`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::success(Context &c) noexcept
@brief 记录区间扣除完成。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-63ef5e7e09f01a63"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_left`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_left(Context &c) noexcept
@brief 裁去左边并推进索引。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f153e8658b155b53"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_right`

```doxygen
@fn static void xv::memory::PhysicalMemoryLayout::SubtractFlow::trim_right(Context &c) noexcept
@brief 裁去右边并推进索引。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-113af34f7c19d193"></a>

## `xv::memory::PhysicalMemoryLayout::SubtractFlow::whole`

```doxygen
@fn static bool xv::memory::PhysicalMemoryLayout::SubtractFlow::whole(Context &c) noexcept
@brief 检查预留是否覆盖整个当前区间。
@details 入口保留原满数组中间拆分预检；迭代选择重叠区间，按覆盖形状删除、裁剪或拆分，紧凑数组移位保持直接循环。 下一节点由图策略声明，业务结果与控制信号分离。
```
