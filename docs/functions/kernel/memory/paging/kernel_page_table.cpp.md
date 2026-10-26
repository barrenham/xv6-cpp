# `kernel/memory/paging/kernel_page_table.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/kernel_page_table.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.arch`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::kernel_page_table`](kernel_page_table.cpp.md#fn-1f3d315da469e7cf)
- [`xv::memory::KernelPageTable::activate_hart`](kernel_page_table.cpp.md#fn-4f82617b1488d29d)
- [`xv::memory::KernelPageTable::initialize`](kernel_page_table.cpp.md#fn-b348ddac60edd845)
- [`xv::memory::KernelPageTable::map_process_stack`](kernel_page_table.cpp.md#fn-e70dfce3f258395b)
- [`xv::memory::KernelPageTable::root`](kernel_page_table.cpp.md#fn-792824e0d2159c11)
- [`xv::memory::KernelPageTable::unmap_process_stack`](kernel_page_table.cpp.md#fn-98155d2b3976236b)
- [`xv::memory::make_kernel_page_table`](kernel_page_table.cpp.md#fn-0da538a390ac61bc)

<a id="fn-b348ddac60edd845"></a>

## `xv::memory::KernelPageTable::initialize`

```doxygen
@fn void xv::memory::KernelPageTable::initialize()
@brief 内核页表管理器：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `make_kernel_page_table` → `allocator` → `layout` → `ram` → `page_count` → `KSTACK` → `walk_entry`。
@details 本函数可见的状态更新包括：`root_ = make_kernel_page_table();`；`stack_slots_ = allocator().layout().ram().page_count();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`root_`；`!walk_entry(static_cast<PageTable>(root_), address, 1)`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4f82617b1488d29d"></a>

## `xv::memory::KernelPageTable::activate_hart`

```doxygen
@fn void xv::memory::KernelPageTable::activate_hart() const
@brief 内核页表管理器：activatehart。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `xv::arch::sfence_vma` → `xv::arch::w_satp` → `MAKE_SATP`。
@details 需要特别跟踪的条件：`!root_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e70dfce3f258395b"></a>

## `xv::memory::KernelPageTable::map_process_stack`

```doxygen
@fn unsigned long xv::memory::KernelPageTable::map_process_stack(unsigned long slot, void *physical)
@brief 内核页表管理器：map进程栈。
@param slot 本次操作使用的 slot 值。
@param physical 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`KSTACK` → `walk_entry` → `xv::print::panic` → `PA2PTE` → `__atomic_compare_exchange_n`。
@details 需要特别跟踪的条件：`!root_ || slot >= stack_slots_`；`!entry`；`!__atomic_compare_exchange_n(entry, &empty, value, false, __ATOMIC_RELEASE, __ATOMIC_RELAXED)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`0`；`address`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-98155d2b3976236b"></a>

## `xv::memory::KernelPageTable::unmap_process_stack`

```doxygen
@fn void *xv::memory::KernelPageTable::unmap_process_stack(unsigned long address)
@brief 内核页表管理器：unmap进程栈。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`KSTACK` → `xv::print::panic` → `walk_entry` → `__atomic_exchange_n` → `xv::arch::sfence_vma` → `PTE2PA`。
@details 需要特别跟踪的条件：`!root_ || address > KSTACK(0UL) || (KSTACK(0UL) - address) % (2 * PageSize) != 0 || (KSTACK(0UL) - address) / (2 * PageSize) >= stack_slots_`；`!(value & PTE_V)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`reinterpret_cast<void *>(PTE2PA(value))`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-792824e0d2159c11"></a>

## `xv::memory::KernelPageTable::root`

```doxygen
@fn const void *xv::memory::KernelPageTable::root() const
@brief 内核页表管理器：读取或计算 `root_`，返回当前查询结果。
@return 各分支实际返回：`root_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1f3d315da469e7cf"></a>

## `xv::memory::kernel_page_table`

```doxygen
@fn KernelPageTable &xv::memory::kernel_page_table()
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `shared_kernel_page_table`，返回当前查询结果。
@return 各分支实际返回：`shared_kernel_page_table`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-0da538a390ac61bc"></a>

## `xv::memory::make_kernel_page_table`

```doxygen
@fn pagetable_t xv::memory::make_kernel_page_table()
@brief 建立内核页表与设备映射；E1000 MMIO 窗口扩至 0x80000，覆盖四个设备各 0x20000 的 BAR，继续保留既有内核映射和分配失败处理。
```
