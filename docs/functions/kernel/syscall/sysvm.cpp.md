# `kernel/syscall/sysvm.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/sysvm.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:arguments`, `:handlers`, `xv.memory`, `xv.process`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::pgpte`](sysvm.cpp.md#fn-58a123e59e099d46)
- [`xv::syscall::sys_ksupernpte`](sysvm.cpp.md#fn-854777261a9f0a46)
- [`xv::syscall::sys_pgpte`](sysvm.cpp.md#fn-ca4e57de1ea0f712)

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
@brief 统计内核大小页并按当前地址空间权限写回结果。
@details 通过持区域锁的 KernelPageTable::statistics 保护页表节点生命周期，计数仍为并发叶映射的观测值；将输出交给带 VirtualRegions 的 copy_to_user；只读、保护或越界用户缓冲失败返回 -1，COW 输出会先私有化。
```

<a id="fn-58a123e59e099d46"></a>

## `xv::syscall::pgpte`

```doxygen
@fn static pte_t *xv::syscall::pgpte(pagetable_t pagetable, uint64 va)
@brief 安全取得用户查询地址对应的 PTE。
@details 超过 MAXVA 时返回空指针，避免把不可信用户地址送入会 panic 的 walk_entry；有效范围按现有叶表查找，不分配新页表。
```
