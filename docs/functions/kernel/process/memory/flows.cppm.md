# `kernel/process/memory/flows.cppm`

所属流程和资源协议见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::process::AddressSpace::CopyFlow::clone_pages`](flows.cppm.md#fn-705d99a8fc5ee0e4)
- [`xv::process::AddressSpace::CopyFlow::commit`](flows.cppm.md#fn-45abcb9d265c54dc)
- [`xv::process::AddressSpace::CopyFlow::copy_regions`](flows.cppm.md#fn-3d43055c077cb95b)
- [`xv::process::AddressSpace::CreateFlow::allocate_table`](flows.cppm.md#fn-7bb91a369e7b15c0)
- [`xv::process::AddressSpace::CreateFlow::commit`](flows.cppm.md#fn-851b942142fad62a)
- [`xv::process::AddressSpace::CreateFlow::describe`](flows.cppm.md#fn-9e63b8dd1a78c6ed)
- [`xv::process::AddressSpace::CreateFlow::free_table`](flows.cppm.md#fn-0999f291f1f67da4)
- [`xv::process::AddressSpace::CreateFlow::map_frame`](flows.cppm.md#fn-9db1235fabcd4fac)
- [`xv::process::AddressSpace::CreateFlow::map_info`](flows.cppm.md#fn-55ea82a749f93078)
- [`xv::process::AddressSpace::CreateFlow::map_trampoline`](flows.cppm.md#fn-9dc4c483558f38b1)
- [`xv::process::AddressSpace::CreateFlow::reset_owner`](flows.cppm.md#fn-85e932a060437e65)
- [`xv::process::AddressSpace::CreateFlow::unmap_frame`](flows.cppm.md#fn-57e6189b0c7e46e4)
- [`xv::process::AddressSpace::CreateFlow::unmap_trampoline`](flows.cppm.md#fn-7011ee43f36828a0)
- [`xv::process::AddressSpace::GrowFlow::allocate`](flows.cppm.md#fn-d6c4f4c19c6cc58b)
- [`xv::process::AddressSpace::GrowFlow::commit`](flows.cppm.md#fn-a35d52828b556a21)
- [`xv::process::AddressSpace::GrowFlow::plan`](flows.cppm.md#fn-5866121f83d68623)
- [`xv::process::AddressSpace::GrowFlow::reserve`](flows.cppm.md#fn-4ae965b7e39ee7cc)
- [`xv::process::AddressSpace::GrowFlow::undo_reservation`](flows.cppm.md#fn-c3aa67a79e90f584)
- [`xv::process::AddressSpace::GuardFlow::protect`](flows.cppm.md#fn-88fbad6bb590d0c5)
- [`xv::process::AddressSpace::ResizeFlow::allocate`](flows.cppm.md#fn-6f8343cc5490a7cd)
- [`xv::process::AddressSpace::ResizeFlow::commit`](flows.cppm.md#fn-0668f715a959353a)
- [`xv::process::AddressSpace::ResizeFlow::deallocate`](flows.cppm.md#fn-62e1550346848e62)
- [`xv::process::AddressSpace::ResizeFlow::erase`](flows.cppm.md#fn-527c65a17d205ee6)
- [`xv::process::AddressSpace::ResizeFlow::negative`](flows.cppm.md#fn-5d1bd6fb8ecf01ff)
- [`xv::process::AddressSpace::ResizeFlow::plan_growth`](flows.cppm.md#fn-a468e4adc1a3ebec)
- [`xv::process::AddressSpace::ResizeFlow::plan_shrink`](flows.cppm.md#fn-0c5c71440f9c7133)
- [`xv::process::AddressSpace::ResizeFlow::positive`](flows.cppm.md#fn-052210f9e7698548)
- [`xv::process::AddressSpace::ResizeFlow::reserve`](flows.cppm.md#fn-33026201fd20c8df)
- [`xv::process::AddressSpace::ResizeFlow::undo_reservation`](flows.cppm.md#fn-1bff6b59bd6d45ee)

<a id="fn-705d99a8fc5ee0e4"></a>

## `xv::process::AddressSpace::CopyFlow::clone_pages`

```doxygen
@fn static bool xv::process::AddressSpace::CopyFlow::clone_pages(Context &c) noexcept
@brief 克隆用户页表并建立写时复制共享。
@details 区域复制先于 COW 页表克隆；保留原有失败清理责任，由调用者处理未成功的子地址空间。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-45abcb9d265c54dc"></a>

## `xv::process::AddressSpace::CopyFlow::commit`

```doxygen
@fn static void xv::process::AddressSpace::CopyFlow::commit(Context &c) noexcept
@brief 提交子地址空间大小和堆基址并发布观察值。
@details 区域复制先于 COW 页表克隆；保留原有失败清理责任，由调用者处理未成功的子地址空间。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-3d43055c077cb95b"></a>

## `xv::process::AddressSpace::CopyFlow::copy_regions`

```doxygen
@fn static bool xv::process::AddressSpace::CopyFlow::copy_regions(Context &c) noexcept
@brief 将虚拟区域描述复制到子地址空间。
@details 区域复制先于 COW 页表克隆；保留原有失败清理责任，由调用者处理未成功的子地址空间。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7bb91a369e7b15c0"></a>

## `xv::process::AddressSpace::CreateFlow::allocate_table`

```doxygen
@fn static bool xv::process::AddressSpace::CreateFlow::allocate_table(Context &c) noexcept
@brief 创建空用户页表。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-851b942142fad62a"></a>

## `xv::process::AddressSpace::CreateFlow::commit`

```doxygen
@fn static void xv::process::AddressSpace::CreateFlow::commit(Context &c) noexcept
@brief 提交零用户大小并以 release 发布观察值。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9e63b8dd1a78c6ed"></a>

## `xv::process::AddressSpace::CreateFlow::describe`

```doxygen
@fn static bool xv::process::AddressSpace::CreateFlow::describe(Context &c) noexcept
@brief 将页表交给 owner 并登记所有特殊虚拟区域。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```

<a id="fn-0999f291f1f67da4"></a>

## `xv::process::AddressSpace::CreateFlow::free_table`

```doxygen
@fn static void xv::process::AddressSpace::CreateFlow::free_table(Context &c) noexcept
@brief 释放尚未交给 owner 的候选页表。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9db1235fabcd4fac"></a>

## `xv::process::AddressSpace::CreateFlow::map_frame`

```doxygen
@fn static bool xv::process::AddressSpace::CreateFlow::map_frame(Context &c) noexcept
@brief 映射借用 trapframe 的读写页。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-55ea82a749f93078"></a>

## `xv::process::AddressSpace::CreateFlow::map_info`

```doxygen
@fn static bool xv::process::AddressSpace::CreateFlow::map_info(Context &c) noexcept
@brief 在 LAB_PGTBL 配置下映射借用用户信息页，否则直接成功。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-9dc4c483558f38b1"></a>

## `xv::process::AddressSpace::CreateFlow::map_trampoline`

```doxygen
@fn static bool xv::process::AddressSpace::CreateFlow::map_trampoline(Context &c) noexcept
@brief 将借用 trampoline 映射为 supervisor 只读可执行页。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-85e932a060437e65"></a>

## `xv::process::AddressSpace::CreateFlow::reset_owner`

```doxygen
@fn static void xv::process::AddressSpace::CreateFlow::reset_owner(Context &c) noexcept
@brief 通过 owner.reset 回收已接管页表和区域元数据。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-57e6189b0c7e46e4"></a>

## `xv::process::AddressSpace::CreateFlow::unmap_frame`

```doxygen
@fn static void xv::process::AddressSpace::CreateFlow::unmap_frame(Context &c) noexcept
@brief 解除 trapframe 借用映射而不释放页面。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-7011ee43f36828a0"></a>

## `xv::process::AddressSpace::CreateFlow::unmap_trampoline`

```doxygen
@fn static void xv::process::AddressSpace::CreateFlow::unmap_trampoline(Context &c) noexcept
@brief 解除 trampoline 借用映射而不释放页面。
@details 页表拥有新分配的页表页；trampoline、trapframe 和用户信息页仅被借用，回滚解除映射但不释放借用页面。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-d6c4f4c19c6cc58b"></a>

## `xv::process::AddressSpace::GrowFlow::allocate`

```doxygen
@fn static bool xv::process::AddressSpace::GrowFlow::allocate(Context &c) noexcept
@brief 分配用户映射并保存实际新大小。
@details 候选的 Image 或 UserStack 区域先预留后映射，映射失败撤销新增预留；成功同时推进堆基址。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a35d52828b556a21"></a>

## `xv::process::AddressSpace::GrowFlow::commit`

```doxygen
@fn static void xv::process::AddressSpace::GrowFlow::commit(Context &c) noexcept
@brief 提交大小和堆基址并以 release 发布观察值。
@details 候选的 Image 或 UserStack 区域先预留后映射，映射失败撤销新增预留；成功同时推进堆基址。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5866121f83d68623"></a>

## `xv::process::AddressSpace::GrowFlow::plan`

```doxygen
@fn static bool xv::process::AddressSpace::GrowFlow::plan(Context &c) noexcept
@brief 验证目标上界并计算页对齐范围。
@details 候选的 Image 或 UserStack 区域先预留后映射，映射失败撤销新增预留；成功同时推进堆基址。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-4ae965b7e39ee7cc"></a>

## `xv::process::AddressSpace::GrowFlow::reserve`

```doxygen
@fn static bool xv::process::AddressSpace::GrowFlow::reserve(Context &c) noexcept
@brief 预留新增映像或用户栈区域。
@details 候选的 Image 或 UserStack 区域先预留后映射，映射失败撤销新增预留；成功同时推进堆基址。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-c3aa67a79e90f584"></a>

## `xv::process::AddressSpace::GrowFlow::undo_reservation`

```doxygen
@fn static void xv::process::AddressSpace::GrowFlow::undo_reservation(Context &c) noexcept
@brief 撤销映射失败对应的新增区域预留。
@details 候选的 Image 或 UserStack 区域先预留后映射，映射失败撤销新增预留；成功同时推进堆基址。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-88fbad6bb590d0c5"></a>

## `xv::process::AddressSpace::GuardFlow::protect`

```doxygen
@fn static void xv::process::AddressSpace::GuardFlow::protect(Context &c) noexcept
@brief 撤销该页的用户访问权限并记录成功。
@details 保留先删除旧区域再预留保护区的原顺序，预留失败时不补充额外回滚。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-6f8343cc5490a7cd"></a>

## `xv::process::AddressSpace::ResizeFlow::allocate`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::allocate(Context &c) noexcept
@brief 按 eager 策略分配新增物理映射。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0668f715a959353a"></a>

## `xv::process::AddressSpace::ResizeFlow::commit`

```doxygen
@fn static void xv::process::AddressSpace::ResizeFlow::commit(Context &c) noexcept
@brief 提交新大小并以 release 发布观察值，记录零返回码。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-62e1550346848e62"></a>

## `xv::process::AddressSpace::ResizeFlow::deallocate`

```doxygen
@fn static void xv::process::AddressSpace::ResizeFlow::deallocate(Context &c) noexcept
@brief 回收被缩小掉的用户映射。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-527c65a17d205ee6"></a>

## `xv::process::AddressSpace::ResizeFlow::erase`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::erase(Context &c) noexcept
@brief 删除需要收缩的区域范围。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5d1bd6fb8ecf01ff"></a>

## `xv::process::AddressSpace::ResizeFlow::negative`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::negative(Context &c) noexcept
@brief 检查是否请求缩小。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-a468e4adc1a3ebec"></a>

## `xv::process::AddressSpace::ResizeFlow::plan_growth`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::plan_growth(Context &c) noexcept
@brief 检查用户上界并计算增长后的大小和页边界。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-0c5c71440f9c7133"></a>

## `xv::process::AddressSpace::ResizeFlow::plan_shrink`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::plan_shrink(Context &c) noexcept
@brief 验证收缩没有越过当前大小或映像堆基址并计算页边界。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-052210f9e7698548"></a>

## `xv::process::AddressSpace::ResizeFlow::positive`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::positive(Context &c) noexcept
@brief 检查是否请求增长。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-33026201fd20c8df"></a>

## `xv::process::AddressSpace::ResizeFlow::reserve`

```doxygen
@fn static bool xv::process::AddressSpace::ResizeFlow::reserve(Context &c) noexcept
@brief 预留新增的按需堆区域。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-1bff6b59bd6d45ee"></a>

## `xv::process::AddressSpace::ResizeFlow::undo_reservation`

```doxygen
@fn static void xv::process::AddressSpace::ResizeFlow::undo_reservation(Context &c) noexcept
@brief 撤销分配失败对应的新增区域预留。
@details 区域修改和物理映射顺序与原实现一致；提交前失败保留业务错误，按原约定撤销新增预留。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-5fec7d89314b86e0"></a>

## `xv::process::AddressSpace::GuardFlow::describe`

```doxygen
@fn static bool xv::process::AddressSpace::GuardFlow::describe(Context &c) noexcept
@brief 原子修改保护页区域属性，失败时保留原记录，随后才撤销页表用户访问权限。
@details 统一地址池记录地址占用及区域属性；修改由调用者串行，映射与物理页由页表及分配器负责。
```
