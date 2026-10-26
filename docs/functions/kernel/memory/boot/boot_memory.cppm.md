# `kernel/memory/boot/boot_memory.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/boot/boot_memory.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:boot`。

直接模块依赖：`:layout`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::BootMemoryInfo::ram`](boot_memory.cppm.md#fn-6967ca1ba4b08d8d)
- [`xv::memory::BootMemoryInfo::reservation`](boot_memory.cppm.md#fn-36750e48011084b0)
- [`xv::memory::BootMemoryInfo::reservation_count`](boot_memory.cppm.md#fn-f53156fcc0315fbf)
- [`xv::memory::BootMemoryInfo::reserve`](boot_memory.cppm.md#fn-ad72eb349402bcb5)
- [`xv::memory::BootMemoryInfo::set_ram`](boot_memory.cppm.md#fn-b879ecc7451110b5)
- [`xv::memory::BootMemorySource::discover`](boot_memory.cppm.md#fn-748ab993e7b08ce1)
- [`xv::memory::BootMemorySource::name`](boot_memory.cppm.md#fn-cd0fa4d433d1e20f)
- [`xv::memory::BootMemorySource::~BootMemorySource`](boot_memory.cppm.md#fn-438240109583ce50)
- [`xv::memory::FdtMemorySource::FdtMemorySource`](boot_memory.cppm.md#fn-66e67ff370f2ef79)
- [`xv::memory::FdtMemorySource::discover`](boot_memory.cpp.md#fn-9f01195f9922800f)
- [`xv::memory::FdtMemorySource::name`](boot_memory.cppm.md#fn-e2a100ade18dd708)

<a id="fn-b879ecc7451110b5"></a>

## `xv::memory::BootMemoryInfo::set_ram`

```doxygen
@fn bool xv::memory::BootMemoryInfo::set_ram(PhysicalMemoryRange ram)
@brief 引导内存范围集合：登记启动发现的物理 RAM 区间。
@param ram 本次操作使用的 RAM 范围 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ram_.empty` → `ram.empty`。
@details 本函数可见的状态更新包括：`ram_ = ram;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!ram_.empty() || ram.empty()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ad72eb349402bcb5"></a>

## `xv::memory::BootMemoryInfo::reserve`

```doxygen
@fn bool xv::memory::BootMemoryInfo::reserve(PhysicalMemoryRange range, bool no_map = false)
@brief 引导内存范围集合：从可分配区间中排除保留内存。
@param range 本次操作使用的 区间 值。
@param no_map 本次操作使用的 nomap 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`range.empty`。
@details 本函数可见的状态更新包括：`reserved_[count_++] = {range, no_map};`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`range.empty()`；`count_ == MaxReservations`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`true`；`false`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6967ca1ba4b08d8d"></a>

## `xv::memory::BootMemoryInfo::ram`

```doxygen
@fn PhysicalMemoryRange xv::memory::BootMemoryInfo::ram() const
@brief 引导内存范围集合：读取或计算 `ram_`，返回当前查询结果。
@return 各分支实际返回：`ram_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f53156fcc0315fbf"></a>

## `xv::memory::BootMemoryInfo::reservation_count`

```doxygen
@fn unsigned xv::memory::BootMemoryInfo::reservation_count() const
@brief 引导内存范围集合：读取或计算 `count_`，返回当前查询结果。
@return 各分支实际返回：`count_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-36750e48011084b0"></a>

## `xv::memory::BootMemoryInfo::reservation`

```doxygen
@fn const Reservation &xv::memory::BootMemoryInfo::reservation(unsigned index) const
@brief 引导内存范围集合：读取或计算 `reserved_[index]`，返回当前查询结果。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail`。
@details 需要特别跟踪的条件：`index >= count_`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`reserved_[index]`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-cd0fa4d433d1e20f"></a>

## `xv::memory::BootMemorySource::name`

```doxygen
@fn virtual const char *xv::memory::BootMemorySource::name() const
@brief 物理页所有权、地址范围、页表和用户内存复制：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-748ab993e7b08ce1"></a>

## `xv::memory::BootMemorySource::discover`

```doxygen
@fn virtual bool xv::memory::BootMemorySource::discover(BootMemoryInfo &result)
@brief 物理页所有权、地址范围、页表和用户内存复制：从设备树或内存来源解析可用内存及保留区间。
@param result 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `bool (BootMemoryInfo &)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-438240109583ce50"></a>

## `xv::memory::BootMemorySource::~BootMemorySource`

```doxygen
@fn xv::memory::BootMemorySource::~BootMemorySource()
@brief 结束物理页所有权、地址范围、页表和用户内存复制的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-66e67ff370f2ef79"></a>

## `xv::memory::FdtMemorySource::FdtMemorySource`

```doxygen
@fn xv::memory::FdtMemorySource::FdtMemorySource(const void *blob, MemorySize capacity)
@brief 构造设备树内存信息读取器，建立本重载要求的成员初值和借用关系。
@param blob 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param capacity 本次操作使用的 capacity 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`blob_` → `capacity_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e2a100ade18dd708"></a>

## `xv::memory::FdtMemorySource::name`

```doxygen
@fn const char *xv::memory::FdtMemorySource::name() const override
@brief 设备树内存信息读取器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
