# `kernel/io/fs_resources.h`

类型化资源能力的 Version 2 ABI。`QueryIoctl` 传入 `Query` 的用户地址；消息包含操作、索引、目标 PID、偏移、计数、原请求 inspector UID 及 512 字节载荷。后端根据原始记录生成自己的目录和文本，内核提供数据与访问检查。

`ProcessAfter`、`Cpu`、`System`、`Mount`、`Provider`、`Filesystem`、`Cache` 和设备/PTY 操作查询当前机制状态。fd 和 exec 操作先捕获独立快照，再按索引或偏移读取；克隆能力拥有独立捕获状态。关闭能力时释放持有的 PTY 固定引用。

`CloneIoctl` 返回独立资源能力描述符。`StorageIoctl` 的参数是盘索引标量（0 或 1），返回独占可写原始盘描述符；`ManagerIoctl` 返回挂载管理通道。后两项还检查当前进程有效 UID 为 0。普通应用不会从常规 init 继承这些引导能力。

详见[文件系统服务迁移](../../../fs-service-migration.md)。
