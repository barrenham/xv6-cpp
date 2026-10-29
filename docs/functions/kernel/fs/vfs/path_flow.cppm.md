# `kernel/fs/vfs/path_flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::VirtualFileSystem::AncestorFlow::absent`](path_flow.cppm.md#fn-de717d09c2cb934b)
- [`xv::fs::VirtualFileSystem::AncestorFlow::advance`](path_flow.cppm.md#fn-c7f55d40713f1dd0)
- [`xv::fs::VirtualFileSystem::AncestorFlow::ascend`](path_flow.cppm.md#fn-ff4380a39ae7718e)
- [`xv::fs::VirtualFileSystem::AncestorFlow::ended`](path_flow.cppm.md#fn-51d2a1ebcf36083a)
- [`xv::fs::VirtualFileSystem::AncestorFlow::matches`](path_flow.cppm.md#fn-08de0a3762f56c88)
- [`xv::fs::VirtualFileSystem::AncestorFlow::present`](path_flow.cppm.md#fn-067bba26b56dfbb6)
- [`xv::fs::VirtualFileSystem::AncestorFlow::remaining`](path_flow.cppm.md#fn-f61f1ebb0c6af0b0)
- [`xv::fs::VirtualFileSystem::AncestorFlow::retain`](path_flow.cppm.md#fn-8247153198e9cff9)
- [`xv::fs::VirtualFileSystem::CreateFlow::create`](path_flow.cppm.md#fn-db66d82168ed3a92)
- [`xv::fs::VirtualFileSystem::CreateFlow::find_parent`](path_flow.cppm.md#fn-82e660e3ddbad7b9)
- [`xv::fs::VirtualFileSystem::CreateFlow::found`](path_flow.cppm.md#fn-c86632b9ef7a2af1)
- [`xv::fs::VirtualFileSystem::CreateFlow::lock`](path_flow.cppm.md#fn-446dd95551237fe2)
- [`xv::fs::VirtualFileSystem::CreateFlow::lookup`](path_flow.cppm.md#fn-cd335f4ce109df07)
- [`xv::fs::VirtualFileSystem::CreateFlow::open_regular`](path_flow.cppm.md#fn-b2c8deaad23af908)
- [`xv::fs::VirtualFileSystem::CreateFlow::openable`](path_flow.cppm.md#fn-65a032a022238b8e)
- [`xv::fs::VirtualFileSystem::CreateFlow::parent_found`](path_flow.cppm.md#fn-732e475a883c61d5)
- [`xv::fs::VirtualFileSystem::CreateFlow::regular`](path_flow.cppm.md#fn-b5970ff4c87f4166)
- [`xv::fs::VirtualFileSystem::CreateFlow::reject`](path_flow.cppm.md#fn-0a5466deca8133bc)
- [`xv::fs::VirtualFileSystem::CreateFlow::release_parent`](path_flow.cppm.md#fn-fd9f8ee432bb64bc)
- [`xv::fs::VirtualFileSystem::LinkFlow::found`](path_flow.cppm.md#fn-46679c64f18cc915)
- [`xv::fs::VirtualFileSystem::LinkFlow::link`](path_flow.cppm.md#fn-a0aa2b7b237fba5c)
- [`xv::fs::VirtualFileSystem::LinkFlow::lookup`](path_flow.cppm.md#fn-89e7af3b0a9c9125)
- [`xv::fs::VirtualFileSystem::LinkFlow::parent`](path_flow.cppm.md#fn-a346fb7b68eef440)
- [`xv::fs::VirtualFileSystem::LinkFlow::parent_found`](path_flow.cppm.md#fn-762ec9d33df971eb)
- [`xv::fs::VirtualFileSystem::LinkFlow::release_parent`](path_flow.cppm.md#fn-56f2c172895f3b5c)
- [`xv::fs::VirtualFileSystem::LinkFlow::release_target`](path_flow.cppm.md#fn-efa9bd945913f500)
- [`xv::fs::VirtualFileSystem::RenameFlow::dot`](path_flow.cppm.md#fn-62c2365a9f7e9983)
- [`xv::fs::VirtualFileSystem::RenameFlow::eligible`](path_flow.cppm.md#fn-118dfac1e9816692)
- [`xv::fs::VirtualFileSystem::RenameFlow::entries`](path_flow.cppm.md#fn-2cbf45b640ede2e5)
- [`xv::fs::VirtualFileSystem::RenameFlow::parents`](path_flow.cppm.md#fn-5221b678c2bdab6e)
- [`xv::fs::VirtualFileSystem::RenameFlow::protect_descendants`](path_flow.cppm.md#fn-b20119838e8e3064)
- [`xv::fs::VirtualFileSystem::RenameFlow::protect_mounts`](path_flow.cppm.md#fn-49c29a04616db82e)
- [`xv::fs::VirtualFileSystem::RenameFlow::release_entries`](path_flow.cppm.md#fn-cc7de7d29df6d624)
- [`xv::fs::VirtualFileSystem::RenameFlow::release_parents`](path_flow.cppm.md#fn-4a4de714cec103b8)
- [`xv::fs::VirtualFileSystem::RenameFlow::rename`](path_flow.cppm.md#fn-73af78694d710d13)
- [`xv::fs::VirtualFileSystem::RenameFlow::valid`](path_flow.cppm.md#fn-c5682d8b7b60d181)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::create`](path_flow.cppm.md#fn-263582eb6c6e7d3e)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::created`](path_flow.cppm.md#fn-444e14e05a44e162)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::failed`](path_flow.cppm.md#fn-efc0535c0d22d6b2)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::measure`](path_flow.cppm.md#fn-ad001b797e6cc7e4)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::parent`](path_flow.cppm.md#fn-17c8cd0f914b9f8d)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::parent_found`](path_flow.cppm.md#fn-363e1d690d6bc4b2)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::release`](path_flow.cppm.md#fn-7a1ab7f2cab951cf)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::rollback`](path_flow.cppm.md#fn-b3d275eefd5747ad)
- [`xv::fs::VirtualFileSystem::SymlinkFlow::write`](path_flow.cppm.md#fn-23ac7f677278169d)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::directory`](path_flow.cppm.md#fn-862064e9a4bf42fc)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::inspect`](path_flow.cppm.md#fn-993360665f6f658a)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::lock`](path_flow.cppm.md#fn-0c1c530df0739ccc)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::mounted`](path_flow.cppm.md#fn-50217230090ae324)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::unlink`](path_flow.cppm.md#fn-7c76d276c499ca2e)
- [`xv::fs::VirtualFileSystem::UnlinkAtFlow::unlock`](path_flow.cppm.md#fn-92fbcaec2c6e1011)
- [`xv::fs::VirtualFileSystem::UnlinkFlow::found`](path_flow.cppm.md#fn-07f8154f1bc9b3cf)
- [`xv::fs::VirtualFileSystem::UnlinkFlow::parent`](path_flow.cppm.md#fn-5a84ed6fada6cc5c)
- [`xv::fs::VirtualFileSystem::UnlinkFlow::release`](path_flow.cppm.md#fn-575db944fcafa026)
- [`xv::fs::VirtualFileSystem::UnlinkFlow::unlink`](path_flow.cppm.md#fn-23b817c3b1f8b6f6)
- [`xv::fs::VirtualFileSystem::WalkFlow::absolute`](path_flow.cppm.md#fn-d29384e99e2f5883)
- [`xv::fs::VirtualFileSystem::WalkFlow::advance`](path_flow.cppm.md#fn-4666f14fcf13b1e6)
- [`xv::fs::VirtualFileSystem::WalkFlow::check_directory`](path_flow.cppm.md#fn-0d7dbf272af91ab7)
- [`xv::fs::VirtualFileSystem::WalkFlow::complete`](path_flow.cppm.md#fn-d7a9c438b2c09073)
- [`xv::fs::VirtualFileSystem::WalkFlow::component`](path_flow.cppm.md#fn-411a67aea1cccbaf)
- [`xv::fs::VirtualFileSystem::WalkFlow::copy_path`](path_flow.cppm.md#fn-e7482d5e5234464a)
- [`xv::fs::VirtualFileSystem::WalkFlow::create`](path_flow.cppm.md#fn-d4aaf2fa5200aadd)
- [`xv::fs::VirtualFileSystem::WalkFlow::create_last`](path_flow.cppm.md#fn-4c00218086a8d1a7)
- [`xv::fs::VirtualFileSystem::WalkFlow::find_next`](path_flow.cppm.md#fn-c1e6b0485d9e3784)
- [`xv::fs::VirtualFileSystem::WalkFlow::found`](path_flow.cppm.md#fn-fa3ab5607601ce73)
- [`xv::fs::VirtualFileSystem::WalkFlow::lock_next`](path_flow.cppm.md#fn-fb84739d4e5af2bd)
- [`xv::fs::VirtualFileSystem::WalkFlow::more`](path_flow.cppm.md#fn-17d1ef30848312ad)
- [`xv::fs::VirtualFileSystem::WalkFlow::parent_component`](path_flow.cppm.md#fn-a12788353fda2c15)
- [`xv::fs::VirtualFileSystem::WalkFlow::read_target`](path_flow.cppm.md#fn-51c8d6974132cde4)
- [`xv::fs::VirtualFileSystem::WalkFlow::release_current`](path_flow.cppm.md#fn-a92bfb799f45db2b)
- [`xv::fs::VirtualFileSystem::WalkFlow::restart_root`](path_flow.cppm.md#fn-e4648c12bcd278b4)
- [`xv::fs::VirtualFileSystem::WalkFlow::return_parent`](path_flow.cppm.md#fn-c6cdb66a35d3874a)
- [`xv::fs::VirtualFileSystem::WalkFlow::rewrite`](path_flow.cppm.md#fn-fb1b3b4cd1da5e7b)
- [`xv::fs::VirtualFileSystem::WalkFlow::searchable`](path_flow.cppm.md#fn-7cda0753479080cd)
- [`xv::fs::VirtualFileSystem::WalkFlow::skip_slashes`](path_flow.cppm.md#fn-084383eddf2c0379)
- [`xv::fs::VirtualFileSystem::WalkFlow::start`](path_flow.cppm.md#fn-e182420f740298c0)
- [`xv::fs::VirtualFileSystem::WalkFlow::started`](path_flow.cppm.md#fn-0945d9cfeddd0671)
- [`xv::fs::VirtualFileSystem::WalkFlow::symbolic`](path_flow.cppm.md#fn-5f201aa1c14c1c31)
- [`xv::fs::VirtualFileSystem::WalkFlow::trailing`](path_flow.cppm.md#fn-e745b2e8d9ce89dd)
- [`xv::fs::VirtualFileSystem::WalkFlow::unlock_current`](path_flow.cppm.md#fn-caa68230be2609e2)
- [`xv::fs::VirtualFileSystem::WalkFlow::valid`](path_flow.cppm.md#fn-c6381b533580b6d0)
- [`xv::fs::VirtualFileSystem::WalkFlow::wants_parent`](path_flow.cppm.md#fn-e96b2fb3309f31d8)

<a id="fn-de717d09c2cb934b"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::absent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::AncestorFlow::absent(Context &c) noexcept
@brief 归还剩余父目录引用，保留假结果。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c7f55d40713f1dd0"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::advance`

```doxygen
@fn static void xv::fs::VirtualFileSystem::AncestorFlow::advance(Context &c) noexcept
@brief 将父目录作为下一节点并递增搜索深度。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ff4380a39ae7718e"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::ascend`

```doxygen
@fn static void xv::fs::VirtualFileSystem::AncestorFlow::ascend(Context &c) noexcept
@brief 解析挂载父目录或普通点点目录项，判断根边界并释放旧节点。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-51d2a1ebcf36083a"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::ended`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::AncestorFlow::ended(Context &c) noexcept
@brief 检查父目录不存在或已到达自身父目录。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-08de0a3762f56c88"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::matches`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::AncestorFlow::matches(Context &c) noexcept
@brief 检查当前节点是否就是可能的祖先。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-067bba26b56dfbb6"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::present`

```doxygen
@fn static void xv::fs::VirtualFileSystem::AncestorFlow::present(Context &c) noexcept
@brief 归还当前节点引用并记录真结果，包括深度耗尽的保守结果。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f61f1ebb0c6af0b0"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::remaining`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::AncestorFlow::remaining(Context &c) noexcept
@brief 检查祖先搜索是否尚未达到一百二十八层。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8247153198e9cff9"></a>

## `xv::fs::VirtualFileSystem::AncestorFlow::retain`

```doxygen
@fn static void xv::fs::VirtualFileSystem::AncestorFlow::retain(Context &c) noexcept
@brief 取得待检查节点的额外引用。
@details 每轮释放旧节点引用后推进到父目录；挂载根向挂载父目录跳转，一百二十八层耗尽时按原策略保守返回真。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-db66d82168ed3a92"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::create`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::create(Context &c) noexcept
@brief 委托原 create_at 创建指定类型并保存带锁结果。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-82e660e3ddbad7b9"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::find_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::find_parent(Context &c) noexcept
@brief 通过原公开入口取得父目录引用。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c86632b9ef7a2af1"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::CreateFlow::found(Context &c) noexcept
@brief 检查普通文件解析是否成功。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-446dd95551237fe2"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::lock`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::lock(Context &c) noexcept
@brief 锁住已解析的结果节点。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cd335f4ce109df07"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::lookup`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::lookup(Context &c) noexcept
@brief 解析允许创建末尾文件的完整路径并保存节点。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b2c8deaad23af908"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::open_regular`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::open_regular(Context &c) noexcept
@brief 在局部 namespace 锁作用域内执行普通文件解析校验图。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-65a032a022238b8e"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::openable`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::CreateFlow::openable(Context &c) noexcept
@brief 检查结果为普通文件或设备。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-732e475a883c61d5"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::parent_found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::CreateFlow::parent_found(Context &c) noexcept
@brief 检查是否解析到父目录。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b5970ff4c87f4166"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::regular`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::CreateFlow::regular(Context &c) noexcept
@brief 检查是否请求普通文件创建或打开。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0a5466deca8133bc"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::reject`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::reject(Context &c) noexcept
@brief 解锁归还不允许打开的节点并清空结果。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fd9f8ee432bb64bc"></a>

## `xv::fs::VirtualFileSystem::CreateFlow::release_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::CreateFlow::release_parent(Context &c) noexcept
@brief 归还父目录引用。
@details 普通文件路径的 namespace ScopedLock 活在 open_regular 动作中，内部 RegularGraph 返回后解锁；成功保留结果 inode 锁。其他类型沿原 lookup_parent/create_at 锁范围执行。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-46679c64f18cc915"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::LinkFlow::found(Context &c) noexcept
@brief 检查目标是否存在。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a0aa2b7b237fba5c"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::link`

```doxygen
@fn static void xv::fs::VirtualFileSystem::LinkFlow::link(Context &c) noexcept
@brief 委托 link_at 并保存原业务返回值。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-89e7af3b0a9c9125"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::lookup`

```doxygen
@fn static void xv::fs::VirtualFileSystem::LinkFlow::lookup(Context &c) noexcept
@brief 按不跟随末尾链接的策略解析硬链接目标。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a346fb7b68eef440"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::LinkFlow::parent(Context &c) noexcept
@brief 解析新目录项的父目录。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-762ec9d33df971eb"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::parent_found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::LinkFlow::parent_found(Context &c) noexcept
@brief 检查新路径父目录是否存在。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-56f2c172895f3b5c"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::release_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::LinkFlow::release_parent(Context &c) noexcept
@brief 归还新路径父目录引用。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-efa9bd945913f500"></a>

## `xv::fs::VirtualFileSystem::LinkFlow::release_target`

```doxygen
@fn static void xv::fs::VirtualFileSystem::LinkFlow::release_target(Context &c) noexcept
@brief 归还硬链接目标引用。
@details Transaction 留在入口作用域；目标不跟随符号链接，父目录解析失败也必须归还已取得的目标引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-62c2365a9f7e9983"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::dot`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::RenameFlow::dot(const char *name) noexcept
@brief 检查名称是否为点或点点保留分量。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-118dfac1e9816692"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::eligible`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::RenameFlow::eligible(Context &c) noexcept
@brief 检查两个父目录存在、后端相同且两名称均非保留分量。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2cbf45b640ede2e5"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::entries`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::entries(Context &c) noexcept
@brief 分别短暂锁住旧父目录和新父目录取得源与目标引用。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5221b678c2bdab6e"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::parents`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::parents(Context &c) noexcept
@brief 按旧路径、新路径次序分别解析父目录。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b20119838e8e3064"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::protect_descendants`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::protect_descendants(Context &c) noexcept
@brief 对目录源检查目标父目录不是其后代。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-49c29a04616db82e"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::protect_mounts`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::protect_mounts(Context &c) noexcept
@brief 按原短路顺序拒绝挂载点及其祖先的移动或替换。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cc7de7d29df6d624"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::release_entries`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::release_entries(Context &c) noexcept
@brief 依次归还源与目标项引用。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4a4de714cec103b8"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::release_parents`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::release_parents(Context &c) noexcept
@brief 依次归还旧父目录和新父目录引用。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-73af78694d710d13"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::rename`

```doxygen
@fn static void xv::fs::VirtualFileSystem::RenameFlow::rename(Context &c) noexcept
@brief 委托同一后端执行 rename_at 并保存结果。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c5682d8b7b60d181"></a>

## `xv::fs::VirtualFileSystem::RenameFlow::valid`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::RenameFlow::valid(Context &c) noexcept
@brief 检查挂载与后代约束是否全部通过。
@details Transaction 与 namespace ScopedLock 留在入口；即使旧父目录解析失败也解析新父目录，完成后分别归还所有成功取得的引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-263582eb6c6e7d3e"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::create`

```doxygen
@fn static void xv::fs::VirtualFileSystem::SymlinkFlow::create(Context &c) noexcept
@brief 创建带锁符号链接节点。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-444e14e05a44e162"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::created`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::SymlinkFlow::created(Context &c) noexcept
@brief 检查节点创建是否成功。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-efc0535c0d22d6b2"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::failed`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::SymlinkFlow::failed(Context &c) noexcept
@brief 检查链接内容是否未完整写入。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ad001b797e6cc7e4"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::measure`

```doxygen
@fn static unsigned xv::fs::VirtualFileSystem::SymlinkFlow::measure(const char *target) noexcept
@brief 有界测量非空链接目标长度，空串或达到一百二十八字节返回零。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-17c8cd0f914b9f8d"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::SymlinkFlow::parent(Context &c) noexcept
@brief 在已持 namespace 锁下解析新链接的父目录。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-363e1d690d6bc4b2"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::parent_found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::SymlinkFlow::parent_found(Context &c) noexcept
@brief 检查父目录是否存在。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7a1ab7f2cab951cf"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::release`

```doxygen
@fn static void xv::fs::VirtualFileSystem::SymlinkFlow::release(Context &c) noexcept
@brief 归还父目录引用。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b3d275eefd5747ad"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::rollback`

```doxygen
@fn static void xv::fs::VirtualFileSystem::SymlinkFlow::rollback(Context &c) noexcept
@brief 按原策略尝试删除写入失败的新目录项。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-23ac7f677278169d"></a>

## `xv::fs::VirtualFileSystem::SymlinkFlow::write`

```doxygen
@fn static void xv::fs::VirtualFileSystem::SymlinkFlow::write(Context &c) noexcept
@brief 写入完整目标字符串并保存成功码，随后解锁归还新节点。
@details 目标长度检查在入口获取 Transaction 与 namespace 锁前完成；写入失败先解锁归还新节点，再撤销新目录项，最后归还父目录。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-862064e9a4bf42fc"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::directory`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnlinkAtFlow::directory(Context &c) noexcept
@brief 检查节点为目录。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-993360665f6f658a"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::inspect`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkAtFlow::inspect(Context &c) noexcept
@brief 取得目录项引用，按原顺序扫描挂载覆盖点和根，再归还目标引用。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0c1c530df0739ccc"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::lock`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkAtFlow::lock(Context &c) noexcept
@brief 锁住待检查目录。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-50217230090ae324"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::mounted`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnlinkAtFlow::mounted(Context &c) noexcept
@brief 检查目标是否受挂载记录保护。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7c76d276c499ca2e"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::unlink`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkAtFlow::unlink(Context &c) noexcept
@brief 委托具体文件系统删除目录项并保存返回值。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-92fbcaec2c6e1011"></a>

## `xv::fs::VirtualFileSystem::UnlinkAtFlow::unlock`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkAtFlow::unlock(Context &c) noexcept
@brief 释放目录锁。
@details namespace 锁由入口 ScopedLock 持有；目录项检查期间持 inode 锁，调用具体 unlink_at 前释放该锁，挂载节点不允许删除。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-07f8154f1bc9b3cf"></a>

## `xv::fs::VirtualFileSystem::UnlinkFlow::found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnlinkFlow::found(Context &c) noexcept
@brief 检查父目录是否存在。
@details Transaction 留在入口作用域，成功取得的父目录在委托结束后统一归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5a84ed6fada6cc5c"></a>

## `xv::fs::VirtualFileSystem::UnlinkFlow::parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkFlow::parent(Context &c) noexcept
@brief 解析待删除项的父目录。
@details Transaction 留在入口作用域，成功取得的父目录在委托结束后统一归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-575db944fcafa026"></a>

## `xv::fs::VirtualFileSystem::UnlinkFlow::release`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkFlow::release(Context &c) noexcept
@brief 归还父目录引用。
@details Transaction 留在入口作用域，成功取得的父目录在委托结束后统一归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-23b817c3b1f8b6f6"></a>

## `xv::fs::VirtualFileSystem::UnlinkFlow::unlink`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnlinkFlow::unlink(Context &c) noexcept
@brief 委托 unlink_at 并保存原业务返回值。
@details Transaction 留在入口作用域，成功取得的父目录在委托结束后统一归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d29384e99e2f5883"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::absolute`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::absolute(Context &c) noexcept
@brief 检查链接目标是否从根目录开始。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4666f14fcf13b1e6"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::advance`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::advance(Context &c) noexcept
@brief 解锁下一节点、归还旧目录引用并进入覆盖该节点的挂载根。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0d7dbf272af91ab7"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::check_directory`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::check_directory(Context &c) noexcept
@brief 短暂锁住最终节点并记录目录类型检查结果。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d7a9c438b2c09073"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::complete`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::complete(Context &c) noexcept
@brief 保存当前保留节点作为成功结果。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-411a67aea1cccbaf"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::component`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::component(Context &c) noexcept
@brief 截取 DIRSIZ 范围内的分量、推进路径并锁住当前节点。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e7482d5e5234464a"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::copy_path`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::copy_path(Context &c) noexcept
@brief 有界复制输入到待解析缓冲，记录尾斜线，过长路径保持失败状态。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d4aaf2fa5200aadd"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::create`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::create(Context &c) noexcept
@brief 创建并解锁末尾普通文件，归还当前目录引用。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4c00218086a8d1a7"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::create_last`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::create_last(Context &c) noexcept
@brief 检查缺失分量是否为本次允许创建的最后分量。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c1e6b0485d9e3784"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::find_next`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::find_next(Context &c) noexcept
@brief 先处理挂载根的父目录，再查找普通目录项，随后解锁当前节点。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fa3ab5607601ce73"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::found(Context &c) noexcept
@brief 检查是否找到下一节点。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fb84739d4e5af2bd"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::lock_next`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::lock_next(Context &c) noexcept
@brief 锁住下一节点以检查类型。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-17d1ef30848312ad"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::more`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::more(Context &c) noexcept
@brief 检查还有未解析的路径分量。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a12788353fda2c15"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::parent_component`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::parent_component(Context &c) noexcept
@brief 检查是否请求最后分量的父目录。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-51c8d6974132cde4"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::read_target`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::read_target(Context &c) noexcept
@brief 递增链接计数，检查长度与十六次上限并读取目标，随后解锁归还链接节点。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a92bfb799f45db2b"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::release_current`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::release_current(Context &c) noexcept
@brief 归还当前目录引用，保留默认失败结果。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e4648c12bcd278b4"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::restart_root`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::restart_root(Context &c) noexcept
@brief 归还当前目录引用并取得根目录引用。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c6cdb66a35d3874a"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::return_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::return_parent(Context &c) noexcept
@brief 解锁并保存已保留的父目录作为结果。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fb1b3b4cd1da5e7b"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::rewrite`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::rewrite(Context &c) noexcept
@brief 先移动重叠的剩余路径，再复制链接目标并保留尾斜线语义。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7cda0753479080cd"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::searchable`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::searchable(Context &c) noexcept
@brief 按原顺序检查目录类型、链接存活和搜索权限。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-084383eddf2c0379"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::skip_slashes`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::skip_slashes(Context &c) noexcept
@brief 直接跳过连续路径分隔符。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e182420f740298c0"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::start`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::start(Context &c) noexcept
@brief 按绝对路径或相对路径取得根目录或当前目录引用。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0945d9cfeddd0671"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::started`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::started(Context &c) noexcept
@brief 检查是否取得起始节点。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5f201aa1c14c1c31"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::symbolic`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::symbolic(Context &c) noexcept
@brief 检查 follow、剩余分量和尾斜线是否要求展开符号链接。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e745b2e8d9ce89dd"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::trailing`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::trailing(Context &c) noexcept
@brief 检查最终结果是否还需满足尾斜线目录约束。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-caa68230be2609e2"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::unlock_current`

```doxygen
@fn static void xv::fs::VirtualFileSystem::WalkFlow::unlock_current(Context &c) noexcept
@brief 释放当前节点锁。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c6381b533580b6d0"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::valid`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::valid(Context &c) noexcept
@brief 检查本阶段的路径、链接内容或目录校验是否成功。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e96b2fb3309f31d8"></a>

## `xv::fs::VirtualFileSystem::WalkFlow::wants_parent`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::WalkFlow::wants_parent(Context &c) noexcept
@brief 检查路径耗尽时是否仍请求父目录。
@details namespace 锁由调用者持有；当前目录引用在相对链接重写时保留，绝对链接重启才释放；失败路径分别处理锁住的当前节点和已释放的下一节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
