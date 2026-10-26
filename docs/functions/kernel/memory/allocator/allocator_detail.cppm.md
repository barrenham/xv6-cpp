# `kernel/memory/allocator/allocator_detail.cppm`

开发职责：物理页所有权、地址范围、页表和用户内存复制。

源码：[打开实际文件](../../../../../kernel/memory/allocator/allocator_detail.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.memory:detail`。

直接模块依赖：`:allocator`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::memory::detail::block_size`](allocator_detail.cppm.md#fn-764b40233652a82f)
- [`xv::memory::detail::fail`](../boot/kalloc.cpp.md#fn-b1d0993dea3ca642)
- [`xv::memory::detail::fill`](../boot/kalloc.cpp.md#fn-e2dab6f4860894f9)

<a id="fn-764b40233652a82f"></a>

## `xv::memory::detail::block_size`

```doxygen
@fn inline constexpr MemorySize xv::memory::detail::block_size(unsigned order)
@brief 物理页所有权、地址范围、页表和用户内存复制：读取或计算 `MemorySize(PageSize) << order`，返回当前查询结果。
@param order 物理块阶数，一个块包含 2 的 order 次方个页面。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`MemorySize`。
@return 各分支实际返回：`MemorySize(PageSize) << order`。应结合上面的条件检查区分正常值、空值和错误值。
```
