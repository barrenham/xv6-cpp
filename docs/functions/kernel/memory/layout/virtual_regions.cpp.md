# `kernel/memory/layout/virtual_regions.cpp`

## 函数导航

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
```

<a id="fn-f443815bad1b65fd"></a>

## `xv::memory::VirtualRegions::erase`

```doxygen
@fn bool xv::memory::VirtualRegions::erase(unsigned long begin, unsigned long end)
@brief 删除区域范围并保留两侧剩余部分。
@details 连续区间最多导致一个区域中间分裂，先取得所需条目，失败保持原表；再截断或删除所有覆盖项并释放空元数据页。不会操作 PTE 或释放区域对应的数据帧。
```

<a id="fn-d4b14bc4179e1517"></a>

## `xv::memory::VirtualRegions::copy_to`

```doxygen
@fn bool xv::memory::VirtualRegions::copy_to(VirtualRegions &other) const
@brief 显式复制区域登记到另一个对象。
@details 先在临时候选中逐项 reserve，失败清理候选并保持目标；成功才 reset 目标并交换页链。只复制元数据，不复制页表和数据页，调用方分别处理其所有权。
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
