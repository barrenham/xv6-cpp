# procfs 与 ps/free

系统在 `/proc` 挂载只读 procfs，把系统与进程信息提供给普通文件接口。`ps`、`free` 只读取 procfs，不新增专用系统调用。

| 路径 | 内容 |
| --- | --- |
| `/proc/meminfo` | MemTotal、MemFree、MemAvailable、MemUsed，单位 kB |
| `/proc/uptime` | 启动后秒数，格式为小数点后两位 |
| `/proc/PID/status` | Name、State、Pid、PPid、Pgid、Sid、VmSize |
| `/proc/self` | 查找时解析为当前进程的 PID 目录 |
| `/proc/PID/fd` | 已发布描述符的只读目录 |
| `/proc/PID/fd/N` | 描述符编号、打开实例标识、标志、种类和节点身份 |
| `/proc`、`/proc/PID` | 可用 ls/read 浏览的 dirent 目录 |

```sh
ps
free
cat /proc/meminfo
cat /proc/uptime
cat /proc/self/status
ls /proc/self/fd
cat /proc/self/fd/0
ls /proc
```

## 打开时快照

每个独立 open 使用按需增长的 PageBuffer 页链生成内容，read 在固定内容上推进偏移。短读不会重复生成内容，复制失败不推进偏移，fstat 报告本次快照长度。dup/fork 共享同一份内容和偏移，最后关闭归还全部页面。

打开的 status 即使对应进程退出并被回收，也能读完原内容；后续新 open 会失败。快照只保存复制的数据，不保存 Process 指针。根目录同样按每次打开生成，ps 允许枚举到的进程在读取 status 前退出并跳过它。

快照可以跨任意数量的物理页，目录不再受 4 KiB 或 252 个 PID 限制。增长时分配失败会回滚整份快照，不发布截断目录。动态节点缓存仍有 256 个槽，已被引用的槽不会复用；枚举很多进程只占用根目录节点，逐项查询无需同时保留所有 PID 节点。目录 ABI 保留 xv6 的 16 字节 dirent，名称解析按实际 PID 完成。

`self` 是动态目录别名，进入时绑定具体 PID；已经打开的目录或 cwd 不随之后的 fork 改为其他 PID。`fd/N` 是只读信息文件，输出 Identity、Flags、Type、Device、Inode 等字段，用于观察资源。当前不提供符号链接、readlink 或通过该路径复制目标 fd 的功能。

同一 File 的 dup/fork 别名拥有相同 Identity，独立 open 或 fd 槽复用产生新的 Identity。管道、终端和 socket 也有种类与打开实例标识，不暴露内核对象地址。字段中的节点编号表示资源身份，不能唯一表示一个文件路径，因为文件可以有多个硬链接或已经被 unlink。

procfs 禁止写打开、截断、创建、链接和删除，`/proc` 挂载点也受到 VFS 删除保护。相对路径和根目录的 `..` 使用统一 VFS 规则。

## 信息来源

进程信息通过 `snapshot_after` 服务复制。服务取得家族锁保护父指针，再由进程存储逐项加锁，取得名称和状态，组与会话通过组锁查询。回调不睡眠，也不把内部指针交给文件系统。

描述符观测按存储锁、进程锁、描述符表锁的顺序短暂复制已发布 FileInfo。File 绑定完成前的 fd 不进入结果；表锁阻止对应引用被撤下，因此无需额外 retain/release，不会在生成 procfs 内容时执行设备 close 或触发嵌套磁盘事务。各资源的共享标志以原子方式读取。

地址空间由拥有线程继续维护；成功修改后原子发布一个独立的可观测大小。procfs 读取该值，不并发读取页表或拥有线程的普通 size 字段。名称的 exec 更新也使用进程锁，与观测互斥。

MemTotal 是分配器管理的 usable 物理页总量，排除内核、保留区和分配器元数据。两个分配器都按成功分配与归还更新原子空闲页计数；大块分配按页数计量，分裂已分配块不改变空闲总量。MemAvailable 当前等于 MemFree，不估计可回收缓存。VmSize 是进程虚拟地址空间大小，与物理页使用量不同。

uptime 来自系统 TickClock，当前 QEMU virt 每 tick 为 100 ms。

## 验证

`procfstest` 验证内存字段、短读、错误用户地址、只读规则、子进程退出后已有快照存续、共享偏移、相对路径、self、描述符种类和标志、fd 复用、并发关闭以及重复打开。额外创建 300 个子进程，验证目录快照超过 4 KiB 后仍完整。原生 fs_contract 还检查快照更新、复制失败和页面回收；两个 allocator_contract 检查分配、耗尽、碎片归还后的页数统计。

单核/五核集成测试：`ctest --test-dir build/dev -R 'procfs_[15]cpu' --output-on-failure`。函数说明见 [procfs 实现](functions/kernel/fs/procfs/procfs.cpp.md)。

## 用户与权限

status 包含 Uid/Euid/Gid/Egid。fd 目录为目标有效身份的 0500，fd 信息为 0400；实时描述符检查仅允许 root，或有效 UID 同时匹配目标真实及有效 UID。动态节点缓存键包含身份，已打开快照保留打开时内容。

完整规则见 [用户、组与权限](users-permissions.md)。
