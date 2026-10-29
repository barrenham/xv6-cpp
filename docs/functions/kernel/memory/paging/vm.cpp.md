# `kernel/memory/paging/vm.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/vm.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.runtime`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::AllocateUserFlow::advance`](vm.cpp.md#fn-36e3007fe505e041)
- [`xv::memory::AllocateUserFlow::allocated`](vm.cpp.md#fn-607cf2f78247078a)
- [`xv::memory::AllocateUserFlow::choose`](vm.cpp.md#fn-1a234106d3362fb4)
- [`xv::memory::AllocateUserFlow::drop`](vm.cpp.md#fn-2d59349c90ba801e)
- [`xv::memory::AllocateUserFlow::drop_huge`](vm.cpp.md#fn-880d2966fb2248c6)
- [`xv::memory::AllocateUserFlow::huge_step`](vm.cpp.md#fn-f37f1f4b9fbf4b60)
- [`xv::memory::AllocateUserFlow::map`](vm.cpp.md#fn-f2bcd52b461aadf1)
- [`xv::memory::AllocateUserFlow::map_huge`](vm.cpp.md#fn-22f44fea658e16f6)
- [`xv::memory::AllocateUserFlow::ok`](vm.cpp.md#fn-9799fe87f0e0be1b)
- [`xv::memory::AllocateUserFlow::page`](vm.cpp.md#fn-5de4b91c3c3b279b)
- [`xv::memory::AllocateUserFlow::remaining`](vm.cpp.md#fn-aacb6609aaf4bf60)
- [`xv::memory::AllocateUserFlow::rollback`](vm.cpp.md#fn-e900346272cfe317)
- [`xv::memory::AllocateUserFlow::success`](vm.cpp.md#fn-846f5dd4285df85b)
- [`xv::memory::AllocateUserFlow::zero`](vm.cpp.md#fn-30150a942e82e87a)
- [`xv::memory::CloneUserFlow::advance`](vm.cpp.md#fn-2cd26a723b13a5b8)
- [`xv::memory::CloneUserFlow::failure`](vm.cpp.md#fn-0c929b9d75587678)
- [`xv::memory::CloneUserFlow::found`](vm.cpp.md#fn-73f6b9d6bccb22ae)
- [`xv::memory::CloneUserFlow::huge`](vm.cpp.md#fn-962ae9aafae275d9)
- [`xv::memory::CloneUserFlow::level`](vm.cpp.md#fn-bbb184eaac3c2e80)
- [`xv::memory::CloneUserFlow::map`](vm.cpp.md#fn-f3eb2f619917efab)
- [`xv::memory::CloneUserFlow::ok`](vm.cpp.md#fn-f867fef1e216fa8f)
- [`xv::memory::CloneUserFlow::retain`](vm.cpp.md#fn-695b7264fefd8043)
- [`xv::memory::CloneUserFlow::scan`](vm.cpp.md#fn-12284da7a7a6dec0)
- [`xv::memory::CloneUserFlow::share`](vm.cpp.md#fn-e4566bf638a7315b)
- [`xv::memory::CloneUserFlow::split`](vm.cpp.md#fn-f1ef98f65519e7b2)
- [`xv::memory::CloneUserFlow::success`](vm.cpp.md#fn-c96bcc33dec5c7b1)
- [`xv::memory::DemoteUserFlow::advance`](vm.cpp.md#fn-436b2910d37c5e37)
- [`xv::memory::DemoteUserFlow::huge`](vm.cpp.md#fn-b5ba9fe14c77cf32)
- [`xv::memory::DemoteUserFlow::mapped`](vm.cpp.md#fn-8b263e24731807bf)
- [`xv::memory::DemoteUserFlow::ok`](vm.cpp.md#fn-41d51f1d29e9c932)
- [`xv::memory::DemoteUserFlow::remaining`](vm.cpp.md#fn-4abf5ed5a9e0e017)
- [`xv::memory::DemoteUserFlow::split`](vm.cpp.md#fn-1c40fc84b8514950)
- [`xv::memory::DemoteUserFlow::success`](vm.cpp.md#fn-64ecac50fbcb91c3)
- [`xv::memory::DemoteUserFlow::walk`](vm.cpp.md#fn-f4cffe611e72463a)
- [`xv::memory::FaultUserFlow::accessible`](vm.cpp.md#fn-4f2bbf56bf0dbef8)
- [`xv::memory::FaultUserFlow::allocate`](vm.cpp.md#fn-712ff12943070914)
- [`xv::memory::FaultUserFlow::allocated`](vm.cpp.md#fn-641f2baba3b276ac)
- [`xv::memory::FaultUserFlow::copy`](vm.cpp.md#fn-be9db49f65c12189)
- [`xv::memory::FaultUserFlow::cow`](vm.cpp.md#fn-57acccbe87979243)
- [`xv::memory::FaultUserFlow::demand`](vm.cpp.md#fn-55abec27c3340f0b)
- [`xv::memory::FaultUserFlow::existing`](vm.cpp.md#fn-bb61210372a83a1b)
- [`xv::memory::FaultUserFlow::inside`](vm.cpp.md#fn-7ae2a4b7e780faf2)
- [`xv::memory::FaultUserFlow::invalid`](vm.cpp.md#fn-24c31724e3fcf643)
- [`xv::memory::FaultUserFlow::leaf`](vm.cpp.md#fn-3d298cef182f900d)
- [`xv::memory::FaultUserFlow::map`](vm.cpp.md#fn-a7584efd93d067dd)
- [`xv::memory::FaultUserFlow::mapped`](vm.cpp.md#fn-98fe55d24989518a)
- [`xv::memory::FaultUserFlow::ok`](vm.cpp.md#fn-58c9f853c4f84294)
- [`xv::memory::FaultUserFlow::oom`](vm.cpp.md#fn-0466a62889ddb902)
- [`xv::memory::FaultUserFlow::permission`](vm.cpp.md#fn-5001b8987e1752a5)
- [`xv::memory::FaultUserFlow::promote`](vm.cpp.md#fn-cdfc037e4720c0e0)
- [`xv::memory::FaultUserFlow::publish_cow`](vm.cpp.md#fn-675ec2e7201bc3cf)
- [`xv::memory::FaultUserFlow::region`](vm.cpp.md#fn-3e94a0f98f851f01)
- [`xv::memory::FaultUserFlow::release`](vm.cpp.md#fn-a8891bf229b39cf8)
- [`xv::memory::FaultUserFlow::resolved`](vm.cpp.md#fn-195e1e2cb419ebb2)
- [`xv::memory::FaultUserFlow::rollback`](vm.cpp.md#fn-238b45a630ce7ae0)
- [`xv::memory::FaultUserFlow::shared`](vm.cpp.md#fn-a7c36c9ba376d7e5)
- [`xv::memory::FaultUserFlow::small`](vm.cpp.md#fn-0b42fed8fbbcf597)
- [`xv::memory::FaultUserFlow::sole`](vm.cpp.md#fn-ee08f6669a35b09e)
- [`xv::memory::FaultUserFlow::user`](vm.cpp.md#fn-4f9e536d06ba50ff)
- [`xv::memory::FaultUserFlow::walk`](vm.cpp.md#fn-23525c4d75b08e23)
- [`xv::memory::FaultUserFlow::zero`](vm.cpp.md#fn-45442b951df4bef4)
- [`xv::memory::MapUserFlow::advance`](vm.cpp.md#fn-a1b75a04b79f4a29)
- [`xv::memory::MapUserFlow::found`](vm.cpp.md#fn-fbf95bad7b8f56f0)
- [`xv::memory::MapUserFlow::last`](vm.cpp.md#fn-dada4b3ec871576d)
- [`xv::memory::MapUserFlow::publish`](vm.cpp.md#fn-3cc4dfcc108fa5e1)
- [`xv::memory::MapUserFlow::success`](vm.cpp.md#fn-3ca76c13f38f1bc8)
- [`xv::memory::MapUserFlow::walk`](vm.cpp.md#fn-b9435386578d4d8b)
- [`xv::memory::SuperDemoteFlow::allocate`](vm.cpp.md#fn-6d43491cd285b111)
- [`xv::memory::SuperDemoteFlow::allocated`](vm.cpp.md#fn-2807271a6c0f723b)
- [`xv::memory::SuperDemoteFlow::fill`](vm.cpp.md#fn-03e485b5d4e630c1)
- [`xv::memory::SuperDemoteFlow::may_recycle`](vm.cpp.md#fn-5af9ae56e6cc85f1)
- [`xv::memory::SuperDemoteFlow::publish`](vm.cpp.md#fn-432401329de454fc)
- [`xv::memory::SuperDemoteFlow::recycle`](vm.cpp.md#fn-b768adff83897135)
- [`xv::memory::SuperDemoteFlow::split`](vm.cpp.md#fn-ba05a78dc3cef57c)
- [`xv::memory::SuperMapFlow::existing`](vm.cpp.md#fn-2c159286a78afd27)
- [`xv::memory::SuperMapFlow::found`](vm.cpp.md#fn-fd2db23cf748f91d)
- [`xv::memory::SuperMapFlow::invalid`](vm.cpp.md#fn-7aa88e1c60adc7e6)
- [`xv::memory::SuperMapFlow::level`](vm.cpp.md#fn-beaa1b57749632f9)
- [`xv::memory::SuperMapFlow::publish`](vm.cpp.md#fn-d42b6b39acef98de)
- [`xv::memory::SuperMapFlow::reclaim`](vm.cpp.md#fn-36c014ff4a6e5b06)
- [`xv::memory::SuperMapFlow::walk`](vm.cpp.md#fn-569661b0165707d7)
- [`xv::memory::UnmapUserFlow::advance`](vm.cpp.md#fn-7ce448d470e4287b)
- [`xv::memory::UnmapUserFlow::found`](vm.cpp.md#fn-beebc675c129d522)
- [`xv::memory::UnmapUserFlow::huge`](vm.cpp.md#fn-d14121140bc9a817)
- [`xv::memory::UnmapUserFlow::mapped`](vm.cpp.md#fn-7612d1e581e3690a)
- [`xv::memory::UnmapUserFlow::remove_huge`](vm.cpp.md#fn-61e83ed00cc7ba4a)
- [`xv::memory::UnmapUserFlow::remove_leaf`](vm.cpp.md#fn-c41dc117fe559782)
- [`xv::memory::UnmapUserFlow::scan`](vm.cpp.md#fn-e172af7b103ed5a1)
- [`xv::memory::UnmapUserFlow::split`](vm.cpp.md#fn-c3f9c9dd526b2731)
- [`xv::memory::UnmapUserFlow::whole`](vm.cpp.md#fn-03fc141ed6d8ecda)
- [`xv::memory::allocate_user_pages`](vm.cpp.md#fn-607dd438d1bfea8d)
- [`xv::memory::clone_user_pages`](vm.cpp.md#fn-c06f7a1b31c06b45)
- [`xv::memory::create_user_table`](vm.cpp.md#fn-3fcf1b0c323aaf56)
- [`xv::memory::deallocate_user_pages`](vm.cpp.md#fn-c457a06f835dd6c8)
- [`xv::memory::demote_user_pages`](vm.cpp.md#fn-e612c2153ae52e2e)
- [`xv::memory::free_user_table`](vm.cpp.md#fn-e5328f0087ac74d2)
- [`xv::memory::freewalk`](vm.cpp.md#fn-7dc59743121d20a0)
- [`xv::memory::is_user_mapped`](vm.cpp.md#fn-d315e799cdeadf7a)
- [`xv::memory::map_kernel_range`](vm.cpp.md#fn-e6f134e225e772ad)
- [`xv::memory::map_user_pages`](vm.cpp.md#fn-d1d927ee41b35bf8)
- [`xv::memory::permission_for`](vm.cpp.md#fn-3754e1ccd5ea4e83)
- [`xv::memory::print_page_table`](vm.cpp.md#fn-43a58229b9ef1c31)
- [`xv::memory::protect_user_guard`](vm.cpp.md#fn-91a20f4d785cbe8a)
- [`xv::memory::superdemote`](vm.cpp.md#fn-f1c6d677286a7825)
- [`xv::memory::supermap`](vm.cpp.md#fn-ec4f068105064250)
- [`xv::memory::translate_user_page`](vm.cpp.md#fn-3be6479c7074a2cd)
- [`xv::memory::unmap_user_pages`](vm.cpp.md#fn-99f83a0dbe555628)
- [`xv::memory::vmprint_level`](vm.cpp.md#fn-314c07c58169b8b0)
- [`xv::memory::walk_entry`](vm.cpp.md#fn-f2c541337b31c9a9)
- [`xv::memory::walkto`](vm.cpp.md#fn-29d3f02d541fb79c)
- [`xv::memory::copy_to_user`](vm.cpp.md#fn-7eec4370a3d73464)
- [`xv::memory::copy_from_user`](vm.cpp.md#fn-a138f76fb900bf77)
- [`xv::memory::copy_user_string`](vm.cpp.md#fn-1fe74d24bcc0849e)
- [`xv::memory::resolve_user_fault`](vm.cpp.md#fn-b403efa4391d3eee)
- [`xv::memory::accessible_region`](vm.cpp.md#fn-024d604538256e76)
- [`xv::memory::user_access`](vm.cpp.md#fn-1c297ac3ae93896b)
- [`xv::memory::accessible_bytes`](vm.cpp.md#fn-3e44b24c185b75a5)

<a id="fn-e6f134e225e772ad"></a>

## `xv::memory::map_kernel_range`

```doxygen
@fn void xv::memory::map_kernel_range(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
@brief 物理页所有权、地址范围、页表和用户内存复制：为内核地址区间建立直接映射。
@param kpgtbl 本次操作使用的 kpgtbl 值。
@param va 待处理的虚拟地址。
@param pa 对应的物理地址。
@param sz 本次操作使用的 sz 值。
@param perm 映射权限位组合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`table` → `xv::memory::allocator` → `table.map_range` → `xv::memory::PhysicalAddress` → `xv::print::panic`。
@details 需要特别跟踪的条件：`table.map_range(va, xv::memory::PhysicalAddress(pa), sz, perm) != xv::memory::MappingResult::Ok`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d1d927ee41b35bf8"></a>

## `xv::memory::map_user_pages`

```doxygen
@fn int xv::memory::map_user_pages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
@brief 物理页所有权、地址范围、页表和用户内存复制：建立用户页表的逐页映射。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param size 本次区间的字节长度或对象长度。
@param pa 对应的物理地址。
@param perm 映射权限位组合。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 MapUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-3fcf1b0c323aaf56"></a>

## `xv::memory::create_user_table`

```doxygen
@fn pagetable_t xv::memory::create_user_table()
@brief 物理页所有权、地址范围、页表和用户内存复制：创建用户表。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::allocator` → `allocate` → `xv::runtime::fill`。
@details 需要特别跟踪的条件：`pagetable == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`pagetable`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-607dd438d1bfea8d"></a>

## `xv::memory::allocate_user_pages`

```doxygen
@fn uint64 xv::memory::allocate_user_pages(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
@brief 物理页所有权、地址范围、页表和用户内存复制：扩展用户映像并建立所需物理页映射。
@param pagetable Sv39 页表根指针。
@param oldsz 本次操作使用的 oldsz 值。
@param newsz 本次操作使用的 newsz 值。
@param xperm 本次操作使用的 xperm 值。
@return 各分支实际返回：`oldsz`；`0`；`newsz`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 AllocateUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-c457a06f835dd6c8"></a>

## `xv::memory::deallocate_user_pages`

```doxygen
@fn uint64 xv::memory::deallocate_user_pages(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
@brief 物理页所有权、地址范围、页表和用户内存复制：缩小用户映像并释放不再需要的映射。
@param pagetable Sv39 页表根指针。
@param oldsz 本次操作使用的 oldsz 值。
@param newsz 本次操作使用的 newsz 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDUP` → `unmap_user_pages`。
@details 需要特别跟踪的条件：`newsz >= oldsz`；`PGROUNDUP(newsz) < PGROUNDUP(oldsz)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`oldsz`；`newsz`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c06f7a1b31c06b45"></a>

## `xv::memory::clone_user_pages`

```doxygen
@fn int xv::memory::clone_user_pages(pagetable_t old, pagetable_t new_table, uint64 sz)
@brief 共享用户帧并建立父子 COW 页表。
@details 先把 2 MiB 用户叶子拆为独立 4 KiB 帧；每项先建立子映射、增加引用，再将原可写用户页在父子两侧清 W 置 COW。只读页保持只读。失败回收候选子映射并刷新父表，已变为 COW 的父页可在独占时原地恢复。
@details 流程由 CloneUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-e5328f0087ac74d2"></a>

## `xv::memory::free_user_table`

```doxygen
@fn void xv::memory::free_user_table(pagetable_t pagetable, uint64 sz)
@brief 物理页所有权、地址范围、页表和用户内存复制：释放用户映像后递归归还页表页。
@param pagetable Sv39 页表根指针。
@param sz 本次操作使用的 sz 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`unmap_user_pages` → `PGROUNDUP` → `freewalk`。
@details 需要特别跟踪的条件：`sz > 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-99f83a0dbe555628"></a>

## `xv::memory::unmap_user_pages`

```doxygen
@fn void xv::memory::unmap_user_pages(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
@brief 移除用户映射并按共享所有权归还帧。
@details 4 KiB 叶子经 release_page 递减引用，最后一个拥有者才释放；未拆分大页继续按高阶独占分配释放，部分缩小时沿原有拆分页表路径处理。特殊映射可仅清 PTE 而不释放外部帧。
@details 流程由 UnmapUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-91a20f4d785cbe8a"></a>

## `xv::memory::protect_user_guard`

```doxygen
@fn void xv::memory::protect_user_guard(pagetable_t pagetable, uint64 va)
@brief 物理页所有权、地址范围、页表和用户内存复制：移除指定页的用户访问权限以建立保护页。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walk_entry` → `xv::print::panic`。
@details 需要特别跟踪的条件：`pte == 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f2c541337b31c9a9"></a>

## `xv::memory::walk_entry`

```doxygen
@fn pte_t *xv::memory::walk_entry(pagetable_t pagetable, uint64 va, int alloc)
@brief 物理页所有权、地址范围、页表和用户内存复制：沿 Sv39 页表层级取得目标 PTE。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param alloc 本次操作使用的 alloc 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walkto`。
@return 各分支实际返回：`walkto(pagetable, va, alloc, 0, 0)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3be6479c7074a2cd"></a>

## `xv::memory::translate_user_page`

```doxygen
@fn uint64 xv::memory::translate_user_page(pagetable_t pagetable, uint64 va)
@brief 物理页所有权、地址范围、页表和用户内存复制：检查用户权限并解析虚拟页对应的物理页。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walkto` → `PTE2PA` → `PGROUNDDOWN` → `PXSHIFT`。
@details 需要特别跟踪的条件：`va >= MAXVA`；`pte == 0`；`(*pte & PTE_V) == 0`；`(*pte & PTE_U) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`pa`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d315e799cdeadf7a"></a>

## `xv::memory::is_user_mapped`

```doxygen
@fn int xv::memory::is_user_mapped(pagetable_t table, uint64 address)
@brief 检查一个合法虚拟地址的有效 PTE 是否存在。
@details 超过 MAXVA 直接返回零，避免 walk_entry 的内核 panic；该函数只检查存在性，不替代区域、用户位和读写执行权限验证。
```

<a id="fn-43a58229b9ef1c31"></a>

## `xv::memory::print_page_table`

```doxygen
@fn void xv::memory::print_page_table(pagetable_t pagetable)
@brief 物理页所有权、地址范围、页表和用户内存复制：打印页表。
@param pagetable Sv39 页表根指针。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::printk` → `vmprint_level`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e612c2153ae52e2e"></a>

## `xv::memory::demote_user_pages`

```doxygen
@fn int xv::memory::demote_user_pages(pagetable_t pagetable, uint64 va, uint64 npages)
@brief 物理页所有权、地址范围、页表和用户内存复制：把指定用户页范围中的大页降级到需要的粒度。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param npages 本次操作使用的 npages 值。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DemoteUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-29d3f02d541fb79c"></a>

## `xv::memory::walkto`

```doxygen
@fn static pte_t *xv::memory::walkto(pagetable_t pagetable, uint64 va, int alloc, int target, int *leaflevel)
@brief 物理页所有权、地址范围、页表和用户内存复制：沿页表下降到指定层级并识别大页叶子。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param alloc 本次操作使用的 alloc 值。
@param target 本次操作使用的 target 值。
@param leaflevel 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `PX` → `PTE2PA` → `xv::memory::allocator` → `allocate` → `xv::runtime::fill` → `PA2PTE`。
@details 需要特别跟踪的条件：`va >= MAXVA`；`*pte & PTE_V`；`*pte & (PTE_R | PTE_W | PTE_X)`；`leaflevel`；`!alloc || (pagetable = (pde_t *)xv::memory::allocator().allocate()) == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`pte`；`0`；`&pagetable[PX(target, va)]`。应结合上面的条件检查区分正常值、空值和错误值。
@details 内核共享上级目录支持按需建立；遍历用 acquire 读取目录 PTE，清零新子表后以 release 发布。调用方仍保证编辑排他和目录生命周期。
```

<a id="fn-314c07c58169b8b0"></a>

## `xv::memory::vmprint_level`

```doxygen
@fn static void xv::memory::vmprint_level(pagetable_t pagetable, int level, uint64 base)
@brief 物理页所有权、地址范围、页表和用户内存复制：vmprint层级。
@param pagetable Sv39 页表根指针。
@param level 本次操作使用的 层级 值。
@param base 本次操作使用的 base 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PXSHIFT` → `xv::print::printk` → `PTE2PA` → `PTE_LEAF` → `vmprint_level`。
@details 需要特别跟踪的条件：`(pte & PTE_V) == 0`；`PTE_LEAF(pte)`；`level > 0`；`level > 0 && !PTE_LEAF(pte)`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ec4f068105064250"></a>

## `xv::memory::supermap`

```doxygen
@fn static int xv::memory::supermap(pagetable_t pagetable, uint64 va, uint64 pa, int perm)
@brief 物理页所有权、地址范围、页表和用户内存复制：安装一个用户超级页叶子映射。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param pa 对应的物理地址。
@param perm 映射权限位组合。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SuperMapFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-f1c6d677286a7825"></a>

## `xv::memory::superdemote`

```doxygen
@fn static int xv::memory::superdemote(pte_t *pte, uint64 va, int do_free)
@brief 物理页所有权、地址范围、页表和用户内存复制：把超级页映射拆分为更小的页映射。
@param pte 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param va 待处理的虚拟地址。
@param do_free 本次操作使用的 do释放 值。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 SuperDemoteFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-7dc59743121d20a0"></a>

## `xv::memory::freewalk`

```doxygen
@fn static void xv::memory::freewalk(pagetable_t pagetable)
@brief 物理页所有权、地址范围、页表和用户内存复制：递归回收已经没有叶子映射的页表结构。
@param pagetable Sv39 页表根指针。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PTE2PA` → `freewalk` → `xv::print::panic` → `xv::memory::allocator` → `deallocate`。
@details 本函数可见的状态更新包括：`pagetable[i] = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`(pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0`；`pte & PTE_V`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3754e1ccd5ea4e83"></a>

## `xv::memory::permission_for`

```doxygen
@fn unsigned xv::memory::permission_for(FaultAccess access)
@brief 把缺页访问类型转换为页表权限位。
@details 读、写、执行分别对应 R、W、X，供区域与 PTE 的一致性检查使用。
```

<a id="fn-36e3007fe505e041"></a>

## `xv::memory::AllocateUserFlow::advance`

```doxygen
@fn static void xv::memory::AllocateUserFlow::advance(Context &c) noexcept
@brief 按本轮实际映射跨度推进游标。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-607cf2f78247078a"></a>

## `xv::memory::AllocateUserFlow::allocated`

```doxygen
@fn static bool xv::memory::AllocateUserFlow::allocated(Context &c) noexcept
@brief 检查当前数据帧候选是否取得。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1a234106d3362fb4"></a>

## `xv::memory::AllocateUserFlow::choose`

```doxygen
@fn static void xv::memory::AllocateUserFlow::choose(Context &c) noexcept
@brief 重置本轮步长并在实验允许且对齐时尝试大页候选。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2d59349c90ba801e"></a>

## `xv::memory::AllocateUserFlow::drop`

```doxygen
@fn static void xv::memory::AllocateUserFlow::drop(Context &c) noexcept
@brief 普通映射失败时归还未发布候选页。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-880d2966fb2248c6"></a>

## `xv::memory::AllocateUserFlow::drop_huge`

```doxygen
@fn static void xv::memory::AllocateUserFlow::drop_huge(Context &c) noexcept
@brief 大页映射失败时归还未发布候选大页块。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f37f1f4b9fbf4b60"></a>

## `xv::memory::AllocateUserFlow::huge_step`

```doxygen
@fn static void xv::memory::AllocateUserFlow::huge_step(Context &c) noexcept
@brief 记录本轮已映射完整大页的推进跨度。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f2bcd52b461aadf1"></a>

## `xv::memory::AllocateUserFlow::map`

```doxygen
@fn static void xv::memory::AllocateUserFlow::map(Context &c) noexcept
@brief 尝试发布普通页候选的用户映射。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-22f44fea658e16f6"></a>

## `xv::memory::AllocateUserFlow::map_huge`

```doxygen
@fn static void xv::memory::AllocateUserFlow::map_huge(Context &c) noexcept
@brief 清零大页候选并尝试发布其大页映射。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9799fe87f0e0be1b"></a>

## `xv::memory::AllocateUserFlow::ok`

```doxygen
@fn static bool xv::memory::AllocateUserFlow::ok(Context &c) noexcept
@brief 检查本轮映射是否成功。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5de4b91c3c3b279b"></a>

## `xv::memory::AllocateUserFlow::page`

```doxygen
@fn static void xv::memory::AllocateUserFlow::page(Context &c) noexcept
@brief 取得普通数据页候选。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-aacb6609aaf4bf60"></a>

## `xv::memory::AllocateUserFlow::remaining`

```doxygen
@fn static bool xv::memory::AllocateUserFlow::remaining(Context &c) noexcept
@brief 检查增长范围是否仍有未提交字节。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e900346272cfe317"></a>

## `xv::memory::AllocateUserFlow::rollback`

```doxygen
@fn static void xv::memory::AllocateUserFlow::rollback(Context &c) noexcept
@brief 撤销本次增长已建立的前缀，保留原旧大小。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-846f5dd4285df85b"></a>

## `xv::memory::AllocateUserFlow::success`

```doxygen
@fn static void xv::memory::AllocateUserFlow::success(Context &c) noexcept
@brief 记录目标新大小。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-30150a942e82e87a"></a>

## `xv::memory::AllocateUserFlow::zero`

```doxygen
@fn static void xv::memory::AllocateUserFlow::zero(Context &c) noexcept
@brief 按原 LAB_SYSCALL 条件决定是否清零普通数据页。
@details 保留缩小请求直接返回、UTOP 检查和原向上对齐；优先尝试大页且大页分配失败可回退单页，失败先释放未发布候选再撤销本轮增长前缀。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2cd26a723b13a5b8"></a>

## `xv::memory::CloneUserFlow::advance`

```doxygen
@fn static void xv::memory::CloneUserFlow::advance(Context &c) noexcept
@brief 推进到下一个普通页。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0c929b9d75587678"></a>

## `xv::memory::CloneUserFlow::failure`

```doxygen
@fn static void xv::memory::CloneUserFlow::failure(Context &c) noexcept
@brief 回收已经建立的子映射前缀并刷新本核 TLB。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-73f6b9d6bccb22ae"></a>

## `xv::memory::CloneUserFlow::found`

```doxygen
@fn static bool xv::memory::CloneUserFlow::found(Context &c) noexcept
@brief 检查父范围中是否仍有有效映射。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-962ae9aafae275d9"></a>

## `xv::memory::CloneUserFlow::huge`

```doxygen
@fn static bool xv::memory::CloneUserFlow::huge(Context &c) noexcept
@brief 检查父映射是否为需要先拆分的大页。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bbb184eaac3c2e80"></a>

## `xv::memory::CloneUserFlow::level`

```doxygen
@fn static bool xv::memory::CloneUserFlow::level(Context &c) noexcept
@brief 检查将共享的映射处于普通页层。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f3eb2f619917efab"></a>

## `xv::memory::CloneUserFlow::map`

```doxygen
@fn static void xv::memory::CloneUserFlow::map(Context &c) noexcept
@brief 在子页表中建立当前共享映射。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f867fef1e216fa8f"></a>

## `xv::memory::CloneUserFlow::ok`

```doxygen
@fn static bool xv::memory::CloneUserFlow::ok(Context &c) noexcept
@brief 检查本轮拆分或子映射是否成功。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-695b7264fefd8043"></a>

## `xv::memory::CloneUserFlow::retain`

```doxygen
@fn static void xv::memory::CloneUserFlow::retain(Context &c) noexcept
@brief 增加已成功映射页的引用并发布父页对应权限。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-12284da7a7a6dec0"></a>

## `xv::memory::CloneUserFlow::scan`

```doxygen
@fn static void xv::memory::CloneUserFlow::scan(Context &c) noexcept
@brief 直接选择下一个有效父映射并保存层级。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e4566bf638a7315b"></a>

## `xv::memory::CloneUserFlow::share`

```doxygen
@fn static void xv::memory::CloneUserFlow::share(Context &c) noexcept
@brief 保存物理页与标志并按原规则生成用户可写页的 COW 权限。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f1ef98f65519e7b2"></a>

## `xv::memory::CloneUserFlow::split`

```doxygen
@fn static void xv::memory::CloneUserFlow::split(Context &c) noexcept
@brief 拆分父大页并在成功后重新取得普通页项。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c96bcc33dec5c7b1"></a>

## `xv::memory::CloneUserFlow::success`

```doxygen
@fn static void xv::memory::CloneUserFlow::success(Context &c) noexcept
@brief 刷新本核 TLB 并记录整个克隆成功。
@details 直接跳过稀疏空洞；需要时先拆分父大页，成功建立子映射后才增加数据页引用并发布父 COW；失败只回收子映射前缀，保留安全的父 COW 状态并 fence。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-436b2910d37c5e37"></a>

## `xv::memory::DemoteUserFlow::advance`

```doxygen
@fn static void xv::memory::DemoteUserFlow::advance(Context &c) noexcept
@brief 推进到下一个大页边界。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b5ba9fe14c77cf32"></a>

## `xv::memory::DemoteUserFlow::huge`

```doxygen
@fn static bool xv::memory::DemoteUserFlow::huge(Context &c) noexcept
@brief 检查当前映射是否为需要拆分的大页。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8b263e24731807bf"></a>

## `xv::memory::DemoteUserFlow::mapped`

```doxygen
@fn static bool xv::memory::DemoteUserFlow::mapped(Context &c) noexcept
@brief 检查当前页表项存在且有效。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-41d51f1d29e9c932"></a>

## `xv::memory::DemoteUserFlow::ok`

```doxygen
@fn static bool xv::memory::DemoteUserFlow::ok(Context &c) noexcept
@brief 检查当前拆分是否成功。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4abf5ed5a9e0e017"></a>

## `xv::memory::DemoteUserFlow::remaining`

```doxygen
@fn static bool xv::memory::DemoteUserFlow::remaining(Context &c) noexcept
@brief 检查访问位采样范围是否仍有待处理区间。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1c40fc84b8514950"></a>

## `xv::memory::DemoteUserFlow::split`

```doxygen
@fn static void xv::memory::DemoteUserFlow::split(Context &c) noexcept
@brief 按禁止回收数据帧的模式拆分当前大页。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-64ecac50fbcb91c3"></a>

## `xv::memory::DemoteUserFlow::success`

```doxygen
@fn static void xv::memory::DemoteUserFlow::success(Context &c) noexcept
@brief 记录全部访问位采样范围已拆分。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f4cffe611e72463a"></a>

## `xv::memory::DemoteUserFlow::walk`

```doxygen
@fn static void xv::memory::DemoteUserFlow::walk(Context &c) noexcept
@brief 只查询当前范围的有效页表层级。
@details 入口保留原范围验证；按大页跨度逐段查询，需要时拆分且禁止回收数据帧；失败保留已完成前缀，循环迭代。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4f2bbf56bf0dbef8"></a>

## `xv::memory::FaultUserFlow::accessible`

```doxygen
@fn static bool xv::memory::FaultUserFlow::accessible(Context &c) noexcept
@brief 按原独立查询再次检查区域访问权限。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-712ff12943070914"></a>

## `xv::memory::FaultUserFlow::allocate`

```doxygen
@fn static void xv::memory::FaultUserFlow::allocate(Context &c) noexcept
@brief 取得本次共享 COW 或按需故障的候选帧。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-641f2baba3b276ac"></a>

## `xv::memory::FaultUserFlow::allocated`

```doxygen
@fn static bool xv::memory::FaultUserFlow::allocated(Context &c) noexcept
@brief 检查候选帧分配是否成功。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-be9db49f65c12189"></a>

## `xv::memory::FaultUserFlow::copy`

```doxygen
@fn static void xv::memory::FaultUserFlow::copy(Context &c) noexcept
@brief 复制原 COW 数据到新候选帧。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-57acccbe87979243"></a>

## `xv::memory::FaultUserFlow::cow`

```doxygen
@fn static bool xv::memory::FaultUserFlow::cow(Context &c) noexcept
@brief 检查是否为尚未具备写权限的 COW 写故障。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-55abec27c3340f0b"></a>

## `xv::memory::FaultUserFlow::demand`

```doxygen
@fn static bool xv::memory::FaultUserFlow::demand(Context &c) noexcept
@brief 检查原区域允许按需分配且本次不是执行访问。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bb61210372a83a1b"></a>

## `xv::memory::FaultUserFlow::existing`

```doxygen
@fn static void xv::memory::FaultUserFlow::existing(Context &c) noexcept
@brief 根据本次访问权限分类已有映射的结果。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7ae2a4b7e780faf2"></a>

## `xv::memory::FaultUserFlow::inside`

```doxygen
@fn static bool xv::memory::FaultUserFlow::inside(Context &c) noexcept
@brief 检查地址位于有效区域且符合大小或特殊区域规则。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-24c31724e3fcf643"></a>

## `xv::memory::FaultUserFlow::invalid`

```doxygen
@fn static void xv::memory::FaultUserFlow::invalid(Context &c) noexcept
@brief 记录无效映射结果。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3d298cef182f900d"></a>

## `xv::memory::FaultUserFlow::leaf`

```doxygen
@fn static bool xv::memory::FaultUserFlow::leaf(Context &c) noexcept
@brief 检查现有有效项是否为叶。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a7584efd93d067dd"></a>

## `xv::memory::FaultUserFlow::map`

```doxygen
@fn static void xv::memory::FaultUserFlow::map(Context &c) noexcept
@brief 尝试按区域权限建立按需用户映射。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-98fe55d24989518a"></a>

## `xv::memory::FaultUserFlow::mapped`

```doxygen
@fn static bool xv::memory::FaultUserFlow::mapped(Context &c) noexcept
@brief 检查故障地址是否已有有效项。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-58c9f853c4f84294"></a>

## `xv::memory::FaultUserFlow::ok`

```doxygen
@fn static bool xv::memory::FaultUserFlow::ok(Context &c) noexcept
@brief 检查按需映射是否成功。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0466a62889ddb902"></a>

## `xv::memory::FaultUserFlow::oom`

```doxygen
@fn static void xv::memory::FaultUserFlow::oom(Context &c) noexcept
@brief 记录内存不足结果。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5001b8987e1752a5"></a>

## `xv::memory::FaultUserFlow::permission`

```doxygen
@fn static void xv::memory::FaultUserFlow::permission(Context &c) noexcept
@brief 记录权限错误结果。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cdfc037e4720c0e0"></a>

## `xv::memory::FaultUserFlow::promote`

```doxygen
@fn static void xv::memory::FaultUserFlow::promote(Context &c) noexcept
@brief 原地恢复唯一引用页的写权限、刷新 TLB 并记录解决。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-675ec2e7201bc3cf"></a>

## `xv::memory::FaultUserFlow::publish_cow`

```doxygen
@fn static void xv::memory::FaultUserFlow::publish_cow(Context &c) noexcept
@brief 发布新的可写映射并在释放旧引用前刷新 TLB。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3e94a0f98f851f01"></a>

## `xv::memory::FaultUserFlow::region`

```doxygen
@fn static void xv::memory::FaultUserFlow::region(Context &c) noexcept
@brief 读取原虚拟区域登记。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a8891bf229b39cf8"></a>

## `xv::memory::FaultUserFlow::release`

```doxygen
@fn static void xv::memory::FaultUserFlow::release(Context &c) noexcept
@brief 释放旧 COW 引用并记录新物理页已解决。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-195e1e2cb419ebb2"></a>

## `xv::memory::FaultUserFlow::resolved`

```doxygen
@fn static void xv::memory::FaultUserFlow::resolved(Context &c) noexcept
@brief 刷新 TLB 并记录按需映射的物理页已解决。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-238b45a630ce7ae0"></a>

## `xv::memory::FaultUserFlow::rollback`

```doxygen
@fn static void xv::memory::FaultUserFlow::rollback(Context &c) noexcept
@brief 按需映射失败时归还本次候选帧。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a7c36c9ba376d7e5"></a>

## `xv::memory::FaultUserFlow::shared`

```doxygen
@fn static void xv::memory::FaultUserFlow::shared(Context &c) noexcept
@brief 保存 COW 原物理页和恢复写权限后的标志。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0b42fed8fbbcf597"></a>

## `xv::memory::FaultUserFlow::small`

```doxygen
@fn static bool xv::memory::FaultUserFlow::small(Context &c) noexcept
@brief 检查 COW 目标为普通页层。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ee08f6669a35b09e"></a>

## `xv::memory::FaultUserFlow::sole`

```doxygen
@fn static bool xv::memory::FaultUserFlow::sole(Context &c) noexcept
@brief 检查数据帧是否只剩一个引用。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4f9e536d06ba50ff"></a>

## `xv::memory::FaultUserFlow::user`

```doxygen
@fn static bool xv::memory::FaultUserFlow::user(Context &c) noexcept
@brief 检查叶项允许用户访问。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-23525c4d75b08e23"></a>

## `xv::memory::FaultUserFlow::walk`

```doxygen
@fn static void xv::memory::FaultUserFlow::walk(Context &c) noexcept
@brief 只查询故障地址的当前页项与层级。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-45442b951df4bef4"></a>

## `xv::memory::FaultUserFlow::zero`

```doxygen
@fn static void xv::memory::FaultUserFlow::zero(Context &c) noexcept
@brief 清零按需分配候选帧。
@details 保留区域重复查找和原权限顺序；唯一 COW 引用原地提权，共享 COW 先复制发布并 fence 再释放旧引用；按需页映射失败只释放本次候选，不将业务结果用作执行器控制信号。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a1b75a04b79f4a29"></a>

## `xv::memory::MapUserFlow::advance`

```doxygen
@fn static void xv::memory::MapUserFlow::advance(Context &c) noexcept
@brief 同时推进虚拟与物理页游标。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fbf95bad7b8f56f0"></a>

## `xv::memory::MapUserFlow::found`

```doxygen
@fn static bool xv::memory::MapUserFlow::found(Context &c) noexcept
@brief 检查当前项是否取得成功。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dada4b3ec871576d"></a>

## `xv::memory::MapUserFlow::last`

```doxygen
@fn static bool xv::memory::MapUserFlow::last(Context &c) noexcept
@brief 检查当前页是否是目标范围最后一页。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3cc4dfcc108fa5e1"></a>

## `xv::memory::MapUserFlow::publish`

```doxygen
@fn static void xv::memory::MapUserFlow::publish(Context &c) noexcept
@brief 拒绝覆盖有效项并发布本页物理地址与权限。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3ca76c13f38f1bc8"></a>

## `xv::memory::MapUserFlow::success`

```doxygen
@fn static void xv::memory::MapUserFlow::success(Context &c) noexcept
@brief 记录整个映射范围成功。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b9435386578d4d8b"></a>

## `xv::memory::MapUserFlow::walk`

```doxygen
@fn static void xv::memory::MapUserFlow::walk(Context &c) noexcept
@brief 允许分配地取得当前页表项。
@details 入口保留原虚址、长度对齐及非零诊断；逐页取得页表并拒绝重映射，不自行回滚成功前缀，TLB 协议仍由调用者负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6d43491cd285b111"></a>

## `xv::memory::SuperDemoteFlow::allocate`

```doxygen
@fn static void xv::memory::SuperDemoteFlow::allocate(Context &c) noexcept
@brief 尝试取得新的子页表帧。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2807271a6c0f723b"></a>

## `xv::memory::SuperDemoteFlow::allocated`

```doxygen
@fn static bool xv::memory::SuperDemoteFlow::allocated(Context &c) noexcept
@brief 检查子页表帧是否分配成功。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03e485b5d4e630c1"></a>

## `xv::memory::SuperDemoteFlow::fill`

```doxygen
@fn static void xv::memory::SuperDemoteFlow::fill(Context &c) noexcept
@brief 直接填写全部子项并将回收作表的那一项留空。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5af9ae56e6cc85f1"></a>

## `xv::memory::SuperDemoteFlow::may_recycle`

```doxygen
@fn static bool xv::memory::SuperDemoteFlow::may_recycle(Context &c) noexcept
@brief 检查当前解除映射是否允许回收待丢弃帧。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-432401329de454fc"></a>

## `xv::memory::SuperDemoteFlow::publish`

```doxygen
@fn static void xv::memory::SuperDemoteFlow::publish(Context &c) noexcept
@brief 发布子页表目录并记录成功。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b768adff83897135"></a>

## `xv::memory::SuperDemoteFlow::recycle`

```doxygen
@fn static void xv::memory::SuperDemoteFlow::recycle(Context &c) noexcept
@brief 选择当前待丢弃页作为子表并记录应留空的项。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ba05a78dc3cef57c"></a>

## `xv::memory::SuperDemoteFlow::split`

```doxygen
@fn static void xv::memory::SuperDemoteFlow::split(Context &c) noexcept
@brief 将原大页块转移为独立页所有权，失败保留原诊断。
@details 先保存大页物理地址和原标志；普通分配失败时仅在解除并释放映射的路径允许回收待丢弃帧，拆分所有权后填子表，最后发布目录。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2c159286a78afd27"></a>

## `xv::memory::SuperMapFlow::existing`

```doxygen
@fn static bool xv::memory::SuperMapFlow::existing(Context &c) noexcept
@brief 检查目标项是否已有有效映射。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fd2db23cf748f91d"></a>

## `xv::memory::SuperMapFlow::found`

```doxygen
@fn static bool xv::memory::SuperMapFlow::found(Context &c) noexcept
@brief 检查目标页表项是否取得。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7aa88e1c60adc7e6"></a>

## `xv::memory::SuperMapFlow::invalid`

```doxygen
@fn static void xv::memory::SuperMapFlow::invalid(Context &) noexcept
@brief 按原层级错误诊断终止。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-beaa1b57749632f9"></a>

## `xv::memory::SuperMapFlow::level`

```doxygen
@fn static bool xv::memory::SuperMapFlow::level(Context &c) noexcept
@brief 检查实际层级等于大页层。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d42b6b39acef98de"></a>

## `xv::memory::SuperMapFlow::publish`

```doxygen
@fn static void xv::memory::SuperMapFlow::publish(Context &c) noexcept
@brief 发布大页物理地址和权限并记录成功。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-36c014ff4a6e5b06"></a>

## `xv::memory::SuperMapFlow::reclaim`

```doxygen
@fn static void xv::memory::SuperMapFlow::reclaim(Context &c) noexcept
@brief 拒绝重映射或非空子表，只回收已确认空的叶表。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-569661b0165707d7"></a>

## `xv::memory::SuperMapFlow::walk`

```doxygen
@fn static void xv::memory::SuperMapFlow::walk(Context &c) noexcept
@brief 允许分配地查询大页目标层并保存实际层级。
@details 入口先验证大页对齐；查询目标层，不覆盖现有叶或非空子表；仅回收空的旧叶表后发布大页，保持原失败与 panic 语义。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7ce448d470e4287b"></a>

## `xv::memory::UnmapUserFlow::advance`

```doxygen
@fn static void xv::memory::UnmapUserFlow::advance(Context &c) noexcept
@brief 按本轮实际步长推进解除范围。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-beebc675c129d522"></a>

## `xv::memory::UnmapUserFlow::found`

```doxygen
@fn static bool xv::memory::UnmapUserFlow::found(Context &c) noexcept
@brief 检查是否仍有有效映射需要解除。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d14121140bc9a817"></a>

## `xv::memory::UnmapUserFlow::huge`

```doxygen
@fn static bool xv::memory::UnmapUserFlow::huge(Context &c) noexcept
@brief 检查当前项是否为大页。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7612d1e581e3690a"></a>

## `xv::memory::UnmapUserFlow::mapped`

```doxygen
@fn static bool xv::memory::UnmapUserFlow::mapped(Context &c) noexcept
@brief 检查拆分后当前页是否仍有效，跳过回收作表的项。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-61e83ed00cc7ba4a"></a>

## `xv::memory::UnmapUserFlow::remove_huge`

```doxygen
@fn static void xv::memory::UnmapUserFlow::remove_huge(Context &c) noexcept
@brief 按原释放策略归还大页块、清项并设置大页步长。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c41dc117fe559782"></a>

## `xv::memory::UnmapUserFlow::remove_leaf`

```doxygen
@fn static void xv::memory::UnmapUserFlow::remove_leaf(Context &c) noexcept
@brief 验证叶项，按需释放引用并清除页表项。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e172af7b103ed5a1"></a>

## `xv::memory::UnmapUserFlow::scan`

```doxygen
@fn static void xv::memory::UnmapUserFlow::scan(Context &c) noexcept
@brief 直接跳过未分配项并选择下一有效映射。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c3f9c9dd526b2731"></a>

## `xv::memory::UnmapUserFlow::split`

```doxygen
@fn static void xv::memory::UnmapUserFlow::split(Context &c) noexcept
@brief 按当前释放策略拆分大页并重新取得本页项。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03fc141ed6d8ecda"></a>

## `xv::memory::UnmapUserFlow::whole`

```doxygen
@fn static bool xv::memory::UnmapUserFlow::whole(Context &c) noexcept
@brief 检查目标范围是否覆盖对齐的完整大页。
@details 允许稀疏缺页；保留完整大页释放、部分大页拆分回收及单页引用释放的不同协议，循环迭代，不额外刷新 TLB。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7eec4370a3d73464"></a>

## `xv::memory::copy_to_user`

```doxygen
@fn int xv::memory::copy_to_user(pagetable_t table, uint64 size, const VirtualAddressPool &regions, uint64 destination, const char *source, uint64 bytes)
@brief 按区域权限向用户地址复制并打破 COW。
@details 逐页同时验证区域、逻辑字节边界和 PTE；只读 COW 映射先复制或独占提升，再写物理页。真正只读、保护区、越界或 OOM 返回 -1，可能已复制前缀，不提供整段事务性。
```

<a id="fn-a138f76fb900bf77"></a>

## `xv::memory::copy_from_user`

```doxygen
@fn int xv::memory::copy_from_user(pagetable_t table, uint64 size, const VirtualAddressPool &regions, char *destination, uint64 source, uint64 bytes)
@brief 按区域与页表读权限复制用户数据。
@details 逐页限制到有效区域、当前逻辑长度和页尾；合法匿名懒页可补零后读取，未登记、不可读或分配失败返回 -1。实际映射的访问权限不能被区域声明放宽。
```

<a id="fn-1fe74d24bcc0849e"></a>

## `xv::memory::copy_user_string`

```doxygen
@fn int xv::memory::copy_user_string(pagetable_t table, uint64 size, const VirtualAddressPool &regions, char *destination, uint64 source, uint64 maximum)
@brief 在有效用户区域内读取一个终止字符串。
@details 每页校验读权限并按需补齐匿名懒页，扫描不能越过区域、逻辑字节边界或最大容量；发现 NUL 成功，否则越界、OOM 或容量耗尽返回 -1。
```

<a id="fn-b403efa4391d3eee"></a>

## `xv::memory::resolve_user_fault`

```doxygen
@fn FaultOutcome xv::memory::resolve_user_fault(pagetable_t table, uint64 size, const VirtualAddressPool &regions, uint64 address, FaultAccess access)
@brief 分类用户异常并只恢复合法匿名缺页或 COW 写入。
@details 先检查区域及逻辑字节上界，再验证用户位、叶子格式和访问权限。现存普通映射返回 AlreadyMapped 或 Permission，匿名懒页补零，COW 共享页复制而独占页原地提升。OOM 保留原 COW PTE 和引用，返回结构化结果。
@details 流程由 FaultUserFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-024d604538256e76"></a>

## `xv::memory::accessible_region`

```doxygen
@fn const VirtualRegion *xv::memory::accessible_region(const VirtualAddressPool &regions, uint64 size, uint64 address, FaultAccess access)
@brief 取得允许当前用户访问的已登记区域。
@details 同时要求所需权限和 U 位；普通区域还受逻辑 size 限制，特殊用户页按其显式范围处理。无匹配或不允许访问返回空，不修改页表。
```

<a id="fn-1c297ac3ae93896b"></a>

## `xv::memory::user_access`

```doxygen
@fn uint64 xv::memory::user_access(pagetable_t table, uint64 size, const VirtualAddressPool &regions, uint64 address, FaultAccess access)
@brief 取得内核用户复制所需的物理页。
@details 区域和现存 PTE 都允许访问时直接翻译，包含大页内的 4 KiB 偏移；否则只接受 resolve_user_fault 的 Resolved 结果，确保 copyout 不绕过 COW 或只读权限。
```

<a id="fn-3e44b24c185b75a5"></a>

## `xv::memory::accessible_bytes`

```doxygen
@fn uint64 xv::memory::accessible_bytes(const VirtualAddressPool &regions, uint64 size, uint64 address)
@brief 计算当前页中仍处于有效区域和逻辑范围内的字节数。
@details 调用前必须已通过访问验证；取区域终点、普通地址空间 size 和页尾的最小值，特殊区域不受普通 heap size 限制。
```
