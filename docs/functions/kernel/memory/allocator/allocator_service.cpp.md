# `kernel/memory/allocator/allocator_service.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/allocator/allocator_service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`:buddy`, `:freelist`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::PhysicalPageAllocator::page_references`](allocator_service.cpp.md#fn-a8098c94ba5f8e20)
- [`xv::memory::PhysicalPageAllocator::release_page`](allocator_service.cpp.md#fn-75415337725bd4f1)
- [`xv::memory::PhysicalPageAllocator::retain_page`](allocator_service.cpp.md#fn-c818c3445e9c8f6d)
- [`xv::memory::allocator`](allocator_service.cpp.md#fn-f2b789758ee2f161)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e42f08f40f678227)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-6294cdfa90bca295)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e58331769ab453a3)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-8b1954f8cb6eb078)

<a id="fn-e58331769ab453a3"></a>

## `xv::memory::initialize`

```doxygen
@fn bool xv::memory::initialize(AllocatorKind kind, const BootMemoryInfo &memory, PhysicalAddress first_usable)
@brief 校验启动内存并发布物理分配器与共享帧计数。
@details 先保留具体分配器元数据，再按四字节对齐预留每 RAM 页一个原子引用计数；具体分配器初始化后清零计数数组。仅成功完成布局、锁与可用页初始化后发布 active，失败不替换已运行分配器。
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

<a id="fn-c818c3445e9c8f6d"></a>

## `xv::memory::PhysicalPageAllocator::retain_page`

```doxygen
@fn void xv::memory::PhysicalPageAllocator::retain_page(void *address)
@brief 为已拥有的用户帧增加一个共享引用。
@details 调用者必须持有原引用，地址须为可用 RAM 内的对齐 4 KiB 帧。零计数代表隐含的独占引用，首次共享原子变为二；后续 CAS 增加，溢出触发 panic。不能对未分配或高阶未拆分块中的任意页调用。
```

<a id="fn-75415337725bd4f1"></a>

## `xv::memory::PhysicalPageAllocator::release_page`

```doxygen
@fn void xv::memory::PhysicalPageAllocator::release_page(void *address)
@brief 归还一个用户映射持有的物理帧引用。
@details 零计数表示从未共享的独占帧，直接交给具体分配器；非零计数 CAS 减一，仅最后一个引用归还物理页。调用方先保证此映射不会再被访问，不得绕过该入口直接释放共享用户帧。
```

<a id="fn-a8098c94ba5f8e20"></a>

## `xv::memory::PhysicalPageAllocator::page_references`

```doxygen
@fn unsigned xv::memory::PhysicalPageAllocator::page_references(void *address) const
@brief 读取用户帧当前的所有者数量。
@details 验证对齐及可用 RAM 范围，原子读取共享数组；零按隐含独占引用返回一。调用者已持有引用，才能据此决定是否原地提升 COW 权限。
```
