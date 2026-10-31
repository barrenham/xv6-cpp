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
- [`xv::memory::KernelPageTable::AllocateFlow::ok`](kernel_page_table.cpp.md#fn-5d8156815904af2b)
- [`xv::memory::KernelPageTable::AllocateFlow::prepare`](kernel_page_table.cpp.md#fn-7bfb28dc99e1bbce)
- [`xv::memory::KernelPageTable::AllocateFlow::publish`](kernel_page_table.cpp.md#fn-0ea5937c11a261e1)
- [`xv::memory::KernelPageTable::AllocateFlow::region`](kernel_page_table.cpp.md#fn-03b093a19f994089)
- [`xv::memory::KernelPageTable::AllocateFlow::rollback`](kernel_page_table.cpp.md#fn-04c01fcb61c820f0)
- [`xv::memory::KernelPageTable::AllocateFlow::unreserve`](kernel_page_table.cpp.md#fn-e42858da4924218a)
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
- [`xv::memory::KernelPageTable::InitializeFlow::mappings`](kernel_page_table.cpp.md#fn-969ad613e9d459c5)
- [`xv::memory::KernelPageTable::InitializeFlow::pool`](kernel_page_table.cpp.md#fn-2502225076811c6c)
- [`xv::memory::KernelPageTable::ReleaseFlow::detach`](kernel_page_table.cpp.md#fn-a289736b6f264882)
- [`xv::memory::KernelPageTable::ReleaseFlow::detached`](kernel_page_table.cpp.md#fn-0456693b1b7aeace)
- [`xv::memory::KernelPageTable::ReleaseFlow::free`](kernel_page_table.cpp.md#fn-4de0b37e401167cb)
- [`xv::memory::KernelPageTable::activate_hart`](kernel_page_table.cpp.md#fn-4f82617b1488d29d)
- [`xv::memory::KernelPageTable::allocate_process_stack`](kernel_page_table.cpp.md#fn-6829b16b5a4aa2a3)
- [`xv::memory::KernelPageTable::commit_stack`](kernel_page_table.cpp.md#fn-5492462d0bf0c840)
- [`xv::memory::KernelPageTable::free_process_stack`](kernel_page_table.cpp.md#fn-481636a4fc87518c)
- [`xv::memory::KernelPageTable::grow_process_stack`](kernel_page_table.cpp.md#fn-9be72da917a5498c)
- [`xv::memory::KernelPageTable::handle_stack_fault`](kernel_page_table.cpp.md#fn-88c699cd58c7fc23)
- [`xv::memory::KernelPageTable::in_stack_window`](kernel_page_table.cpp.md#fn-1de07bd91a367361)
- [`xv::memory::KernelPageTable::initialize`](kernel_page_table.cpp.md#fn-b348ddac60edd845)
- [`xv::memory::KernelPageTable::owns_stack`](kernel_page_table.cpp.md#fn-b085911204d0ab2c)
- [`xv::memory::KernelPageTable::region`](kernel_page_table.cpp.md#fn-fe0e0ad8b52fd3f2)
- [`xv::memory::KernelPageTable::release_stack`](kernel_page_table.cpp.md#fn-5b254a60dc9c4721)
- [`xv::memory::KernelPageTable::root`](kernel_page_table.cpp.md#fn-792824e0d2159c11)
- [`xv::memory::KernelPageTable::stack_directory`](kernel_page_table.cpp.md#fn-2c7540e00b975f1d)
- [`xv::memory::KernelPageTable::statistics`](kernel_page_table.cpp.md#fn-9d8162325a420bf9)
- [`xv::memory::kernel_page_table`](kernel_page_table.cpp.md#fn-1f3d315da469e7cf)
- [`xv::memory::map_initial_range`](kernel_page_table.cpp.md#fn-90a0ed90fecb0108)
- [`xv::memory::make_kernel_page_table`](kernel_page_table.cpp.md#fn-73b6149db6de6b8a)

<a id="fn-b348ddac60edd845"></a>

## `xv::memory::KernelPageTable::initialize`

```doxygen
@fn void xv::memory::KernelPageTable::initialize()
@brief 建立共享内核页表并初始化动态栈地址池。
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
@details 调用时持区域锁且目标进程不再执行；先清空上级指向并刷新本核 TLB，再归还叶帧和叶表。上级目录没有任何有效子项后也摘除并回收；运行栈固定仍被使用的各级目录，调度切换执行原有 fencing。
@details 流程由 ReleaseFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-6829b16b5a4aa2a3"></a>

## `xv::memory::KernelPageTable::allocate_process_stack`

```doxygen
@fn unsigned long xv::memory::KernelPageTable::allocate_process_stack()
@brief 保留完整栈与保护区并准备无锁补页所需叶表。
@details 区域锁内向地址池申请栈及等大保护区，登记区域，预建每个 2 MiB 段的叶表，再提交顶部 32 KiB。失败回收栈页、叶表、区域和区间；成功后以 release 设置首个目录的 RSW bit 9 发布标记。
@details 流程由 AllocateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-9be72da917a5498c"></a>

## `xv::memory::KernelPageTable::grow_process_stack`

```doxygen
@fn bool xv::memory::KernelPageTable::grow_process_stack(unsigned long address, unsigned long lowest)
@brief 在有效且已保留的栈内补齐物理页。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-481636a4fc87518c"></a>

## `xv::memory::KernelPageTable::free_process_stack`

```doxygen
@fn void xv::memory::KernelPageTable::free_process_stack(unsigned long address)
@brief 回收已停止执行的栈及其完整地址归属。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-88c699cd58c7fc23"></a>

## `xv::memory::KernelPageTable::handle_stack_fault`

```doxygen
@fn FaultResult xv::memory::KernelPageTable::handle_stack_fault(unsigned long base, unsigned long stack, unsigned long fault, FaultAccess access, void *(*frame)())
@brief 在不取得区域或分配器锁的路径上处理栈缺页。
@details 验证当前所有者的范围、首个目录发布标记和访问类型；已映射页返回权限错误或 AlreadyMapped，禁止假装补页成功。仅使用预建叶表和传入帧提供者补齐缺失页，成功及部分失败均刷新本核 TLB。处理期间当前栈不能被销毁。
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
@brief 检查是否已经取得地址池区间。
@details 入口词法 ScopedLock 持栈区域锁；先申请地址区间，再登记 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布目录标记；失败时撤销区域、映射及地址区间占用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-88ca206c5e90a254"></a>

## `xv::memory::KernelPageTable::AllocateFlow::commit`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::commit(Context &c) noexcept
@brief 提交原要求的初始栈页。
@details 入口词法 ScopedLock 持栈区域锁；先申请地址区间，再登记 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布目录标记；失败时撤销区域、映射及地址区间占用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0046a5affa33bce4"></a>

## `xv::memory::KernelPageTable::AllocateFlow::erase_guard`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::erase_guard(Context &c) noexcept
@brief 栈区域预留失败时只撤销 guard 区域。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-55b586b038909582"></a>

## `xv::memory::KernelPageTable::AllocateFlow::find`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::find(Context &c) noexcept
@brief 从红黑树地址池申请栈与保护区所需的连续虚拟区间。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-5d8156815904af2b"></a>

## `xv::memory::KernelPageTable::AllocateFlow::ok`

```doxygen
@fn static bool xv::memory::KernelPageTable::AllocateFlow::ok(Context &c) noexcept
@brief 检查最近一次区域或页表阶段是否成功。
@details 入口词法 ScopedLock 持栈区域锁；先申请地址区间，再登记 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布目录标记；失败时撤销区域、映射及地址区间占用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7bfb28dc99e1bbce"></a>

## `xv::memory::KernelPageTable::AllocateFlow::prepare`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::prepare(Context &c) noexcept
@brief 按叶表跨度预建该栈的全部叶表。
@details 入口词法 ScopedLock 持栈区域锁；先申请地址区间，再登记 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布目录标记；失败时撤销区域、映射及地址区间占用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0ea5937c11a261e1"></a>

## `xv::memory::KernelPageTable::AllocateFlow::publish`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::publish(Context &c) noexcept
@brief 以 release 设置首个栈目录发布标记并记录成功地址。
@details 入口词法 ScopedLock 持栈区域锁；先申请地址区间，再登记 guard 与栈区域、准备叶表、提交初始页，成功才以 release 发布目录标记；失败时撤销区域、映射及地址区间占用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03b093a19f994089"></a>

## `xv::memory::KernelPageTable::AllocateFlow::region`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::region(Context &c) noexcept
@brief 预留可按需增长的内核栈区域。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-04c01fcb61c820f0"></a>

## `xv::memory::KernelPageTable::AllocateFlow::rollback`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::rollback(Context &c) noexcept
@brief 释放候选栈页表与帧并撤销两段区域。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
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
@brief 以 acquire 检查首个栈目录仍有有效的发布标记。
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
@details 入口词法锁保护地址池与区域表并验证准确区间归属；先摘除目录和回收映射，再删除区域，最后归还地址区间，防止提前重用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a31a077954fcfa63"></a>

## `xv::memory::KernelPageTable::FreeFlow::release`

```doxygen
@fn static void xv::memory::KernelPageTable::FreeFlow::release(Context &c) noexcept
@brief 释放已停止栈的叶表与数据帧。
@details 入口词法锁保护地址池与区域表并验证准确区间归属；先摘除目录和回收映射，再删除区域，最后归还地址区间，防止提前重用。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-969ad613e9d459c5"></a>

## `xv::memory::KernelPageTable::InitializeFlow::mappings`

```doxygen
@fn static void xv::memory::KernelPageTable::InitializeFlow::mappings(Context &c) noexcept
@brief 构建内核初始映射并设置根指针。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-a289736b6f264882"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::detach`

```doxygen
@fn static void xv::memory::KernelPageTable::ReleaseFlow::detach(Context &c) noexcept
@brief 扫描并摘除叶目录及已空的上级目录，再刷新 TLB。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链，同时摘除已无有效子项的上级目录，fence 后归还帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0456693b1b7aeace"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::detached`

```doxygen
@fn static bool xv::memory::KernelPageTable::ReleaseFlow::detached(Context &c) noexcept
@brief 检查本轮是否取得已摘除叶表。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链，同时摘除已无有效子项的上级目录，fence 后归还帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4de0b37e401167cb"></a>

## `xv::memory::KernelPageTable::ReleaseFlow::free`

```doxygen
@fn static void xv::memory::KernelPageTable::ReleaseFlow::free(Context &c) noexcept
@brief 回收已摘除叶表的数据帧、叶表页和空上级目录，并推进区间游标。
@details 调用者持栈区域锁且栈已停止执行；逐叶目录摘链，同时摘除已无有效子项的上级目录，fence 后归还帧和页表页，不读取已归还页。 下一节点由图策略声明，业务结果与控制信号分离。
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
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
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
<a id="fn-2502225076811c6c"></a>

## `xv::memory::KernelPageTable::InitializeFlow::pool`

```doxygen
@fn static void xv::memory::KernelPageTable::InitializeFlow::pool(Context &c) noexcept
@brief 初始化栈区域锁，并在发现的 RAM 末端对齐后建立动态虚拟地址池；失败终止启动。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-e42858da4924218a"></a>

## `xv::memory::KernelPageTable::AllocateFlow::unreserve`

```doxygen
@fn static void xv::memory::KernelPageTable::AllocateFlow::unreserve(Context &c) noexcept
@brief 将尚未成功发布的栈和保护区区间归还地址池，归还失败表示所有权协议损坏。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-1de07bd91a367361"></a>

## `xv::memory::KernelPageTable::in_stack_window`

```doxygen
@fn bool xv::memory::KernelPageTable::in_stack_window(unsigned long address) const
@brief 检查地址是否位于 RAM 末端对齐之后到 KSTACK_TOP 之前的运行时窗口；不查询树或取得锁。
@details 遵守 KernelPageTable 的运行栈生命周期、区域锁与发布协议；详见内核虚拟地址池设计说明。
```

<a id="fn-2c7540e00b975f1d"></a>

## `xv::memory::KernelPageTable::stack_directory`

```doxygen
@fn unsigned long *xv::memory::KernelPageTable::stack_directory(unsigned long address) const
@brief 以 acquire 读取稳定根目录，返回目标地址的中间目录 PTE 指针；缺失或大页目录返回空指针。
@details 遵守 KernelPageTable 的运行栈生命周期、区域锁与发布协议；详见内核虚拟地址池设计说明。
```

<a id="fn-b085911204d0ab2c"></a>

## `xv::memory::KernelPageTable::owns_stack`

```doxygen
@fn bool xv::memory::KernelPageTable::owns_stack(unsigned long address) const
@brief 以 acquire 检查首个栈目录的有效位及 RSW bit 9 发布标记；调用方固定栈生命周期，不遍历可变树。
@details 遵守 KernelPageTable 的运行栈生命周期、区域锁与发布协议；详见内核虚拟地址池设计说明。
```

<a id="fn-90a0ed90fecb0108"></a>

## `xv::memory::map_initial_range`

```doxygen
@fn void xv::memory::map_initial_range(VirtualAddressPool &regions, pagetable_t table, unsigned long va, unsigned long pa, unsigned long bytes, int permissions)
@brief 在登记虚拟归属后建立一个启动映射。
@details 按地址识别设备、直接映射或 trampoline 特殊区域；reserve 拒绝重叠或 OOM 时启动失败，否则交给实际 Sv39 映射器选择页大小和写入权限。
```

<a id="fn-73b6149db6de6b8a"></a>

## `xv::memory::make_kernel_page_table`

```doxygen
@fn pagetable_t xv::memory::make_kernel_page_table(VirtualAddressPool &regions)
@brief 构建带区域登记的内核直接映射根表。
@details 所有设备、内核文本/数据、可用 RAM 与 trampoline 映射先进入共享 VirtualAddressPool，再由最大页策略建立 PTE；原有文本权限、RAM 边界与映射统计自检保留。
@details 流程由 KernelBuildFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```
