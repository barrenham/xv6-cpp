# `kernel/memory/allocator/freelist/freelist_page_allocator.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../../kernel/memory/allocator/freelist/freelist_page_allocator.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:freelist`。

直接模块依赖：`xv.sync`, `:allocator`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::FreeListPageAllocator::FreeListPageAllocator`](freelist_page_allocator.cppm.md#fn-d3481d8264cdce85)
- [`xv::memory::FreeListPageAllocator::allocate`](freelist_page_allocator.cpp.md#fn-04a2ef715486f832)
- [`xv::memory::FreeListPageAllocator::deallocate`](freelist_page_allocator.cpp.md#fn-52f97fcca1c3f61d)
- [`xv::memory::FreeListPageAllocator::initialize_layout`](freelist_page_allocator.cpp.md#fn-cde7c1a4166fbfc3)
- [`xv::memory::FreeListPageAllocator::max_order`](freelist_page_allocator.cppm.md#fn-8ab6af7068f17cd6)
- [`xv::memory::FreeListPageAllocator::metadata_bytes`](freelist_page_allocator.cppm.md#fn-cf7ecd14a3aa440e)
- [`xv::memory::FreeListPageAllocator::name`](freelist_page_allocator.cppm.md#fn-e790ea81aab42143)
- [`xv::memory::FreeListPageAllocator::split_to_pages`](freelist_page_allocator.cpp.md#fn-d6e5d9e01e5480ed)
- [`xv::memory::FreeListPageAllocator::valid_page`](freelist_page_allocator.cpp.md#fn-6ab5756247d1a4c1)
- [`xv::memory::FreeListPageAllocator::~FreeListPageAllocator`](freelist_page_allocator.cppm.md#fn-62ca932bc6eb44b1)

<a id="fn-e790ea81aab42143"></a>

## `xv::memory::FreeListPageAllocator::name`

```doxygen
@fn const char *xv::memory::FreeListPageAllocator::name() const override
@brief 单页空闲链分配器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8ab6af7068f17cd6"></a>

## `xv::memory::FreeListPageAllocator::max_order`

```doxygen
@fn unsigned xv::memory::FreeListPageAllocator::max_order() const override
@brief 单页空闲链分配器：读取或计算 `0`，返回当前查询结果。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cf7ecd14a3aa440e"></a>

## `xv::memory::FreeListPageAllocator::metadata_bytes`

```doxygen
@fn MemorySize xv::memory::FreeListPageAllocator::metadata_bytes(PageIndex pages) const override
@brief 单页空闲链分配器：计算给定页面数量需要的元数据字节数。
@param pages 本次操作使用的 页集合 值。
@return 各分支实际返回：`pages * sizeof(unsigned char)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d3481d8264cdce85"></a>

## `xv::memory::FreeListPageAllocator::FreeListPageAllocator`

```doxygen
@fn xv::memory::FreeListPageAllocator::FreeListPageAllocator()
@brief 构造单页空闲链分配器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-62ca932bc6eb44b1"></a>

## `xv::memory::FreeListPageAllocator::~FreeListPageAllocator`

```doxygen
@fn xv::memory::FreeListPageAllocator::~FreeListPageAllocator()
@brief 结束单页空闲链分配器的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
