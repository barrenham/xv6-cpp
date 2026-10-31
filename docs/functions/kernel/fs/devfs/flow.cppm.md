# `kernel/fs/devfs/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::DevFileSystem::CreateFlow::device`](flow.cppm.md#fn-fa0e5a3d7a1b6460)
- [`xv::fs::DevFileSystem::CreateFlow::found`](flow.cppm.md#fn-c3f079d3f53f216f)
- [`xv::fs::DevFileSystem::CreateFlow::lock`](flow.cppm.md#fn-53d36c4608c0fbe1)
- [`xv::fs::DevFileSystem::CreateFlow::lookup`](flow.cppm.md#fn-6755a3fbf26857b3)
- [`xv::fs::DevFileSystem::CreateFlow::regular`](flow.cppm.md#fn-4c703985b27e32a5)
- [`xv::fs::DevFileSystem::CreateFlow::reject`](flow.cppm.md#fn-c849d043d03e78d4)
- [`xv::fs::DevFileSystem::LookupFlow::already_pinned`](flow.cppm.md#fn-961d236a1e3e762d)
- [`xv::fs::DevFileSystem::LookupFlow::directory`](flow.cppm.md#fn-3b97581a9a7e0075)
- [`xv::fs::DevFileSystem::LookupFlow::owned`](flow.cppm.md#fn-5b053f173214f010)
- [`xv::fs::DevFileSystem::LookupFlow::ownership`](flow.cppm.md#fn-f974def4c87cecc3)
- [`xv::fs::DevFileSystem::LookupFlow::parent`](flow.cppm.md#fn-fff233ab113b04d1)
- [`xv::fs::DevFileSystem::LookupFlow::parse`](flow.cppm.md#fn-c854158036f18019)
- [`xv::fs::DevFileSystem::LookupFlow::pin`](flow.cppm.md#fn-794fadea5ec25406)
- [`xv::fs::DevFileSystem::LookupFlow::pts`](flow.cppm.md#fn-971d29a7f908f5a2)
- [`xv::fs::DevFileSystem::LookupFlow::pts_name`](flow.cppm.md#fn-b06b82ad14854b3b)
- [`xv::fs::DevFileSystem::LookupFlow::publish`](flow.cppm.md#fn-4d02f156cdee2c66)
- [`xv::fs::DevFileSystem::LookupFlow::registered`](flow.cppm.md#fn-f1c5763c20346511)
- [`xv::fs::DevFileSystem::LookupFlow::release_pin`](flow.cppm.md#fn-e0bdd64709d68792)
- [`xv::fs::DevFileSystem::LookupFlow::retain_node`](flow.cppm.md#fn-0ed79a752ff4f7bd)
- [`xv::fs::DevFileSystem::LookupFlow::retain_pts`](flow.cppm.md#fn-ee063687928b6b0a)
- [`xv::fs::DevFileSystem::LookupFlow::retain_root`](flow.cppm.md#fn-73378b4aa0a6567f)
- [`xv::fs::DevFileSystem::LookupFlow::retain_self`](flow.cppm.md#fn-dd449af8a68ec031)
- [`xv::fs::DevFileSystem::LookupFlow::self`](flow.cppm.md#fn-277e3a76262c5b37)
- [`xv::fs::DevFileSystem::ReadFlow::clamp`](flow.cppm.md#fn-cdbc2e77b0a9a85c)
- [`xv::fs::DevFileSystem::ReadFlow::complete`](flow.cppm.md#fn-3e7d1500753b4077)
- [`xv::fs::DevFileSystem::ReadFlow::copy`](flow.cppm.md#fn-34e7cae3e02fffa8)
- [`xv::fs::DevFileSystem::ReadFlow::decimal_name`](flow.cppm.md#fn-a48e2cb851a6af2e)
- [`xv::fs::DevFileSystem::ReadFlow::directory`](flow.cppm.md#fn-297aed2d04431e0d)
- [`xv::fs::DevFileSystem::ReadFlow::failure`](flow.cppm.md#fn-d064e5a6967a9d8c)
- [`xv::fs::DevFileSystem::ReadFlow::in_range`](flow.cppm.md#fn-49127e1d0738e810)
- [`xv::fs::DevFileSystem::ReadFlow::more`](flow.cppm.md#fn-4ce53ad06d5cb43e)
- [`xv::fs::DevFileSystem::ReadFlow::prepare`](flow.cppm.md#fn-fd035243bed189cb)
- [`xv::fs::DevFileSystem::ReadFlow::progress`](flow.cppm.md#fn-b82633aff815a45f)
- [`xv::fs::DevFileSystem::ReadFlow::snapshot`](flow.cppm.md#fn-095b458bba37e6ba)
- [`xv::fs::DevFileSystem::ReleaseFlow::drop`](flow.cppm.md#fn-82caa8685e4ca1cc)
- [`xv::fs::DevFileSystem::ReleaseFlow::invalid`](flow.cppm.md#fn-125680a03beaba5c)
- [`xv::fs::DevFileSystem::ReleaseFlow::live`](flow.cppm.md#fn-d34884139ee6353d)
- [`xv::fs::DevFileSystem::ReleaseFlow::release_pty`](flow.cppm.md#fn-cff7a738c1653db5)
- [`xv::fs::DevFileSystem::ReleaseFlow::unlock`](flow.cppm.md#fn-5e193a19bac5d58e)
- [`xv::fs::DevFileSystem::ReleaseFlow::unpin`](flow.cppm.md#fn-0309248a65308402)

<a id="fn-fa0e5a3d7a1b6460"></a>

## `xv::fs::DevFileSystem::CreateFlow::device`

```doxygen
@fn static bool xv::fs::DevFileSystem::CreateFlow::device(Context &c) noexcept
@brief 检查结果类型为设备。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-c3f079d3f53f216f"></a>

## `xv::fs::DevFileSystem::CreateFlow::found`

```doxygen
@fn static bool xv::fs::DevFileSystem::CreateFlow::found(Context &c) noexcept
@brief 检查目录项是否存在。
@details 只允许以普通文件请求打开既有设备；查找时锁目录，查找完成后改锁目标，成功返回设备锁仍持有。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-53d36c4608c0fbe1"></a>

## `xv::fs::DevFileSystem::CreateFlow::lock`

```doxygen
@fn static void xv::fs::DevFileSystem::CreateFlow::lock(Context &c) noexcept
@brief 取得结果 inode 锁。
@details 只允许以普通文件请求打开既有设备；查找时锁目录，查找完成后改锁目标，成功返回设备锁仍持有。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6755a3fbf26857b3"></a>

## `xv::fs::DevFileSystem::CreateFlow::lookup`

```doxygen
@fn static void xv::fs::DevFileSystem::CreateFlow::lookup(Context &c) noexcept
@brief 在目录锁范围内查询既有设备路径并保存引用。
@details 只允许以普通文件请求打开既有设备；查找时锁目录，查找完成后改锁目标，成功返回设备锁仍持有。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4c703985b27e32a5"></a>

## `xv::fs::DevFileSystem::CreateFlow::regular`

```doxygen
@fn static bool xv::fs::DevFileSystem::CreateFlow::regular(Context &c) noexcept
@brief 检查请求类型是否为普通文件。
@details 只允许以普通文件请求打开既有设备；查找时锁目录，查找完成后改锁目标，成功返回设备锁仍持有。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c849d043d03e78d4"></a>

## `xv::fs::DevFileSystem::CreateFlow::reject`

```doxygen
@fn static void xv::fs::DevFileSystem::CreateFlow::reject(Context &c) noexcept
@brief 解锁归还类型不匹配的结果并清空结果。
@details 只允许以普通文件请求打开既有设备；查找时锁目录，查找完成后改锁目标，成功返回设备锁仍持有。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-961d236a1e3e762d"></a>

## `xv::fs::DevFileSystem::LookupFlow::already_pinned`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::already_pinned(Context &c) noexcept
@brief 检查之前是否已经存在持有该 PTY pin 的 inode 引用。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3b97581a9a7e0075"></a>

## `xv::fs::DevFileSystem::LookupFlow::directory`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::directory(Context &c) noexcept
@brief 检查输入为设备根或 pts 目录。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5b053f173214f010"></a>

## `xv::fs::DevFileSystem::LookupFlow::owned`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::owned(Context &c) noexcept
@brief 检查外部 PTY 所有者查询是否成功。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f974def4c87cecc3"></a>

## `xv::fs::DevFileSystem::LookupFlow::ownership`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::ownership(Context &c) noexcept
@brief 选择 PTY inode 并查询所有者与组。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fff233ab113b04d1"></a>

## `xv::fs::DevFileSystem::LookupFlow::parent`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::parent(Context &c) noexcept
@brief 检查点点目录项。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c854158036f18019"></a>

## `xv::fs::DevFileSystem::LookupFlow::parse`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::parse(Context &c) noexcept
@brief 有界解析不带多余前导零的 PTY 编号并检查最大编号。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-794fadea5ec25406"></a>

## `xv::fs::DevFileSystem::LookupFlow::pin`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::pin(Context &c) noexcept
@brief 尝试保留指定具名 PTY 的外部 pin。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-971d29a7f908f5a2"></a>

## `xv::fs::DevFileSystem::LookupFlow::pts`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::pts(Context &c) noexcept
@brief 检查是否需要解析具名 PTY 编号。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b06b82ad14854b3b"></a>

## `xv::fs::DevFileSystem::LookupFlow::pts_name`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::pts_name(Context &c) noexcept
@brief 检查设备根下是否请求 pts 名称。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4d02f156cdee2c66"></a>

## `xv::fs::DevFileSystem::LookupFlow::publish`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::publish(Context &c) noexcept
@brief 按需写目录项偏移并保存 PTY inode 结果。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f1c5763c20346511"></a>

## `xv::fs::DevFileSystem::LookupFlow::registered`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::registered(Context &c) noexcept
@brief 直接扫描登记设备名称，命中时设置偏移并保留对应 inode。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e0bdd64709d68792"></a>

## `xv::fs::DevFileSystem::LookupFlow::release_pin`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::release_pin(Context &c) noexcept
@brief 归还失败或重复取得的 PTY pin。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0ed79a752ff4f7bd"></a>

## `xv::fs::DevFileSystem::LookupFlow::retain_node`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::retain_node(Context &c) noexcept
@brief 在引用锁内保留 PTY inode，首个引用写入本次所有者与组。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ee063687928b6b0a"></a>

## `xv::fs::DevFileSystem::LookupFlow::retain_pts`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::retain_pts(Context &c) noexcept
@brief 保留 pts 目录作为结果。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-73378b4aa0a6567f"></a>

## `xv::fs::DevFileSystem::LookupFlow::retain_root`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::retain_root(Context &c) noexcept
@brief 保留设备根作为父目录结果。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-dd449af8a68ec031"></a>

## `xv::fs::DevFileSystem::LookupFlow::retain_self`

```doxygen
@fn static void xv::fs::DevFileSystem::LookupFlow::retain_self(Context &c) noexcept
@brief 保留输入目录作为结果。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-277e3a76262c5b37"></a>

## `xv::fs::DevFileSystem::LookupFlow::self`

```doxygen
@fn static bool xv::fs::DevFileSystem::LookupFlow::self(Context &c) noexcept
@brief 检查点目录项。
@details 具名 PTY 先取得外部 pin 再查询所有者；首个 inode 引用接管该 pin，已有引用则归还多余 pin，失败也必须归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cdbc2e77b0a9a85c"></a>

## `xv::fs::DevFileSystem::ReadFlow::clamp`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::clamp(Context &c) noexcept
@brief 将读取长度裁剪到目录末尾。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3e7d1500753b4077"></a>

## `xv::fs::DevFileSystem::ReadFlow::complete`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::complete(Context &c) noexcept
@brief 记录本次读取字节数，包括 EOF。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-34e7cae3e02fffa8"></a>

## `xv::fs::DevFileSystem::ReadFlow::copy`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReadFlow::copy(Context &c) noexcept
@brief 复制当前记录的一段并报告是否成功。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a48e2cb851a6af2e"></a>

## `xv::fs::DevFileSystem::ReadFlow::decimal_name`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::decimal_name(unsigned value, char *name) noexcept
@brief 将无符号编号直接转换为十进制名称。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-297aed2d04431e0d"></a>

## `xv::fs::DevFileSystem::ReadFlow::directory`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReadFlow::directory(Context &c) noexcept
@brief 检查输入为设备根或 pts 目录。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d064e5a6967a9d8c"></a>

## `xv::fs::DevFileSystem::ReadFlow::failure`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::failure(Context &c) noexcept
@brief 记录首段错误或此前完整字节进展。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-49127e1d0738e810"></a>

## `xv::fs::DevFileSystem::ReadFlow::in_range`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReadFlow::in_range(Context &c) noexcept
@brief 检查读取偏移是否在目录范围内。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4ce53ad06d5cb43e"></a>

## `xv::fs::DevFileSystem::ReadFlow::more`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReadFlow::more(Context &c) noexcept
@brief 检查是否还有目录字节待复制。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fd035243bed189cb"></a>

## `xv::fs::DevFileSystem::ReadFlow::prepare`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::prepare(Context &c) noexcept
@brief 构造点、点点、pts、登记设备或具名 PTY 记录，并计算记录内复制范围。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b82633aff815a45f"></a>

## `xv::fs::DevFileSystem::ReadFlow::progress`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::progress(Context &c) noexcept
@brief 累计本段成功复制的字节数。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-095b458bba37e6ba"></a>

## `xv::fs::DevFileSystem::ReadFlow::snapshot`

```doxygen
@fn static void xv::fs::DevFileSystem::ReadFlow::snapshot(Context &c) noexcept
@brief 按 pts 策略取得具名 PTY ID 快照并计算固定目录大小。
@details 目录读取先取得具名 PTY ID 快照，后续按固定目录槽位构造记录，空闲 PTY 槽仍输出零记录；逐记录复制失败保留此前进展。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-82caa8685e4ca1cc"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::drop`

```doxygen
@fn static void xv::fs::DevFileSystem::ReleaseFlow::drop(Context &c) noexcept
@brief 减少节点引用，保存是否归还 PTY pin 及其编号。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-125680a03beaba5c"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::invalid`

```doxygen
@fn static void xv::fs::DevFileSystem::ReleaseFlow::invalid(Context &) noexcept
@brief 诊断归还失效设备 inode 引用。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d34884139ee6353d"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::live`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReleaseFlow::live(Context &c) noexcept
@brief 检查节点引用有效。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cff7a738c1653db5"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::release_pty`

```doxygen
@fn static void xv::fs::DevFileSystem::ReleaseFlow::release_pty(Context &c) noexcept
@brief 在引用锁外归还具名 PTY pin。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5e193a19bac5d58e"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::unlock`

```doxygen
@fn static void xv::fs::DevFileSystem::ReleaseFlow::unlock(Context &c) noexcept
@brief 释放引用自旋锁。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0309248a65308402"></a>

## `xv::fs::DevFileSystem::ReleaseFlow::unpin`

```doxygen
@fn static bool xv::fs::DevFileSystem::ReleaseFlow::unpin(Context &c) noexcept
@brief 检查是否为具名 PTY 的最后引用。
@details 入口持引用锁，最后 PTY 引用的外部 pin 必须在释放引用锁后归还。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
