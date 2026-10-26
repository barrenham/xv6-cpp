# `kernel/memory/allocator/buddy/buddy_page_allocator.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../../kernel/memory/allocator/buddy/buddy_page_allocator.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:buddy`。

直接模块依赖：`xv.sync`, `:allocator`, `:detail`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::BuddyPageAllocator::BuddyPageAllocator`](buddy_page_allocator.cppm.md#fn-bcbd84edb39e5051)
- [`xv::memory::BuddyPageAllocator::allocate`](buddy_page_allocator.cpp.md#fn-0e4aa39ee90982b8)
- [`xv::memory::BuddyPageAllocator::claim`](buddy_page_allocator.cpp.md#fn-6f33683dd35acc36)
- [`xv::memory::BuddyPageAllocator::deallocate`](buddy_page_allocator.cpp.md#fn-3eb32f523d5d265d)
- [`xv::memory::BuddyPageAllocator::initialize_layout`](buddy_page_allocator.cpp.md#fn-2d9e53492f4ac4fd)
- [`xv::memory::BuddyPageAllocator::insert`](buddy_page_allocator.cpp.md#fn-4e685c08c7401b4d)
- [`xv::memory::BuddyPageAllocator::max_order`](buddy_page_allocator.cppm.md#fn-cc393f319a6a966a)
- [`xv::memory::BuddyPageAllocator::metadata_bytes`](buddy_page_allocator.cppm.md#fn-8718caadd85615ee)
- [`xv::memory::BuddyPageAllocator::name`](buddy_page_allocator.cppm.md#fn-5eefe99fade9422f)
- [`xv::memory::BuddyPageAllocator::page_tag`](buddy_page_allocator.cpp.md#fn-c14f515bed1c0679)
- [`xv::memory::BuddyPageAllocator::remove_block`](buddy_page_allocator.cpp.md#fn-de65a654acaf47be)
- [`xv::memory::BuddyPageAllocator::set_tag`](buddy_page_allocator.cpp.md#fn-7257ac4207c21591)
- [`xv::memory::BuddyPageAllocator::split_to_pages`](buddy_page_allocator.cpp.md#fn-98b0a2320cb39511)
- [`xv::memory::BuddyPageAllocator::valid_block`](buddy_page_allocator.cpp.md#fn-904352b78a66a7f9)
- [`xv::memory::BuddyPageAllocator::~BuddyPageAllocator`](buddy_page_allocator.cppm.md#fn-7d5e2f2a56410636)

<a id="fn-5eefe99fade9422f"></a>

## `xv::memory::BuddyPageAllocator::name`

```doxygen
@fn const char *xv::memory::BuddyPageAllocator::name() const override
@brief 伙伴物理页分配器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cc393f319a6a966a"></a>

## `xv::memory::BuddyPageAllocator::max_order`

```doxygen
@fn unsigned xv::memory::BuddyPageAllocator::max_order() const override
@brief 伙伴物理页分配器：读取或计算 `SuperpageOrder`，返回当前查询结果。
@return 各分支实际返回：`SuperpageOrder`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8718caadd85615ee"></a>

## `xv::memory::BuddyPageAllocator::metadata_bytes`

```doxygen
@fn MemorySize xv::memory::BuddyPageAllocator::metadata_bytes(PageIndex pages) const override
@brief 伙伴物理页分配器：计算给定页面数量需要的元数据字节数。
@param pages 本次操作使用的 页集合 值。
@return 各分支实际返回：`pages * sizeof(int)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bcbd84edb39e5051"></a>

## `xv::memory::BuddyPageAllocator::BuddyPageAllocator`

```doxygen
@fn xv::memory::BuddyPageAllocator::BuddyPageAllocator()
@brief 构造伙伴物理页分配器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-7d5e2f2a56410636"></a>

## `xv::memory::BuddyPageAllocator::~BuddyPageAllocator`

```doxygen
@fn xv::memory::BuddyPageAllocator::~BuddyPageAllocator()
@brief 结束伙伴物理页分配器的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
