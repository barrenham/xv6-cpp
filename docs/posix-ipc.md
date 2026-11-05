# POSIX IPC 与 FS 批量传输

当前提供 Unix socketpair、具名 Unix 流/记录连接、POSIX 共享内存及[具名消息队列](posix-message-queues.md)。应用使用标准头文件、函数签名以及 `-1/errno` 错误约定。FS 复用同一套队列、等待唤醒、文件引用和共享页机制。

## 应用接口

| 接口 | 当前支持 |
| --- | --- |
| `<sys/socket.h>` | `socketpair(AF_UNIX, SOCK_STREAM/SOCK_DGRAM/SOCK_SEQPACKET, 0, fds)` |
| 路径连接 | `socket(AF_UNIX, SOCK_STREAM/SOCK_SEQPACKET, 0)`、`bind/connect/listen/accept`、`getsockname/getpeername`、`SO_TYPE/SO_ERROR` |
| 消息 | `sendmsg/recvmsg`、iovec、CMSG_*、`SCM_RIGHTS`，以及 read/write/send/recv |
| 流控制 | 阻塞、O_NONBLOCK、MSG_DONTWAIT、poll、shutdown、读完后 EOF、关闭唤醒、EINTR/SA_RESTART |
| 信号 | 连接关闭后的 EPIPE/SIGPIPE，MSG_NOSIGNAL；pipe 同样产生 SIGPIPE |
| 描述符 | F_GETFD/F_SETFD、FD_CLOEXEC、F_DUPFD/F_DUPFD_CLOEXEC；状态标志由共享 File 持有，CLOEXEC 属于单个描述符 |
| `<sys/mman.h>` | shm_open/shm_unlink、ftruncate、mmap/munmap/mprotect/msync；另支持匿名内存和普通文件，见[映射接口](memory-mapping.md) |
| 共享内存生命周期 | 名称、打开引用和映射各自独立；unlink 后不能按旧名重新打开，已有描述符和映射继续有效；fork 共享 MAP_SHARED、隔离 MAP_PRIVATE；exec/退出释放映射 |

`shm_open` 返回的描述符默认设置 FD_CLOEXEC。共享对象权限使用有效 uid/gid、补充组和 umask；普通用户只能删除自己拥有的共享对象名称。端点之间传递的是同一打开文件的引用，因此共享文件偏移；消息队列持有引用，即使发送者关闭原描述符，接收者仍能获得有效描述符。接收时控制区不足设置 MSG_CTRUNC，未交付的引用会释放。

队列中的 socket 描述符可能形成环。回收器以仍在描述符表或内核中使用的普通 File 引用为根，遍历队列中的 SCM_RIGHTS 引用，释放不可达环。收发用户指针只在系统调用内使用，不保存在队列中。

当前资源上限：每方向 16 个队列项，单条记录 3072 字节，每条最多 16 个 iovec 和 4 个描述符；stream 可拆成多条内部记录。共享内存最多 32 个具名对象、单对象 1 MiB、每进程 32 段映射。名称采用 `/name`，长度最多 255 字节，不接受内部斜线。errno 为进程私有；系统尚无用户线程。

这不是完整 POSIX 系统认证。具名 Unix socket 支持路径绑定，暂不提供抽象名称空间及具名 SOCK_DGRAM；sendmsg/recvmsg 暂不支持原有 IPv4 socket。mmap 支持匿名内存、普通文件和 shm_open 对象；MAP_FIXED 尚未实现。普通文件映射支持 PROT_EXEC 和截断后的 SIGBUS；共享内存对象不提供执行映射，其缩短后的越界访问尚未接入同样的 EOF 撤销机制。具体边界见[内存映射](memory-mapping.md)。既有文件 stat/wait 等接口仍有项目 ABI，不能假定整个用户库已兼容 Linux libc。程序须使用本项目头文件和运行库重新构建。

## 具名 Unix socket

服务端与客户端可以独立启动。服务端绑定真实 VFS socket 节点并监听，客户端按路径
连接，不要求继承 socket 描述符。示例使用 `<sys/socket.h>`、`<sys/un.h>` 和
`<unistd.h>`：

```cpp
sockaddr_un address{};
address.sun_family = AF_UNIX;
strcpy(address.sun_path, "/tmp/service.sock");
int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address));
listen(server, 8);
int peer = accept(server, nullptr, nullptr);
// Client in another process:
// int client = socket(AF_UNIX, SOCK_STREAM, 0);
// connect(client, reinterpret_cast<sockaddr *>(&address), sizeof(address));
```

路径最多 107 字节加 NUL，支持相对路径、符号链接、重命名和挂载点。创建应用
uid/gid、0777 与 umask；连接检查目录搜索权限及 socket 节点写权限。ramfs、ext3、
xv6 后端均保存 T_SOCKET，普通 open 不把它当作数据文件。离线镜像更新与转换也
识别并保留 socket 节点的类型、所有者和权限。

注册表按文件系统实例与 inode 编号查找。关闭监听描述符会关闭尚未 accept 的端点，
唤醒等待 backlog 空位的 connect，保留路径节点；后续连接返回 ECONNREFUSED，
显式 unlink 后才能重新绑定。unlink 后旧连接继续可用，同名重建使用新 inode，
不会将新连接送给旧监听器。getpeername 返回对端绑定的名称，连接时使用的符号
链接或后来重命名不改变已记录的名称。

backlog 最多 16，0/负值按 1 处理。满队列的非阻塞 connect 返回 EAGAIN，可以
稍后重试；阻塞 connect/accept 睡眠等待条件改变，支持信号中断与 SA_RESTART。
accept 产生独立的阻塞描述符，默认不继承 O_NONBLOCK/FD_CLOEXEC。监听器有
待接收连接时 poll 返回 POLLIN。建立连接后 read/write、sendmsg/recvmsg、半关闭
和 SCM_RIGHTS 复用原有内核通道，不在每次数据 I/O 时查询 FS 后端。

待接收的端点通过监听 File 参与描述符可达性分析，不作为永久回收根。回收动作
可能释放绑定的 inode，因此在 FS 命名空间、元数据或 RPC 锁内推迟回收，在系统
调用结束及进程退出的安全位置处理，避免回收回调递归进入同一个 FS 请求锁。
FS 服务进程也不回收自己的 FS 对象；延迟对象由事件驱动的 `ipc-gc` 内核任务
处理。该任务先订阅再检查代数，空闲时睡眠；其他回收者结束时唤醒它，不周期轮询。
测试将最后一个 socket 描述符传给它所属的 FS 服务进程，由该进程关闭，要求
服务随后处理内核任务发来的 Forget 并允许卸载，以验证没有向自己发 RPC 的死锁。

`unix_socket_{1,5}cpu` 使用私有根盘与 xv6 数据盘，测试三种后端与相对路径；
包括四个独立 exec 客户端、backlog/关闭唤醒、权限、节点重命名/重建、记录边界、
描述符传递及单监听器/跨监听器引用环。测试后卸载卷并用 e2fsck 检查根盘。
来宾命令为 `/usr/tests/unixsockettest /tmp/uds`。测试名称与消息队列不同：
Unix 路径为 `/tmp/uds`，mq_open 的名称仍为独立命名空间中的 `/name`。

## FS 快速通道

1. C++ SDK 先以原有版本 3 协议注册 FS，再在实例可挂载前创建 SOCK_SEQPACKET socketpair 和 shm_open 对象。
2. 共享对象立即 unlink，ftruncate 后 mmap(MAP_SHARED)，只保留描述符和映射。通用桥接 ioctl 接收 socket、共享内存描述符及载荷上限，验证能力后保留 File 引用。
3. 请求编号、操作、身份和属性继续使用原 Message 布局，通过 socket 交换。Read/Write/SnapshotRead 的数据放在共享页；服务直接访问映射，避免服务边界上的逐条数据复制。
4. 普通后端默认 16 KiB；xv6 覆盖 max_transfer 为 3072 字节，遵守自己的日志事务容量。RAMFS 的预分配和失败回滚数组随批次扩大。内核普通文件写入批次也调整为 16 KiB，再按后端上限拆分。
5. 每个 FS 实例仍串行，确保请求及共享缓冲区不会互相覆盖。请求有 5 秒截止时间，超时或协议错误后断开连接，避免迟到回复被下一条请求接收。已发布的 FS 请求在有界的不可中断等待中完成，再递送客户端信号，避免 shell 的 SIGCHLD、停止或退出信号误断开所有进程共享的根 FS；普通用户 socket 等待仍支持 EINTR/SA_RESTART。取消只修改通道状态并唤醒等待者，不在 FS 自旋锁下等待 socket 的睡眠锁或释放 File。

协商失败时 SDK 保留原来的 512 字节端点。现有 C 插件或旧 SDK ELF 不需要更换消息布局；小于 512 的自定义注册载荷也保留原通道。`Backend::max_transfer()` 返回 512 可禁用快速通道。Control 的输入输出仍最多 512 字节，proc 快照为保持有界临时缓冲区仍分 512 字节读取。

没有取消 FS 进程切换，也没有消除客户端到共享页的复制。优化主要降低大块数据的 RPC 次数与服务边界的搬运，512 字节小请求仍有相同次数的往返，额外队列机制可能增加其开销。

## 验证与测量

[验收记录](ipc-acceptance.json) 保存本轮 60 项回归的最终状态与 244 个文件的 clangd 检查结果。

`ctest --test-dir build/dev -R '^posix_ipc_' --output-on-failure -j 1` 使用独立磁盘副本，串行运行 1 核和 5 核场景。独立 C11 程序的公开头文件由 posix_headers_contract 校验。来宾测试覆盖流/记录语义、背压、poll、关闭、SCM_RIGHTS、循环引用回收、复制故障、SIGPIPE、fork/exec、共享/私有映射和局部 munmap，并校验 RAMFS/ext3 的数据与请求数。

来宾中可执行：

```sh
/usr/tests/posixipc
/usr/tests/ipcbench /tmp/ipcbench
/usr/tests/ipcbench /root/ipcbench
```

ipcbench 写 64 KiB，再读 64 KiB，并校验每个字节。`rpc` 为该实例请求计数差；`copied` 统计服务消息边界的完整 Message 字节，包含旧端点内嵌载荷，未计入共享页、客户端复制、设备和磁盘 I/O；不是全系统复制量。`waits` 在新通道中表示等待回复的请求数，在旧通道中表示进入等待的次数。ticks 是 10 MHz 单调时钟，时间会随宿主机负载变化。

基线及优化结果保存在 [ipc-transport-results.json](ipc-transport-results.json)。判断收益优先使用相同数据量下的请求数和复制计数；耗时为单次观察，不作性能门禁。

以下为 5 核、128 MiB、每次 16 KiB 应用读写的单次记录：

| 后端 | 请求数：旧 → 新 | 服务消息字节：旧 → 新 | 耗时 ms：旧 → 新 |
| --- | --- | --- | --- |
| RAMFS | 256 → 8 | 569344 → 17792 | 375.84 → 19.35 |
| ext3 | 256 → 8 | 569344 → 17792 | 6387.28 → 1142.42 |

同次采样中，RAMFS 的 512 字节批次从约 388 ms 增至 476 ms，说明小请求并非普遍加速。大块请求的确定性收益是减少 96.875% 的消息往返和服务消息复制量。
