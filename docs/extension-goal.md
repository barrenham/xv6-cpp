# 六项系统扩展与验证

本轮六项均已实现并通过最终验证。`build/extension-six-final-validation.log`
记录最终内核与镜像的 28/28 通过，耗时 784.25 秒；验证按各项目要求覆盖
实现行为、生命周期、错误清理、单核/五核运行及相关既有功能回归。

| 项目 | 已实现行为 | 验证入口 |
| --- | --- | --- |
| ① 设备树设备发现 | 从 DT 读取 MMIO、IRQ、总线范围与中断路由；动态映射和 PLIC 上下文；已有类别的新设备进入资源列表 | device_tree_contract、device_discovery_1cpu/5cpu |
| ② 运行中自动绑定 | drvctl load 默认匹配并绑定空闲设备；--no-bind 保留手动选择；不抢占已绑定设备 | device_discovery_1cpu/5cpu 内独立 ELF 加载、第二块盘绑定与挂载读写 |
| ③ 具名 Unix socket | 独立进程按路径 bind/connect/listen/accept；权限、关闭、backlog、非阻塞、poll 和 SCM_RIGHTS 生命周期 | unix_socket_1cpu/5cpu，覆盖 ramfs/ext3/xv6、相对路径和独立 exec |
| ④ POSIX 消息队列 | mq_open/close/unlink、发送接收、属性、优先级/FIFO、绝对超时、阻塞/非阻塞、权限及命名生命周期 | posix_ipc_1cpu/5cpu 中 mqtest；posix_headers_contract |
| ⑤ FS 并发与缓存 | 独立元数据与数据通道、不同 inode 的数据锁；显式声明的路径/属性缓存和失效；断开唤醒及请求清理 | fs_concurrency_1cpu/5cpu、fs_contract、固定工作量性能记录 |
| ⑥ 内存映射 | 匿名/普通文件 mmap、共享/私有、mprotect、同步 msync、fork/exec、分段解除映射、关闭/删除/退出、SIGBUS 和跨 hart 截断同步 | memory_mapping_1cpu/5cpu；vm_buddy_contract/vm_freelist_contract；fs_contract |

接口与实现边界分别见[驱动模块](kernel-driver-modules.md)、[POSIX IPC](posix-ipc.md)、
[消息队列](posix-message-queues.md)、[FS 并发缓存](fs-concurrency.md)、
[内存映射](memory-mapping.md)。

## 关键验证内容

设备树测试同时使用真实 QEMU DTB 和资源变更输入，运行测试把根盘和熵源移到
第 7、8 个 MMIO 槽位，并检查 PCI 设备。另一次启动留下未绑定磁盘，下载独立
驱动 ELF，要求 drvctl load 自动绑定、产生 vdb、挂载读写，并验证重复扫描不抢占。
早期 DT 解析还修复了大聚合临时对象导致的启动栈越界，RV64 discover 栈帧为
448 字节，源文件有 2048 字节栈帧编译检查。

路径 socket 在三种后端保存真实 socket inode，覆盖符号链接、rename、unlink
后重新绑定、权限、四个独立 exec 客户端、backlog、非阻塞、poll 和描述符环。
事件驱动的 ipc-gc 测试要求所属 FS 进程关闭最后一个 socket 后还能处理 Forget
并卸载，防止回收向自身发 RPC 后等待。

消息队列测试验证最高优先级先交付、相同优先级保持 FIFO、错误用户缓冲区不
丢消息、独立 exec、阻塞唤醒、绝对发送/接收超时、权限、unlink、信号/重启和
poll。同时回归旧 socketpair、SCM_RIGHTS、共享内存和 FS 批量传输。

FS 并发测试后端必须收到两个独立数据请求后才回复，并逆序返回，统计要求
maximum_in_flight=2、结束后 in_flight=0。缓存测试对 ramfs/ext3 的 96 次重复
路径/属性访问要求新增 RPC 为 0，覆盖重命名、替换后的打开 inode、链接计数、
权限、淘汰、同步失败与后端断开。默认 SDK 仍是单事务处理循环；不能将多通道
等同于默认后端处理函数在多个 CPU 上执行，性能范围见专门文档。

内存映射测试包含匿名零填充、PROT_NONE 保留/恢复、共享和私有 fork、生成代码
执行及映射代码信号处理。普通文件在 ramfs/ext3/xv6 上检查独立 exec 共享、
读写一致性、msync/fsync、分段偏移、只读权限上限、关闭/删除、退出/exec 写回、
私有尾部清零、可捕获 SIGBUS。四个子进程持续读映射且不调用 syscall，要求
截断后全部以 SIGBUS 结束，验证其他 hart 的旧转换不能越过同步点继续使用。

## 运行记录

- `build/named-sockets-final-validation.log`：早期路径 socket 与离线镜像工具回归 10/10。
- `build/named-sockets-abi-validation.log`：错误返回、网络、IPC 和头文件复核 7/7。
- `build/fs-cache-regression.log`：FS 插件、权限、部分写入、路径 socket 和 IPC 9/9。
- `build/file-mapping-cache-regression.log`：映射后备页、原生契约及既有行为回归 9/9。
- `build/file-mmap-complete-functional.log`：ramfs/ext3 文件映射、跨 hart 截断、SIGBUS、
  两种分配器、FS/进程契约与 C 头文件 7/7。
- `build/extension-six-final-validation.log`：最终内核和镜像的 28/28 整体验证通过，含
  单核/五核内存压力、信号、驱动启动、发现/自动绑定、IPC、FS 并发及三后端映射。

测试使用私有磁盘并串行运行 QEMU，不修改持久根盘及本机 VS Code 设置。
保留功能性 fork 验证，不运行 fork 性能门禁。
