# `kernel/memory/allocator/allocator_service.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/allocator/allocator_service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:buddy`, `:freelist`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::allocator`](allocator_service.cpp.md#fn-f2b789758ee2f161)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e42f08f40f678227)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-6294cdfa90bca295)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e58331769ab453a3)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-8b1954f8cb6eb078)

<a id="fn-e58331769ab453a3"></a>

## `xv::memory::initialize`

```doxygen
@fn bool xv::memory::initialize(AllocatorKind kind, const BootMemoryInfo &memory, PhysicalAddress first_usable)
@brief 物理页所有权、地址范围、页表和用户内存复制：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress::from_pointer` → `initialize` → `PhysicalMemoryRange`。 布局初始化完成后按 usable 区间总页数设置空闲页计数，再发布活动分配器。
```

<a id="fn-6294cdfa90bca295"></a>

## `xv::memory::initialize`

```doxygen
@fn bool xv::memory::initialize(AllocatorKind kind, BootMemorySource &source, PhysicalAddress first_usable)
@brief 物理页所有权、地址范围、页表和用户内存复制：建立该对象开始工作所需的初始状态。
@param kind 本次操作使用的 种类 值。
@param source 输入数据位置或源对象。
@param first_usable 允许分配器使用的首个物理地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`source.discover` → `initialize`。
@return 各分支实际返回：`source.discover(memory) && initialize(kind, memory, first_usable)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e42f08f40f678227"></a>

## `xv::memory::initialize`

```doxygen
@fn bool xv::memory::initialize(AllocatorKind kind, PhysicalMemoryRange range)
@brief 物理页所有权、地址范围、页表和用户内存复制：建立该对象开始工作所需的初始状态。
@param kind 本次操作使用的 种类 值。
@param range 本次操作使用的 区间 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`range.empty` → `range.begin` → `range.end` → `align_down` → `memory.set_ram` → `PhysicalMemoryRange` → `initialize`。
@details 需要特别跟踪的条件：`range.empty() || range.begin() < detail::KernelLoadAddress || range.end() > detail::DirectMapLimit`；`ram_end <= range.begin()`；`!memory.set_ram(PhysicalMemoryRange(detail::KernelLoadAddress, ram_end))`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`initialize(kind, memory, range.begin())`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8b1954f8cb6eb078"></a>

## `xv::memory::initialize`

```doxygen
@fn bool xv::memory::initialize(AllocatorKind kind, void *first, void *last)
@brief 物理页所有权、地址范围、页表和用户内存复制：建立该对象开始工作所需的初始状态。
@param kind 本次操作使用的 种类 值。
@param first 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param last 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PhysicalAddress::from_pointer` → `initialize` → `PhysicalMemoryRange`。
@details 需要特别跟踪的条件：`begin >= end`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`initialize(kind, PhysicalMemoryRange(begin, end))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f2b789758ee2f161"></a>

## `xv::memory::allocator`

```doxygen
@fn PhysicalPageAllocator &xv::memory::allocator()
@brief 物理页所有权、地址范围、页表和用户内存复制：分配器。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `detail::fail`。
@details 需要特别跟踪的条件：`!selected`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`*selected`。应结合上面的条件检查区分正常值、空值和错误值。
```
