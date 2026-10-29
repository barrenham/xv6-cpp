# `kernel/memory/layout/virtual_address_pool.cppm`

源码：[打开实际文件](../../../../../kernel/memory/layout/virtual_address_pool.cppm)。

该文件的红黑树或地址池职责见 [内核虚拟地址池](../../../../virtual-address-pool.md)。

## 函数导航

- [`xv::memory::VirtualAddressPool::Less::operator()`](virtual_address_pool.cppm.md#fn-812cc19810f3983b)
- [`xv::memory::VirtualAddressPool::Node::update`](virtual_address_pool.cppm.md#fn-40b589964ab8b031)
- [`xv::memory::VirtualAddressPool::VirtualAddressPool`](virtual_address_pool.cppm.md#fn-126cc52d8db34a4d)
- [`xv::memory::VirtualAddressPool::VirtualAddressPool`](virtual_address_pool.cppm.md#fn-90d3a71be5c9cdfa)
- [`xv::memory::VirtualAddressPool::allocation`](virtual_address_pool.cpp.md#fn-d6a9e9d61906627d)
- [`xv::memory::VirtualAddressPool::initialize`](virtual_address_pool.cpp.md#fn-0009ac3368aaa857)
- [`xv::memory::VirtualAddressPool::largest_free`](virtual_address_pool.cpp.md#fn-ba3cdfa791efba66)
- [`xv::memory::VirtualAddressPool::operator=`](virtual_address_pool.cppm.md#fn-60f577b4f1cc9cbf)
- [`xv::memory::VirtualAddressPool::put_node`](virtual_address_pool.cpp.md#fn-017f88ea27af8b28)
- [`xv::memory::VirtualAddressPool::release`](virtual_address_pool.cpp.md#fn-9806246ea6f6e910)
- [`xv::memory::VirtualAddressPool::reset`](virtual_address_pool.cpp.md#fn-11eceb419e804590)
- [`xv::memory::VirtualAddressPool::take_node`](virtual_address_pool.cpp.md#fn-70afa19e0dd61a33)

<a id="fn-126cc52d8db34a4d"></a>

## `xv::memory::VirtualAddressPool::VirtualAddressPool`

```doxygen
@fn constexpr xv::memory::VirtualAddressPool::VirtualAddressPool()
@brief 构造空池；复制构造被删除，区间元数据生命周期需显式管理。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```

<a id="fn-90d3a71be5c9cdfa"></a>

## `xv::memory::VirtualAddressPool::VirtualAddressPool`

```doxygen
@fn xv::memory::VirtualAddressPool::VirtualAddressPool(const VirtualAddressPool &)
@brief 构造空池；复制构造被删除，区间元数据生命周期需显式管理。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```

<a id="fn-60f577b4f1cc9cbf"></a>

## `xv::memory::VirtualAddressPool::operator=`

```doxygen
@fn VirtualAddressPool &xv::memory::VirtualAddressPool::operator=(const VirtualAddressPool &)
@brief 禁止复制地址池，避免共享树根及元数据所有权。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```

<a id="fn-40b589964ab8b031"></a>

## `xv::memory::VirtualAddressPool::Node::update`

```doxygen
@fn void xv::memory::VirtualAddressPool::Node::update()
@brief 从自身区间长度和左右子树聚合值计算最大空闲长度。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```

<a id="fn-812cc19810f3983b"></a>

## `xv::memory::VirtualAddressPool::Less::operator()`

```doxygen
@fn bool xv::memory::VirtualAddressPool::Less::operator()(const Node &a, const Node &b) const
@brief 仅按区间起点比较节点，保持两棵区间树的地址顺序。
@details 仅管理虚拟区间及其元数据，不分配映射目标；修改和查询由调用方串行，空元数据页立即回收，reset 释放剩余元数据。
```
