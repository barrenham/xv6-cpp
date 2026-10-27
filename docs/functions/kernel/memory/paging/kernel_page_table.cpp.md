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
- [`xv::memory::KernelPageTable::initialize`](kernel_page_table.cpp.md#fn-b348ddac60edd845)
- [`xv::memory::KernelPageTable::release_stack`](kernel_page_table.cpp.md#fn-5b254a60dc9c4721)
- [`xv::memory::KernelPageTable::root`](kernel_page_table.cpp.md#fn-792824e0d2159c11)
- [`xv::memory::kernel_page_table`](kernel_page_table.cpp.md#fn-1f3d315da469e7cf)
- [`xv::memory::make_kernel_page_table`](kernel_page_table.cpp.md#fn-0da538a390ac61bc)

<a id="fn-b348ddac60edd845"></a>

## `xv::memory::KernelPageTable::initialize`

```doxygen
@fn void xv::memory::KernelPageTable::initialize()
@brief 建立内核共享页表及虚拟栈区域锁。
@details 只预建高地址栈窗口的上级目录，后续每个栈独占其叶页表；目录在系统生命周期内稳定，初始不分配全部栈的物理帧。
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
@fn pagetable_t xv::memory::make_kernel_page_table()
@brief 建立内核恒等映射及设备窗口。
@details VirtIO MMIO 映射扩为相邻两页以容纳系统盘和数据盘；其余内核映射、权限和大页策略保持原有流程。
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
@brief 保留独立虚拟区域并提交初始栈页。
@details 区域位图与物理页编号解耦；默认 2 MiB 容量、32 KiB 初始物理页，旁边保留等容量保护区。失败清理部分帧与叶表，返回零；成功标记槽并返回栈底。
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
@brief 归还已停止进程的完整栈区域。
@details 区域锁内校验地址和槽归属，释放已提交页和独占叶表，再清位图。非法或重复释放触发 panic；调用方必须已完成运行上下文交接。
```
