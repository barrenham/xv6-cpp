# `kernel/fs/procfs/system_providers.cpp`

<a id="fn-5d218d64f2782b2e"></a>

## `xv::fs::install_proc_providers`

```doxygen
@fn void xv::fs::install_proc_providers(ProcFileSystem &filesystem)
@brief 初始化时登记系统及PID文本提供者，失败终止启动。
@details 新增只读 /proc/bcache，展示根缓存实例的累计命中、未命中、淘汰、查找、物理 I/O 和锁等待计数及 10MHz 时基；不合并第二块盘，快照不冻结并发访问。其余 proc 提供器保留原行为。
```

<a id="fn-db10a975ebb747b4"></a>

## `xv::fs::memory`

```doxygen
@fn bool xv::fs::memory(PageBuffer &buffer)
@brief 使用真实分配器总量和空闲量输出meminfo。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-54f6a6b75d3608cc"></a>

## `xv::fs::seconds`

```doxygen
@fn void xv::fs::seconds(SnapshotText &text,unsigned long ticks)
@brief 实现 xv::fs::seconds 的模块操作，按声明参数读写其封装状态。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-dcf9a4ed75528abc"></a>

## `xv::fs::uptime`

```doxygen
@fn bool xv::fs::uptime(PageBuffer &buffer)
@brief 输出运行秒数和各CPU累计空闲采样秒数。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-a9a30ca895949623"></a>

## `xv::fs::cpu_line`

```doxygen
@fn void xv::fs::cpu_line(SnapshotText &text,const detail::CpuTime &time)
@brief 实现 xv::fs::cpu_line 的模块操作，按声明参数读写其封装状态。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-f729a8588810f9bd"></a>

## `xv::fs::statistics`

```doxygen
@fn bool xv::fs::statistics(PageBuffer &buffer)
@brief 输出各CPU采样计时、中断、调度、创建和可运行进程统计。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-a8b7e9b95c6314aa"></a>

## `xv::fs::cpuinfo`

```doxygen
@fn bool xv::fs::cpuinfo(PageBuffer &buffer)
@brief 输出实际上线hart及机器CSR可确定的ISA和实现标识。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-6c084425f8635164"></a>

## `xv::fs::filesystems`

```doxygen
@fn bool xv::fs::filesystems(PageBuffer &buffer)
@brief 输出实际注册的后端及nodev标记。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-9446d38b78040217"></a>

## `xv::fs::mounts`

```doxygen
@fn bool xv::fs::mounts(PageBuffer &buffer)
@brief 输出独立发布的挂载快照，避免与VFS拓扑锁递归。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-ab0edcb9527c0899"></a>

## `xv::fs::devices_list`

```doxygen
@fn bool xv::fs::devices_list(PageBuffer &buffer)
@brief 按字符与块设备命名空间输出登记的设备major。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-4d660bdbaf1dabd2"></a>

## `xv::fs::partitions`

```doxygen
@fn bool xv::fs::partitions(PageBuffer &buffer)
@brief 根据实际块设备容量输出KiB和主次号。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-db586c5d41bbee77"></a>

## `xv::fs::version`

```doxygen
@fn bool xv::fs::version(PageBuffer &buffer)
@brief 输出项目内核标识。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-de07421d8919c05e"></a>

## `xv::fs::command_line`

```doxygen
@fn bool xv::fs::command_line(unsigned pid,PageBuffer &buffer)
@brief 输出exec时保存的NUL分隔argv快照。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-fece7381206b86b8"></a>

## `xv::fs::environment`

```doxygen
@fn bool xv::fs::environment(unsigned pid,PageBuffer &buffer)
@brief 在权限核对后输出exec时保存的NUL分隔环境快照。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

<a id="fn-efe7e7bcb7dc8576"></a>

## `xv::fs::command_name`

```doxygen
@fn bool xv::fs::command_name(unsigned pid,PageBuffer &buffer)
@brief 核对PID仍存在后输出进程comm。
@details 动态只读信息提供者。来源为硬件、分配器、计时器及注册表；结果通过PageBuffer形成打开快照，错误不返回伪造成功。CPU计时是100ms采样并导出100Hz单位。
```

- [`xv::fs::install_proc_providers`](system_providers.cpp.md#fn-5d218d64f2782b2e)

- [`xv::fs::command_line`](system_providers.cpp.md#fn-de07421d8919c05e)

- [`xv::fs::command_name`](system_providers.cpp.md#fn-efe7e7bcb7dc8576)

- [`xv::fs::cpu_line`](system_providers.cpp.md#fn-a9a30ca895949623)

- [`xv::fs::cpuinfo`](system_providers.cpp.md#fn-a8b7e9b95c6314aa)

- [`xv::fs::devices_list`](system_providers.cpp.md#fn-ab0edcb9527c0899)

- [`xv::fs::environment`](system_providers.cpp.md#fn-fece7381206b86b8)

- [`xv::fs::filesystems`](system_providers.cpp.md#fn-6c084425f8635164)

- [`xv::fs::memory`](system_providers.cpp.md#fn-db10a975ebb747b4)

- [`xv::fs::mounts`](system_providers.cpp.md#fn-9446d38b78040217)

- [`xv::fs::partitions`](system_providers.cpp.md#fn-4d660bdbaf1dabd2)

- [`xv::fs::seconds`](system_providers.cpp.md#fn-54f6a6b75d3608cc)

- [`xv::fs::statistics`](system_providers.cpp.md#fn-f729a8588810f9bd)

- [`xv::fs::uptime`](system_providers.cpp.md#fn-dcf9a4ed75528abc)

- [`xv::fs::version`](system_providers.cpp.md#fn-db586c5d41bbee77)

<a id="fn-3b55e24fadddceba"></a>

## `xv::fs::block_cache_statistics`

```doxygen
@fn bool xv::fs::block_cache_statistics(PageBuffer &buffer)
@brief 新增只读 /proc/bcache，展示根缓存实例的累计命中、未命中、淘汰、查找、物理 I/O 和锁等待计数及 10MHz 时基；不合并第二块盘，快照不冻结并发访问。其余 proc 提供器保留原行为。
@details 新增只读 /proc/bcache，展示根缓存实例的累计命中、未命中、淘汰、查找、物理 I/O 和锁等待计数及 10MHz 时基；不合并第二块盘，快照不冻结并发访问。其余 proc 提供器保留原行为。
```

- [`xv::fs::block_cache_statistics`](system_providers.cpp.md#fn-3b55e24fadddceba)
