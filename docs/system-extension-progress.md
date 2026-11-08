> 更新：本文记录早期阶段验收。FS 进程池及快照读路径现已移除，当前实现统一采用
> pthread 工作池，详见 [统一 FS 线程池](fs-persistent-workers.md)。下方进程池描述为历史记录。

# 设备、缓存、IPC、FS 并发与服务恢复

本轮五项完整目标已实现，并在整合后的内核、引导归档和测试镜像上完成最终回归。
以下历史阶段记录用于说明验证顺序；最终验收以本文末尾的整合记录为准。

| 项目 | 当前实现状态 | 最终验证 |
| --- | --- | --- |
| 驱动匹配与依赖 | ABI 3 匹配表、PCI 身份/DT compatible、依赖排序和延迟自动绑定；保留 V1/V2 | 原生匹配/DT/驱动契约、单/五核真实 ELF 依赖、旧 ABI 在线加载通过 |
| 全局文件缓存回收 | 全局闲置/干净映射页回收、1024 页目标与 512 页空闲低水位、物理页引用和睡眠进程页表清理、分配失败回收重试、proc 统计 | FS/两种分配器契约、单/五核容量及低内存压力验证通过 |
| 共享内存生命周期 | mmap 对象租约、EOF/SIGBUS、跨 hart 截断同步、关闭/删除名称后的映射寿命、重新增长清零 | 单/五核 memory_mapping 及 POSIX IPC 通过 |
| FS 后端并发 | 常驻读写工作池、共享存储、ramfs workers 模式、同 inode 读写互斥与全局元数据屏障 | 单/五核共享堆、稳定 PID、读写重叠、故障取消及实际 ramfs 验证通过 |
| 服务恢复与日志 | 独立服务监护、管理器接管、核心登记原子保存、未配置存活服务发现、管道日志轮转、bootstrap 自动恢复 | 整合后的单/五核完整故障与日志尾部检查通过 |

## 已有阶段证据

- build/next-five-driver-validation.log 中的 device_elf_boundary、device_tree_contract、
  driver_matching_contract、kernel_driver_boot_1cpu/5cpu、memory_mapping_1cpu/5cpu 通过。
  当轮新依赖测试的工具链参数和断言问题随后修正。
- build/next-five-dependencies-complete.log 的两项测试通过：真实 ABI 3 ELF 在消费者
  先列出的启动清单下，先绑定提供者；运行时补齐依赖后自动绑定消费者，检查厂商
  不匹配、循环/非法描述拒绝和设备身份查询。此后又增加了重复依赖和坏指针/数量
  用例；增加用例后的两个 CPU 配置已在下述缓存基础回归中重新通过。
- build/next-five-compatibility.log 的五项通过：旧 ABI 1 在线独立构建/加载/绑定、
  实际驱动契约、POSIX IPC、单核/五核内存映射。共享内存用例包含空对象映射、
  纯用户循环中的远端截断、O_TRUNC、内核拷贝拒绝越界、部分解除映射、fork、
  同名重新创建隔离和 EOF 填充区存储后的重新增长。

以上三份日志先于全局缓存回收修改，不作为该修改的通过证明。

build/next-five-cache-final-build.log 的当前内核/镜像构建成功。
build/next-five-cache-foundation-validation.log 中 FS 契约、两种分配器 VM 契约、
新增全部坏描述/循环用例的单核/五核驱动依赖检查，以及单核 system_extensions
通过。新增原生检查证明闲置页全局回收、映射租约保护、从真实 RAM 后端重读，
以及物理分配失败后的回收/重试和精确页面归还。

该轮五核 system_extensions 的功能均执行到最后，日志前缀与异步 shell 提示符
交错导致文本等待超时；改为检查稳定后缀，仍检查进程消失和控制端失联状态。
build/next-five-cache-recheck.log 的五核复验通过。此处尚未实现管理器恢复，
杀死管理器后报告 unavailable 仍是旧行为，不能视为第⑤项已完成。

## 映射页回收阶段

build/mapped-reclaim-formatted-build.log 的内核及测试镜像构建成功。
build/mapped-reclaim-native.log 的 FS 契约及两种物理分配器 VM 契约通过。
新增用例验证帧撤销标志与引用计数、最终释放、独占 COW 收养、跨地址空间
清理旧 PTE、重新加载和私有修改保留；FS 用例还验证真实 RAM 后端的重读、
缺页过程持有物理页引用，以及可写共享页不被丢弃。

build/mapped-reclaim-qemu.log 的单核/五核内存映射检查通过，包含 6 个
1 MiB 文件、父子进程映射、睡眠子进程、全局水位、待释放页归零和重读校验。
随后增加低内存检查：用 eager sbrk 逐步占用空闲内存，在缓存尚未超额时
检查映射页撤销次数增长、缓存持有页下降及待释放页归零，释放压力内存后
验证父子私有 COW 内容与所有文件数据。该版本的单核、五核检查均已在
build/mapped-reclaim-pressure-validation.log 通过；该日志的 FS/两种 VM 契约及
单核/五核 system_extensions 也通过，共 7/7。服务测试此时仍验证既有行为，
不证明第⑤项的管理器接管及日志轮转已完成。

私有修改页及可写共享文件页保留；未实现 swap 或共享脏页的后台写回回收。
回收路径及 `/proc/meminfo` 字段见 [内存映射](memory-mapping.md)。

追加的并发原生用例让一个线程阻塞在真实 AddressSpace 的文件缺页/用户复制
路径，另一线程撤销已有文件帧并尝试后台清理：复制期间必须跳过该地址空间，
复制完成后才释放待回收页表引用。build/mapped-reclaim-copy-guard-build.log
构建成功，build/mapped-reclaim-copy-guard-validation.log 中 Buddy/FreeList
两个版本均通过，结束时所有物理页精确归还。

运行测试使用独立磁盘副本。本轮尚未更新用户的持久根盘，也未提交或推送。

## 服务恢复阶段

实现与故障边界见 [服务接管与日志](service-recovery.md)。
build/service-recovery-mapped-arguments-build.log 的内核/镜像构建成功，
build/service-recovery-outage-build.log 安装了最终加严的服务测试到测试镜像。

build/service-recovery-verified-validation.log 中的 system_extensions_1cpu/5cpu
均通过。servicetest 实际强杀并重启管理器，验证原应用/监护 PID 不变、停止状态、
已完成 oneshot、耗尽的失败预算、外部依赖身份恢复；管理器停机期间再杀应用，
由原监护进程完成重启，新管理器接管该 PID 及重试计数。监护进程也被强杀时，
新管理器拒绝重复启动尚存活的旧应用，测试确认旧应用终止后才删除故障端点。
还检查配置移除后的服务发现和控制、第二管理器被拒绝、权限和进程清理。

日志测试覆盖管理器停机前/中/后的 stdout/stderr 批次，通过实际捕获字节计数
确定收集完成，再逐字节校验归档和当前文件的有序尾部，检查 1024 字节上限、
两份归档以及轮转次数。启动器故障测试检查新管理器 PID、核心控制状态和
64 KiB 管理器诊断日志的归档。该复验不依赖可能被 UART 提示符打断的诊断文本。

监护进程使用独立 16 KiB 映射栈及 guard 页；为其补齐信号进入/返回、exec
argv/envp 的映射地址检查。build/service-recovery-signal-memory-validation.log
的 signals_1cpu/5cpu、memory_mapping_1cpu/5cpu 共 4 项通过；新增映射参数
检查包含高地址 argv/envp 成功执行、PROT_NONE 拒绝、指针字跨不可读页拒绝。
服务测试还实际向映射栈上的监护进程发 TERM，验证处理函数返回后先停止应用
再退出。此阶段的早期栈越界和测试时序问题已修正，以上最终日志作为通过依据。

## 常驻 FS 工作池与最终整合验收

保留旧的逐请求 fork 读者作为兼容模式，新增 concurrent_workers/共享存储
模式；[FS 常驻并发工作池](fs-persistent-workers.md)说明使用和状态共享契约。
ramfs 的 `workers=2` 在启动时创建两名工作进程，重复处理不同 inode 的读写。
同 inode 写入独占，元数据操作和维护为全局屏障。工作进程继承 FS 服务身份，
避免低内存慢速回收递归等待自身 RPC。任一工作进程故障会取消整个池，清理
共享内存时不遍历可能处于修改中间状态的分配器和节点链表。

本次最终构建为 build/fs-persistent-integration-build.log，内核/测试镜像成功。
随后源代码未改变，最终检查共 23 项，全部通过：

| 记录 | 通过数 | 覆盖 |
| --- | --- | --- |
| build/fs-persistent-integration-validation.log | 2/2 | 单/五核 sharedheap、fspooltest、实际 workers=2 ramfs 的 demandcache/filemap/fscache 与 e2fsck |
| build/five-extensions-final-native.log | 8/8 | process_storage、fs、driver、Buddy/FreeList VM、userfs_memory、device_tree、driver_matching 契约 |
| build/five-extensions-final-integration.log | 13/13 | device_elf/fs_runtime 边界；单/五核驱动依赖、信号、memory_mapping、system_extensions；五核驱动热加载、独立 FS SDK 热加载、POSIX IPC |

fspooltest 通过常驻 PID 集合和处理函数进入/完成同步证明并行关系，不依赖耗时
门槛。它还校验重复请求使用同一进程、同文件写入不重叠、元数据等待活跃写入，
并在两个写入处理中强杀一个工作进程，确认未完成调用失败、另一个进程退出和
后端失败回调执行。实际 ramfs 测试覆盖 fork 后新建节点的共享状态、缓存与 mmap
一致性、重命名、权限及释放。独立 SDK 热加载测试在客体启动后导出并编译新
FS ELF，经网络安装到同一次运行的内核，确保新增模块/汇编入口正确导出。

单/五核 memory_mapping 包含多进程共享内存远端截断、SIGBUS、对象租约与
重新增长清零，并检查低内存压力下映射页真正归还、再次缺页加载和私有 COW
保留。system_extensions 在最终版本再次验证管理器接管与日志轮转完整流程。

验收产物 SHA-256：

```text
kernel.elf       02b856445ce408ddd0c4987e1cb102d3a24c81590f66f03dc6cbb61709f626e8
image/_fsboot    52ffc8cb92d57959257df4c4469b11c7f328a4ebf5e2831764b862c9da12d2b5
fs-test.img     fbf34505e731df75bc688abdb4285f47585bd6b531cd32d09b2e5806a9e03319
```

边界：没有运行 fork 性能门禁；不承诺通用吞吐提升。文件回收不包含私有脏页、
可写共享文件页或 swap；shm 缩短后保留物理高水位到对象最终销毁；常驻共享
ramfs 存储容量为 16 MiB，默认串行 ramfs 不受这个可选存储容量约束。磁盘 FS
未默认开启并行写入，需各自满足事务/共享状态契约。管理器故障可自动恢复，
监护进程本身被强杀时则保留故障标记、拒绝重复启动旧应用。

最终测试使用独立磁盘副本，没有更新持久根盘，没有提交或推送；用户已有的
.vscode/settings.json 修改保持原样。
