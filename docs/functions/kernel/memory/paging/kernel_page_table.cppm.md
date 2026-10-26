# `kernel/memory/paging/kernel_page_table.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/paging/kernel_page_table.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:kernel_paging`。

## 函数导航

- [`xv::memory::kernel_page_table`](kernel_page_table.cpp.md#fn-1f3d315da469e7cf)
- [`xv::memory::KernelPageTable::KernelPageTable`](kernel_page_table.cppm.md#fn-53592a8c15ae7380)
- [`xv::memory::KernelPageTable::KernelPageTable`](kernel_page_table.cppm.md#fn-63812dcc872dc61f)
- [`xv::memory::KernelPageTable::activate_hart`](kernel_page_table.cpp.md#fn-4f82617b1488d29d)
- [`xv::memory::KernelPageTable::initialize`](kernel_page_table.cpp.md#fn-b348ddac60edd845)
- [`xv::memory::KernelPageTable::map_process_stack`](kernel_page_table.cpp.md#fn-e70dfce3f258395b)
- [`xv::memory::KernelPageTable::operator=`](kernel_page_table.cppm.md#fn-609d30eed538e9ac)
- [`xv::memory::KernelPageTable::root`](kernel_page_table.cpp.md#fn-792824e0d2159c11)
- [`xv::memory::KernelPageTable::unmap_process_stack`](kernel_page_table.cpp.md#fn-98155d2b3976236b)

<a id="fn-53592a8c15ae7380"></a>

## `xv::memory::KernelPageTable::KernelPageTable`

```doxygen
@fn constexpr xv::memory::KernelPageTable::KernelPageTable()
@brief 构造内核页表管理器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-63812dcc872dc61f"></a>

## `xv::memory::KernelPageTable::KernelPageTable`

```doxygen
@fn xv::memory::KernelPageTable::KernelPageTable(const KernelPageTable &)
@brief 内核页表管理器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-609d30eed538e9ac"></a>

## `xv::memory::KernelPageTable::operator=`

```doxygen
@fn KernelPageTable &xv::memory::KernelPageTable::operator=(const KernelPageTable &)
@brief 内核页表管理器的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
