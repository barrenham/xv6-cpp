# `kernel/memory/layout/virtual_regions.cpp`

## 函数导航

- [`xv::memory::VirtualRegions::CopyFlow::commit`](virtual_regions.cpp.md#fn-47bdedccc4f2ca85)
- [`xv::memory::VirtualRegions::CopyFlow::populate`](virtual_regions.cpp.md#fn-aef0abe608e881df)
- [`xv::memory::VirtualRegions::CopyFlow::rollback`](virtual_regions.cpp.md#fn-a8bf87696c9865ff)
- [`xv::memory::VirtualRegions::EraseFlow::allocate`](virtual_regions.cpp.md#fn-5439ea1fdf69ea25)
- [`xv::memory::VirtualRegions::EraseFlow::find_split`](virtual_regions.cpp.md#fn-a06c1958aa7ab8f3)
- [`xv::memory::VirtualRegions::EraseFlow::reclaim`](virtual_regions.cpp.md#fn-7598c9c7b9d15d09)
- [`xv::memory::VirtualRegions::EraseFlow::split`](virtual_regions.cpp.md#fn-2282436089c126c1)
- [`xv::memory::VirtualRegions::EraseFlow::split_needed`](virtual_regions.cpp.md#fn-d64b6d65be3c4c09)
- [`xv::memory::VirtualRegions::EraseFlow::success`](virtual_regions.cpp.md#fn-6526de33144958ec)
- [`xv::memory::VirtualRegions::EraseFlow::trim`](virtual_regions.cpp.md#fn-7fbcd7250aec755c)
- [`xv::memory::VirtualRegions::EraseFlow::valid`](virtual_regions.cpp.md#fn-0016fb93b07742bd)
- [`xv::memory::VirtualRegions::ReserveFlow::adjacent`](virtual_regions.cpp.md#fn-8f033f8fd3225632)
- [`xv::memory::VirtualRegions::ReserveFlow::allocate`](virtual_regions.cpp.md#fn-1118cc96a18f103c)
- [`xv::memory::VirtualRegions::ReserveFlow::extend`](virtual_regions.cpp.md#fn-1cc983ce57206225)
- [`xv::memory::VirtualRegions::ReserveFlow::insert`](virtual_regions.cpp.md#fn-5f274b1cd5da3ea7)
- [`xv::memory::VirtualRegions::ReserveFlow::scan`](virtual_regions.cpp.md#fn-c49336d3a6c77a6c)
- [`xv::memory::VirtualRegions::ReserveFlow::success`](virtual_regions.cpp.md#fn-f4c67b2b854b6a05)
- [`xv::memory::VirtualRegions::ReserveFlow::valid`](virtual_regions.cpp.md#fn-494987f2d4702d17)
- [`xv::memory::VirtualRegions::copy_to`](virtual_regions.cpp.md#fn-d4b14bc4179e1517)
- [`xv::memory::VirtualRegions::erase`](virtual_regions.cpp.md#fn-f443815bad1b65fd)
- [`xv::memory::VirtualRegions::find`](virtual_regions.cpp.md#fn-0b34c082f4337941)
- [`xv::memory::VirtualRegions::reserve`](virtual_regions.cpp.md#fn-df1507921742a0ed)
- [`xv::memory::VirtualRegions::reset`](virtual_regions.cpp.md#fn-199c0092b683e521)
- [`xv::memory::VirtualRegions::swap`](virtual_regions.cpp.md#fn-6db77b90e60c05c2)
- [`xv::memory::VirtualRegions::vacant`](virtual_regions.cpp.md#fn-31bbbc071056140a)

实现可增长的虚拟地址区域登记，记录半开边界、权限、用途及匿名按需补页策略。
元数据明确由一个地址空间拥有，调用者串行化修改，reset 显式结束所有权。

源码：[打开实际文件](../../../../../kernel/memory/layout/virtual_regions.cpp)。


<a id="fn-31bbbc071056140a"></a>

## `xv::memory::VirtualRegions::vacant`

```doxygen
@fn VirtualRegion *xv::memory::VirtualRegions::vacant()
@brief 取得一项空闲区域元数据。
@details 遍历已分配页中的 64 个条目；不足时从物理分配器取得一页并清零，接入当前对象的页链。失败返回空指针，不改变已有条目；调用者提供串行化。
```

<a id="fn-0b34c082f4337941"></a>

## `xv::memory::VirtualRegions::find`

```doxygen
@fn const VirtualRegion *xv::memory::VirtualRegions::find(unsigned long address) const
@brief 查询包含给定地址的半开区域。
@details 遍历动态元数据页，只有 begin 不大于地址且地址小于 end 才匹配；未登记返回空。返回指针借用表内存，调用方必须防止同时修改或 reset。
```

<a id="fn-df1507921742a0ed"></a>

## `xv::memory::VirtualRegions::reserve`

```doxygen
@fn bool xv::memory::VirtualRegions::reserve(unsigned long begin, unsigned long end, unsigned permissions, RegionKind kind, bool demand)
@brief 登记一段不重叠的虚拟地址及访问策略。
@details 拒绝反向、超过低半 Sv39 上界、非法权限和带权限的保护区；检查全表无重叠后可合并相邻同属性条目，否则分配新条目。元数据 OOM 不改变已有区域。demand 仅表示允许匿名补页，不代表文件后备存储。
@details 编排由 ReserveFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-f443815bad1b65fd"></a>

## `xv::memory::VirtualRegions::erase`

```doxygen
@fn bool xv::memory::VirtualRegions::erase(unsigned long begin, unsigned long end)
@brief 删除区域范围并保留两侧剩余部分。
@details 连续区间最多导致一个区域中间分裂，先取得所需条目，失败保持原表；再截断或删除所有覆盖项并释放空元数据页。不会操作 PTE 或释放区域对应的数据帧。
@details 编排由 EraseFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-d4b14bc4179e1517"></a>

## `xv::memory::VirtualRegions::copy_to`

```doxygen
@fn bool xv::memory::VirtualRegions::copy_to(VirtualRegions &other) const
@brief 显式复制区域登记到另一个对象。
@details 先在临时候选中逐项 reserve，失败清理候选并保持目标；成功才 reset 目标并交换页链。只复制元数据，不复制页表和数据页，调用方分别处理其所有权。
@details 编排由 CopyFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-6db77b90e60c05c2"></a>

## `xv::memory::VirtualRegions::swap`

```doxygen
@fn void xv::memory::VirtualRegions::swap(VirtualRegions &other)
@brief 交换两个地址区域登记对象的页链。
@details 不分配、不释放内存，供 exec 候选镜像提交及显式复制使用；双方不得同时被其他上下文修改。
```

<a id="fn-199c0092b683e521"></a>

## `xv::memory::VirtualRegions::reset`

```doxygen
@fn void xv::memory::VirtualRegions::reset()
@brief 释放本对象全部区域元数据页。
@details 逐页归还物理分配器并清空页链，可重复调用；不撤销 PTE、不归还映射目标，地址空间拥有者须另外处理。
```

<a id="fn-47bdedccc4f2ca85"></a>

## `xv::memory::VirtualRegions::CopyFlow::commit`

```doxygen
@fn static void xv::memory::VirtualRegions::CopyFlow::commit(Context &c) noexcept
@brief 释放目标旧区域集并交换候选，记录成功。
@details 先构造候选区域集；失败仅释放候选，成功才替换目标。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-aef0abe608e881df"></a>

## `xv::memory::VirtualRegions::CopyFlow::populate`

```doxygen
@fn static bool xv::memory::VirtualRegions::CopyFlow::populate(Context &c) noexcept
@brief 复制全部有效区域到候选集，遇到预留失败立即报告失败。
@details 先构造候选区域集；失败仅释放候选，成功才替换目标。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a8bf87696c9865ff"></a>

## `xv::memory::VirtualRegions::CopyFlow::rollback`

```doxygen
@fn static void xv::memory::VirtualRegions::CopyFlow::rollback(Context &c) noexcept
@brief 释放未提交候选的全部元数据。
@details 先构造候选区域集；失败仅释放候选，成功才替换目标。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5439ea1fdf69ea25"></a>

## `xv::memory::VirtualRegions::EraseFlow::allocate`

```doxygen
@fn static bool xv::memory::VirtualRegions::EraseFlow::allocate(Context &c) noexcept
@brief 在修改原条目前取得分裂尾部的元数据。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a06c1958aa7ab8f3"></a>

## `xv::memory::VirtualRegions::EraseFlow::find_split`

```doxygen
@fn static void xv::memory::VirtualRegions::EraseFlow::find_split(Context &c) noexcept
@brief 直接扫描必须保留两侧的中部分裂条目。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7598c9c7b9d15d09"></a>

## `xv::memory::VirtualRegions::EraseFlow::reclaim`

```doxygen
@fn static void xv::memory::VirtualRegions::EraseFlow::reclaim(Context &c) noexcept
@brief 直接遍历并回收已经没有有效条目的元数据页。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-2282436089c126c1"></a>

## `xv::memory::VirtualRegions::EraseFlow::split`

```doxygen
@fn static void xv::memory::VirtualRegions::EraseFlow::split(Context &c) noexcept
@brief 复制尾部并缩短原条目，保留删除区间两侧。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d64b6d65be3c4c09"></a>

## `xv::memory::VirtualRegions::EraseFlow::split_needed`

```doxygen
@fn static bool xv::memory::VirtualRegions::EraseFlow::split_needed(Context &c) noexcept
@brief 检查是否需要中部分裂。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-6526de33144958ec"></a>

## `xv::memory::VirtualRegions::EraseFlow::success`

```doxygen
@fn static void xv::memory::VirtualRegions::EraseFlow::success(Context &c) noexcept
@brief 记录删除成功。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7fbcd7250aec755c"></a>

## `xv::memory::VirtualRegions::EraseFlow::trim`

```doxygen
@fn static void xv::memory::VirtualRegions::EraseFlow::trim(Context &c) noexcept
@brief 直接裁剪或清除所有相交条目。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0016fb93b07742bd"></a>

## `xv::memory::VirtualRegions::EraseFlow::valid`

```doxygen
@fn static bool xv::memory::VirtualRegions::EraseFlow::valid(Context &c) noexcept
@brief 检查待删除区间是否非空且有序。
@details 需要中部分裂时先取得空闲元数据，失败不修改区间；普通裁剪后回收空元数据页。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-8f033f8fd3225632"></a>

## `xv::memory::VirtualRegions::ReserveFlow::adjacent`

```doxygen
@fn static bool xv::memory::VirtualRegions::ReserveFlow::adjacent(Context &c) noexcept
@brief 检查是否已找到兼容邻接项。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1118cc96a18f103c"></a>

## `xv::memory::VirtualRegions::ReserveFlow::allocate`

```doxygen
@fn static bool xv::memory::VirtualRegions::ReserveFlow::allocate(Context &c) noexcept
@brief 取得空闲元数据条目，必要时分配元数据页。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1cc983ce57206225"></a>

## `xv::memory::VirtualRegions::ReserveFlow::extend`

```doxygen
@fn static void xv::memory::VirtualRegions::ReserveFlow::extend(Context &c) noexcept
@brief 扩展选中的邻接区间。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5f274b1cd5da3ea7"></a>

## `xv::memory::VirtualRegions::ReserveFlow::insert`

```doxygen
@fn static void xv::memory::VirtualRegions::ReserveFlow::insert(Context &c) noexcept
@brief 写入新的区间、权限、类型和按需分配属性。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-c49336d3a6c77a6c"></a>

## `xv::memory::VirtualRegions::ReserveFlow::scan`

```doxygen
@fn static bool xv::memory::VirtualRegions::ReserveFlow::scan(Context &c) noexcept
@brief 直接扫描已有条目，拒绝重叠并保存兼容邻接项。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f4c67b2b854b6a05"></a>

## `xv::memory::VirtualRegions::ReserveFlow::success`

```doxygen
@fn static void xv::memory::VirtualRegions::ReserveFlow::success(Context &c) noexcept
@brief 记录预留成功。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-494987f2d4702d17"></a>

## `xv::memory::VirtualRegions::ReserveFlow::valid`

```doxygen
@fn static bool xv::memory::VirtualRegions::ReserveFlow::valid(Context &c) noexcept
@brief 验证区间、虚拟地址上限、权限组合和保护区约束。
@details 调用者串行化元数据修改；扫描拒绝重叠，沿用最后一个兼容邻接项的选择规则。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
