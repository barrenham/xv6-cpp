# `kernel/fs/userfs/registry.cpp`


<a id="fn-f30bd6e2ab01ca2c"></a>

## `xv::fs::initialize_userfs_plugins`

```doxygen
@fn void xv::fs::initialize_userfs_plugins()
@brief 在启动期初始化通用插件注册锁。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-cd43bdd5477a298a"></a>

## `xv::fs::mount_registered_userfs`

```doxygen
@fn int xv::fs::mount_registered_userfs(const char *type, const char *path)
@brief 按动态类型选择待挂载服务，释放注册锁后执行路径挂载。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-882968ffadba4034"></a>

## `xv::fs::register_userfs`

```doxygen
@fn File *xv::fs::register_userfs(const xv::userfs::Registration &registration)
@brief 验证通用协议描述符并注册独立用户态 FS 实例，返回服务 File。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-0c4cc1076e132b14"></a>

## `xv::fs::userfs_provider`

```doxygen
@fn bool xv::fs::userfs_provider(unsigned index, xv::userfs::ProviderInfo &info)
@brief 复制一个活动插件实例的状态快照。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-50e3b4205e956533"></a>

## `xv::fs::userfs_type`

```doxygen
@fn bool xv::fs::userfs_type(unsigned index, char *name)
@brief 复制去重后的活动动态类型名，不返回可回收对象中的指针。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-9d242cb7b9e59a71"></a>

## `xv::fs::collect`

```doxygen
@fn void xv::fs::collect()
@brief 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-bdbdbf21f29604f5"></a>

## `xv::fs::create`

```doxygen
@fn File *xv::fs::create(const xv::userfs::Registration &registration, bool anonymous)
@brief 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

<a id="fn-ae110941476e9937"></a>

## `xv::fs::valid`

```doxygen
@fn bool xv::fs::valid(const xv::userfs::Registration &r)
@brief 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
@details 通用运行时插件注册表，不按 memfsd/tarfsd 等具体实现分支。校验长度/版本/类型名/根/能力后按需分配实例；最多 16 个活动或尚未清理实例，同名端点可承载独立挂载。注册表自旋锁仅保护选择、快照与摘除，不持锁查路径或做 I/O。claim_mount 固定候选，失败后恢复待挂载；关闭且未挂载、未被选择的实例才可摘除，锁外释放所有页。名称快照复制到调用者缓冲区，避免回收后的悬挂指针。
```

- [`xv::fs::create_userfs`](../service.cpp.md#fn-9da4875b4cc7f28f)

- [`xv::fs::initialize_userfs_plugins`](registry.cpp.md#fn-f30bd6e2ab01ca2c)

- [`xv::fs::mount_registered_userfs`](registry.cpp.md#fn-cd43bdd5477a298a)

- [`xv::fs::mount_userfs`](../service.cpp.md#fn-382424c9e0c25947)

- [`xv::fs::register_userfs`](registry.cpp.md#fn-882968ffadba4034)

- [`xv::fs::userfs_provider`](registry.cpp.md#fn-0c4cc1076e132b14)

- [`xv::fs::userfs_type`](registry.cpp.md#fn-50e3b4205e956533)

- [`xv::fs::collect`](registry.cpp.md#fn-9d242cb7b9e59a71)

- [`xv::fs::create`](registry.cpp.md#fn-bdbdbf21f29604f5)

- [`xv::fs::valid`](registry.cpp.md#fn-ae110941476e9937)
