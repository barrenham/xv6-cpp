# `kernel/memory/paging/kernel_page_table.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/kernel_page_table.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.arch`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::KernelPageTable::activate_hart`](kernel_page_table.cpp.md#fn-4f82617b1488d29d)
- [`xv::memory::KernelPageTable::allocate_process_stack`](kernel_page_table.cpp.md#fn-6829b16b5a4aa2a3)
- [`xv::memory::KernelPageTable::commit_stack`](kernel_page_table.cpp.md#fn-5492462d0bf0c840)
- [`xv::memory::KernelPageTable::free_process_stack`](kernel_page_table.cpp.md#fn-481636a4fc87518c)
- [`xv::memory::KernelPageTable::grow_process_stack`](kernel_page_table.cpp.md#fn-9be72da917a5498c)
- [`xv::memory::KernelPageTable::handle_stack_fault`](kernel_page_table.cpp.md#fn-88c699cd58c7fc23)
- [`xv::memory::KernelPageTable::initialize`](kernel_page_table.cpp.md#fn-b348ddac60edd845)
- [`xv::memory::KernelPageTable::region`](kernel_page_table.cpp.md#fn-fe0e0ad8b52fd3f2)
- [`xv::memory::KernelPageTable::release_stack`](kernel_page_table.cpp.md#fn-5b254a60dc9c4721)
- [`xv::memory::KernelPageTable::root`](kernel_page_table.cpp.md#fn-792824e0d2159c11)
- [`xv::memory::KernelPageTable::statistics`](kernel_page_table.cpp.md#fn-9d8162325a420bf9)
- [`xv::memory::kernel_page_table`](kernel_page_table.cpp.md#fn-1f3d315da469e7cf)
- [`xv::memory::make_kernel_page_table`](kernel_page_table.cpp.md#fn-0da538a390ac61bc)
- [`xv::memory::map_initial_range`](kernel_page_table.cpp.md#fn-ed33004ce7e7efb6)

<a id="fn-b348ddac60edd845"></a>

## `xv::memory::KernelPageTable::initialize`

```doxygen
@fn void xv::memory::KernelPageTable::initialize()
@brief 建立共享内核页表和栈窗口稳定目录。
@details 构建时登记静态映射，初始化区域锁，并预建高地址栈窗口的共享上级目录；这些目录终生存在，栈的独占叶表在每次创建时准备。
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
@fn pagetable_t xv::memory::make_kernel_page_table(VirtualRegions &regions)
@brief 构建带区域登记的内核直接映射根表。
@details 所有设备、内核文本/数据、可用 RAM 与 trampoline 映射先进入共享 VirtualRegions，再由最大页策略建立 PTE；原有文本权限、RAM 边界与映射统计自检保留。
```

<a id="fn-5492462d0bf0c840"></a>

## `xv::memory::KernelPageTable::commit_stack`

```doxygen
@fn bool xv::memory::KernelPageTable::commit_stack(unsigned long address, unsigned long lowest)
@brief 为一个独占栈区域补齐向下增长的物理页。
@details 持栈区域锁，按 4 KiB 向下补齐到最低地址；成功或分配失败返回前均刷新本核 TLB。失败可能保留已映射前缀，由创建回滚或最终销毁回收。保护区始终不映射。
```

<a id="fn-5b254a60dc9c4721"></a>

## `xv::memory::KernelPageTable::release_stack`

```doxygen
@fn void xv::memory::KernelPageTable::release_stack(unsigned long address)
@brief 撤销独占栈的叶表与全部已提交物理页。
@details 调用时持区域锁且目标进程不再执行；先清空上级指向并刷新本核 TLB，再归还叶帧和叶表。共享上级目录不回收；调度切换的 fencing 防止跨核复用旧翻译。
```

<a id="fn-6829b16b5a4aa2a3"></a>

## `xv::memory::KernelPageTable::allocate_process_stack`

```doxygen
@fn unsigned long xv::memory::KernelPageTable::allocate_process_stack()
@brief 保留完整栈与保护区并准备无锁补页所需叶表。
@details 区域锁内选空槽，登记等容量保护区及默认 2 MiB 栈，预建每个 2 MiB 段的叶表，再提交顶部 32 KiB。任一步失败回收 PTE、叶表和区域，成功后才以 release 语义发布位图。
```

<a id="fn-9be72da917a5498c"></a>

## `xv::memory::KernelPageTable::grow_process_stack`

```doxygen
@fn bool xv::memory::KernelPageTable::grow_process_stack(unsigned long address, unsigned long lowest)
@brief 在有效且已保留的栈内补齐物理页。
@details 检查区域对齐、槽归属及最低地址不越界；区域锁内调用 commit_stack，成功返回 true。容量由 XV_KERNEL_STACK_MIB 构建参数决定，超出预留范围不扩张。
```

<a id="fn-481636a4fc87518c"></a>

## `xv::memory::KernelPageTable::free_process_stack`

```doxygen
@fn void xv::memory::KernelPageTable::free_process_stack(unsigned long address)
@brief 回收已停止执行的栈及其完整地址归属。
@details 区域锁内验证槽归属，取消映射并释放全部已提交页和独占叶表，删除栈与保护区登记，最后清除发布位。调用方须先等待目标上下文和进程锁交接完成。
```

<a id="fn-ed33004ce7e7efb6"></a>

## `xv::memory::map_initial_range`

```doxygen
@fn void xv::memory::map_initial_range(VirtualRegions &regions, pagetable_t table, unsigned long va, unsigned long pa, unsigned long bytes, int permissions)
@brief 在登记虚拟归属后建立一个启动映射。
@details 按地址识别设备、直接映射或 trampoline 特殊区域；reserve 拒绝重叠或 OOM 时启动失败，否则交给实际 Sv39 映射器选择页大小和写入权限。
```

<a id="fn-88c699cd58c7fc23"></a>

## `xv::memory::KernelPageTable::handle_stack_fault`

```doxygen
@fn FaultResult xv::memory::KernelPageTable::handle_stack_fault(unsigned long base, unsigned long stack, unsigned long fault, FaultAccess access, void *(*frame)())
@brief 在不取得区域或分配器锁的路径上处理栈缺页。
@details 验证当前所有者的范围、槽位和访问类型；已映射页返回权限错误或 AlreadyMapped，禁止假装补页成功。仅使用预建叶表和传入帧提供者补齐缺失页，成功及部分失败均刷新本核 TLB。处理期间当前栈不能被销毁。
```

<a id="fn-fe0e0ad8b52fd3f2"></a>

## `xv::memory::KernelPageTable::region`

```doxygen
@fn bool xv::memory::KernelPageTable::region(unsigned long address, VirtualRegion &result)
@brief 复制一条稳定的内核虚拟区域记录。
@details 持区域锁查询并把匹配项复制到调用者缓冲，找不到返回 false；不把锁释放后可能失效的内部指针暴露给调用者。
```

<a id="fn-9d8162325a420bf9"></a>

## `xv::memory::KernelPageTable::statistics`

```doxygen
@fn PageTableStatistics xv::memory::KernelPageTable::statistics()
@brief 在保护页表节点生命周期的锁内统计内核映射。
@details 取得区域锁后构造 Sv39 只读视图，避免与独占栈叶表回收并发而访问释放的元数据。无锁栈补页仍可能增加叶映射，因此计数是观测值，不是全局原子快照。
```
