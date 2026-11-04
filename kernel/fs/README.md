# 内核文件系统机制

所有实际文件系统后端在用户态 ELF 中运行。内核只保留 VFS、inode 代理、
打开文件、管道、设备注册表、块缓存、启动资源能力和 FS 消息桥接。

| 目录 | 当前职责 |
| --- | --- |
| `vfs/` | 通用路径解析、服务根发布、挂载表、卸载与挂载快照 |
| `userfs/` | 版本化协议、用户态后端注册、RPC、inode 元数据和服务生命周期 |
| `file/` | File 引用计数、I/O 能力分派、设备注册、启动磁盘及资源能力 |
| `cache/` | 启动/块能力使用的真实块缓存与统计 |
| `buffer/` | 动态页缓冲 |
| `pipe/` | 管道和阻塞/非阻塞等待 |
| `inode/` | 通用 inode 和 FileSystem 能力接口 |
| `platform/` | 复制、凭据、等待、时钟和资源查询 |

ext3、xv6、devfs、procfs、ramfs、ext4 的真实实现位于 `user/fs/backends/`。
ext3 的 Volume/JBD 和 xv6 磁盘格式也位于对应用户态后端目录；镜像工具与
原生测试复用这些实际源码。内核不再保留旧磁盘 FS、旧内核伪 FS、旧日志事务、
内置后端选择器或 native/service 两套挂载分支。

`kernel/io/legacy_dirent.h` 是仍被 read/snapshot 接口使用的目录兼容记录，
`kernel/io/dirent.h` 定义 getdents64；两者均为正在使用的 ABI。

同步由 FS 服务处理：fsync 调用所属后端，sync 同步已挂载服务，块设备同步
刷新实际设备。日志容量、事务和恢复策略由用户态磁盘后端管理。

`fs_contract` 测试当前块缓存、管道、动态缓冲、File 并发关闭，以及内核
UserFileSystem/VFS 经协议与真实用户态 MemoryBackend 和 wire dispatcher 的
读写、重命名和资源回收。文件系统算法另外由实际用户态后端原生测试、
e2fsprogs 一致性检查及 QEMU 功能回归覆盖。

2026-11-04：44 项相关回归最终全部通过，包含单核/五核设备 ELF 启动、
根服务持久化、真实用户后端桥接、ext3/JBD/xv6/ext4、在线新 FS ELF 加载、
挂载/卸载、权限、快照、设备、GPU 共享映射和网络。日志位于
`build/kernel-cleanup-tests.log` 和 `build/kernel-cleanup-recheck.log`；
后者重跑了补齐设备驱动归档的三个自定义引导用例。结果及二进制 SHA-256
汇总在 `build/kernel-cleanup-acceptance.json`。244 个文件的 clangd 检查通过。
