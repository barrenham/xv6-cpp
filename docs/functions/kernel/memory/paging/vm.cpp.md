# `kernel/memory/paging/vm.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/vm.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.runtime`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::accessible_bytes`](vm.cpp.md#fn-02d74129c3a981dd)
- [`xv::memory::accessible_region`](vm.cpp.md#fn-23dd8981edb0551d)
- [`xv::memory::allocate_user_pages`](vm.cpp.md#fn-607dd438d1bfea8d)
- [`xv::memory::clone_user_pages`](vm.cpp.md#fn-c06f7a1b31c06b45)
- [`xv::memory::copy_from_user`](vm.cpp.md#fn-702f4fdb8fe6d6ed)
- [`xv::memory::copy_to_user`](vm.cpp.md#fn-c818984f3c716ba3)
- [`xv::memory::copy_user_string`](vm.cpp.md#fn-7bd0107be1202abf)
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
- [`xv::memory::resolve_user_fault`](vm.cpp.md#fn-3fa77ef7a3677ae6)
- [`xv::memory::superdemote`](vm.cpp.md#fn-f1c6d677286a7825)
- [`xv::memory::supermap`](vm.cpp.md#fn-ec4f068105064250)
- [`xv::memory::translate_user_page`](vm.cpp.md#fn-3be6479c7074a2cd)
- [`xv::memory::unmap_user_pages`](vm.cpp.md#fn-99f83a0dbe555628)
- [`xv::memory::user_access`](vm.cpp.md#fn-b8d6297fb97d39c6)
- [`xv::memory::vmprint_level`](vm.cpp.md#fn-314c07c58169b8b0)
- [`xv::memory::walk_entry`](vm.cpp.md#fn-f2c541337b31c9a9)
- [`xv::memory::walkto`](vm.cpp.md#fn-29d3f02d541fb79c)

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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `walk_entry` → `PA2PTE`。
@details 需要特别跟踪的条件：`(va % PGSIZE) != 0`；`(size % PGSIZE) != 0`；`size == 0`；`(pte = walk_entry(pagetable, a, 1)) == 0`；`*pte & PTE_V`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDUP` → `xv::memory::allocator` → `allocate` → `xv::runtime::fill` → `supermap` → `deallocate` → `deallocate_user_pages` → `map_user_pages`。
@details 需要特别跟踪的条件：`newsz < oldsz`；`newsz > UTOP`；`a % SUPERPGSIZE == 0 && newsz - a >= SUPERPGSIZE && (mem = static_cast<char *>( xv::memory::allocator().allocate(xv::memory::SuperpageOrder))) != 0`；`supermap(pagetable, a, (uint64)mem, PTE_R | PTE_U | xperm) < 0`；`mem == 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`oldsz`；`0`；`newsz`。应结合上面的条件检查区分正常值、空值和错误值。
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

<a id="fn-c818984f3c716ba3"></a>

## `xv::memory::copy_to_user`

```doxygen
@fn int xv::memory::copy_to_user(pagetable_t table, uint64 size, const VirtualRegions &regions, uint64 destination, const char *source, uint64 bytes)
@brief 按区域权限向用户地址复制并打破 COW。
@details 逐页同时验证区域、逻辑字节边界和 PTE；只读 COW 映射先复制或独占提升，再写物理页。真正只读、保护区、越界或 OOM 返回 -1，可能已复制前缀，不提供整段事务性。
```

<a id="fn-702f4fdb8fe6d6ed"></a>

## `xv::memory::copy_from_user`

```doxygen
@fn int xv::memory::copy_from_user(pagetable_t table, uint64 size, const VirtualRegions &regions, char *destination, uint64 source, uint64 bytes)
@brief 按区域与页表读权限复制用户数据。
@details 逐页限制到有效区域、当前逻辑长度和页尾；合法匿名懒页可补零后读取，未登记、不可读或分配失败返回 -1。实际映射的访问权限不能被区域声明放宽。
```

<a id="fn-7bd0107be1202abf"></a>

## `xv::memory::copy_user_string`

```doxygen
@fn int xv::memory::copy_user_string(pagetable_t table, uint64 size, const VirtualRegions &regions, char *destination, uint64 source, uint64 maximum)
@brief 在有效用户区域内读取一个终止字符串。
@details 每页校验读权限并按需补齐匿名懒页，扫描不能越过区域、逻辑字节边界或最大容量；发现 NUL 成功，否则越界、OOM 或容量耗尽返回 -1。
```

<a id="fn-d315e799cdeadf7a"></a>

## `xv::memory::is_user_mapped`

```doxygen
@fn int xv::memory::is_user_mapped(pagetable_t pagetable, uint64 va)
@brief 检查一个合法虚拟地址的有效 PTE 是否存在。
@details 超过 MAXVA 直接返回零，避免 walk_entry 的内核 panic；该函数只检查存在性，不替代区域、用户位和读写执行权限验证。
```

<a id="fn-3fa77ef7a3677ae6"></a>

## `xv::memory::resolve_user_fault`

```doxygen
@fn FaultOutcome xv::memory::resolve_user_fault(pagetable_t table, uint64 size, const VirtualRegions &regions, uint64 address, FaultAccess access)
@brief 分类用户异常并只恢复合法匿名缺页或 COW 写入。
@details 先检查区域及逻辑字节上界，再验证用户位、叶子格式和访问权限。现存普通映射返回 AlreadyMapped 或 Permission，匿名懒页补零，COW 共享页复制而独占页原地提升。OOM 保留原 COW PTE 和引用，返回结构化结果。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`SUPERPGROUNDDOWN` → `walkto` → `superdemote`。
@details 需要特别跟踪的条件：`va % PGSIZE != 0 || va >= MAXVA || npages > (MAXVA - va) / PGSIZE`；`pte == 0 || (*pte & PTE_V) == 0`；`level == 1 && superdemote(pte, a, 0) < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `walkto` → `PTE_LEAF` → `PTE2PA` → `xv::memory::allocator` → `deallocate` → `PA2PTE`。
@details 需要特别跟踪的条件：`(va | pa) % SUPERPGSIZE != 0`；`pte == 0`；`level != 1`；`*pte & PTE_V`；`PTE_LEAF(*pte)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f1c6d677286a7825"></a>

## `xv::memory::superdemote`

```doxygen
@fn static int xv::memory::superdemote(pte_t *pte, uint64 va, int do_free)
@brief 物理页所有权、地址范围、页表和用户内存复制：把超级页映射拆分为更小的页映射。
@param pte 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param va 待处理的虚拟地址。
@param do_free 本次操作使用的 do释放 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PTE2PA` → `PTE_FLAGS` → `xv::memory::allocator` → `allocate` → `PX` → `split_to_pages` → `xv::print::panic` → `PA2PTE`。
@details 本函数可见的状态更新包括：`child[i] = i == recycled ? 0 : PA2PTE(pa + (uint64)i * PGSIZE) | flags;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`child == 0`；`!do_free`；`!xv::memory::allocator().split_to_pages((void *)pa, xv::memory::SuperpageOrder)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
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

<a id="fn-23dd8981edb0551d"></a>

## `xv::memory::accessible_region`

```doxygen
@fn const VirtualRegion *xv::memory::accessible_region(const VirtualRegions &regions, uint64 size, uint64 address, FaultAccess access)
@brief 取得允许当前用户访问的已登记区域。
@details 同时要求所需权限和 U 位；普通区域还受逻辑 size 限制，特殊用户页按其显式范围处理。无匹配或不允许访问返回空，不修改页表。
```

<a id="fn-b8d6297fb97d39c6"></a>

## `xv::memory::user_access`

```doxygen
@fn uint64 xv::memory::user_access(pagetable_t table, uint64 size, const VirtualRegions &regions, uint64 address, FaultAccess access)
@brief 取得内核用户复制所需的物理页。
@details 区域和现存 PTE 都允许访问时直接翻译，包含大页内的 4 KiB 偏移；否则只接受 resolve_user_fault 的 Resolved 结果，确保 copyout 不绕过 COW 或只读权限。
```

<a id="fn-02d74129c3a981dd"></a>

## `xv::memory::accessible_bytes`

```doxygen
@fn uint64 xv::memory::accessible_bytes(const VirtualRegions &regions, uint64 size, uint64 address)
@brief 计算当前页中仍处于有效区域和逻辑范围内的字节数。
@details 调用前必须已通过访问验证；取区域终点、普通地址空间 size 和页尾的最小值，特殊区域不受普通 heap size 限制。
```
