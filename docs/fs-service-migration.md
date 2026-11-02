# 全部文件系统迁至用户态服务

目标范围为 ext3、xv6、ramfs、procfs、devfs，包含默认根文件系统。
此迁移仍在进行，当前默认交互启动继续使用原有内核后端；下面的已验证引导路径并不代表全部迁移完成。

## 已实现的根服务路径

QEMU 仍使用 `-kernel kernel.elf` 启动内核。`-initrd` 额外装载一个独立用户 ELF 的原始字节，
设备树 `/chosen/linux,initrd-start` 与 `linux,initrd-end` 告知内核地址。
内核验证完整 RAM 范围并预留覆盖到的所有页，通过公共 ELF 加载流程启动 PID 1。
该流程不进行文件查找，也不要求先挂载一个内核 FS。
不提供 `-initrd` 时保留原有启动流程，方便迁移期间进行对照回归。

PID 1 获得不依赖 `/dev` 的控制台描述符 0..2 和原始系统磁盘描述符 3。
描述符 4 是受控的内核资源能力，当前提供设备枚举和 PTY 枚举、保留、已保留对象属性及释放。
服务处理的全是类型化记录，目录布局、节点编号和名称处理均在用户态 devfs。
每次新的 PTY 路径查询都验证主端仍发布该名称；已打开从端的属性查询允许在主端挂断后继续工作。
服务死亡时资源描述符释放全部保留，避免编号泄漏。

磁盘描述符提供有界读写、seek、容量查询、刷新以及用于文件时间戳的时钟查询；
它不解析 ext3，也不替用户后端提交文件系统日志。
服务经 fork 取得磁盘能力后，监督进程关闭自己的引用。
服务先读取磁盘、恢复 JBD/孤儿链、注册 `ext3`；PID 1 随后使用 `mount("ext3", "/")`
一次性发布根后端。只有启用服务引导时的 PID 1 可以发布第一个根；已发布的根不能被覆盖。

`user/runtime/userfs/ext3.cpp` 实现全部目录、读写、属性、链接及孤儿回收回调。
它调用可移植的 ext3/JBD 格式引擎，通过预先取得的块描述符访问存储；回调不再通过根路径打开资源。
每个修改请求提交用户态 JBD，打开但已删除的文件在最后一次 Forget 后按批次回收。
提交失败会令该实例停止接受后续操作，避免继续使用不确定的磁盘状态。
全局 `sync()` 在服务模式下请求各挂载后端同步，`fsync()` 同步具体文件所属后端。

服务模式下，内核 VFS 原有的事务边界不再预留内核磁盘日志。
如果旧内核文件系统尝试记录磁盘修改，会触发明确错误，不能悄悄回退为内核后端。
原生 ext3/xv6 等实现目前还参与旧启动方式的构建；默认服务启动切换以及移除这些实现是后续必做项。

## 用户态引导监督进程

`user/apps/fsboot/fsboot.cpp` 是独立用户程序，目前启动 ext3 根服务和 devfs 服务，
将根目录与 `/dev` 分别挂到两个独立服务进程，再启动原有 `/sbin/init`。
PID 1 保持监督和回收职责；init 退出时重启 init，FS 服务退出时报告该挂载不可用。
它不在故障后静默切回内核 FS，也没有声称能够透明重启已有根连接。
只有 FS 服务保留相应磁盘/资源能力；init、登录程序和普通应用不会继承这些能力描述符。
该程序尚未接入默认交互 QEMU 目标，因为其余后端及挂载管理还在迁移。

## 已加入的实际验证

- `fs_bootstrap_1cpu`、`fs_bootstrap_5cpu`：未格式化临时磁盘上的用户态入口、fork/wait/exit，拒绝无效与截断 ELF。
- `allocator_contract_buddy`、`allocator_contract_freelist`：引导镜像地址宽度、成对属性、重复属性、范围校验和页保留。
- `fs_service_root_1cpu`、`fs_service_root_5cpu`：真实 ext3 根由独立用户进程提供；跨消息/跨页读写、打开后删除、目录改名、链接、非 root 权限和所有权、从根加载 ELF、同步、后端被杀后的错误返回。
- 根服务测试同时挂载独立用户态 devfs，验证 null/zero、PTY 创建/收发、关闭后名称消失、服务被杀后的错误返回；执行原有完整 `namedptytest`，包括挂断、编号复用、描述符耗尽与并发。
- `fs_service_boot_1cpu`、`fs_service_boot_5cpu`：实际 fsboot → FS 服务 → init → 首次账户设置 → 登录 → shell → C++/页表回归，不经过内核根 FS 初始化。
- 同一份私有磁盘连续启动两次，验证持久文件；关机后执行 `e2fsck -fn` 检查元数据及孤儿回收一致性。

构建和运行：

```sh
cmake --build build/dev --target kernel user_fsboot user_fsboottest user_fsrootboottest test-image
ctest --test-dir build/dev -R '^(fs_bootstrap_[15]cpu|fs_service_root_[15]cpu|fs_service_boot_[15]cpu|allocator_contract_(buddy|freelist))$' --output-on-failure
```

测试仅访问各自的临时磁盘副本，不更新交互会话的 `data/rootfs.img`，也不运行 fork 性能门禁。

## 尚未完成

- 用户态 xv6、ramfs、procfs 后端，以及监督程序中的全部后端启动与通用挂载管理。
- 第二块磁盘能力，以及进程/系统信息的受控资源接口。
- procfs 每次打开的快照与调用者身份、完整设备语义及所有旧回归的服务模式覆盖。
- 将交互启动和测试默认切至服务模式，移除内核中的各 FS 算法及日志实现。
- RPC 批量传输优化、文档索引与流程审计的最终重建，以及全范围验收。
