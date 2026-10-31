# procfs 与 ps/free

系统在 `/proc` 挂载只读 procfs，把系统与进程信息提供给普通文件接口。`ps`、`free` 只读取 procfs，不新增专用系统调用。

| 路径 | 内容 |
| --- | --- |
| `/proc/meminfo` | MemTotal、MemFree、MemAvailable、MemUsed，单位 kB |
| `/proc/uptime` | 启动后秒数、各核空闲秒数之和，均保留两位小数 |
| `/proc/stat` | 总计及逐核 CPU 时间采样、已处理的中断、调度切换、累计进程创建与当前可运行进程数 |
| `/proc/cpuinfo` | 在线 hart、misa 单字母扩展和硬件标识寄存器 |
| `/proc/filesystems` | 已装配的文件系统类型及 nodev 属性 |
| `/proc/mounts` | 已发布挂载点、规范类型名和只读属性 |
| `/proc/devices` | 分别列出字符设备、块设备的主设备号及名称 |
| `/proc/partitions` | 已登记块设备的实际容量（KiB）及主、次设备号 |
| `/proc/version` | 当前内核版本及构建编译器 |
| `/proc/PID/status` | Name、State、Pid、PPid、Pgid、Sid、VmSize |
| `/proc/self` | 查找时解析为当前进程的 PID 目录 |
| `/proc/PID/cmdline` | exec 时的参数，逐项以 NUL 分隔 |
| `/proc/PID/environ` | exec 时的环境变量，逐项以 NUL 分隔 |
| `/proc/PID/comm` | 进程名称和换行 |
| `/proc/PID/fdinfo/N` | 共享文件偏移、八进制打开标志和 inode 编号 |
| `/proc/PID/fd` | 已发布描述符的只读目录 |
| `/proc/PID/fd/N` | 描述符编号、打开实例标识、标志、种类和节点身份 |
| `/proc`、`/proc/PID` | 可用 ls/read 浏览的 dirent 目录 |

```sh
ps
free
cat /proc/meminfo
cat /proc/uptime
cat /proc/cpuinfo
cat /proc/filesystems
cat /proc/mounts
cat /proc/self/status
ls /proc/self/fd
cat /proc/self/fd/0
ls /proc
```

## 信息提供者

根目录信息文件通过 `ProcProviderRegistry` 注册名称、只读权限和快照生成
函数。PID 下的信息文件通过相同模板的 `ProcProcessRegistry` 注册。注册表复制名称，拒绝重复名称及 PID/self 保留名称，并在挂载发布前
封闭。增加信息文件无需修改逐个文件名判断的路径解析状态机。共同的
`SnapshotText` 格式化器管理输出分配失败，动态 PID/fd 查找继续由 procfs
自己的状态机处理。

CPU 标识在机器模式读取，完成 supervisor 初始化的 hart 才标为在线；
supervisor 下读取快照不会访问机器权限 CSR。misa 的单字母扩展位依据
[RISC-V 特权规范](https://docs.riscv.org/reference/isa/priv/machine.html)，
当前不补造多字母扩展或未知微架构名称。`filesystems` 取自服务层装配的
后端对象；`mounts` 读取 VFS 发布的副本，不在持有 proc inode 锁时获取
VFS 命名空间锁。挂载增删同步更新该副本，避免两类锁发生顺序反转。

## 打开时快照

每个独立 open 使用按需增长的 PageBuffer 页链生成内容，read 在固定内容上推进偏移。短读不会重复生成内容，复制失败不推进偏移，fstat 报告本次快照长度。dup/fork 共享同一份内容和偏移，最后关闭归还全部页面。

打开的 status 即使对应进程退出并被回收，也能读完原内容；后续新 open 会失败。快照只保存复制的数据，不保存 Process 指针。根目录同样按每次打开生成，ps 允许枚举到的进程在读取 status 前退出并跳过它。

快照可以跨任意数量的物理页，目录不再受 4 KiB 或 252 个 PID 限制。增长时分配失败会回滚整份快照，不发布截断目录。inode 缓存共有 256 个槽，其中 33 个保留给根节点和可注册信息文件，其余动态复用；已被引用的槽不会复用。枚举很多进程只占用根目录节点，逐项查询无需同时保留所有 PID 节点。目录 ABI 保留 xv6 的 16 字节 dirent，名称解析按实际 PID 完成。

`self` 是动态目录别名，进入时绑定具体 PID；已经打开的目录或 cwd 不随之后的 fork 改为其他 PID。`fd/N` 是只读信息文件，输出 Identity、Flags、Type、Device、Inode 等字段，用于观察资源。当前不提供符号链接、readlink 或通过该路径复制目标 fd 的功能。

同一 File 的 dup/fork 别名拥有相同 Identity，独立 open 或 fd 槽复用产生新的 Identity。管道、终端和 socket 也有种类与打开实例标识，不暴露内核对象地址。字段中的节点编号表示资源身份，不能唯一表示一个文件路径，因为文件可以有多个硬链接或已经被 unlink。

procfs 禁止写打开、截断、创建、链接和删除，`/proc` 挂载点也受到 VFS 删除保护。相对路径和根目录的 `..` 使用统一 VFS 规则。

## 参数与环境

ELF 装载器在提交新映像前复制路径、参数和环境到不可变页链；分配失败会保留原映像和原信息。成功提交时名称与快照在进程锁内一起发布，失败 exec 不改变它们。fork 仅增加引用计数，不逐页复制这些字符串；最后一个持有者退出时归还页面。

`cmdline` 和 `environ` 保留 NUL 分隔，不能按普通换行文本理解。这里显示 exec 时的输入；之后程序修改 argv 内存或用户态环境不会改变该快照。打开 proc 文件时再复制到独立 PageBuffer，所以目标退出、回收后，已打开文件仍可读完。读取只允许 root，或有效 UID 同时匹配目标真实及有效 UID；不会读取目标正在修改的用户页表。

## 信息来源

进程信息通过 `snapshot_after` 服务复制。服务取得家族锁保护父指针，再由进程存储逐项加锁，取得名称和状态，组与会话通过组锁查询。回调不睡眠，也不把内部指针交给文件系统。

描述符观测按存储锁、进程锁、描述符表锁的顺序短暂复制已发布 FileInfo。File 绑定完成前的 fd 不进入结果；表锁阻止对应引用被撤下，因此无需额外 retain/release，不会在生成 procfs 内容时执行设备 close 或触发嵌套磁盘事务。各资源的共享标志及已发布文件偏移以原子方式读取，不在描述符表锁内获取 inode 睡眠锁。`fdinfo/N` 的 `pos` 为字节偏移，`flags` 为本内核打开标志的八进制表示（位值仍使用本内核 ABI），`ino` 为节点编号；不补造尚未维护的 mount ID。dup/fork 共享同一打开实例，因此新打开的 fdinfo 能观察共享 seek/read/write 的进展。

地址空间由拥有线程继续维护；成功修改后原子发布一个独立的可观测大小。procfs 读取该值，不并发读取页表或拥有线程的普通 size 字段。名称的 exec 更新也使用进程锁，与观测互斥。

MemTotal 是分配器管理的 usable 物理页总量，排除内核、保留区和分配器元数据。两个分配器都按成功分配与归还更新原子空闲页计数；大块分配按页数计量，分裂已分配块不改变空闲总量。MemAvailable 当前等于 MemFree，不估计可回收缓存。VmSize 是进程虚拟地址空间大小，与物理页使用量不同。

uptime 来自系统 TickClock，当前 QEMU virt 每 tick 为 100 ms。第二列是
在线 hart 定时采样的空闲时间之和，多核下可超过墙钟启动时间。

`stat` 的 CPU 行按 Linux 的 1/100 秒单位输出 user、nice、system、idle
四列；定时器实际每 100 ms 采样一次，因此前三类维护的计数乘以 10。
从用户态陷入的定时中断计 user；内核态且有当前进程计 system；调度器
没有当前进程时计 idle。nice 恒为 0，因为当前没有 nice 时间类别。
这些是采样时间，不代表纳秒级执行时间，也不拆分尚未独立维护的
irq、softirq、iowait、steal 类别。

`intr` 累加各核实际处理的定时和外部中断；`ctxt` 在调度器切换进入进程
时递增；`processes` 累计初始进程及成功 fork，失败 fork 不增加；
`procs_running` 枚举当前 Running/Runnable 状态。各计数原子读取，但不会
暂停其他 CPU，因此整个文件不承诺同一个全局瞬间的一致性。
`btime` 来自启动时读取的 Goldfish RTC epoch，读取一对低、高寄存器时
用自旋锁保护设备的共享高位锁存。读取顺序依据
[QEMU 设备实现](https://github.com/qemu/qemu/blob/master/hw/rtc/goldfish_rtc.c)，
寄存器布局也与 [Linux RTC 驱动](https://github.com/torvalds/linux/blob/master/drivers/rtc/rtc-goldfish.c)核对。
尚未维护独立 I/O 等待计数，暂不输出 procs_blocked。

## 验证

`procfstest` 验证内存字段、短读、错误用户地址、只读规则、子进程退出后已有快照存续、共享偏移、相对路径、self、描述符种类和标志、fd 复用、并发关闭以及重复打开。额外创建 300 个子进程，验证目录快照超过 4 KiB 后仍完整。原生 fs_contract 还检查快照更新、复制失败和页面回收；两个 allocator_contract 检查分配、耗尽、碎片归还后的页数统计。

单核/五核集成测试：`ctest --test-dir build/dev -R 'procfs_[15]cpu' --output-on-failure`。函数说明见 [procfs 实现](functions/kernel/fs/procfs/procfs.cpp.md)。

## 用户与权限

status 包含 Uid/Euid/Gid/Egid。fd/fdinfo 目录为目标有效身份的 0500，fd 信息及 cmdline/environ 为 0400；实时描述符检查仅允许 root，或有效 UID 同时匹配目标真实及有效 UID。动态节点缓存键包含身份，已打开快照保留打开时内容。

完整规则见 [用户、组与权限](users-permissions.md)。
