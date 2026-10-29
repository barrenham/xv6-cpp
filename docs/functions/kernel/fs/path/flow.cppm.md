# `kernel/fs/path/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::Xv6FileSystem::CreateFlow::allocate`](flow.cppm.md#fn-2d027c7cb1f8c5ba)
- [`xv::fs::Xv6FileSystem::CreateFlow::capacity`](flow.cppm.md#fn-ac02002f52d1a8b7)
- [`xv::fs::Xv6FileSystem::CreateFlow::configure`](flow.cppm.md#fn-0353d95b80c4db8a)
- [`xv::fs::Xv6FileSystem::CreateFlow::directory`](flow.cppm.md#fn-437f1e330e905821)
- [`xv::fs::Xv6FileSystem::CreateFlow::found`](flow.cppm.md#fn-a422328be2aae169)
- [`xv::fs::Xv6FileSystem::CreateFlow::lock_existing`](flow.cppm.md#fn-3c95e9e568333da6)
- [`xv::fs::Xv6FileSystem::CreateFlow::lookup`](flow.cppm.md#fn-63f62cb4b5c999fd)
- [`xv::fs::Xv6FileSystem::CreateFlow::named_entry`](flow.cppm.md#fn-d259695fd8ce032b)
- [`xv::fs::Xv6FileSystem::CreateFlow::parent_entry`](flow.cppm.md#fn-62a1a1d6e4a3ff0d)
- [`xv::fs::Xv6FileSystem::CreateFlow::parent_link`](flow.cppm.md#fn-fd2eb3d73854c1de)
- [`xv::fs::Xv6FileSystem::CreateFlow::permitted`](flow.cppm.md#fn-5ec9690f4274cd4c)
- [`xv::fs::Xv6FileSystem::CreateFlow::reject_existing`](flow.cppm.md#fn-07d6de6fc65b3da5)
- [`xv::fs::Xv6FileSystem::CreateFlow::reusable`](flow.cppm.md#fn-71647c821d3665bc)
- [`xv::fs::Xv6FileSystem::CreateFlow::reuse`](flow.cppm.md#fn-ad44571920e874e1)
- [`xv::fs::Xv6FileSystem::CreateFlow::rollback`](flow.cppm.md#fn-45646bd9d962f844)
- [`xv::fs::Xv6FileSystem::CreateFlow::self_entry`](flow.cppm.md#fn-ee1943bb6cea604c)
- [`xv::fs::Xv6FileSystem::CreateFlow::success`](flow.cppm.md#fn-24644c21618da0d0)
- [`xv::fs::Xv6FileSystem::CreateFlow::unlock_parent`](flow.cppm.md#fn-5fbd6edce0767ba7)
- [`xv::fs::Xv6FileSystem::LinkFlow::failed`](flow.cppm.md#fn-da8ab882e0008b78)
- [`xv::fs::Xv6FileSystem::LinkFlow::insert`](flow.cppm.md#fn-3dc1b385e467b0a1)
- [`xv::fs::Xv6FileSystem::LinkFlow::linkable`](flow.cppm.md#fn-2c44dd4f44d68dc9)
- [`xv::fs::Xv6FileSystem::LinkFlow::lock_target`](flow.cppm.md#fn-448b84e7ae4aaeb1)
- [`xv::fs::Xv6FileSystem::LinkFlow::reserve`](flow.cppm.md#fn-2b31070df24866d7)
- [`xv::fs::Xv6FileSystem::LinkFlow::rollback`](flow.cppm.md#fn-5b29fe45448841de)
- [`xv::fs::Xv6FileSystem::LinkFlow::same_owner`](flow.cppm.md#fn-831b53c2ae8c9e5b)
- [`xv::fs::Xv6FileSystem::LinkFlow::unlock_target`](flow.cppm.md#fn-904cc148222701fd)
- [`xv::fs::Xv6FileSystem::RenameFlow::aliases_parent`](flow.cppm.md#fn-326f3fb0974b172d)
- [`xv::fs::Xv6FileSystem::RenameFlow::clear_old`](flow.cppm.md#fn-f3b37b3e4035fbbd)
- [`xv::fs::Xv6FileSystem::RenameFlow::drop_target`](flow.cppm.md#fn-e30cc4a45c000be1)
- [`xv::fs::Xv6FileSystem::RenameFlow::identical`](flow.cppm.md#fn-ba2145d509356d97)
- [`xv::fs::Xv6FileSystem::RenameFlow::in_place`](flow.cppm.md#fn-fcbc87a9dafb69e8)
- [`xv::fs::Xv6FileSystem::RenameFlow::insert`](flow.cppm.md#fn-e826c6bd6aec6de9)
- [`xv::fs::Xv6FileSystem::RenameFlow::lock_entries`](flow.cppm.md#fn-0e9ae936240c5d72)
- [`xv::fs::Xv6FileSystem::RenameFlow::lock_parents`](flow.cppm.md#fn-124e073860e55865)
- [`xv::fs::Xv6FileSystem::RenameFlow::lookup`](flow.cppm.md#fn-7415ad8f31e60933)
- [`xv::fs::Xv6FileSystem::RenameFlow::prepare`](flow.cppm.md#fn-2d817178a3d1e7fe)
- [`xv::fs::Xv6FileSystem::RenameFlow::publish`](flow.cppm.md#fn-6774b99a469c1095)
- [`xv::fs::Xv6FileSystem::RenameFlow::release_entries`](flow.cppm.md#fn-47cc6b2d44134cda)
- [`xv::fs::Xv6FileSystem::RenameFlow::remove_old`](flow.cppm.md#fn-45f76fc900754084)
- [`xv::fs::Xv6FileSystem::RenameFlow::reparent`](flow.cppm.md#fn-fe7d00b2a8e27544)
- [`xv::fs::Xv6FileSystem::RenameFlow::reparenting`](flow.cppm.md#fn-72905cdc435cb5f2)
- [`xv::fs::Xv6FileSystem::RenameFlow::replace`](flow.cppm.md#fn-7e6f8f9f22807131)
- [`xv::fs::Xv6FileSystem::RenameFlow::replacing`](flow.cppm.md#fn-c555d3f0b73e2cca)
- [`xv::fs::Xv6FileSystem::RenameFlow::same_parent`](flow.cppm.md#fn-5fb9347b237bce30)
- [`xv::fs::Xv6FileSystem::RenameFlow::success`](flow.cppm.md#fn-96b7066148b4784f)
- [`xv::fs::Xv6FileSystem::RenameFlow::unlock_entries`](flow.cppm.md#fn-a5c0da035b5af75f)
- [`xv::fs::Xv6FileSystem::RenameFlow::unlock_parents`](flow.cppm.md#fn-9f3c15b1f1d07eba)
- [`xv::fs::Xv6FileSystem::RenameFlow::valid`](flow.cppm.md#fn-610d7ff7b950ee83)
- [`xv::fs::Xv6FileSystem::RenameFlow::validate`](flow.cppm.md#fn-349ae0ca9562835c)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::allowed_name`](flow.cppm.md#fn-ca6a03f8a9ea8fd6)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::clear`](flow.cppm.md#fn-7b069d5b932c80eb)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::directory`](flow.cppm.md#fn-f91f215f5540117c)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::drop_parent_link`](flow.cppm.md#fn-e3eb8cddff6cf83f)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::drop_target_link`](flow.cppm.md#fn-71a194f1883bd8c0)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::empty`](flow.cppm.md#fn-ddda5c4112a3eb5e)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::found`](flow.cppm.md#fn-e1e69c3f430d7442)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::invalid_links`](flow.cppm.md#fn-b08ceb7add96ba1d)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::linked`](flow.cppm.md#fn-5dbe81afd2f07a5c)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::lock_target`](flow.cppm.md#fn-224e2a2ed1c74c46)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::lookup`](flow.cppm.md#fn-263883c3d93c38a1)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::reject_target`](flow.cppm.md#fn-5d36f9523be31eb6)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::removable`](flow.cppm.md#fn-6b0af357ebfc87ab)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::unlock_parent`](flow.cppm.md#fn-bebe777736468d5a)
- [`xv::fs::Xv6FileSystem::UnlinkFlow::write_failed`](flow.cppm.md#fn-06ca428d54685fca)

<a id="fn-2d027c7cb1f8c5ba"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::allocate`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::allocate(Context &c) noexcept
@brief 为缺失名称分配新的磁盘 inode。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ac02002f52d1a8b7"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::capacity`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::capacity(Context &c) noexcept
@brief 检查新目录不会超过父目录链接数上限。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0353d95b80c4db8a"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::configure`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::configure(Context &c) noexcept
@brief 锁住候选并设置设备编号、一个链接和身份权限，随后更新磁盘 inode。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-437f1e330e905821"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::directory`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::directory(Context &c) noexcept
@brief 检查本次创建类型是否为目录。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a422328be2aae169"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::found`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::found(Context &c) noexcept
@brief 检查当前阶段是否取得既有或新分配节点。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3c95e9e568333da6"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::lock_existing`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::lock_existing(Context &c) noexcept
@brief 释放父目录锁后取得既有目标锁。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-63f62cb4b5c999fd"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::lookup`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::lookup(Context &c) noexcept
@brief 查找父目录内的既有名称。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d259695fd8ce032b"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::named_entry`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::named_entry(Context &c) noexcept
@brief 尝试在父目录安装最终名称。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-62a1a1d6e4a3ff0d"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::parent_entry`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::parent_entry(Context &c) noexcept
@brief 尝试为新目录安装点点记录。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fd2eb3d73854c1de"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::parent_link`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::parent_link(Context &c) noexcept
@brief 成功安装后增加父目录链接数并更新磁盘 inode。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5ec9690f4274cd4c"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::permitted`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::permitted(Context &c) noexcept
@brief 检查父目录存活、写搜索权限以及设备创建特权。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-07d6de6fc65b3da5"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::reject_existing`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::reject_existing(Context &c) noexcept
@brief 解锁归还不兼容的既有节点。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-71647c821d3665bc"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::reusable`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::reusable(Context &c) noexcept
@brief 检查普通文件请求可复用既有普通文件或设备。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ad44571920e874e1"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::reuse`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::reuse(Context &c) noexcept
@brief 保存带锁既有节点作为结果。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-45646bd9d962f844"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::rollback`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::rollback(Context &c) noexcept
@brief 将候选链接数归零、更新并解锁归还候选以触发回收。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ee1943bb6cea604c"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::self_entry`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::CreateFlow::self_entry(Context &c) noexcept
@brief 尝试为新目录安装点记录。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-24644c21618da0d0"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::success`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::success(Context &c) noexcept
@brief 释放父目录锁并保存仍带锁的候选结果。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5fbd6edce0767ba7"></a>

## `xv::fs::Xv6FileSystem::CreateFlow::unlock_parent`

```doxygen
@fn static void xv::fs::Xv6FileSystem::CreateFlow::unlock_parent(Context &c) noexcept
@brief 在失败路径释放父目录 inode 锁。
@details 入口已锁父目录且调用者处于事务；复用既有项先换锁到目标。新目录点和点点、父名称均安装成功后才增加父链接数，失败将候选置零链接并回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-da8ab882e0008b78"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::failed`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LinkFlow::failed(Context &c) noexcept
@brief 检查名称安装是否失败。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3dc1b385e467b0a1"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::insert`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LinkFlow::insert(Context &c) noexcept
@brief 在父目录锁内检查权限与设备并尝试插入名称，随后解锁。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2c44dd4f44d68dc9"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::linkable`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LinkFlow::linkable(Context &c) noexcept
@brief 检查目标非目录、链接数非零且未达上限。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-448b84e7ae4aaeb1"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::lock_target`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LinkFlow::lock_target(Context &c) noexcept
@brief 取得硬链接目标的 inode 锁。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2b31070df24866d7"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::reserve`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LinkFlow::reserve(Context &c) noexcept
@brief 增加目标链接数、更新磁盘 inode 并释放目标锁。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5b29fe45448841de"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::rollback`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LinkFlow::rollback(Context &c) noexcept
@brief 重新锁住目标，撤销先前增加的链接数并更新后解锁。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-831b53c2ae8c9e5b"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::same_owner`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LinkFlow::same_owner(Context &c) noexcept
@brief 按原次序验证目标及父目录均属于该磁盘后端。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-904cc148222701fd"></a>

## `xv::fs::Xv6FileSystem::LinkFlow::unlock_target`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LinkFlow::unlock_target(Context &c) noexcept
@brief 释放被拒绝目标的 inode 锁。
@details 调用者处于事务；先保留并更新目标链接数，再短暂锁住父目录安装名称，安装失败重新锁目标撤销增加的链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-326f3fb0974b172d"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::aliases_parent`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::aliases_parent(Context &c) noexcept
@brief 检查任一目录项是否就是两个父目录之一。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f3b37b3e4035fbbd"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::clear_old`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::clear_old(Context &c) noexcept
@brief 检查是否仍需清空原源目录槽。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e30cc4a45c000be1"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::drop_target`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::drop_target(Context &c) noexcept
@brief 减少被替换节点链接数并更新，目录目标还减少目标父链接数。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ba2145d509356d97"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::identical`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::identical(Context &c) noexcept
@brief 检查目标与源已经是同一 inode。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fcbc87a9dafb69e8"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::in_place`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::in_place(Context &c) noexcept
@brief 同父无目标时尝试原位改写源目录槽。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e826c6bd6aec6de9"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::insert`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::insert(Context &c) noexcept
@brief 跨父无目标时尝试插入新目录项。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0e9ae936240c5d72"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::lock_entries`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::lock_entries(Context &c) noexcept
@brief 先锁存在的源，再锁与源不同的目标。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-124e073860e55865"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::lock_parents`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::lock_parents(Context &c) noexcept
@brief 按已选择顺序锁住两个不同父目录。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7415ad8f31e60933"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::lookup`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::lookup(Context &c) noexcept
@brief 按源、目标次序取得目录项及其记录偏移。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2d817178a3d1e7fe"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::prepare`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::prepare(Context &c) noexcept
@brief 编码指向源 inode 的新名称目录记录。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6774b99a469c1095"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::publish`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::publish(Context &c) noexcept
@brief 更新源父目录以及必要时目标父目录的磁盘元数据。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-47cc6b2d44134cda"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::release_entries`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::release_entries(Context &c) noexcept
@brief 按源、目标次序归还各自取得的引用。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-45f76fc900754084"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::remove_old`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::remove_old(Context &c) noexcept
@brief 清空原源目录记录，失败按原策略诊断为致命错误。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fe7d00b2a8e27544"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::reparent`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::reparent(Context &c) noexcept
@brief 更新移动目录的点点记录及两个父目录链接数。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-72905cdc435cb5f2"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::reparenting`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::reparenting(Context &c) noexcept
@brief 检查源为跨父移动目录。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7e6f8f9f22807131"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::replace`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::replace(Context &c) noexcept
@brief 尝试将新记录写入既有目标目录槽。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c555d3f0b73e2cca"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::replacing`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::replacing(Context &c) noexcept
@brief 检查目标名称是否已存在。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5fb9347b237bce30"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::same_parent`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::same_parent(Context &c) noexcept
@brief 检查源与目标父目录相同。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-96b7066148b4784f"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::success`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::success(Context &c) noexcept
@brief 记录重命名成功码。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a5c0da035b5af75f"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::unlock_entries`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::unlock_entries(Context &c) noexcept
@brief 依次释放不同目标和源的 inode 锁。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9f3c15b1f1d07eba"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::unlock_parents`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::unlock_parents(Context &c) noexcept
@brief 按反向顺序释放不同的第二父目录和第一父目录锁。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-610d7ff7b950ee83"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::valid`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::RenameFlow::valid(Context &c) noexcept
@brief 检查本次重命名校验通过。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-349ae0ca9562835c"></a>

## `xv::fs::Xv6FileSystem::RenameFlow::validate`

```doxygen
@fn static void xv::fs::Xv6FileSystem::RenameFlow::validate(Context &c) noexcept
@brief 按原短路和位与复查顺序验证存活、权限、替换类型和链接数。
@details 调用者持 namespace 锁并处于事务；父目录按 inode 号顺序加锁，父别名拒绝路径不加项锁。所有路径按项锁、父锁、项引用次序清理，保留相同 inode 的两次引用归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ca6a03f8a9ea8fd6"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::allowed_name`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::allowed_name(Context &c) noexcept
@brief 检查名称不是点或点点分量。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7b069d5b932c80eb"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::clear`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::clear(Context &c) noexcept
@brief 写入零目录记录并报告是否完整成功。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f91f215f5540117c"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::directory`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::directory(Context &c) noexcept
@brief 检查目标是否为目录。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e3eb8cddff6cf83f"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::drop_parent_link`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::drop_parent_link(Context &c) noexcept
@brief 减少父目录链接数并更新磁盘 inode。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-71a194f1883bd8c0"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::drop_target_link`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::drop_target_link(Context &c) noexcept
@brief 减少目标链接数、更新并解锁归还引用，记录成功码。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ddda5c4112a3eb5e"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::empty`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::empty(Context &c) noexcept
@brief 检查目标非目录或目录内无普通项目。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e1e69c3f430d7442"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::found`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::found(Context &c) noexcept
@brief 检查目标是否存在。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b08ceb7add96ba1d"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::invalid_links`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::invalid_links(Context &) noexcept
@brief 诊断待删除目标的链接计数无效。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5dbe81afd2f07a5c"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::linked`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::linked(Context &c) noexcept
@brief 检查目标链接数至少为一。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-224e2a2ed1c74c46"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::lock_target`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::lock_target(Context &c) noexcept
@brief 取得目标 inode 锁。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-263883c3d93c38a1"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::lookup`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::lookup(Context &c) noexcept
@brief 查找目标并保存目录记录偏移。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5d36f9523be31eb6"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::reject_target`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::reject_target(Context &c) noexcept
@brief 在拒绝路径解锁归还目标引用。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6b0af357ebfc87ab"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::removable`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::UnlinkFlow::removable(Context &c) noexcept
@brief 检查当前身份的删除权限。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-bebe777736468d5a"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::unlock_parent`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::unlock_parent(Context &c) noexcept
@brief 释放父目录 inode 锁。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-06ca428d54685fca"></a>

## `xv::fs::Xv6FileSystem::UnlinkFlow::write_failed`

```doxygen
@fn static void xv::fs::Xv6FileSystem::UnlinkFlow::write_failed(Context &) noexcept
@brief 诊断清除目录记录时写入失败。
@details 入口已锁父目录且调用者处于事务；拒绝路径先归还带锁目标再解父目录，成功先清除名称并更新父目录，之后减少目标链接数并归还引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
