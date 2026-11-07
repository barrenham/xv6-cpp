# 文件分页、内容缓存、并行 FS 与服务管理

本轮目标包括四项运行机制，以及用户应用的目录与源文件组织。

## 文件按需分页与 COW

普通文件 mmap 只保留 inode、文件偏移、长度、权限和映射租约，不读取内容，
也不为所有文件页建立叶项。缺页时读取对应文件页并建立 PTE；越过 EOF 的整页
访问仍产生 SIGBUS。缺页处理允许等待 FS 回复，不持自旋锁睡眠。

私有映射首先借用文件缓存页，PTE 暂时只读并标记 COW。首次写入复制该页；
fork 共享已有私有页，并在任一方写入时拆分。尚未访问的文件页在 fork 时仍
保持未加载。只读映射、PROT_NONE、mprotect、部分 munmap、截断与退出继续受
VMA 权限及租约保护。COW OOM 不修改原物理页或撤销另一进程的引用。

未形成私有副本的页可能看到文件缓存的后续修改；MAP_PRIVATE 不提供 mmap
调用时刻的整文件快照。已经写入形成的私有副本不写回文件。
当前保留原有每次映射 1 MiB、每地址空间 32 个映射片段的接口上限。

read/write/getdents64/fstat 的映射缓冲区在取得文件锁之前预先触页。
用户拷贝本身也处理缺页/COW；处于不允许阻塞的锁上下文时，尚未准备的文件页
返回拷贝失败，避免递归进入 FS。应用应在这类专用设备控制路径之前准备好缓冲区。

## 统一文件内容缓存

普通 read 与文件 mmap 按 inode/页号复用同一物理帧。普通 write 采用写穿：
先发布已有映射修改，再写入后端并用已提交请求的快照更新缓存，避免二次读取
用户缓冲区或额外回读。部分失败路径仍校验后端；fsync/msync 报告后端
失败，不将失联后端误报为持久化成功。

每个 inode 最多保留 8 个没有映射租约的闲置页，按最近使用时间淘汰。正在被
映射使用的页保持固定引用，不能直接释放；解除映射后进入闲置回收范围。最后
一个 inode 引用消失时释放全部缓存。每个 FS 实例有 64 个 inode 槽，因而闲置
缓存最多 512 页/实例；映射中的页另外受地址空间上限和物理内存约束。
这是闲置文件页的回收机制；活跃映射页不换出，也没有磁盘 swap。
缓存元数据只由短自旋锁保护，逐页加载在锁外等待 RPC，同页缺失合并，不同页
允许同时加载。文件缺页在 inode 共享读锁下取得租约，并以 CAS 发布，防止两个
进程同时触页导致重复租约。冷顺序读取保留批量传输，从返回数据填充最近缓存页。

仅对主动声明缓存一致性契约的普通文件后端启用内容缓存；动态 proc/devfs 和
未声明能力的旧 FS ELF 继续使用原传输。传输统计中的 read_hits 计数命中页。
完整页读取的 RPC 数取决于协商 payload；xv6 的 3072 字节 payload 需要两次 RPC
才能填充一个 4096 字节缓存页，不能把“加载一页”固定解释为“一次消息”。

2026-11-07 的固定工作量检查：写入 64 KiB 后读取 64 KiB，512 字节分块的 RPC
从原传输的 256 次降到 144 次，减少 43.75%；4096 字节与 16384 字节分块分别
保持 32 次和 8 次。ramfs/ext3 均验证这些计数。原始结果见
`build/dev/posix-ipc-5cpu.log`；这证明消息数量变化，不代表所有工作负载的耗时提升。

## 可选的 FS 并行读处理

SDK 的 Backend::parallel_read_workers() 默认返回 0。主动启用时，SDK 使用
至多两个独立工作进程处理数据通道的 Read，输入来自主进程后端状态的稳定 fork
快照，输出只写各自共享内存切片与回复通道。原有堆页由已有 fork COW 共享；
工作进程中的局部缓存/内存变更不会合并回主进程。

写入、元数据、Forget 和维护任务必须先等待所有读者退出，然后在主进程执行。
下一批读者从最新主进程状态建立快照，避免将可变内存后端变成多个不同步副本。
这是可选的真实进程并行读取，不宣称所有写入也可并行。小请求的进程建立成本
可能高于串行处理，默认保持串行；没有对所有工作负载作性能承诺。

ramfs 支持显式选择：

```sh
mkdir /tmp/parallel
mount -t ramfs -o read-workers=2 /tmp/parallel
```

后端必须保证 read 的持久效果只在回复中，不能依赖工作进程修改父进程私有堆。
同一进程内的共享存储文件偏移也不能用于并行 seek+read；当前 ramfs 的内存读取
符合此契约，磁盘后端默认不启用。测试后端要求两个处理函数在任一方完成前
均已进入，并校验不同处理 PID、写入后快照和元数据屏障。

## 服务管理

fsboot 在核心挂载与驱动加载之后启动 /sbin/serviced。它登记 rootfs、挂载管理器、
启动挂载及驱动自动加载的状态。核心项由 bootstrap 拥有生命周期，控制端只读；
失效后标记 unavailable，不自动替换已有挂载、打开文件或未完成 I/O。
普通应用服务由 serviced 建立独立进程组并监督。
管理器自身崩溃时，bootstrap 保留登录并报告 unavailable；当前不接管其存活子进程，
因此不会盲目重启管理器而重复启动服务。这个策略与普通受管服务的自动重启分开。

默认读取 /etc/services.conf，控制 socket 为 /tmp/serviced.sock，权限 0600。
配置最多 16 项，核心与普通服务合计最多 24 项，命令最多 8 个参数，每参数
127 字节。配置示例：

```ini
[worker]
exec=/bin/worker --foreground
requires=rootfs
ready=exec
restart=on-failure
restart-limit=5
restart-delay-ms=250
log=/tmp/worker.log
enabled=yes
```

requires/after 在此版本都表示启动所需依赖；配置检查依赖环，缺失/失效依赖
保持 waiting。oneshot=yes 的依赖必须成功退出才满足条件。ready=exec 表示成功
执行 ELF；ready=notify 则等待应用向环境变量 SERVICE_READY_FD 指定的描述符
写入一个 R 并关闭它。通知超时或未通知退出视为失败。

自动重启使用指数退避，最多 30 秒，达到 restart-limit 后停止；手动 start/restart
重置重试预算。stop 先停止依赖者，再向进程组发送 TERM，超时发送 KILL。
输出写入每服务日志；管理器日志为 /tmp/serviced.log。日志当前不支持持续轮转。

```sh
servicectl list
servicectl status worker
servicectl start worker
servicectl stop worker
servicectl restart worker
servicectl log worker
servicectl load /etc/services.conf
```

load 先验证新配置；存在运行中的普通服务时拒绝替换，先停止相关服务再加载。
运行时加载的配置不会改写默认配置文件。旧持久根盘若尚未安装 serviced，启动器
保留原登录流程并报告缺失；更新根盘后，新启动自动启用管理器。

## 应用组织

user/apps 按 accounts、coreutils、examples、filesystem、network、shell、system、
terminal 分类。每个应用拥有 main.cpp、application.h、实现源文件和 sources.cmake；
入口只调用应用实现，安装路径、命令名及参数兼容。apps/programs.cmake 维护
分类目录下的应用清单；新增实现文件在应用自己的 sources.cmake 中登记。

Shell 拆分解析、执行、交互/作业管理、编辑与环境；fbterm 拆分生命周期、输入、
绘制、显示工作进程和诊断；memfsd 拆分 CLI/后端，tarfsd 拆分 CLI/归档/后端。
服务管理器的配置、控制协议、进程监督、启动和主循环也分别实现。

## 验证入口

- vm_buddy_contract/vm_freelist_contract：未触页无加载、fork 物理页共享、首次写入
  拆分、COW OOM 与 mprotect 不破坏共享帧。
- fs_contract：按需租约、逐页 OOM、跨缓存元数据块清理、映射/描述符一致性。
- system_extensions_1cpu/5cpu：ramfs/ext3/xv6 的分页、缓存与回收，真实 FS 工作
  进程，ramfs 可选并行模式，以及服务启动、依赖、通知、日志、重启预算、权限和清理；
  杀死启动管理器后保持登录可用，报告失联且不重复启动服务。
- memory_mapping_1cpu/5cpu：既有普通文件映射、SIGBUS、跨 hart 截断与生命周期。
- fs_hotplug_5cpu：登录后在仓库外创建、编译新 FS ELF，传入同一个运行中内核，
  挂载、读取及执行文件；不重启 QEMU，也不重编译内核或 bootstrap。

所有运行验证使用私有磁盘副本，保留用户持久根盘和本机编辑器设置。

2026-11-07 验证：内核与测试镜像构建成功，37 个选定的原生契约/系统回归均通过。
日志位于 build/four-extensions-final-native.log、
build/four-extensions-final-functional.log、
build/four-extensions-compatibility-validation.log 和
build/four-extensions-followup-validation.log。兼容性首轮暴露的独立 SDK 链接遗漏
已补齐，fs_hotplug_5cpu 在最终复测通过；启动管理器故障策略修改后，驱动启动及
system_extensions 的单/多 CPU 回归也重新通过。52 个应用的入口、源文件清单
和编译数据库对应关系全部检查通过。

要使用已有持久根盘体验本轮程序，先在 QEMU 内 sync 并退出，再在仓库目录运行：

```sh
cmake --build build/dev --target rootfs-update
```

然后重新启动 QEMU。本轮测试没有代替用户更新持久根盘。
