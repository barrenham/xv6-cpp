# `kernel/fs/procfs/provider_registry.cppm`

<a id="fn-cb80a780d0d6b5d6"></a>

## `xv::fs::ProcRegistry::add`

```doxygen
@fn bool xv::fs::ProcRegistry::add(const char *name,Render render,unsigned mode=0444)
@brief 校验provider名称、只读权限和容量后登记，拒绝重复及保留名称。
@details 固定32项provider注册表；初始化完成后seal，禁止运行期修改。只接受非重复只读名字，PID数字目录及self保留。
```

<a id="fn-f84fc98dfa1aab32"></a>

## `xv::fs::ProcRegistry::find`

```doxygen
@fn int xv::fs::ProcRegistry::find(const char *name) const
@brief 在只读provider表内按组件名字查找稳定索引。
@details 固定32项provider注册表；初始化完成后seal，禁止运行期修改。只接受非重复只读名字，PID数字目录及self保留。
```

<a id="fn-767e78ffed0a42d3"></a>

## `xv::fs::ProcRegistry::count`

```doxygen
@fn unsigned xv::fs::ProcRegistry::count() const
@brief 读取已登记provider数量。
@details 固定32项provider注册表；初始化完成后seal，禁止运行期修改。只接受非重复只读名字，PID数字目录及self保留。
```

<a id="fn-5bbd326f8379112d"></a>

## `xv::fs::ProcRegistry::at`

```doxygen
@fn const Entry &xv::fs::ProcRegistry::at(unsigned index) const
@brief 读取指定索引的provider记录；索引由调用方保证有效。
@details 固定32项provider注册表；初始化完成后seal，禁止运行期修改。只接受非重复只读名字，PID数字目录及self保留。
```

<a id="fn-1fee57612344e71d"></a>

## `xv::fs::ProcRegistry::seal`

```doxygen
@fn void xv::fs::ProcRegistry::seal()
@brief 禁止后续修改provider注册表。
@details 固定32项provider注册表；初始化完成后seal，禁止运行期修改。只接受非重复只读名字，PID数字目录及self保留。
```
