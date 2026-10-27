# `kernel/memory/layout/virtual_regions.cppm`

## 函数导航

- [`xv::memory::VirtualRegions::VirtualRegions`](virtual_regions.cppm.md#fn-1ff79176ef567392)
- [`xv::memory::VirtualRegions::VirtualRegions`](virtual_regions.cppm.md#fn-1195caf03662eaaa)
- [`xv::memory::VirtualRegions::copy_to`](virtual_regions.cpp.md#fn-d4b14bc4179e1517)
- [`xv::memory::VirtualRegions::erase`](virtual_regions.cpp.md#fn-f443815bad1b65fd)
- [`xv::memory::VirtualRegions::find`](virtual_regions.cpp.md#fn-0b34c082f4337941)
- [`xv::memory::VirtualRegions::operator=`](virtual_regions.cppm.md#fn-25eaebefbac3b978)
- [`xv::memory::VirtualRegions::reserve`](virtual_regions.cpp.md#fn-df1507921742a0ed)
- [`xv::memory::VirtualRegions::reset`](virtual_regions.cpp.md#fn-199c0092b683e521)
- [`xv::memory::VirtualRegions::swap`](virtual_regions.cpp.md#fn-6db77b90e60c05c2)
- [`xv::memory::VirtualRegions::vacant`](virtual_regions.cpp.md#fn-31bbbc071056140a)


实现可增长的虚拟地址区域登记，记录半开边界、权限、用途及匿名按需补页策略。
元数据明确由一个地址空间拥有，调用者串行化修改，reset 显式结束所有权。

源码：[打开实际文件](../../../../../kernel/memory/layout/virtual_regions.cppm)。


<a id="fn-1ff79176ef567392"></a>

## `xv::memory::VirtualRegions::VirtualRegions`

```doxygen
@fn constexpr xv::memory::VirtualRegions::VirtualRegions()
@brief 定义空区域登记对象及禁止隐式复制的构造约定。
@details 默认构造不分配内存，复制构造显式删除；需要副本时调用可返回 OOM 的 copy_to，销毁所有权通过 reset 显式结束。
```

<a id="fn-1195caf03662eaaa"></a>

## `xv::memory::VirtualRegions::VirtualRegions`

```doxygen
@fn xv::memory::VirtualRegions::VirtualRegions(const VirtualRegions &)
@brief 定义空区域登记对象及禁止隐式复制的构造约定。
@details 默认构造不分配内存，复制构造显式删除；需要副本时调用可返回 OOM 的 copy_to，销毁所有权通过 reset 显式结束。
```

<a id="fn-25eaebefbac3b978"></a>

## `xv::memory::VirtualRegions::operator=`

```doxygen
@fn VirtualRegions &xv::memory::VirtualRegions::operator=(const VirtualRegions &) = delete
@brief 禁止区域页链的隐式复制赋值。
@details 删除此操作避免两个拥有者释放同一元数据页；显式副本使用 copy_to，镜像提交使用 swap。
```
