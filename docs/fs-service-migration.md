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
描述符 4 是受控的内核资源能力，提供设备与 PTY 枚举、保留、已保留对象属性、释放，以及原始进程、CPU、系统计数和描述符记录。
服务处理的全是类型化记录，目录布局、节点编号和名称处理均在用户态 devfs。
每次新的 PTY 路径查询都验证主端仍发布该名称；已打开从端的属性查询允许在主端挂断后继续工作。
服务死亡时资源描述符释放全部保留，避免编号泄漏。
`CloneIoctl` 创建独立资源能力；每个 devfs/procfs 后端持有自己的副本。
原始 exec 字节捕获与描述符捕获按副本隔离，一个消费者的重新捕获、释放或退出不会覆盖另一个消费者的数据。
对 fd/exec 的代理检查在进程锁内按原始客户端 uid 执行；只有有效 uid 0 的代理可以指定检查身份。

磁盘描述符提供有界读写、seek、容量查询、刷新以及用于文件时间戳的时钟查询；
它不解析 ext3，也不替用户后端提交文件系统日志。
服务经 fork 取得磁盘能力后，监督进程关闭自己的引用。
资源能力的 `StorageIoctl` 按设备索引获取有界的可写块描述符（0 系统盘，1 可选数据盘），只允许服务模式下的有效 uid 0 使用。
每块磁盘只有一个可写租约；dup/fork 共用租约，直到最后一个描述符关闭才释放。
获取能力时的内存、文件槽或描述符安装失败不会泄漏租约。普通 `/dev/vda`、`/dev/vdb` 保持只读。
监督进程在用户态读取磁盘签名，选择 ext3 或 xv6 服务。服务先恢复对应日志与孤儿对象，注册类型；PID 1 随后使用 `mount(type, "/")`
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

`user/apps/fsboot/fsboot.cpp` 是独立用户程序，目前启动 ext3/xv6 根服务、devfs、procfs 和 ramfs 服务，
将根目录、`/dev`、`/proc` 和 `/tmp` 分别挂到四个独立服务进程，再启动原有 `/sbin/init`。
PID 1 保持监督和回收职责；init 退出时重启 init，FS 服务退出时报告该挂载不可用。
它不在故障后静默切回内核 FS，也没有声称能够透明重启已有根连接。
只有 FS 服务保留相应磁盘/资源能力；init、登录程序和普通应用不会继承这些能力描述符。
该程序尚未接入默认交互 QEMU 目标；默认切换及移除内核后端仍在迁移范围内。

## 通用挂载管理

fsboot 启动独立的用户态挂载管理进程，并通过资源能力的 `ManagerIoctl` 取得控制端点。
内核只转交类型、绝对挂载路径、请求序号和原始调用者 PID，不根据 ext3、xv6、ramfs 等名称选择实现。
控制消息定义在 `kernel/io/fs_mount.h`。读写、poll、版本/序号检查、30 秒截止时间和断开唤醒由通道机制负责。
管理进程对同一通道的递归请求会立即失败；等待期间不持有 VFS 命名空间、inode 或提供者注册表锁。
取消或超时可能发生在服务已产生副作用之后，因此会退休该连接，避免迟到回复完成另一次请求。

管理通道有效期间，普通 mount 请求统一交给用户态管理进程；只有管理进程自己的 mount 直接查通用注册表。
实例选择、注册和发布因而不会被另一个 mount 请求抢占。管理进程先尝试使用已注册的实例，支持独立安装的新插件；
没有实例时，由用户态策略为 ramfs、proc/procfs、devfs 创建新服务，为 ext3/xv6 获取可选数据盘并校验实际格式。
服务端通过 fork 继承预先取得的设备能力，不在回调中通过被挂载的路径获取自身资源。
注册就绪与挂载发布使用双向管道确认；新服务在收到发布确认前等待，父进程中途死亡则因 EOF 退出，关闭未发布端点与设备租约。
默认 init 和普通应用不继承磁盘、资源或管理端点。

例如，`mount -t ramfs /tmp/new` 创建一个独立内存实例；第二块磁盘存在且格式匹配时，
`mount -t ext3 /tmp/data` 或 `mount -t xv6 /tmp/data` 创建独立磁盘服务。
目标目录必须已经存在。忙挂载无法卸载；空闲卸载使服务收到 EOF 并退出，管理进程回收子进程。
管理进程死亡时不能自动创建新实例，已有挂载继续工作；已经独立注册的插件仍可直接挂载。
没有透明重启管理进程的承诺。

## 用户态 ramfs

`user/runtime/userfs/memory.cpp` 实现 ramfs 的 inode、目录、稀疏页、属性、链接、改名和对象寿命。
目录项和数据页独立按需增长，没有示例 memfsd 的固定 128 个节点、256 个目录项与 1 MiB 文件限制。
扩展文件只改变逻辑长度；读取未分配的洞返回零。一次写请求先准备所需页面，失败时撤销新分配且保留旧内容与长度。
打开但已删除的对象在最后一次 Forget 后回收；已删除的 cwd 会保留父目录链，直到不再需要该链。
目录项按递增 cookie 保存，删除不会把后续 cookie 改成别的记录；旧 `read(dir)` 接口也支持跨记录的部分读取。

fsboot 将独立 ramfs 服务挂到 `/tmp`，其根模式为 `01777`。
还提供独立可执行程序 ramfsd，使用同一个后端实现。示例：

```sh
mkdir /tmp/layer
ramfsd --mount custom-ram /tmp/layer &
# 关闭该挂载下的文件后：
umount /tmp/layer
```

服务可以先用 `ramfsd --register custom-ram` 注册，再由另一个进程挂载。
卸载令 SDK 收到 EOF，服务退出并释放整个实例；覆盖前的目录内容恢复可见。
独立程序与通用自动挂载使用同一后端实现；自定义类型也可先注册，再由管理进程挂载已有实例。

## 用户态 procfs

`processes.cpp` 管理动态目录、进程身份、fd/fdinfo、命令行、环境和每次打开的内容；
`process_system.cpp` 在用户态生成 meminfo、uptime、stat、cpuinfo、filesystems、mounts、devices、partitions、bcache、fsplugins 和 version。
内核资源接口返回类型化数值与原始 argv/env 字节，不生成这些文件的路径、目录项或格式化文本。
`self` 保留项目原有的当前进程目录别名语义，由桥接层传入的客户端 PID 解析。

协议版本 3 增加原始客户端身份与快照回调。
进程退出、fd 复用后，原已打开的文件保留自己的字节流；新的 open 重新捕获并检查当前目标与权限。
目录捕获也独立保存，300 个子进程的枚举不受内核 inode 缓存数量约束。
进程记录的原始采集与私有信息的身份检查仍由内核进程机制负责，目录、文本与缓存策略在用户态。

服务模式下 `/proc/filesystems` 枚举通用桥接和实际注册的用户态类型，不把未启用的旧内核实现冒充为可用服务。
因此 procfstest 检查实际根类型（ext3/xv6 之一）及已注册系统服务，而不是强制同时存在两个磁盘后端实例。
未挂载的磁盘格式不占用一个虚假的提供者实例；需要创建该格式的后端服务后才能挂载。

## 用户态 xv6

`xv6_volume.cpp` 在独立用户进程中解释原有磁盘格式：1024 字节块、14 字符名称、12 个直接块及一个间接块。
一次修改先在私有页面准备，准备失败丢弃全部修改；提交依次刷新 redo 数据、日志头、原位数据和清空的日志头。
日志恢复先校验全部目标块和重复项，再执行重放；启动时回收持久化的零链接 inode。
打开后删除的对象由桥接 Forget 回调决定释放时间，目录对象保留仍被 cwd 引用的父链。
VFS 原有的禁止从已删除 cwd 解析相对路径规则继续生效，进程仍可通过绝对路径离开该目录。

`xv6.cpp` 将该格式引擎适配到通用桥接协议，提供目录、属性、稀疏文件、链接、改名及设备节点等回调。
用户态后端严格拒绝超过 14 字符的名称，不把不同名称截断成同一个文件。
现有约 268 KiB 文件上限保留；镜像安装时剥离 ELF 调试段，完整 DWARF 文件保存在构建目录 `user/_程序名`，
实际安装的 ELF 位于 `user/image/_程序名`。两个新增测试程序的镜像名使用 `ramfssvctest` 和 `fsresboot`，符合格式的名称上限。

## 已加入的实际验证

- `fs_bootstrap_1cpu`、`fs_bootstrap_5cpu`：未格式化临时磁盘上的用户态入口、fork/wait/exit，拒绝无效与截断 ELF。
- `allocator_contract_buddy`、`allocator_contract_freelist`：引导镜像地址宽度、成对属性、重复属性、范围校验和页保留。
- `fs_service_root_1cpu`、`fs_service_root_5cpu`：真实 ext3 根由独立用户进程提供；跨消息/跨页读写、打开后删除、目录改名、链接、非 root 权限和所有权、从根加载 ELF、同步、后端被杀后的错误返回。
- 根服务测试同时挂载独立用户态 devfs，验证 null/zero、PTY 创建/收发、关闭后名称消失、服务被杀后的错误返回；执行原有完整 `namedptytest`，包括挂断、编号复用、描述符耗尽与并发。
- `fs_service_boot_1cpu`、`fs_service_boot_5cpu`：实际 fsboot → FS 服务 → init → 首次账户设置 → 登录 → shell → C++/页表回归，不经过内核根 FS 初始化。
- `userfs_memory_contract`：分配失败回滚、跨页原子失败、2 GiB 稀疏洞、截断后零填充、400 个目录项、跨记录部分读取、打开后的改名替换、删除 cwd 的父链与全部释放；相同源码通过 ASan/UBSan 和泄漏检查。
- `fs_service_ramfs_1cpu`、`fs_service_ramfs_5cpu`：在真实用户态 `/tmp` 上运行原有 ramfstest，包括 1 MiB 文件、220 个目录项、并发、截断、硬链接和 orphan cwd。
- `fs_service_ramfs_lifecycle_1cpu`、`fs_service_ramfs_lifecycle_5cpu`：独立 exec ramfsd，三轮注册/覆盖挂载/拒绝忙卸载/空闲卸载/服务退出/原目录恢复。
- `fs_service_permissions_1cpu`、`fs_service_permissions_5cpu`：在用户态根、devfs 和 ramfs 上运行权限、账户及 su 回归。procfs 接入后已重新运行；其私有信息检查另由完整 procfstest 和资源身份用例覆盖。
- `fs_service_procfs_1cpu`、`fs_service_procfs_5cpu`：原有完整 procfstest、ps/free、缓存/时钟/CPU 文本回归，包含 cmdline/environ 权限、fdinfo 偏移、旧快照在退出/复用后的保持、重复读取和 300 子进程枚举。
- `fs_resources_1cpu`、`fs_resources_5cpu`：未挂载 FS 的资源能力验证，覆盖版本/长度/无效地址、两个独立捕获者、委托 uid 拒绝、原始 exec 字节、fd 表捕获和 dup 身份。
- `fs_service_snapshots_1cpu`、`fs_service_snapshots_5cpu`：合成后端故意提前 EOF，验证失败 open 回收及下一次捕获恢复；8193 字节独立打开、dup 偏移、无效目的地址及子进程 uid/PID 的实际传递。
- 同一份私有磁盘连续启动两次，验证持久文件；关机后执行 `e2fsck -fn` 检查元数据及孤儿回收一致性。
- `xv6_volume_contract`：直接/间接块、稀疏洞、最大文件、截断清零、目录和链接、准备阶段分配失败的磁盘不变性；在创建及跨目录改名替换的各写入/刷新边界模拟掉电，验证完整旧状态或新状态、重放和被替换孤儿回收；拒绝损坏日志。相同源码通过 ASan/UBSan 与泄漏检查。
- `fs_service_xv6_boot_[15]cpu`：真实 xv6 用户态根启动、账户设置、登录、执行程序及页表回归。
- `fs_service_xv6_persistence_[15]cpu`：真实监督进程在同一份私有 xv6 磁盘连续启动两次，验证文件模式/所有权、间接块、稀疏文件、最大长度、打开后删除、目录改名和删除 cwd 释放；每次启动由四个并发客户端完成共 80 轮创建、读写、删除及孤儿关闭，覆盖 inode 重用；根文件持久化，ramfs 文件在重启后消失。
- `fs_service_xv6_{procfs,permissions,ramfs}_[15]cpu`：将原有完整进程信息、权限和内存文件系统回归运行于用户态 xv6 根之上。
- `fs_mount_manager_rpc_[15]cpu`：不挂载任何 FS 的控制通道验证，包括原始调用者、poll、非阻塞、错误指针、版本/序号/状态校验、递归拒绝、挂起请求的断开唤醒和重新注册。
- `fs_storage_capability_[15]cpu`：未格式化的私有第二磁盘，有界及非对齐读写、容量、互斥租约、dup/fork 生命周期、非 root 拒绝，以及描述符耗尽后的租约回收。
- `fs_mount_services_{none,ext3,xv6}_[15]cpu`：真实 shell 自动创建多个 ramfs，覆盖忙卸载、cwd、权限、嵌套挂载、24 次实例回收、四个并发 mount 客户端、错误类型/路径和管理进程死亡。混合使用 xv6 根/ext3 数据盘或 ext3 根/xv6 数据盘，在同一私有磁盘上连续启动两次，验证卸载重挂、写租约释放和持久化；ext3 数据盘另经 e2fsck 检查。
- 同组挂载测试还自动创建 devfs/procfs 实例，验证设备代理、原始调用者解析和只读策略；在新 ramfs 已注册但挂载路径查询尚未完成时终止管理进程，验证等待客户端返回错误、未发布服务关闭，以及已断开的父 FS 仍能卸载。
- `fs_service_{userfs,fsplugins}_[15]cpu`：完整原有匿名桥接/独立插件回归运行在服务引导模式下；注册表容量用例将已运行的系统服务计入基线，仍验证全部 16 个槽被占满及容量回收。旧模式的同组测试保留作对照。

构建和运行：

```sh
cmake --build build/dev --target kernel test-image legacy-test-image
ctest --test-dir build/dev -R '^(fs_bootstrap_[15]cpu|fs_service_.*_[15]cpu|fs_mount_manager_rpc_[15]cpu|fs_storage_capability_[15]cpu|fs_mount_services_.*_[15]cpu|fs_resources_[15]cpu|xv6_volume_contract|userfs_memory_contract|allocator_contract_(buddy|freelist))$' --output-on-failure
```

测试仅访问各自的临时磁盘副本，不更新交互会话的 `data/rootfs.img`，也不运行 fork 性能门禁。

## 尚未完成

- 所有旧回归的完整服务模式覆盖，以及内存压力下各后端的进展保证。
- 将交互启动和测试默认切至服务模式，移除内核中的各 FS 算法及日志实现。
- RPC 批量传输优化、文档索引与流程审计的最终重建，以及全范围验收。
