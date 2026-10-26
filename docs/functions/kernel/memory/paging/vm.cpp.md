# `kernel/memory/paging/vm.cpp`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/vm.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory`。

直接模块依赖：`xv.runtime`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::allocate_user_pages`](vm.cpp.md#fn-607dd438d1bfea8d)
- [`xv::memory::clone_user_pages`](vm.cpp.md#fn-c06f7a1b31c06b45)
- [`xv::memory::copy_from_user`](vm.cpp.md#fn-702f4fdb8fe6d6ed)
- [`xv::memory::copy_to_user`](vm.cpp.md#fn-c818984f3c716ba3)
- [`xv::memory::copy_user_string`](vm.cpp.md#fn-7bd0107be1202abf)
- [`xv::memory::create_user_table`](vm.cpp.md#fn-3fcf1b0c323aaf56)
- [`xv::memory::deallocate_user_pages`](vm.cpp.md#fn-c457a06f835dd6c8)
- [`xv::memory::demote_user_pages`](vm.cpp.md#fn-e612c2153ae52e2e)
- [`xv::memory::free_user_table`](vm.cpp.md#fn-e5328f0087ac74d2)
- [`xv::memory::is_user_mapped`](vm.cpp.md#fn-d315e799cdeadf7a)
- [`xv::memory::map_kernel_range`](vm.cpp.md#fn-e6f134e225e772ad)
- [`xv::memory::map_user_pages`](vm.cpp.md#fn-d1d927ee41b35bf8)
- [`xv::memory::print_page_table`](vm.cpp.md#fn-43a58229b9ef1c31)
- [`xv::memory::protect_user_guard`](vm.cpp.md#fn-91a20f4d785cbe8a)
- [`xv::memory::resolve_user_fault`](vm.cpp.md#fn-3fa77ef7a3677ae6)
- [`xv::memory::translate_user_page`](vm.cpp.md#fn-3be6479c7074a2cd)
- [`xv::memory::unmap_user_pages`](vm.cpp.md#fn-99f83a0dbe555628)
- [`xv::memory::walk_entry`](vm.cpp.md#fn-f2c541337b31c9a9)
- [`xv::memory::freewalk`](vm.cpp.md#fn-7dc59743121d20a0)
- [`xv::memory::superdemote`](vm.cpp.md#fn-f1c6d677286a7825)
- [`xv::memory::supermap`](vm.cpp.md#fn-ec4f068105064250)
- [`xv::memory::vmprint_level`](vm.cpp.md#fn-314c07c58169b8b0)
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
@brief 物理页所有权、地址范围、页表和用户内存复制：复制用户物理页及权限并在失败时回滚。
@param old 原有状态或源页表。
@param new_table 接收复制结果的新页表。
@param sz 本次操作使用的 sz 值。
@details 遍历已有用户映射并保留权限，跳过未分配惰性洞；遇到支持的超级页按相应阶分配复制。任一分配或映射失败会撤销已经完成的目标范围，不遗留半个子映像。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walkto` → `PTE2PA` → `PTE_FLAGS` → `xv::memory::allocator` → `allocate` → `xv::runtime::move` → `supermap` → `deallocate` → `map_user_pages` → `unmap_user_pages`。
@details 需要特别跟踪的条件：`(pte = walkto(old, i, 0, 0, &level)) == 0`；`(*pte & PTE_V) == 0`；`level == 1`；`(mem = static_cast<char *>( xv::memory::allocator().allocate(xv::memory::SuperpageOrder))) == 0`；`supermap(new_table, i, (uint64)mem, flags) != 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`-1`。应结合上面的条件检查区分正常值、空值和错误值。
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
@brief 物理页所有权、地址范围、页表和用户内存复制：撤销用户映射并按选项归还物理页。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@param npages 本次操作使用的 npages 值。
@param do_free 本次操作使用的 do释放 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::print::panic` → `walkto` → `xv::memory::allocator` → `deallocate` → `PTE2PA` → `superdemote` → `walk_entry` → `PTE_FLAGS`。
@details 需要特别跟踪的条件：`(va % PGSIZE) != 0`；`(pte = walkto(pagetable, a, 0, 0, &level)) == 0`；`(*pte & PTE_V) == 0`；`level == 1`；`a % SUPERPGSIZE == 0 && va + npages * PGSIZE - a >= SUPERPGSIZE`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@fn int xv::memory::copy_to_user(pagetable_t pagetable, uint64 psz, uint64 dstva, const char *src, uint64 len)
@brief 物理页所有权、地址范围、页表和用户内存复制：逐页验证可写权限并复制内核数据到用户地址。
@param pagetable Sv39 页表根指针。
@param psz 用户地址空间的有效逻辑长度。
@param dstva 目标用户虚拟地址。
@param src 输入数据起点。
@param len 请求处理的字节上限。
@details 逐 4KiB 页切片解析目标虚拟地址；缺少合法惰性映射时尝试补页，再检查 PTE_W 后移动字节。保护页、越界或只读文本不能靠内核复制绕过；跨页失败可能已复制前面的切片。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDDOWN` → `translate_user_page` → `resolve_user_fault` → `walk_entry` → `xv::runtime::move`。
@details 需要特别跟踪的条件：`len > 0`；`va0 >= MAXVA`；`pa0 == 0`；`(pa0 = resolve_user_fault(pagetable, psz, va0, 0)) == 0`；`(pte = walk_entry(pagetable, va0, 0)) == 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-702f4fdb8fe6d6ed"></a>

## `xv::memory::copy_from_user`

```doxygen
@fn int xv::memory::copy_from_user(pagetable_t pagetable, uint64 psz, char *dst, uint64 srcva, uint64 len)
@brief 物理页所有权、地址范围、页表和用户内存复制：逐页解析用户映射并把数据复制到内核。
@param pagetable Sv39 页表根指针。
@param psz 用户地址空间的有效逻辑长度。
@param dst 结果缓冲起点。
@param srcva 源用户虚拟地址。
@param len 请求处理的字节上限。
@details 逐 4KiB 页解析用户源地址，按需要处理合法惰性缺页，再移动每页剩余范围内的字节到内核缓冲。返回成功不把用户虚拟地址当作可直接解引用的内核指针。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDDOWN` → `translate_user_page` → `resolve_user_fault` → `xv::runtime::move`。
@details 需要特别跟踪的条件：`len > 0`；`pa0 == 0`；`(pa0 = resolve_user_fault(pagetable, psz, va0, 1)) == 0`；`n > len`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7bd0107be1202abf"></a>

## `xv::memory::copy_user_string`

```doxygen
@fn int xv::memory::copy_user_string(pagetable_t pagetable, uint64 psz, char *dst, uint64 srcva, uint64 max)
@brief 物理页所有权、地址范围、页表和用户内存复制：跨页复制用户字符串直到零结尾或长度上限。
@param pagetable Sv39 页表根指针。
@param psz 用户地址空间的有效逻辑长度。
@param dst 结果缓冲起点。
@param srcva 源用户虚拟地址。
@param max 本次操作使用的 最大 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDDOWN` → `translate_user_page` → `resolve_user_fault`。
@details 需要特别跟踪的条件：`got_null == 0 && max > 0`；`pa0 == 0`；`(pa0 = resolve_user_fault(pagetable, psz, va0, 1)) == 0`；`n > max`；`n > 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d315e799cdeadf7a"></a>

## `xv::memory::is_user_mapped`

```doxygen
@fn int xv::memory::is_user_mapped(pagetable_t pagetable, uint64 va)
@brief 物理页所有权、地址范围、页表和用户内存复制：是否用户mapped。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`walk_entry`。
@details 需要特别跟踪的条件：`pte == 0`；`*pte & PTE_V`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3fa77ef7a3677ae6"></a>

## `xv::memory::resolve_user_fault`

```doxygen
@fn uint64 xv::memory::resolve_user_fault(pagetable_t pagetable, uint64 psz, uint64 va, int read)
@brief 物理页所有权、地址范围、页表和用户内存复制：为合法的惰性用户地址补充分配映射。
@param pagetable Sv39 页表根指针。
@param psz 用户地址空间的有效逻辑长度。
@param va 待处理的虚拟地址。
@param read 本次操作使用的 读取 值。
@details 先限制地址小于用户逻辑长度并页对齐，拒绝对已映射地址重复补页；分配物理页、清零并建立用户读写映射，失败时归还物理页。它不是按任意内核地址分配内存的入口。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`PGROUNDDOWN` → `is_user_mapped` → `xv::memory::allocator` → `allocate` → `xv::runtime::fill` → `map_user_pages` → `deallocate`。
@details 需要特别跟踪的条件：`va >= psz`；`is_user_mapped(pagetable, va)`；`mem == 0`；`map_user_pages(pagetable, va, PGSIZE, mem, PTE_W | PTE_U | PTE_R) != 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`0`；`mem`。应结合上面的条件检查区分正常值、空值和错误值。
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
