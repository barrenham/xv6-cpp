# 统一 FS 线程池

所有仓库内 FS 后端的 SDK `serve()` 现在都进入固定 pthread 线程池。
旧的 fork 快照读路径和常驻进程池已删除，没有进程池/线程池切换选项。
每个 FS ELF 仍是独立服务进程，默认有一个分发线程和两个工作线程。
监督器启动 FS ELF 所需的 fork/exec 与请求工作池是不同的生命周期。

## 使用和观测

```sh
ps -T
ps -T 6
mkdir /tmp/concurrent
mount -t ramfs /tmp/concurrent
```

默认两个工作线程；`-o workers=1` 或 `-o workers=2` 仅调整工作线程数。
`workers=0` 和旧 `read-workers=` 参数被拒绝。单个工作线程也通过任务队列执行请求，
不回退到主线程处理。其他 FS 自有的选项仍交给其具体后端解释。

`ps -T [PID]` 显示 PID、TID、状态、累计 CPU 毫秒、KiB 和名称。
状态来自实际任务；CPU 时间包含该线程在用户态和内核态运行的时间，排除睡眠。
KiB 是共享地址空间大小，每个线程可能显示同一个值，不能相加当作物理内存。
原始线程快照可读 `/proc/PID/threads`；内核通过资源能力的 ThreadAfter 查询提供
结构化事实，procfs 负责格式化，不把 /proc 文本生成移回内核。

## 分发和生命周期

`user/fs/sdk/thread_pool.cpp` 持有任务队列、每个工作线程独立的 Message 和按通道
隔离的共享 bulk 切片。主线程轮询传输通道，在普通内存中发布任务，使用 mutex 和
condition 唤醒工作线程；不再用 socket 复制任务到另一个工作进程。
工作线程直接发送协议回复，再用共享完成管道的一个字节通知分发线程。
同一传输通道只保留一个在途请求，完成前不复用其消息或 bulk 切片。

所有后端回调，包括元数据和维护任务，都在工作线程执行。普通关闭先完成在途任务，
再广播停止并 join 全部工作线程，最后关闭传输和释放映射。部分创建失败也清理已创建线程。
线程创建失败不会退回进程池。传输损坏或回调擅自 pthread_exit 时，终止整个 FS 服务；
内核取消等待中的请求，其他工作线程随进程退出。不能继续使用可能仍持锁的后端。
有在途任务时，分发线程最多每 100 ms 检查对应工作线程是否仍存活；完全空闲时无限等待，
不作周期轮询，也不尝试异步取消某个回调。
常用等待集合的七个通道放在内核的八个内联订阅槽内，不依赖临时物理页；
更大集合分配失败返回 ENOMEM，SDK 通过短暂睡眠重试，不因瞬时内存压力退出服务。

## 锁边界

锁顺序是命名空间锁、inode 条带锁、后端分配器/IO。分发队列 mutex 不跨后端调用。

| 后端/对象 | 保护方式 | 可以并行的范围 |
| --- | --- | --- |
| ramfs 数据请求 | 命名空间共享锁 + 64 个 inode 条带读写锁 | 同 inode 多读；不同条带 inode 可读写并行 |
| 命名空间、属性、Forget、snapshot、sync、control | 全局屏障及命名空间独占锁 | 等待先前数据任务完成，再执行该任务 |
| ext3、xv6 | 整卷独占锁覆盖完整回调 | 不同 FS 服务可并行，同卷事务保持串行 |
| ext4、devfs、procfs | 默认后端独占锁 | 先保护缓存、临时状态、PTY 引用及快照状态 |
| ramfs 分配器 | 线程安全 malloc/free | 不同 inode 的分配保持安全 |
| 磁盘块缓存和文件偏移 | 被整卷锁覆盖 | seek/read、LRU 更新和失效不会互相穿插 |
| 日志和块分配 | 被同一整卷锁覆盖 | 事务开始、位图/元数据更新、提交和 flush 不交错 |

磁盘 FS 使用了线程池，但尚未宣称同卷并行事务。不能在现有全局错误状态、共享
seek 偏移和事务缓冲上直接开启并行写入。下一步若细化磁盘锁，应先拆分这些状态。
inode 条带冲突会保守串行，不会破坏正确性。基础 pthread rwlock 允许递归读，
不保证写者公平，也不支持持读锁升级为写锁。

新后端默认保持独占保护。只有数据回调和 context() 满足线程安全契约后，才覆写
`Backend::concurrent_data()` 返回 true。`userfs::current_context()` 通过静态 TLS
提供当前请求身份，在回调返回时恢复；禁止用一个无保护的共享 actor 字段承载并行请求。
后端对象和缓存使用普通共享地址空间，不再为了工作池要求 MAP_SHARED 分配器。

## POSIX 同步接口

新增 `pthread_rwlock_*` 的初始化、属性、读/写锁、try、unlock 和销毁；同步仅支持
进程内共享，PROCESS_SHARED 返回 EOPNOTSUPP。
`pthread_cond_timedwait()` 使用绝对截止时间，支持 condition 属性选择 CLOCK_REALTIME
或 CLOCK_MONOTONIC；超时仍重新取得 mutex，再返回正的 ETIMEDOUT，保留 errno。
内核在比较等待字和登记唤醒的同一队列锁下注册 PollWait，复用现有定时队列，不忙等。

## 验证与测量

- pthreadsync：并发读、独占写、递归读、竞争、两类时钟超时/唤醒、mutex 重新取得、线程快照。
- fsworkertest：同 inode 并发读、独立回复、相同服务 PID/不同 TID、跨阻塞的请求身份隔离。
- fspooltest：不同 inode 并发写、同 inode 排斥、元数据屏障、固定 TID 复用、工作线程意外退出取消请求。
- system_extensions：默认四类 FS 都有两个工作线程，workers=1 有一个，旧模式参数被拒绝；实际 ramfs/xv6 缺页与服务管理。
- FS/映射/IPC、独立 ELF 插件、ext4、ext3 日志契约和 e2fsck 回归。

`tests/fs_pool_benchmark.py` 在独立盘上运行两名客户端，每名进行 256 对 4 KiB 写/读，
输出吞吐、平均/最大一对操作延迟、全系统 CPU 和挂载加文件准备后的 MemFree 差值。
CPU 统计较粗；内存差值包含缓存和文件页，不是纯线程池开销。第一轮与后续轮的缓存温度不同，
必须对应比较。迁移前镜像仅保存在忽略的 build 目录用于基线测量，生产代码没有旧模式。

验证记录：

- build/fs-thread-final-validation.log：28 项中 26 项通过，涵盖驱动、FS/IPC/VM、
  ext4、ext3 日志、内存耗尽后的核心 FS 可用性、线程池及 pthread。
  当时的 procfs 运行任务漏计和单核服务管理控制延迟随后修复。
- build/fs-thread-observation-services-validation.log：修复后 procfs 单/五核与
  system_extensions 单/五核 4/4 通过，保留所有原有断言。
- build/fs-thread-rw-final-validation.log：无等待者免唤醒优化后，五核 FS 池和
  单/五核 pthread 全套 3/3 通过，包含读写锁、条件变量超时、fork/exec、信号和 COW I/O。
- build/fs-thread-independent-sdk-validation.log：完整导出 SDK，在独立目录构建 ext4 ELF，
  验证 PT_TLS 段通过。build/fs-thread-integrated-validation.log 另记录独立 C ABI ELF
  插件加载/兼容测试通过；该轮早期低内存缺陷的失败记录也保留。
- build/fs-thread-poll-final-validation.log：poll 单/五核 2/2 通过。
  单个 poll 测试的整体 watchdog 从 15 秒调整为 45 秒，包含完整启动和登录；
  API 的超时/唤醒/信号断言保持不变。

服务兼容修复包括：/proc/stat 的 procs_running 改为统计所有任务；服务管理器每轮
只轮转处理一项，避免积累多个同步握手阻塞控制端；接受完整控制帧的等待预算从
100 ms 增至 1 秒，传输错误保留明确 errno。

### 测量结果

128 MiB QEMU，单核/五核，各两轮，同一 4 MiB ramfs 写读负载；测量时没有其他
QEMU 或编译任务。表中时间为两轮平均，CPU 秒是各 CPU 的全系统累计时间。

| CPU | 迁移前耗时 | 线程池耗时 | 耗时比 | CPU 秒：前 → 后 | 平均写读对延迟：前 → 后 |
| --- | --- | --- | --- | --- | --- |
| 1 | 57.49 s | 7.49 s | 7.67× | 55.10 → 7.40 | 223.63 → 28.78 ms |
| 5 | 32.32 s | 7.16 s | 4.51× | 54.90 → 10.40 | 125.19 → 27.48 ms |

第一轮挂载加预置文件后的全局空闲内存差值，两种 CPU 配置均为 2072 → 1652 KiB。
后续轮为约 364..368 → 1588 KiB，受到缓存状态和异步卸载回收影响，不能据此推导
线程池的净内存开销。原始计数、构建哈希和方法见
[测量数据](fs-thread-pool-benchmarks.json)；执行记录为 build/fs-thread-pool-comparison.log。
这些样本反映整系统迁移及等待路径优化在该负载的效果，不代表所有磁盘负载同样提升。

重测当前版本，例如：

```sh
python3 tests/fs_pool_benchmark.py --qemu qemu-system-riscv64 \
  --kernel build/dev/kernel.elf --bootstrap build/dev/user/image/_fsboot \
  --image build/dev/fs-test.img --cpus 5 --output build/fs-pool-current.json
```

本目标的实现、功能回归与测量已完成。没有安装持久根盘、提交或推送。
