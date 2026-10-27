# `kernel/memory/allocator/page_allocator.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/allocator/page_allocator.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:allocator`。

直接模块依赖：`:boot`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::OwnedPage::OwnedPage`](page_allocator.cppm.md#fn-53943cb33dde74c2)
- [`xv::memory::OwnedPage::OwnedPage`](page_allocator.cppm.md#fn-4546c6c36537a1d1)
- [`xv::memory::OwnedPage::get`](page_allocator.cppm.md#fn-20606b5bb86c3931)
- [`xv::memory::OwnedPage::operator bool`](page_allocator.cppm.md#fn-822832484382860f)
- [`xv::memory::OwnedPage::operator=`](page_allocator.cppm.md#fn-30338e7893347d66)
- [`xv::memory::OwnedPage::release`](page_allocator.cppm.md#fn-cf4cbb181cd9fbcb)
- [`xv::memory::OwnedPage::~OwnedPage`](page_allocator.cppm.md#fn-3aed0e148ae49606)
- [`xv::memory::PhysicalPageAllocator::PhysicalPageAllocator`](page_allocator.cppm.md#fn-79ac4dcfa04f3171)
- [`xv::memory::PhysicalPageAllocator::PhysicalPageAllocator`](page_allocator.cppm.md#fn-3eb49283b5507fd3)
- [`xv::memory::PhysicalPageAllocator::account_free`](page_allocator.cppm.md#fn-975602334a091791)
- [`xv::memory::PhysicalPageAllocator::allocate`](page_allocator.cppm.md#fn-7ac143f8956b1adf)
- [`xv::memory::PhysicalPageAllocator::deallocate`](page_allocator.cppm.md#fn-c57d63731d54d1aa)
- [`xv::memory::PhysicalPageAllocator::free_pages`](page_allocator.cppm.md#fn-e59eaf9e12a96638)
- [`xv::memory::PhysicalPageAllocator::initialize_layout`](page_allocator.cppm.md#fn-96aea7f14e47df24)
- [`xv::memory::PhysicalPageAllocator::layout`](page_allocator.cppm.md#fn-2ca98cd28a6d8551)
- [`xv::memory::PhysicalPageAllocator::max_order`](page_allocator.cppm.md#fn-454d11f0d5090566)
- [`xv::memory::PhysicalPageAllocator::metadata_bytes`](page_allocator.cppm.md#fn-88a96da96a940daa)
- [`xv::memory::PhysicalPageAllocator::name`](page_allocator.cppm.md#fn-5de127c5e916c7dc)
- [`xv::memory::PhysicalPageAllocator::operator=`](page_allocator.cppm.md#fn-b92d580cbe03e08f)
- [`xv::memory::PhysicalPageAllocator::page_references`](allocator_service.cpp.md#fn-a8098c94ba5f8e20)
- [`xv::memory::PhysicalPageAllocator::release_page`](allocator_service.cpp.md#fn-75415337725bd4f1)
- [`xv::memory::PhysicalPageAllocator::retain_page`](allocator_service.cpp.md#fn-c818c3445e9c8f6d)
- [`xv::memory::PhysicalPageAllocator::split_to_pages`](page_allocator.cppm.md#fn-e80d757ba18133f2)
- [`xv::memory::PhysicalPageAllocator::supports`](page_allocator.cppm.md#fn-15a873757be5f764)
- [`xv::memory::PhysicalPageAllocator::~PhysicalPageAllocator`](page_allocator.cppm.md#fn-f8b502c882826938)
- [`xv::memory::allocator`](allocator_service.cpp.md#fn-f2b789758ee2f161)
- [`xv::memory::boot_initialize`](../boot/kalloc.cpp.md#fn-df1a8ede82b392e0)
- [`xv::memory::configured_allocator_kind`](../boot/kalloc.cpp.md#fn-d516d0e9e55b7d40)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e42f08f40f678227)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-6294cdfa90bca295)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-e58331769ab453a3)
- [`xv::memory::initialize`](allocator_service.cpp.md#fn-8b1954f8cb6eb078)

<a id="fn-3eb49283b5507fd3"></a>

## `xv::memory::PhysicalPageAllocator::PhysicalPageAllocator`

```doxygen
@fn xv::memory::PhysicalPageAllocator::PhysicalPageAllocator(const PhysicalPageAllocator &)
@brief 物理页所有权、地址范围、页表和用户内存复制的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-b92d580cbe03e08f"></a>

## `xv::memory::PhysicalPageAllocator::operator=`

```doxygen
@fn PhysicalPageAllocator &xv::memory::PhysicalPageAllocator::operator=(const PhysicalPageAllocator &)
@brief 物理页所有权、地址范围、页表和用户内存复制的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-5de127c5e916c7dc"></a>

## `xv::memory::PhysicalPageAllocator::name`

```doxygen
@fn virtual const char *xv::memory::PhysicalPageAllocator::name() const
@brief 物理页所有权、地址范围、页表和用户内存复制：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-454d11f0d5090566"></a>

## `xv::memory::PhysicalPageAllocator::max_order`

```doxygen
@fn virtual unsigned xv::memory::PhysicalPageAllocator::max_order() const
@brief 物理页所有权、地址范围、页表和用户内存复制：最大阶数。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `unsigned int () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-15a873757be5f764"></a>

## `xv::memory::PhysicalPageAllocator::supports`

```doxygen
@fn bool xv::memory::PhysicalPageAllocator::supports(unsigned order) const
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `order <= max_order()`，返回当前查询结果。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`max_order`。
@return 各分支实际返回：`order <= max_order()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2ca98cd28a6d8551"></a>

## `xv::memory::PhysicalPageAllocator::layout`

```doxygen
@fn const PhysicalMemoryLayout &xv::memory::PhysicalPageAllocator::layout() const
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `layout_`，返回当前查询结果。
@return 各分支实际返回：`layout_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-88a96da96a940daa"></a>

## `xv::memory::PhysicalPageAllocator::metadata_bytes`

```doxygen
@fn virtual MemorySize xv::memory::PhysicalPageAllocator::metadata_bytes(PageIndex pages) const
@brief 物理页所有权、地址范围、页表和用户内存复制：计算给定页面数量需要的元数据字节数。
@param pages 本次操作使用的 页集合 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `MemorySize (PageIndex) const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-7ac143f8956b1adf"></a>

## `xv::memory::PhysicalPageAllocator::allocate`

```doxygen
@fn virtual void *xv::memory::PhysicalPageAllocator::allocate(unsigned order = 0)
@brief 物理页所有权、地址范围、页表和用户内存复制：分配并取得对象或存储资源。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c57d63731d54d1aa"></a>

## `xv::memory::PhysicalPageAllocator::deallocate`

```doxygen
@fn virtual void xv::memory::PhysicalPageAllocator::deallocate(void *address, unsigned order = 0)
@brief 物理页所有权、地址范围、页表和用户内存复制：把已分配的存储归还分配器。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e80d757ba18133f2"></a>

## `xv::memory::PhysicalPageAllocator::split_to_pages`

```doxygen
@fn virtual bool xv::memory::PhysicalPageAllocator::split_to_pages(void *address, unsigned order)
@brief 物理页所有权、地址范围、页表和用户内存复制：把高阶物理块转换为独立页面的所有权状态。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `bool (void *, unsigned int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-79ac4dcfa04f3171"></a>

## `xv::memory::PhysicalPageAllocator::PhysicalPageAllocator`

```doxygen
@fn xv::memory::PhysicalPageAllocator::PhysicalPageAllocator()
@brief 构造物理页所有权、地址范围、页表和用户内存复制，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f8b502c882826938"></a>

## `xv::memory::PhysicalPageAllocator::~PhysicalPageAllocator`

```doxygen
@fn xv::memory::PhysicalPageAllocator::~PhysicalPageAllocator()
@brief 结束物理页所有权、地址范围、页表和用户内存复制的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-96aea7f14e47df24"></a>

## `xv::memory::PhysicalPageAllocator::initialize_layout`

```doxygen
@fn virtual void xv::memory::PhysicalPageAllocator::initialize_layout()
@brief 物理页所有权、地址范围、页表和用户内存复制：按已规划内存布局建立分配器元数据与空闲集合。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-53943cb33dde74c2"></a>

## `xv::memory::OwnedPage::OwnedPage`

```doxygen
@fn explicit xv::memory::OwnedPage::OwnedPage(PhysicalPageAllocator &allocator)
@brief 构造物理页所有权句柄，建立本重载要求的成员初值和借用关系。
@param allocator 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocator_` → `page_` → `allocator.allocate`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3aed0e148ae49606"></a>

## `xv::memory::OwnedPage::~OwnedPage`

```doxygen
@fn xv::memory::OwnedPage::~OwnedPage()
@brief 结束物理页所有权句柄的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocator_.deallocate`。
@details 需要特别跟踪的条件：`page_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4546c6c36537a1d1"></a>

## `xv::memory::OwnedPage::OwnedPage`

```doxygen
@fn xv::memory::OwnedPage::OwnedPage(const OwnedPage &)
@brief 物理页所有权句柄的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-30338e7893347d66"></a>

## `xv::memory::OwnedPage::operator=`

```doxygen
@fn OwnedPage &xv::memory::OwnedPage::operator=(const OwnedPage &)
@brief 物理页所有权句柄的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-20606b5bb86c3931"></a>

## `xv::memory::OwnedPage::get`

```doxygen
@fn void *xv::memory::OwnedPage::get() const
@brief 物理页所有权句柄：读取或计算 `page_`，返回当前查询结果。
@return 各分支实际返回：`page_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-822832484382860f"></a>

## `xv::memory::OwnedPage::operator bool`

```doxygen
@fn explicit xv::memory::OwnedPage::operator bool() const
@brief 检查物理页所有权句柄是否持有有效状态。
@return 各分支实际返回：`page_ != nullptr`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cf4cbb181cd9fbcb"></a>

## `xv::memory::OwnedPage::release`

```doxygen
@fn void *xv::memory::OwnedPage::release()
@brief 物理页所有权句柄：释放当前引用或归还对象占用的资源。
@details 本函数可见的状态更新包括：`page_ = nullptr;`。局部临时变量与对象字段应按作用域区分。
@return 各分支实际返回：`page`。应结合上面的条件检查区分正常值、空值和错误值。
```
<a id="fn-e59eaf9e12a96638"></a>

## `xv::memory::PhysicalPageAllocator::free_pages`

```doxygen
@fn unsigned long xv::memory::PhysicalPageAllocator::free_pages() const
@brief 原子读取当前空闲物理页计数。
@details 以 4 KiB 页为单位，任意阶块按页数计入；并发分配与归还期间为观测值，调用返回后无保留责任。
```

<a id="fn-975602334a091791"></a>

## `xv::memory::PhysicalPageAllocator::account_free`

```doxygen
@fn void xv::memory::PhysicalPageAllocator::account_free(long pages)
@brief 原子调整空闲物理页计数。
@details 派生分配器在成功分配时减去 2^order，归还时加回原始块页数；分裂已拥有块不改变数量。仅调整统计，不改变空闲链表或资源所有权。
```
