# `kernel/fs/ramfs/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::RamFileSystem::CreateFlow::found`](flow.cppm.md#fn-a74a48e28f9589fc)
- [`xv::fs::RamFileSystem::CreateFlow::insert`](flow.cppm.md#fn-12cf62c7b55d7835)
- [`xv::fs::RamFileSystem::CreateFlow::inserted`](flow.cppm.md#fn-c4fbab96f38c8b66)
- [`xv::fs::RamFileSystem::CreateFlow::lookup`](flow.cppm.md#fn-2bdfe1f0d3c7535d)
- [`xv::fs::RamFileSystem::CreateFlow::made`](flow.cppm.md#fn-e7c3248d4d7e90be)
- [`xv::fs::RamFileSystem::CreateFlow::make`](flow.cppm.md#fn-23f0605d806b244a)
- [`xv::fs::RamFileSystem::CreateFlow::publish`](flow.cppm.md#fn-cd89299bb2ae54c8)
- [`xv::fs::RamFileSystem::CreateFlow::reject_existing`](flow.cppm.md#fn-e0624c3e8fb4d764)
- [`xv::fs::RamFileSystem::CreateFlow::reusable`](flow.cppm.md#fn-3cf8fc635a0c5b9f)
- [`xv::fs::RamFileSystem::CreateFlow::reuse`](flow.cppm.md#fn-2dc924dee10d803f)
- [`xv::fs::RamFileSystem::CreateFlow::rollback`](flow.cppm.md#fn-e0f86dcf93f59d1c)
- [`xv::fs::RamFileSystem::CreateFlow::unlock`](flow.cppm.md#fn-329dbba2f4408b05)
- [`xv::fs::RamFileSystem::CreateFlow::valid`](flow.cppm.md#fn-9e845d4f80be1145)
- [`xv::fs::RamFileSystem::InitializeMountFlow::configure`](flow.cppm.md#fn-5835d311c9c82813)
- [`xv::fs::RamFileSystem::InitializeMountFlow::made`](flow.cppm.md#fn-8c45000b8880ab7d)
- [`xv::fs::RamFileSystem::InitializeMountFlow::make_root`](flow.cppm.md#fn-2f1db7d541780814)
- [`xv::fs::RamFileSystem::InitializeMountFlow::unlock`](flow.cppm.md#fn-1a62a009ea07573d)
- [`xv::fs::RamFileSystem::LinkFlow::insert`](flow.cppm.md#fn-40a3b02857165040)
- [`xv::fs::RamFileSystem::LinkFlow::publish`](flow.cppm.md#fn-099fb6df203089d8)
- [`xv::fs::RamFileSystem::LinkFlow::valid`](flow.cppm.md#fn-970ecab12bdc9d75)
- [`xv::fs::RamFileSystem::MakeFlow::allocate`](flow.cppm.md#fn-609acb6eaab12a21)
- [`xv::fs::RamFileSystem::MakeFlow::allocated`](flow.cppm.md#fn-164f912e08f0b3af)
- [`xv::fs::RamFileSystem::MakeFlow::available`](flow.cppm.md#fn-4dd06e7caca3ba75)
- [`xv::fs::RamFileSystem::MakeFlow::configure_directory`](flow.cppm.md#fn-a8a9bb2d5adb76c0)
- [`xv::fs::RamFileSystem::MakeFlow::construct`](flow.cppm.md#fn-dc3d9d96967f3f37)
- [`xv::fs::RamFileSystem::MakeFlow::directory`](flow.cppm.md#fn-df4a0094fc460182)
- [`xv::fs::RamFileSystem::MakeFlow::has_parent`](flow.cppm.md#fn-0974ec7e326da046)
- [`xv::fs::RamFileSystem::MakeFlow::number`](flow.cppm.md#fn-a9826311bb86a513)
- [`xv::fs::RamFileSystem::MakeFlow::pin_parent`](flow.cppm.md#fn-265a60afcb1e3eef)
- [`xv::fs::RamFileSystem::ReadFlow::clamp`](flow.cppm.md#fn-50837abd5e17c2d2)
- [`xv::fs::RamFileSystem::ReadFlow::complete`](flow.cppm.md#fn-50ec431c452ac367)
- [`xv::fs::RamFileSystem::ReadFlow::copy`](flow.cppm.md#fn-ece4f8db0e008899)
- [`xv::fs::RamFileSystem::ReadFlow::directory`](flow.cppm.md#fn-06c7d7f82963049a)
- [`xv::fs::RamFileSystem::ReadFlow::failure`](flow.cppm.md#fn-24953cf6b9c2037b)
- [`xv::fs::RamFileSystem::ReadFlow::file`](flow.cppm.md#fn-9dafad037d1f938b)
- [`xv::fs::RamFileSystem::ReadFlow::in_range`](flow.cppm.md#fn-1b59319310f257fe)
- [`xv::fs::RamFileSystem::ReadFlow::more`](flow.cppm.md#fn-924cd22d79ffc0e1)
- [`xv::fs::RamFileSystem::ReadFlow::prepare`](flow.cppm.md#fn-8bbfeaa29c8d699e)
- [`xv::fs::RamFileSystem::ReadFlow::progress`](flow.cppm.md#fn-9f30e9f8ca94bcd7)
- [`xv::fs::RamFileSystem::ReadFlow::read_file`](flow.cppm.md#fn-d06285c301e28e70)
- [`xv::fs::RamFileSystem::ReclaimFlow::advance`](flow.cppm.md#fn-b9b4bc6aa92c2fa3)
- [`xv::fs::RamFileSystem::ReclaimFlow::detach`](flow.cppm.md#fn-4dcc2ab2232d1f63)
- [`xv::fs::RamFileSystem::ReclaimFlow::free`](flow.cppm.md#fn-3d1d111e0175c8db)
- [`xv::fs::RamFileSystem::ReclaimFlow::more`](flow.cppm.md#fn-bf48abf38767cc00)
- [`xv::fs::RamFileSystem::ReclaimFlow::parent`](flow.cppm.md#fn-1e4afca2b11a1d7f)
- [`xv::fs::RamFileSystem::ReleaseFlow::destroy`](flow.cppm.md#fn-84b4043a1984ce32)
- [`xv::fs::RamFileSystem::ReleaseFlow::drop_last`](flow.cppm.md#fn-ed941c55fcd3ffea)
- [`xv::fs::RamFileSystem::ReleaseFlow::drop_shared`](flow.cppm.md#fn-51135233fb5b9dbc)
- [`xv::fs::RamFileSystem::ReleaseFlow::held`](flow.cppm.md#fn-747e072a41014590)
- [`xv::fs::RamFileSystem::ReleaseFlow::inspect_tree`](flow.cppm.md#fn-d93af876e00a3bcf)
- [`xv::fs::RamFileSystem::ReleaseFlow::invalid`](flow.cppm.md#fn-338c4049f5c14d25)
- [`xv::fs::RamFileSystem::ReleaseFlow::live`](flow.cppm.md#fn-52efbb21fc734556)
- [`xv::fs::RamFileSystem::ReleaseFlow::lock_tree`](flow.cppm.md#fn-8a2dacae1c83ec18)
- [`xv::fs::RamFileSystem::ReleaseFlow::reclaim`](flow.cppm.md#fn-59dba9018f35968d)
- [`xv::fs::RamFileSystem::ReleaseFlow::shared`](flow.cppm.md#fn-23e5a132e52807d9)
- [`xv::fs::RamFileSystem::ReleaseFlow::unlock_tree`](flow.cppm.md#fn-17fea2e84f864959)
- [`xv::fs::RamFileSystem::RenameFlow::compatible`](flow.cppm.md#fn-278fd819caee5d07)
- [`xv::fs::RamFileSystem::RenameFlow::detach`](flow.cppm.md#fn-97c845597799e902)
- [`xv::fs::RamFileSystem::RenameFlow::find`](flow.cppm.md#fn-28a651e6472cea8d)
- [`xv::fs::RamFileSystem::RenameFlow::identical`](flow.cppm.md#fn-d693e2468dedad11)
- [`xv::fs::RamFileSystem::RenameFlow::insert`](flow.cppm.md#fn-c62f70a3882ecacb)
- [`xv::fs::RamFileSystem::RenameFlow::link_capacity`](flow.cppm.md#fn-e621008e149c28d1)
- [`xv::fs::RamFileSystem::RenameFlow::reparent`](flow.cppm.md#fn-a7894478b9c9bbb6)
- [`xv::fs::RamFileSystem::RenameFlow::reparenting`](flow.cppm.md#fn-ba72ff342194f697)
- [`xv::fs::RamFileSystem::RenameFlow::replace`](flow.cppm.md#fn-197ec7e0d0d71db1)
- [`xv::fs::RamFileSystem::RenameFlow::replacing`](flow.cppm.md#fn-b91aedd735a87c43)
- [`xv::fs::RamFileSystem::RenameFlow::select`](flow.cppm.md#fn-9a1f38d738275614)
- [`xv::fs::RamFileSystem::RenameFlow::success`](flow.cppm.md#fn-76ad822728d40a2a)
- [`xv::fs::RamFileSystem::RenameFlow::valid`](flow.cppm.md#fn-7ee7efbc60aa015e)
- [`xv::fs::RamFileSystem::UnlinkFlow::allowed`](flow.cppm.md#fn-1ffd84efc65e9681)
- [`xv::fs::RamFileSystem::UnlinkFlow::find`](flow.cppm.md#fn-34190140f4ab5e0e)
- [`xv::fs::RamFileSystem::UnlinkFlow::removable`](flow.cppm.md#fn-942c1180ac9e0065)
- [`xv::fs::RamFileSystem::UnlinkFlow::unlink`](flow.cppm.md#fn-00aa304151c55670)
- [`xv::fs::RamFileSystem::UnlinkFlow::valid`](flow.cppm.md#fn-c534f8481a66dad7)

<a id="fn-a74a48e28f9589fc"></a>

## `xv::fs::RamFileSystem::CreateFlow::found`

```doxygen
@fn static bool xv::fs::RamFileSystem::CreateFlow::found(Context &c) noexcept
@brief 检查是否取得既有节点引用。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-12cf62c7b55d7835"></a>

## `xv::fs::RamFileSystem::CreateFlow::insert`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::insert(Context &c) noexcept
@brief 尝试插入新目录项并保存分配结果。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c4fbab96f38c8b66"></a>

## `xv::fs::RamFileSystem::CreateFlow::inserted`

```doxygen
@fn static bool xv::fs::RamFileSystem::CreateFlow::inserted(Context &c) noexcept
@brief 检查新目录项是否已发布。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2bdfe1f0d3c7535d"></a>

## `xv::fs::RamFileSystem::CreateFlow::lookup`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::lookup(Context &c) noexcept
@brief 查找父目录内的既有目录项。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e7c3248d4d7e90be"></a>

## `xv::fs::RamFileSystem::CreateFlow::made`

```doxygen
@fn static bool xv::fs::RamFileSystem::CreateFlow::made(Context &c) noexcept
@brief 检查候选节点是否已取得。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-23f0605d806b244a"></a>

## `xv::fs::RamFileSystem::CreateFlow::make`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::make(Context &c) noexcept
@brief 为新项目分配节点，只有目录类型保留父引用。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cd89299bb2ae54c8"></a>

## `xv::fs::RamFileSystem::CreateFlow::publish`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::publish(Context &c) noexcept
@brief 提交新节点链接数及目录父链接数，保存带锁结果。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e0624c3e8fb4d764"></a>

## `xv::fs::RamFileSystem::CreateFlow::reject_existing`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::reject_existing(Context &c) noexcept
@brief 归还类型不兼容的既有节点引用。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3cf8fc635a0c5b9f"></a>

## `xv::fs::RamFileSystem::CreateFlow::reusable`

```doxygen
@fn static bool xv::fs::RamFileSystem::CreateFlow::reusable(Context &c) noexcept
@brief 检查普通文件请求是否可复用既有普通文件。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2dc924dee10d803f"></a>

## `xv::fs::RamFileSystem::CreateFlow::reuse`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::reuse(Context &c) noexcept
@brief 保存既有带锁节点作为成功结果。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e0f86dcf93f59d1c"></a>

## `xv::fs::RamFileSystem::CreateFlow::rollback`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::rollback(Context &c) noexcept
@brief 归还未链接候选引用并回收候选及其内部父 pin。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-329dbba2f4408b05"></a>

## `xv::fs::RamFileSystem::CreateFlow::unlock`

```doxygen
@fn static void xv::fs::RamFileSystem::CreateFlow::unlock(Context &c) noexcept
@brief 在失败路径释放树锁。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9e845d4f80be1145"></a>

## `xv::fs::RamFileSystem::CreateFlow::valid`

```doxygen
@fn static bool xv::fs::RamFileSystem::CreateFlow::valid(Context &c) noexcept
@brief 按原顺序验证父目录、写搜索权限、允许类型及目录链接数上限。
@details 入口持树锁；成功返回时仍持树锁。失败归还已取得的 existing 引用或回收未链接候选后才解锁，插入前不发布链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5835d311c9c82813"></a>

## `xv::fs::RamFileSystem::InitializeMountFlow::configure`

```doxygen
@fn static void xv::fs::RamFileSystem::InitializeMountFlow::configure(Context &c) noexcept
@brief 设置根的自身父指针、链接数和默认所有者权限，记录成功。
@details 入口设置设备号并持树锁，成功或失败均经过同一个解锁节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8c45000b8880ab7d"></a>

## `xv::fs::RamFileSystem::InitializeMountFlow::made`

```doxygen
@fn static bool xv::fs::RamFileSystem::InitializeMountFlow::made(Context &c) noexcept
@brief 检查根候选是否已分配。
@details 入口设置设备号并持树锁，成功或失败均经过同一个解锁节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2f1db7d541780814"></a>

## `xv::fs::RamFileSystem::InitializeMountFlow::make_root`

```doxygen
@fn static void xv::fs::RamFileSystem::InitializeMountFlow::make_root(Context &c) noexcept
@brief 分配并登记根目录候选。
@details 入口设置设备号并持树锁，成功或失败均经过同一个解锁节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1a62a009ea07573d"></a>

## `xv::fs::RamFileSystem::InitializeMountFlow::unlock`

```doxygen
@fn static void xv::fs::RamFileSystem::InitializeMountFlow::unlock(Context &c) noexcept
@brief 释放树锁。
@details 入口设置设备号并持树锁，成功或失败均经过同一个解锁节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-40a3b02857165040"></a>

## `xv::fs::RamFileSystem::LinkFlow::insert`

```doxygen
@fn static bool xv::fs::RamFileSystem::LinkFlow::insert(Context &c) noexcept
@brief 尝试插入指向目标节点的新目录项。
@details 入口 ScopedLock 持树锁，目录项成功插入后才增加目标链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-099fb6df203089d8"></a>

## `xv::fs::RamFileSystem::LinkFlow::publish`

```doxygen
@fn static void xv::fs::RamFileSystem::LinkFlow::publish(Context &c) noexcept
@brief 递增目标链接数并记录成功码。
@details 入口 ScopedLock 持树锁，目录项成功插入后才增加目标链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-970ecab12bdc9d75"></a>

## `xv::fs::RamFileSystem::LinkFlow::valid`

```doxygen
@fn static bool xv::fs::RamFileSystem::LinkFlow::valid(Context &c) noexcept
@brief 按原次序验证同一后端、目录权限、目标类型和链接数、名称及不存在同名项。
@details 入口 ScopedLock 持树锁，目录项成功插入后才增加目标链接数。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-609acb6eaab12a21"></a>

## `xv::fs::RamFileSystem::MakeFlow::allocate`

```doxygen
@fn static void xv::fs::RamFileSystem::MakeFlow::allocate(Context &c) noexcept
@brief 申请节点存储页。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-164f912e08f0b3af"></a>

## `xv::fs::RamFileSystem::MakeFlow::allocated`

```doxygen
@fn static bool xv::fs::RamFileSystem::MakeFlow::allocated(Context &c) noexcept
@brief 检查节点页是否已取得。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4dd06e7caca3ba75"></a>

## `xv::fs::RamFileSystem::MakeFlow::available`

```doxygen
@fn static bool xv::fs::RamFileSystem::MakeFlow::available(Context &c) noexcept
@brief 检查 inode 号空间是否尚未耗尽。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a8a9bb2d5adb76c0"></a>

## `xv::fs::RamFileSystem::MakeFlow::configure_directory`

```doxygen
@fn static void xv::fs::RamFileSystem::MakeFlow::configure_directory(Context &c) noexcept
@brief 设置目录初始大小和父节点指针。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-dc3d9d96967f3f37"></a>

## `xv::fs::RamFileSystem::MakeFlow::construct`

```doxygen
@fn static void xv::fs::RamFileSystem::MakeFlow::construct(Context &c) noexcept
@brief 原位构造节点，登记链表、引用和号码，设置身份、类型和初始模式。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-df4a0094fc460182"></a>

## `xv::fs::RamFileSystem::MakeFlow::directory`

```doxygen
@fn static bool xv::fs::RamFileSystem::MakeFlow::directory(Context &c) noexcept
@brief 检查候选类型是否为目录。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0974ec7e326da046"></a>

## `xv::fs::RamFileSystem::MakeFlow::has_parent`

```doxygen
@fn static bool xv::fs::RamFileSystem::MakeFlow::has_parent(Context &c) noexcept
@brief 检查目录是否有需要保留的父节点。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a9826311bb86a513"></a>

## `xv::fs::RamFileSystem::MakeFlow::number`

```doxygen
@fn static void xv::fs::RamFileSystem::MakeFlow::number(Context &c) noexcept
@brief 直接扫描位图选首个空闲非零 inode 号。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-265a60afcb1e3eef"></a>

## `xv::fs::RamFileSystem::MakeFlow::pin_parent`

```doxygen
@fn static void xv::fs::RamFileSystem::MakeFlow::pin_parent(Context &c) noexcept
@brief 保留父目录引用并从外部引用计数中排除该内部 pin。
@details 调用者持树锁；页面构造、节点链登记、外部引用数与 inode 号位图按原次序更新，目录的父引用不计入外部 live_references。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-50837abd5e17c2d2"></a>

## `xv::fs::RamFileSystem::ReadFlow::clamp`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::clamp(Context &c) noexcept
@brief 将目录读取长度裁剪到末尾。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-50ec431c452ac367"></a>

## `xv::fs::RamFileSystem::ReadFlow::complete`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::complete(Context &c) noexcept
@brief 记录完整读取或 EOF 的字节数。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ece4f8db0e008899"></a>

## `xv::fs::RamFileSystem::ReadFlow::copy`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReadFlow::copy(Context &c) noexcept
@brief 复制当前目录记录的一段并报告是否成功。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-06c7d7f82963049a"></a>

## `xv::fs::RamFileSystem::ReadFlow::directory`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReadFlow::directory(Context &c) noexcept
@brief 检查节点是否为目录。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-24953cf6b9c2037b"></a>

## `xv::fs::RamFileSystem::ReadFlow::failure`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::failure(Context &c) noexcept
@brief 记录此前完整进展，首段失败记录负值。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9dafad037d1f938b"></a>

## `xv::fs::RamFileSystem::ReadFlow::file`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReadFlow::file(Context &c) noexcept
@brief 检查节点是否为普通文件或符号链接。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1b59319310f257fe"></a>

## `xv::fs::RamFileSystem::ReadFlow::in_range`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReadFlow::in_range(Context &c) noexcept
@brief 检查目录读取偏移是否在逻辑末尾之前。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-924cd22d79ffc0e1"></a>

## `xv::fs::RamFileSystem::ReadFlow::more`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReadFlow::more(Context &c) noexcept
@brief 检查是否还有目录字节待复制。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8bbfeaa29c8d699e"></a>

## `xv::fs::RamFileSystem::ReadFlow::prepare`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::prepare(Context &c) noexcept
@brief 直接构造点、点点或普通目录记录并计算本次记录内复制范围。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9f30e9f8ca94bcd7"></a>

## `xv::fs::RamFileSystem::ReadFlow::progress`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::progress(Context &c) noexcept
@brief 累计成功复制的目录字节数。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d06285c301e28e70"></a>

## `xv::fs::RamFileSystem::ReadFlow::read_file`

```doxygen
@fn static void xv::fs::RamFileSystem::ReadFlow::read_file(Context &c) noexcept
@brief 委托 PageBuffer 读取并保存业务结果。
@details 调用者沿原树锁协议串行化节点访问；普通文件和链接委托 PageBuffer，目录按记录推进且复制失败保留此前完整进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b9b4bc6aa92c2fa3"></a>

## `xv::fs::RamFileSystem::ReclaimFlow::advance`

```doxygen
@fn static void xv::fs::RamFileSystem::ReclaimFlow::advance(Context &c) noexcept
@brief 在引用锁内归还父目录 pin，仅在零引用且零链接时推进到父节点。
@details 调用者持树锁；每轮释放当前节点后只保存父指针并迭代，父目录连锁回收不递归增长 C++ 调用栈。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4dcc2ab2232d1f63"></a>

## `xv::fs::RamFileSystem::ReclaimFlow::detach`

```doxygen
@fn static void xv::fs::RamFileSystem::ReclaimFlow::detach(Context &c) noexcept
@brief 从拥有链表移除节点、保存非自身父指针并归还 inode 号。
@details 调用者持树锁；每轮释放当前节点后只保存父指针并迭代，父目录连锁回收不递归增长 C++ 调用栈。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3d1d111e0175c8db"></a>

## `xv::fs::RamFileSystem::ReclaimFlow::free`

```doxygen
@fn static void xv::fs::RamFileSystem::ReclaimFlow::free(Context &c) noexcept
@brief 直接销毁目录页链、节点内容与节点存储页。
@details 调用者持树锁；每轮释放当前节点后只保存父指针并迭代，父目录连锁回收不递归增长 C++ 调用栈。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-bf48abf38767cc00"></a>

## `xv::fs::RamFileSystem::ReclaimFlow::more`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReclaimFlow::more(Context &c) noexcept
@brief 检查是否还有待回收节点。
@details 调用者持树锁；每轮释放当前节点后只保存父指针并迭代，父目录连锁回收不递归增长 C++ 调用栈。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1e4afca2b11a1d7f"></a>

## `xv::fs::RamFileSystem::ReclaimFlow::parent`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReclaimFlow::parent(Context &c) noexcept
@brief 检查被回收节点是否保留了父目录引用。
@details 调用者持树锁；每轮释放当前节点后只保存父指针并迭代，父目录连锁回收不递归增长 C++ 调用栈。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-84b4043a1984ce32"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::destroy`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReleaseFlow::destroy(Context &c) noexcept
@brief 检查本阶段是否决定回收节点。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ed941c55fcd3ffea"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::drop_last`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::drop_last(Context &c) noexcept
@brief 重新持引用锁减少外部和节点引用，并判断零链接节点是否需回收。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-51135233fb5b9dbc"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::drop_shared`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::drop_shared(Context &c) noexcept
@brief 减少外部及节点引用计数并释放引用锁。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-747e072a41014590"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::held`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReleaseFlow::held(Context &c) noexcept
@brief 检查树锁是否由调用者原先持有。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d93af876e00a3bcf"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::inspect_tree`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::inspect_tree(Context &c) noexcept
@brief 释放引用锁并记录调用者是否已有树锁。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-338c4049f5c14d25"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::invalid`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::invalid(Context &) noexcept
@brief 诊断归还已经失效的 inode 引用。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-52efbb21fc734556"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::live`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReleaseFlow::live(Context &c) noexcept
@brief 检查被归还引用仍然有效。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8a2dacae1c83ec18"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::lock_tree`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::lock_tree(Context &c) noexcept
@brief 在未持引用锁时取得树锁。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-59dba9018f35968d"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::reclaim`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::reclaim(Context &c) noexcept
@brief 执行迭代节点与父目录回收流程。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-23e5a132e52807d9"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::shared`

```doxygen
@fn static bool xv::fs::RamFileSystem::ReleaseFlow::shared(Context &c) noexcept
@brief 检查初次持锁观察时是否还有其他引用。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-17fea2e84f864959"></a>

## `xv::fs::RamFileSystem::ReleaseFlow::unlock_tree`

```doxygen
@fn static void xv::fs::RamFileSystem::ReleaseFlow::unlock_tree(Context &c) noexcept
@brief 释放本次额外取得的树锁。
@details 入口持引用自旋锁；疑似最后引用先放引用锁再按需取树锁，重新持引用锁后再次减数。只释放本次取得的树锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-278fd819caee5d07"></a>

## `xv::fs::RamFileSystem::RenameFlow::compatible`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::compatible(Context &c) noexcept
@brief 验证可替换目标的删除权限、目录类型一致性和空目录约束。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-97c845597799e902"></a>

## `xv::fs::RamFileSystem::RenameFlow::detach`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::detach(Context &c) noexcept
@brief 清空源目录槽。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-28a651e6472cea8d"></a>

## `xv::fs::RamFileSystem::RenameFlow::find`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::find(Context &c) noexcept
@brief 按原次序查找源槽和目标槽。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d693e2468dedad11"></a>

## `xv::fs::RamFileSystem::RenameFlow::identical`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::identical(Context &c) noexcept
@brief 检查新旧目录项是否已经指向同一节点。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c62f70a3882ecacb"></a>

## `xv::fs::RamFileSystem::RenameFlow::insert`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::insert(Context &c) noexcept
@brief 尝试在目标目录插入新项并报告是否成功。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e621008e149c28d1"></a>

## `xv::fs::RamFileSystem::RenameFlow::link_capacity`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::link_capacity(Context &c) noexcept
@brief 检查目录跨父移动不会超过目标链接数上限。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a7894478b9c9bbb6"></a>

## `xv::fs::RamFileSystem::RenameFlow::reparent`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::reparent(Context &c) noexcept
@brief 更新父目录链接数与引用计数，并提交移动目录的新父指针。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ba72ff342194f697"></a>

## `xv::fs::RamFileSystem::RenameFlow::reparenting`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::reparenting(Context &c) noexcept
@brief 检查是否需要为跨父目录移动转移父 pin。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-197ec7e0d0d71db1"></a>

## `xv::fs::RamFileSystem::RenameFlow::replace`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::replace(Context &c) noexcept
@brief 保留被替换节点、更新链接与目标槽，再归还被替换引用。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b91aedd735a87c43"></a>

## `xv::fs::RamFileSystem::RenameFlow::replacing`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::replacing(Context &c) noexcept
@brief 检查是否需要替换既有目标目录项。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9a1f38d738275614"></a>

## `xv::fs::RamFileSystem::RenameFlow::select`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::select(Context &c) noexcept
@brief 保存将要移动的节点。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-76ad822728d40a2a"></a>

## `xv::fs::RamFileSystem::RenameFlow::success`

```doxygen
@fn static void xv::fs::RamFileSystem::RenameFlow::success(Context &c) noexcept
@brief 记录重命名成功码。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7ee7efbc60aa015e"></a>

## `xv::fs::RamFileSystem::RenameFlow::valid`

```doxygen
@fn static bool xv::fs::RamFileSystem::RenameFlow::valid(Context &c) noexcept
@brief 验证源存在、父目录存活和写搜索、删除权限。
@details 入口 ScopedLock 持树锁，替换或插入目标成功后才清空源槽；跨父目录移动时转移内部父 pin，保留被替换但仍被打开的 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1ffd84efc65e9681"></a>

## `xv::fs::RamFileSystem::UnlinkFlow::allowed`

```doxygen
@fn static bool xv::fs::RamFileSystem::UnlinkFlow::allowed(Context &c) noexcept
@brief 检查 sticky 等删除权限策略。
@details 入口 ScopedLock 持树锁；清除目录槽和减少链接后才归还保留的目标引用，允许立即触发迭代回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-34190140f4ab5e0e"></a>

## `xv::fs::RamFileSystem::UnlinkFlow::find`

```doxygen
@fn static void xv::fs::RamFileSystem::UnlinkFlow::find(Context &c) noexcept
@brief 查找待删除的目录槽。
@details 入口 ScopedLock 持树锁；清除目录槽和减少链接后才归还保留的目标引用，允许立即触发迭代回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-942c1180ac9e0065"></a>

## `xv::fs::RamFileSystem::UnlinkFlow::removable`

```doxygen
@fn static bool xv::fs::RamFileSystem::UnlinkFlow::removable(Context &c) noexcept
@brief 检查目录项存在且目录目标为空。
@details 入口 ScopedLock 持树锁；清除目录槽和减少链接后才归还保留的目标引用，允许立即触发迭代回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-00aa304151c55670"></a>

## `xv::fs::RamFileSystem::UnlinkFlow::unlink`

```doxygen
@fn static void xv::fs::RamFileSystem::UnlinkFlow::unlink(Context &c) noexcept
@brief 保留目标、清空目录槽、更新链接数、归还目标并记录成功码。
@details 入口 ScopedLock 持树锁；清除目录槽和减少链接后才归还保留的目标引用，允许立即触发迭代回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c534f8481a66dad7"></a>

## `xv::fs::RamFileSystem::UnlinkFlow::valid`

```doxygen
@fn static bool xv::fs::RamFileSystem::UnlinkFlow::valid(Context &c) noexcept
@brief 验证目录存活、写搜索权限及非点分量。
@details 入口 ScopedLock 持树锁；清除目录槽和减少链接后才归还保留的目标引用，允许立即触发迭代回收。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
