# `kernel/syscall/sysvm.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysvm.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:arguments`, `:handlers`, `xv.memory`, `xv.process`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::sys_ksupernpte`](sysvm.cpp.md#fn-854777261a9f0a46)
- [`xv::syscall::sys_pgpte`](sysvm.cpp.md#fn-ca4e57de1ea0f712)
- [`xv::syscall::cnt_level`](sysvm.cpp.md#fn-60d98a67c33e4923)
- [`xv::syscall::pgpte`](sysvm.cpp.md#fn-58a123e59e099d46)

<a id="fn-ca4e57de1ea0f712"></a>

## `xv::syscall::sys_pgpte`

```doxygen
@fn uint64 xv::syscall::sys_pgpte(void)
@brief 系统调用入口：pgpte；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `argument_address` → `pgpte` → `p->memory` → `page_table`。
@details 需要特别跟踪的条件：`pte != 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`(uint64)*pte`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-854777261a9f0a46"></a>

## `xv::syscall::sys_ksupernpte`

```doxygen
@fn uint64 xv::syscall::sys_ksupernpte()
@brief 系统调用入口：ksupernpte；从用户参数转入内核服务并返回 ABI 结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cnt_level` → `xv::memory::kernel_page_table` → `root` → `xv::process::current` → `argument_address` → `xv::memory::copy_to_user` → `p->memory` → `page_table` → `size`。
@details 需要特别跟踪的条件：`xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), as, (char *)&s, sizeof(uint64)) < 0`；`xv::memory::copy_to_user(p->memory().page_table(), p->memory().size(), an, (char *)&n, sizeof(uint64)) < 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```

<a id="fn-58a123e59e099d46"></a>

## `xv::syscall::pgpte`

```doxygen
@fn static pte_t *xv::syscall::pgpte(pagetable_t pagetable, uint64 va)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：读取或计算 `xv::memory::walk_entry(pagetable, va, 0)`，返回当前查询结果。
@param pagetable Sv39 页表根指针。
@param va 待处理的虚拟地址。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::memory::walk_entry`。
@return 各分支实际返回：`xv::memory::walk_entry(pagetable, va, 0)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-60d98a67c33e4923"></a>

## `xv::syscall::cnt_level`

```doxygen
@fn static void xv::syscall::cnt_level(pagetable_t pagetable, uint64 *nsuper, uint64 *n2, uint64 va_start, int level)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：cnt层级。
@param pagetable Sv39 页表根指针。
@param nsuper 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param n2 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param va_start 本次操作使用的 va入口 值。
@param level 本次操作使用的 层级 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `PXSHIFT` → `PTE2PA` → `PTE_LEAF` → `cnt_level`。
@details 需要特别跟踪的条件：`pte & PTE_V`；`level > 0`；`!PTE_LEAF(pte)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
