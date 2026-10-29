# `kernel/memory/paging/kernel_page_table.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/kernel_page_table.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.arch`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::KernelBuildFlow::check`](kernel_page_table.cpp.md#fn-20b859e92795e7ea)
- [`xv::memory::KernelBuildFlow::devices_text`](kernel_page_table.cpp.md#fn-a25fe9f111046a0a)
- [`xv::memory::KernelBuildFlow::direct`](kernel_page_table.cpp.md#fn-0478db392b7e5ee1)
- [`xv::memory::KernelBuildFlow::fail`](kernel_page_table.cpp.md#fn-b77cebb2373f54a3)
- [`xv::memory::KernelBuildFlow::map_trampoline`](kernel_page_table.cpp.md#fn-a25ae30475175976)
- [`xv::memory::KernelBuildFlow::release`](kernel_page_table.cpp.md#fn-12ecce2f235bc71b)
- [`xv::memory::KernelBuildFlow::report`](kernel_page_table.cpp.md#fn-39041b732ac152bf)
- [`xv::memory::KernelBuildFlow::root`](kernel_page_table.cpp.md#fn-8b95db6db93e514e)
- [`xv::memory::KernelPageTable::AllocateFlow::available`](kernel_page_table.cpp.md#fn-0111f97804df8692)
- [`xv::memory::KernelPageTable::AllocateFlow::commit`](kernel_page_table.cpp.md#fn-88ca206c5e90a254)
- [`xv::memory::KernelPageTable::AllocateFlow::erase_guard`](kernel_page_table.cpp.md#fn-0046a5affa33bce4)
- [`xv::memory::KernelPageTable::AllocateFlow::find`](kernel_page_table.cpp.md#fn-55b586b038909582)
- [`xv::memory::KernelPageTable::AllocateFlow::guard`](kernel_page_table.cpp.md#fn-348c2b2f098b8b70)
- [`xv::memory::KernelPageTable::AllocateFlow::ok`](kernel_page_table.cpp.md#fn-5d8156815904af2b)
- [`xv::memory::KernelPageTable::AllocateFlow::prepare`](kernel_page_table.cpp.md#fn-7bfb28dc99e1bbce)
- [`xv::memory::KernelPageTable::AllocateFlow::publish`](kernel_page_table.cpp.md#fn-0ea5937c11a261e1)
- [`xv::memory::KernelPageTable::AllocateFlow::region`](kernel_page_table.cpp.md#fn-03b093a19f994089)
- [`xv::memory::KernelPageTable::AllocateFlow::rollback`](kernel_page_table.cpp.md#fn-04c01fcb61c820f0)
- [`xv::memory::KernelPageTable::CommitFlow::allocate`](kernel_page_table.cpp.md#fn-2206fece6fa60e00)
- [`xv::memory::KernelPageTable::CommitFlow::allocated`](kernel_page_table.cpp.md#fn-fb2587f34e4250ee)
- [`xv::memory::KernelPageTable::CommitFlow::entry`](kernel_page_table.cpp.md#fn-fe930c51205577d6)
- [`xv::memory::KernelPageTable::CommitFlow::fence`](kernel_page_table.cpp.md#fn-3a48446e96131915)
- [`xv::memory::KernelPageTable::CommitFlow::mapped`](kernel_page_table.cpp.md#fn-4e434f627e717692)
- [`xv::memory::KernelPageTable::CommitFlow::publish`](kernel_page_table.cpp.md#fn-b60405c1ffecdda6)
- [`xv::memory::KernelPageTable::CommitFlow::remaining`](kernel_page_table.cpp.md#fn-50cf8171cd5889ea)
- [`xv::memory::KernelPageTable::CommitFlow::success`](kernel_page_table.cpp.md#fn-313e0212678bcb33)
- [`xv::memory::KernelPageTable::CommitFlow::walk`](kernel_page_table.cpp.md#fn-d3c5d1a48e159e72)
- [`xv::memory::KernelPageTable::FaultFlow::allocate`](kernel_page_table.cpp.md#fn-552fb4ce0919694e)
- [`xv::memory::KernelPageTable::FaultFlow::allocated`](kernel_page_table.cpp.md#fn-ff263f2e429b2782)
- [`xv::memory::KernelPageTable::FaultFlow::data`](kernel_page_table.cpp.md#fn-85f114bdb2148c99)
- [`xv::memory::KernelPageTable::FaultFlow::entry`](kernel_page_table.cpp.md#fn-d6f02229c3c18484)
- [`xv::memory::KernelPageTable::FaultFlow::existing`](kernel_page_table.cpp.md#fn-7eea0ccf4561fd23)
- [`xv::memory::KernelPageTable::FaultFlow::fence`](kernel_page_table.cpp.md#fn-a7d5f80d80c9f54c)
- [`xv::memory::KernelPageTable::FaultFlow::invalid`](kernel_page_table.cpp.md#fn-eec02370da490f4f)
- [`xv::memory::KernelPageTable::FaultFlow::lookup`](kernel_page_table.cpp.md#fn-9f77a487c88673ba)
- [`xv::memory::KernelPageTable::FaultFlow::mapped`](kernel_page_table.cpp.md#fn-47ca91daaf8f018d)
- [`xv::memory::KernelPageTable::FaultFlow::out_of_memory`](kernel_page_table.cpp.md#fn-696ba7791a09d47f)
- [`xv::memory::KernelPageTable::FaultFlow::owned`](kernel_page_table.cpp.md#fn-c27bb44afe277970)
- [`xv::memory::KernelPageTable::FaultFlow::permission`](kernel_page_table.cpp.md#fn-01572d9fcc8e2ab1)
- [`xv::memory::KernelPageTable::FaultFlow::publish`](kernel_page_table.cpp.md#fn-f9a1fe224afae5a6)
- [`xv::memory::KernelPageTable::FaultFlow::remaining`](kernel_page_table.cpp.md#fn-a3ba1d269e293042)
- [`xv::memory::KernelPageTable::FaultFlow::resolved`](kernel_page_table.cpp.md#fn-e4330cf0b0610365)
- [`xv::memory::KernelPageTable::FaultFlow::start`](kernel_page_table.cpp.md#fn-ff580005f91b555b)
- [`xv::memory::KernelPageTable::FaultFlow::valid`](kernel_page_table.cpp.md#fn-96495e812c2538c2)
- [`xv::memory::KernelPageTable::FaultFlow::walk`](kernel_page_table.cpp.md#fn-3fb74ae6ced342cb)
- [`xv::memory::KernelPageTable::FreeFlow::erase`](kernel_page_table.cpp.md#fn-36947fecf4b42f8e)
- [`xv::memory::KernelPageTable::FreeFlow::release`](kernel_page_table.cpp.md#fn-a31a077954fcfa63)
- [`xv::memory::KernelPageTable::FreeFlow::unpublish`](kernel_page_table.cpp.md#fn-3372014d429f8d3e)
- [`xv::memory::KernelPageTable::InitializeFlow::directories`](kernel_page_table.cpp.md#fn-824611c86cc95507)
- [`xv::memory::KernelPageTable::InitializeFlow::mappings`](kernel_page_table.cpp.md#fn-969ad613e9d459c5)
- [`xv::memory::KernelPageTable::ReleaseFlow::detach`](kernel_page_table.cpp.md#fn-a289736b6f264882)
- [`xv::memory::KernelPageTable::ReleaseFlow::detached`](kernel_page_table.cpp.md#fn-0456693b1b7aeace)
- [`xv::memory::KernelPageTable::ReleaseFlow::free`](kernel_page_table.cpp.md#fn-4de0b37e401167cb)
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
@details 流程由 InitializeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 KernelBuildFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5492462d0bf0c840"></a>

## `xv::memory::KernelPageTable::commit_stack`

```doxygen
@fn bool xv::memory::KernelPageTable::commit_stack(unsigned long address, unsigned long lowest)
@brief 为一个独占栈区域补齐向下增长的物理页。
@details 持栈区域锁，按 4 KiB 向下补齐到最低地址；成功或分配失败返回前均刷新本核 TLB。失败可能保留已映射前缀，由创建回滚或最终销毁回收。保护区始终不映射。
@details 流程由 CommitFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-5b254a60dc9c4721"></a>

## `xv::memory::KernelPageTable::release_stack`

```doxygen
@fn void xv::memory::KernelPageTable::release_stack(unsigned long address)
@brief 撤销独占栈的叶表与全部已提交物理页。
@details 调用时持区域锁且目标进程不再执行；先清空上级指向并刷新本核 TLB，再归还叶帧和叶表。共享上级目录不回收；调度切换的 fencing 防止跨核复用旧翻译。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-6829b16b5a4aa2a3"></a>

## `xv::memory::KernelPageTable::allocate_process_stack`

```doxygen
@fn unsigned long xv::memory::KernelPageTable::allocate_process_stack()
@brief 保留完整栈与保护区并准备无锁补页所需叶表。
@details 区域锁内选空槽，登记等容量保护区及默认 2 MiB 栈，预建每个 2 MiB 段的叶表，再提交顶部 32 KiB。任一步失败回收 PTE、叶表和区域，成功后才以 release 语义发布位图。
@details 流程由 AllocateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 FreeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 FaultFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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

<a id="fn-0111f97804df8692"></a>

## `xv::memory::KernelPageTable::AllocateFlow::available`

```doxygen
@fn static bool xv::memory::KernelPageTable::AllocateFlow::available(Context &c) noexcept
@brief 检查是否仍有可用槽。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-88ca206c5e90a254"></a>

## `xv::memory::KernelPageTable::AllocateFlow::commit`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::commit(Context &c) noexcept
@brief 提交原要求的初始栈页。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0046a5affa33bce4"></a>

## `xv::memory::KernelPageTable::AllocateFlow::erase_guard`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::erase_guard(Context &c) noexcept
@brief 栈区域预留失败时只撤销 guard 区域。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-55b586b038909582"></a>

## `xv::memory::KernelPageTable::AllocateFlow::find`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::find(Context &c) noexcept
@brief 直接扫描位图寻找首个可用栈槽。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-348c2b2f098b8b70"></a>

## `xv::memory::KernelPageTable::AllocateFlow::guard`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::guard(Context &c) noexcept
@brief 计算槽地址并预留相邻 guard 区域。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5d8156815904af2b"></a>

## `xv::memory::KernelPageTable::AllocateFlow::ok`

```doxygen
@fn static bool xv::memory::KernelPageTable::AllocateFlow::ok(Context &c) noexcept
@brief 检查最近一次区域或页表阶段是否成功。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7bfb28dc99e1bbce"></a>

## `xv::memory::KernelPageTable::AllocateFlow::prepare`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::prepare(Context &c) noexcept
@brief 按叶表跨度预建该栈的全部叶表。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0ea5937c11a261e1"></a>

## `xv::memory::KernelPageTable::AllocateFlow::publish`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::publish(Context &c) noexcept
@brief 以 release 发布槽所有权并记录成功地址。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03b093a19f994089"></a>

## `xv::memory::KernelPageTable::AllocateFlow::region`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::region(Context &c) noexcept
@brief 预留可按需增长的内核栈区域。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-04c01fcb61c820f0"></a>

## `xv::memory::KernelPageTable::AllocateFlow::rollback`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::rollback(Context &c) noexcept
@brief 释放候选栈页表与帧并撤销两段区域。
@details 入口词法 ScopedLock 持栈区域锁；依次保留 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布槽位；各失败边保持原回滚范围。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2206fece6fa60e00"></a>

## `xv::memory::KernelPageTable::CommitFlow::allocate`

```doxygen
@fn static void xv::memory::KernelPageTable::CommitFlow::allocate(Context &c) noexcept
@brief 通过原物理分配器取得栈数据帧。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fb2587f34e4250ee"></a>

## `xv::memory::KernelPageTable::CommitFlow::allocated`

```doxygen
@fn static bool xv::memory::KernelPageTable::CommitFlow::allocated(Context &c) noexcept
@brief 检查数据帧是否取得成功。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fe930c51205577d6"></a>

## `xv::memory::KernelPageTable::CommitFlow::entry`

```doxygen
@fn static bool xv::memory::KernelPageTable::CommitFlow::entry(Context &c) noexcept
@brief 检查是否取得页表项。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3a48446e96131915"></a>

## `xv::memory::KernelPageTable::CommitFlow::fence`

```doxygen
@fn static void xv::memory::KernelPageTable::CommitFlow::fence(Context &) noexcept
@brief 在成功或失败返回前刷新本核 TLB。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4e434f627e717692"></a>

## `xv::memory::KernelPageTable::CommitFlow::mapped`

```doxygen
@fn static bool xv::memory::KernelPageTable::CommitFlow::mapped(Context &c) noexcept
@brief 检查当前项是否已有有效映射。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b60405c1ffecdda6"></a>

## `xv::memory::KernelPageTable::CommitFlow::publish`

```doxygen
@fn static void xv::memory::KernelPageTable::CommitFlow::publish(Context &c) noexcept
@brief 发布当前页的读写栈帧映射。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-50cf8171cd5889ea"></a>

## `xv::memory::KernelPageTable::CommitFlow::remaining`

```doxygen
@fn static bool xv::memory::KernelPageTable::CommitFlow::remaining(Context &c) noexcept
@brief 检查是否仍有低于当前游标的页需要提交。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-313e0212678bcb33"></a>

## `xv::memory::KernelPageTable::CommitFlow::success`

```doxygen
@fn static void xv::memory::KernelPageTable::CommitFlow::success(Context &c) noexcept
@brief 记录全部目标页已提交。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d3c5d1a48e159e72"></a>

## `xv::memory::KernelPageTable::CommitFlow::walk`

```doxygen
@fn static void xv::memory::KernelPageTable::CommitFlow::walk(Context &c) noexcept
@brief 下移一页并按原允许分配策略查询页表项。
@details 调用者提供栈区域排他；按页向下提交，已映射页跳过，缺页表或帧时保留既有进度并失败，所有退出统一本核 TLB fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-552fb4ce0919694e"></a>

## `xv::memory::KernelPageTable::FaultFlow::allocate`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::allocate(Context &c) noexcept
@brief 调用调用者提供的无锁帧来源。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ff263f2e429b2782"></a>

## `xv::memory::KernelPageTable::FaultFlow::allocated`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::allocated(Context &c) noexcept
@brief 检查应急帧回调是否取得帧。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-85f114bdb2148c99"></a>

## `xv::memory::KernelPageTable::FaultFlow::data`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::data(Context &c) noexcept
@brief 检查故障不是执行访问。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d6f02229c3c18484"></a>

## `xv::memory::KernelPageTable::FaultFlow::entry`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::entry(Context &c) noexcept
@brief 检查预建页表项是否存在。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7eea0ccf4561fd23"></a>

## `xv::memory::KernelPageTable::FaultFlow::existing`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::existing(Context &c) noexcept
@brief 按原叶合法性和读写权限分类已有映射。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a7d5f80d80c9f54c"></a>

## `xv::memory::KernelPageTable::FaultFlow::fence`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::fence(Context &) noexcept
@brief 在原要求的部分失败或成功边上刷新本核 TLB。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eec02370da490f4f"></a>

## `xv::memory::KernelPageTable::FaultFlow::invalid`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::invalid(Context &c) noexcept
@brief 记录页表或叶格式无效。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9f77a487c88673ba"></a>

## `xv::memory::KernelPageTable::FaultFlow::lookup`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::lookup(Context &c) noexcept
@brief 不分配页表地查询故障页项。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-47ca91daaf8f018d"></a>

## `xv::memory::KernelPageTable::FaultFlow::mapped`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::mapped(Context &c) noexcept
@brief 检查当前页项是否已经有效。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-696ba7791a09d47f"></a>

## `xv::memory::KernelPageTable::FaultFlow::out_of_memory`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::out_of_memory(Context &c) noexcept
@brief 记录帧来源耗尽。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c27bb44afe277970"></a>

## `xv::memory::KernelPageTable::FaultFlow::owned`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::owned(Context &c) noexcept
@brief 以 acquire 检查目标栈槽仍被占用。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-01572d9fcc8e2ab1"></a>

## `xv::memory::KernelPageTable::FaultFlow::permission`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::permission(Context &c) noexcept
@brief 记录禁止执行内核栈的权限结果。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f9a1fe224afae5a6"></a>

## `xv::memory::KernelPageTable::FaultFlow::publish`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::publish(Context &c) noexcept
@brief 以 release 发布新读写栈映射。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a3ba1d269e293042"></a>

## `xv::memory::KernelPageTable::FaultFlow::remaining`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::remaining(Context &c) noexcept
@brief 检查是否还有需要向下检查的栈页。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e4330cf0b0610365"></a>

## `xv::memory::KernelPageTable::FaultFlow::resolved`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::resolved(Context &c) noexcept
@brief 记录目标栈页均已解决。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ff580005f91b555b"></a>

## `xv::memory::KernelPageTable::FaultFlow::start`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::start(Context &c) noexcept
@brief 对齐较低的故障或栈指针并从栈顶准备向下遍历。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-96495e812c2538c2"></a>

## `xv::memory::KernelPageTable::FaultFlow::valid`

```doxygen
@fn static bool xv::memory::KernelPageTable::FaultFlow::valid(Context &c) noexcept
@brief 验证栈基址、故障地址与传入的栈游标位于原范围。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3fb74ae6ced342cb"></a>

## `xv::memory::KernelPageTable::FaultFlow::walk`

```doxygen
@fn static void xv::memory::KernelPageTable::FaultFlow::walk(Context &c) noexcept
@brief 向下推进一页并只查询预建页表。
@details 保持无锁故障路径和调用者提供的应急帧回调；仅查询预建页表，按原读写与已映射规则返回，新增映射以 release 发布，循环迭代且保留每条 fence 边。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-36947fecf4b42f8e"></a>

## `xv::memory::KernelPageTable::FreeFlow::erase`

```doxygen
@fn static void xv::memory::KernelPageTable::FreeFlow::erase(Context &c) noexcept
@brief 删除 guard 与栈的虚拟区域登记。
@details 入口词法锁保护槽位并完成原形状及所有权诊断；先回收映射再删除区域，最后清除位图，防止提前重用槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a31a077954fcfa63"></a>

## `xv::memory::KernelPageTable::FreeFlow::release`

```doxygen
@fn static void xv::memory::KernelPageTable::FreeFlow::release(Context &c) noexcept
@brief 释放已停止栈的叶表与数据帧。
@details 入口词法锁保护槽位并完成原形状及所有权诊断；先回收映射再删除区域，最后清除位图，防止提前重用槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3372014d429f8d3e"></a>

## `xv::memory::KernelPageTable::FreeFlow::unpublish`

```doxygen
@fn static void xv::memory::KernelPageTable::FreeFlow::unpublish(Context &c) noexcept
@brief 以 release 清除槽位所有权。
@details 入口词法锁保护槽位并完成原形状及所有权诊断；先回收映射再删除区域，最后清除位图，防止提前重用槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-824611c86cc95507"></a>

## `xv::memory::KernelPageTable::InitializeFlow::directories`

```doxygen
@fn static void xv::memory::KernelPageTable::InitializeFlow::directories(Context &c) noexcept
@brief 初始化栈区域锁并预建稳定上层页表目录。
@details 入口拒绝重复初始化；先构建并发布根，再初始化栈区域锁并预建稳定上层栈目录，内存不足仍按原协议 panic。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-969ad613e9d459c5"></a>

## `xv::memory::KernelPageTable::InitializeFlow::mappings`

```doxygen
@fn static void xv::memory::KernelPageTable::InitializeFlow::mappings(Context &c) noexcept
@brief 构建内核初始映射并设置根指针。
@details 入口拒绝重复初始化；先构建并发布根，再初始化栈区域锁并预建稳定上层栈目录，内存不足仍按原协议 panic。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a289736b6f264882"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::detach`

```doxygen
@fn static void xv::memory::KernelPageTable::ReleaseFlow::detach(Context &c) noexcept
@brief 扫描至有效叶目录并先清目录项、刷新 TLB。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链并 fence，之后归还该叶的帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0456693b1b7aeace"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::detached`

```doxygen
@fn static bool xv::memory::KernelPageTable::ReleaseFlow::detached(Context &c) noexcept
@brief 检查本轮是否取得已摘除叶表。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链并 fence，之后归还该叶的帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4de0b37e401167cb"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::free`

```doxygen
@fn static void xv::memory::KernelPageTable::ReleaseFlow::free(Context &c) noexcept
@brief 直接回收该叶所有有效帧及叶表页并推进区间游标。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链并 fence，之后归还该叶的帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-20b859e92795e7ea"></a>

## `xv::memory::KernelBuildFlow::check`

```doxygen
@fn static void xv::memory::KernelBuildFlow::check(Context &c) noexcept
@brief 验证文本数据权限、区间边界与 RAM 尾后未映射。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a25fe9f111046a0a"></a>

## `xv::memory::KernelBuildFlow::devices_text`

```doxygen
@fn static void xv::memory::KernelBuildFlow::devices_text(Context &c) noexcept
@brief 按原实验条件映射设备、PLIC 与只读可执行内核文本。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0478db392b7e5ee1"></a>

## `xv::memory::KernelBuildFlow::direct`

```doxygen
@fn static void xv::memory::KernelBuildFlow::direct(Context &c) noexcept
@brief 直接枚举布局中的 RAM 区间并映射数据直映范围。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b77cebb2373f54a3"></a>

## `xv::memory::KernelBuildFlow::fail`

```doxygen
@fn static void xv::memory::KernelBuildFlow::fail(Context &) noexcept
@brief 按原根页分配失败诊断终止。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a25ae30475175976"></a>

## `xv::memory::KernelBuildFlow::map_trampoline`

```doxygen
@fn static void xv::memory::KernelBuildFlow::map_trampoline(Context &c) noexcept
@brief 映射原汇编 trampoline 符号到最高陷阱页。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-12ecce2f235bc71b"></a>

## `xv::memory::KernelBuildFlow::release`

```doxygen
@fn static void xv::memory::KernelBuildFlow::release(Context &c) noexcept
@brief 转移完整页表树所有权并保存根指针。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-39041b732ac152bf"></a>

## `xv::memory::KernelBuildFlow::report`

```doxygen
@fn static void xv::memory::KernelBuildFlow::report(Context &c) noexcept
@brief 读取页表统计并输出原直映自检信息。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8b95db6db93e514e"></a>

## `xv::memory::KernelBuildFlow::root`

```doxygen
@fn static bool xv::memory::KernelBuildFlow::root(Context &c) noexcept
@brief 检查持有者已取得根页表。
@details LargestPagePolicy 与 OwnedSv39PageTable 保持在原构建函数的词法作用域；按原设备、文本、RAM、trampoline 顺序映射与自检，成功最后 release 整棵树。 下一节点由图策略声明，业务结果与控制信号分离。
```
