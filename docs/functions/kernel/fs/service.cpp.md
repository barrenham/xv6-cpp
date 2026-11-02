# `kernel/fs/service.cpp`

开发职责：装配通用 VFS、文件表、设备注册表及原始块缓存；文件系统算法由用户态服务实现。

源码：[打开实际文件](../../../../kernel/fs/service.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:lru`, `:vfs`, `:platform`, `xv.print`。生产内核不再装配 ext3/xv6、ramfs、procfs 或 devfs 的旧内核后端。

## 函数导航

- [`xv::fs::cache`](service.cpp.md#fn-8340fd0d2696b2e9)
- [`xv::fs::devices`](service.cpp.md#fn-94c869e5070c351f)
- [`xv::fs::files`](service.cpp.md#fn-f6456e38e2f03d00)
- [`xv::fs::filesystem`](service.cpp.md#fn-33ca000fe0c21f95)
- [`xv::fs::initialize_cache`](service.cpp.md#fn-6945466a2423e97b)
- [`xv::fs::journal`](service.cpp.md#fn-fde437052d31cdd9)

<a id="fn-8340fd0d2696b2e9"></a>

## `xv::fs::cache`

```doxygen
@fn BlockCache &xv::fs::cache()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_cache`，返回当前查询结果。
@return 各分支实际返回：`system_cache`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-6945466a2423e97b"></a>

## `xv::fs::initialize_cache`

```doxygen
@fn void xv::fs::initialize_cache()
@brief 初始化原始块缓存、设备机制和通用用户态 VFS 装配。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-f6456e38e2f03d00"></a>

## `xv::fs::files`

```doxygen
@fn OpenFileTable &xv::fs::files()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_files`，返回当前查询结果。
@return 各分支实际返回：`system_files`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-94c869e5070c351f"></a>

## `xv::fs::devices`

```doxygen
@fn DeviceRegistry &xv::fs::devices()
@brief 文件描述、inode、路径、块缓存与日志事务：读取或计算 `system_devices`，返回当前查询结果。
@return 各分支实际返回：`system_devices`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-33ca000fe0c21f95"></a>

## `xv::fs::filesystem`

```doxygen
@fn VirtualFileSystem &xv::fs::filesystem()
@brief 取得统一路径命名空间的 VFS 实例。
@details 返回默认处于服务模式的 VirtualFileSystem。根服务在用户态引导期间发布；节点 owner 指向通用桥接实例，经协议转交独立服务。VFS 只负责路径、挂载与通用节点契约。
```

<a id="fn-fde437052d31cdd9"></a>

## `xv::fs::journal`

```doxygen
@fn Journal &xv::fs::journal()
@brief 返回仅保留调用边界的服务模式事务适配器。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-2764ac52db897bd0"></a>

## `xv::fs::detail::registered_filesystem`

```doxygen
@fn bool xv::fs::detail::registered_filesystem(unsigned index,detail::FileSystemType &record)
@brief 枚举通用桥接和已注册用户态类型。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-9da4875b4cc7f28f"></a>

## `xv::fs::create_userfs`

此函数已移至 [通用注册表](userfs/registry.cpp.md)；保留本锚点供旧链接访问。实际源码是 [registry.cpp](../../../../kernel/fs/userfs/registry.cpp)。

```doxygen
@fn File *xv::fs::create_userfs()
@brief 分配一个由独立用户态服务处理请求的双向文件描述。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-382424c9e0c25947"></a>

## `xv::fs::mount_userfs`

此函数已移至 [通用注册表](userfs/registry.cpp.md)；保留本锚点供旧链接访问。实际源码是 [registry.cpp](../../../../kernel/fs/userfs/registry.cpp)。

```doxygen
@fn bool xv::fs::mount_userfs(File &file, const char *path)
@brief 将已保留的服务实例接入现有挂载目录。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

- [`xv::fs::create_userfs`](service.cpp.md#fn-9da4875b4cc7f28f)

- [`xv::fs::mount_userfs`](service.cpp.md#fn-382424c9e0c25947)

<a id="fn-9d2848029ed8f189"></a>

## `xv::fs::bootstrap_cache`

```doxygen
@fn BlockCache *xv::fs::bootstrap_cache(unsigned index)
@brief 查询供原始块能力使用的指定盘缓存。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-cf5815fec1a8ac11"></a>

## `xv::fs::ServiceJournal::begin`

```doxygen
@fn void xv::fs::ServiceJournal::begin() override
@brief 保留调用方事务边界，不启动内核磁盘日志。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-ef5b68e12b33e47f"></a>

## `xv::fs::ServiceJournal::end`

```doxygen
@fn void xv::fs::ServiceJournal::end() override
@brief 结束兼容事务边界，不提交用户态日志。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-ae6d52335f798732"></a>

## `xv::fs::ServiceJournal::record`

```doxygen
@fn void xv::fs::ServiceJournal::record(Buffer &) override
@brief 拒绝把生产磁盘修改记录到已移除的内核日志。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

<a id="fn-a60de63cfa230518"></a>

## `xv::fs::ServiceJournal::sync`

```doxygen
@fn void xv::fs::ServiceJournal::sync() override
@brief 不在内核执行后端日志同步。
@details 生产配置仅装配 VFS、通用用户态桥接、设备能力和原始块缓存。ServiceJournal 保留内核调用点的事务边界但不实现磁盘日志，record 误用触发 panic；ext3 JBD 和 xv6 redo 在各自用户进程提交。旧内核文件系统类仅作为主机合同测试参考实现编译。
```

- [`xv::fs::bootstrap_cache`](service.cpp.md#fn-9d2848029ed8f189)

- [`xv::fs::detail::registered_filesystem`](service.cpp.md#fn-2764ac52db897bd0)

- [`xv::fs::ServiceJournal::begin`](service.cpp.md#fn-cf5815fec1a8ac11)

- [`xv::fs::ServiceJournal::end`](service.cpp.md#fn-ef5b68e12b33e47f)

- [`xv::fs::ServiceJournal::record`](service.cpp.md#fn-ae6d52335f798732)

- [`xv::fs::ServiceJournal::sync`](service.cpp.md#fn-a60de63cfa230518)
