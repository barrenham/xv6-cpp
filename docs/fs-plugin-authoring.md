# 开发独立用户态 FS 插件

FS 插件是一个用户态可执行程序。内核只安装一次通用桥接层；插件按公共 ABI 注册自己，文件操作通过服务描述符交换。内核不动态加载插件代码到自己的地址空间。

## 最小服务结构

`import xv.userfs;`，继承 `userfs::Backend`。只读插件至少需要实现：

| 方法 | 职责 |
| --- | --- |
| root | 返回根对象 1 的 uid/gid/mode、目录类型和链接数 |
| lookup | 通过父对象编号和名称返回稳定对象编号及属性，包括 `.`、`..` |
| getattr | 返回已存在对象的当前属性 |
| readdir | 返回目录项及严格前进的 cookie，1 为一项、0 为 EOF、负数为错误 |
| read | 在给定偏移读取至多请求长度的数据，返回实际字节数 |
| forget | 撤销该编号的聚合内核引用，必要时回收已删除对象；只读静态索引可不做工作 |

`flags()` 返回 `xv::userfs::ReadOnly`，`operations()` 返回 `xv::userfs::ReadOperations`。可写插件按需覆盖其他操作，并声明对应能力；默认未实现操作返回错误。对持久插件应实现 sync，在返回成功前完成自己的写入/日志/设备屏障。

除 read/write 返回实际字节数、readdir 返回 1/0/负数外，其余 int 回调约定 0 表示成功、负数表示失败；对象编号写入 Attributes，不作为 int 成功返回值。

耗时回收可覆盖 `maintenance_pending()` 和 `maintain()`：前者表示仍有后台工作，后者只执行一个有界批次，成功返回 true。SDK 在请求之间执行批次，空闲时继续处理；有新请求时在批次之间分派。端点正常关闭后也会排空剩余维护任务。每批应明显短于桥接的 5 秒截止时间，不能把整份大文件的回收藏在一个批次里。持久后端必须先提交删除及可恢复的孤儿记录，再回复成功，维护回调只负责后续回收。默认回调不产生后台工作。

准备阶段可回滚的临时资源不足不应当作持久 I/O 故障：后端可以让 `maintain()` 返回 true、保留待处理状态，并通过 `maintenance_delay_ms()` 请求短暂等待。SDK 将等待限制在 0..1000 毫秒，有新请求时可提前唤醒；正常关闭后的排空循环同样遵守该延迟，避免在内存不足时空转。默认延迟为 0，正常批次不增加等待。ext3 在分配失败后采用 10 毫秒重试；提交阶段依赖的工作缓冲在挂载时预留，实际磁盘错误仍停止该实例。

本版接入普通文件、目录和符号链接；设备、socket 等内核对象继续由现有内核机制管理。

服务入口的公共部分如下：

```cpp
import xv.userfs.plugin;

extern "C" int main(int argc, char **argv)
{
  userfs::plugin::Invocation invocation(argc, argv);
  if (!invocation.valid || invocation.probe) return 2;
  MyBackend backend(userfs::plugin::Source);
  return userfs::plugin::run(invocation, backend) < 0;
}
```

后端文件、块设备或网络连接应在注册前准备好。客户端在挂载后立即发起请求也应能处理；不能依赖在第一次请求后才通过路径查找打开后端，否则可能重入 VFS 的命名空间锁。

完整的独立只读后端见 [tarfsd](../user/apps/filesystem/tarfsd/tarfsd.cpp)，它只实现 Backend 回调，SDK 处理所有协议分派；[memfsd](../user/apps/filesystem/memfsd/memfsd.cpp) 演示可写操作和对象回收。两个后端没有对应的内核专用分支。它们保留手工服务命令行；按清单自动加载的入口应使用上面的 `plugin::run` 握手，独立构建示例见 `examples/fs-plugin/`。

## 独立 ELF 与构建

默认五种后端分别编译成独立 ELF：`ext3.elf`、`xv6.elf`、`devfs.elf`、`procfs.elf`、`ramfs.elf`。
`fsboot` 只导入监督模块，不链接这些算法。用户态模块位于 `user/fs/`：

| 目录 | 职责 |
| --- | --- |
| `sdk` | 协议分派、资源访问、插件入口；公共 C ABI 和独立启动代码 |
| `backends/<name>` | 各后端的实际 C++ 模块接口及实现 |
| `supervisor` | 清单解析、ELF 启动、挂载管理、根引导 |
| `plugins/<name>` | 各 ELF 的入口和清单 |

第三方无需修改项目 CMake。可复制 `user/fs/sdk/{include,standalone}` 和
[独立示例](../examples/fs-plugin/CMakeLists.txt) 到仓库外构建：

```sh
cmake -S examples/fs-plugin -B build/hello-plugin -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/user/fs/sdk/standalone/riscv64-toolchain.cmake" \
  -DXVFS_SDK="$PWD/user/fs/sdk"
cmake --build build/hello-plugin -j 1
```

产物为 `hello.elf`、`hello.conf`、`install.tsv`。SDK 的独立 C 入口不运行 C++ 全局构造函数；
C++ 插件可链接项目用户运行库，并 `import xv.userfs.plugin;`、`import xv.userfs;`。
项目内后端目标为 `user_ext3_plugin` 等，分别链接自己的模块。

## 插件清单 ABI 1

运行时管理器每次请求都扫描 `/etc/fsplugins/*.conf`，不需要更新类型表或重启管理器。
ELF 通常装在 `/usr/lib/fsplugins/`，清单中的 `exec` 可指定其他绝对路径。
格式严格为 `key=value`，支持空行及以 `#` 开头的注释；不支持重复或未知字段、CRLF 和空格裁剪。
清单最大 2048 字节，类型名最长 31 字节，只接受小写字母、数字、`-_.`。

```ini
abi=1
type=hello
exec=/usr/lib/fsplugins/hello.elf
resource=none
mode=0555
```

| 字段 | 含义 |
| --- | --- |
| `abi`、`type`、`exec`、`resource` | 必填；ABI 固定 1；resource 为 none/block/file/system |
| `alias` | 一个可选类型别名；同名或别名冲突的请求失败 |
| `image` | 根挂载前使用的引导资产名，如 fsplugins/hello.elf |
| `source` | 默认源；block 为 0/1，file 为绝对路径或 boot:资产名 |
| `access` | file 的打开权限 ro/rw，默认 ro；不决定插件注册的只读标志 |
| `system` | 设为 1 时额外授予资源描述符；默认 0 |
| `options` | 传给插件的一个字符串，最长 255 字节 |
| `mode` | 根目录八进制权限，默认 0755 |
| `startup_ms` | 注册就绪期限，正整数毫秒，默认 20000 |

`resource=block` 获取独占可写块租约；`file` 预先打开源，避免服务回调递归打开自身路径。
`system` 提供设备/进程等类型化资源；`none` 可用于内存或自建网络连接的插件。
这些是资源类别，不是文件系统格式判断。普通应用不会继承管理器的资源能力。

```sh
mkdir /tmp/hello
mount -t hello -o example /tmp/hello
cat /tmp/hello/hello
umount /tmp/hello
```

`mount -t TYPE -s SOURCE -o OPTIONS /path` 可覆盖默认源和选项。
示例在资源为 none 时提供静态内容，在 file 模式读取源文件；options=rw 时演示写回源。
插件清单和代码具有服务权限，应由管理员安装；独立进程不是对不可信代码的完整沙箱。

离线安装到**已停机的目标镜像**：先创建仅含插件的种子，再使用保留账户的合并工具。
这里的 `TARGET.img` 是管理员明确选择的镜像，不能用重新生成整个 rootfs 代替安装。

```sh
build/dev/mkfs build/hello-seed.img --filesystem ext3 --manifest build/hello-plugin/install.tsv
python3 tools/update_rootfs.py --seed build/hello-seed.img --image TARGET.img \
  --manifest build/hello-plugin/install.tsv --dry-run
python3 tools/update_rootfs.py --seed build/hello-seed.img --image TARGET.img \
  --manifest build/hello-plugin/install.tsv
```

工具锁定目标、生成备份并原子替换；不覆盖账户数据库和用户目录。第三方插件的加载不要求重新构建 kernel 或 fsboot。

## 进程入口和发布握手

管理器启动 `ELF --fs-plugin 1 TYPE MODE_OCTAL OPTIONS`，包括程序名共 6 个参数。
自动根探测使用 `ELF --fs-probe 1 TYPE OPTIONS`，以退出码 0 表示匹配；格式识别由插件执行。

| fd | 内容 |
| --- | --- |
| 0..2 | 继承的控制台 |
| 3 | 可选源描述符 |
| 4 | 可选资源能力 |
| 5 | 就绪管道写端 |
| 6 | 发布确认管道读端 |
| 7 | 引导 ELF 字节能力，入口先关闭 |

插件先准备资源并注册，向 fd5 写入一个 `R` 后关闭它，再从 fd6 读取 `G` 并关闭它，之后开始处理请求。
收到 EOF 或其他字节时清理退出。父进程只在注册就绪后挂载，成功后发 `G`。
失败和超时会终止并回收子进程，释放尚未发布的端点和源；卸载使服务收到 EOF。
管理器等待期间定期报告进度，续期内核 30 秒无响应期限；插件自身仍受 startup_ms 限制。
这允许慢恢复，而不会永久等待从不就绪的插件。

## 在内核运行后新增驱动

无需重启或重编译内核。可以在运行期间独立构建 ELF，传入系统并最后发布清单，再执行 mount。具体步骤和不重启验收见 [运行时加载](fs-hot-loading.md)。

## 根插件也是 ELF

QEMU 仍以 `-kernel kernel.elf` 引导；`-initrd` 提供用户态 fsboot ELF 及附加的不可变资产。
内核资产目录仅提供字节和受控 fexecve，不识别任何文件系统格式。
引导 `boot.cfg` 的 `root=auto` 逐个执行块后端的探测入口；`root=TYPE` 可选择任意已打包插件，
包括无磁盘源的根服务。块设备根使用内核预先交付的系统盘 fd3；清单中的 block source 用于根发布后的运行时挂载。
其余配置如 `init=/sbin/init`、`mount=proc:/proc:0555` 控制后续启动。
根插件必须提供 init 和后续 ELF/配置需要的文件，以及 /dev、/proc、/tmp 挂载点。

在 hello.conf 增加 `image=fsplugins/hello.elf` 后，可扩展现有引导包，保留原资产：

```sh
python3 tools/pack_boot_assets.py --extend build/dev/user/image/_fsboot \
  --output build/custom-fsboot.elf \
  --executable fsplugins/hello.elf=build/hello-plugin/hello.elf \
  --asset fsplugins/hello.conf=build/hello-plugin/hello.conf \
  --asset boot.cfg=MY_BOOT.cfg
```

这里仅重新打包用户态资产；kernel 和 fsboot 的机器代码不变。静态 hello 示例只提供一个文件，
不能直接用作根；`tests/fixtures/fs_root_plugin.c` 展示携带 init 的最小独立根插件。

## 注册契约

低层 `xv::userfs::Registration` 定义在 [protocol.h](../kernel/fs/userfs/protocol.h)：

- size/version 标识完整结构及公共协议版本；不匹配则注册失败。
- type 是唯一的类型名称标识，可以存在多个同名实例，注册不自动挂载。
- root 提供初始根属性，根对象编号必须为 1 且为目录。
- flags 支持 ReadOnly 和 Snapshots；内核独立执行只读策略，Snapshots 必须同时设置 ReadOnly。
- operations 声明服务支持的操作位；Lookup/Getattr/Read/Readdir/Forget/Sync 是基础集合。
- max_payload 在旧消息端点中为 1..512 字节；当前 C++ SDK 可在挂载前协商最大 16 KiB 的共享内存通道。覆盖 `Backend::max_transfer()` 可以降低上限，返回 512 则禁用快速通道。显式传入小于 512 的 register_backend payload 同样保留旧通道。

挂载使用 `mount -t type /existing/directory`。不同服务进程可以使用同一个可执行文件、不同参数和不同存储源注册同一类型的多个实例。挂载器会选择尚未挂载的实例；已经挂载、停止或失去连接的实例不会再被选择。内核不会代替用户态进程管理器启动 FS 程序。

服务引导模式下，fsboot 的独立用户态管理进程通过通用控制端点接收 mount 请求。
它先挂载已注册的实例；没有实例时，解析清单并执行独立 ELF，不按文件系统类型分支。
独立插件仍可按上面的协议注册自己的类型，无需修改管理通道或重新构建内核。
管理进程统一串行执行实例选择、启动和挂载，避免并发请求抢占尚未发布的实例。
资源能力及控制端点不会继承给普通应用，详见 [FS 服务迁移](fs-service-migration.md)。

读取 `/proc/fsplugins` 可以观察 registered、mounting、mounted、disconnected、stopped 状态。注册失败、挂载失败、进程退出和资源容量耗尽都是可观察的错误，不能用无限等待处理。服务注册后应立即进入接收循环，不使用终端输入作为文件系统请求通道。

## 引用和存储一致性

编号必须在仍被内核引用时稳定。多次 Lookup/Create 可能对应同一编号；内核合并本地引用，最终一次释放只发送一个 Forget。服务可以维护“内核仍引用该编号”的标志。链接计数为零的对象仍可能被打开，必须在最终 Forget 后才回收并复用编号。

Readdir 只是枚举，不建立引用。Getattr 查询已有编号，不建立新引用。本地缓存不足时内核会撤回不能接纳的借用。卸载是整个连接生命周期的终止，SDK 接收到 EOF 返回；重新注册建立新连接，不继承旧句柄。

用户态意味着插件可以独立演进和退出，不代表任意插件都天然具备持久化、ACL、崩溃恢复或高吞吐。具体存储语义由插件实现；当前桥接版本仍有串行请求和 VFS 命名空间锁的限制。

## 用户态系统后端迁移中的 ABI 变更

当前源码的协议版本为 **3**。`Attributes` 包含 `major`、`minor`；
桥接层据此将字符设备、块设备节点绑定到内核设备机制。普通文件与目录应将这两个字段置零。
版本 1/2 的服务需要用当前头文件及 SDK 重新编译，不可混用旧消息布局。
每个请求携带内核填写的 `RequestContext{pid, uid, gid}`，其中 uid/gid 是原始客户端的有效身份，
不是服务进程自身身份。SDK 在分派前调用 `Backend::context()`；需要跨回调保留时应复制该记录。
用户态根服务的引导及验证进度见 [FS 服务迁移](fs-service-migration.md)。

## 每次打开的独立快照

只读合成文件系统可设置 `ReadOnly | Snapshots`，并声明完整的 `SnapshotOperations`。
内核每次 open 使用以下回调构造独立字节流：

1. `snapshot(node, size)` 捕获内容并返回准确长度。
2. `snapshot_read(offset, data, count)` 按协商载荷传输，返回实际字节数。
3. `snapshot_end()` 释放临时捕获；捕获或传输失败时也会调用，因此必须允许清理未完成状态。

内核在整个捕获期间串行化该连接。提前 EOF、负返回值、超过请求长度或本地缓冲分配失败会令这次 open 失败，
并回收文件槽；普通错误不应污染后续捕获。成功后，每个打开文件拥有独立的只读缓冲，
dup/fork 仍共享该打开文件的偏移。目标进程退出、fd 复用或后来再次打开文件都不会替换既有缓冲。
目录快照沿用项目的 16 字节 dirent 字节流，原有 read(dir)/getdents64 的不可混用约束继续生效。
`user/fs/backends/procfs/backend.cpp` 提供完整实现，`fssnapshottest` 验证捕获失败、独立打开和客户端身份。

## 文件系统驱动的原生功能

普通回调之外，驱动可通过版本化的 Control 命名空间提供自己的功能。契约、权限与元数据刷新约定、ext3 实现和 fsctl 用法见 [驱动契约](fs-driver-contract.md)。

## 当前扩展边界

清单和 ELF 插件可以扩展文件系统算法、存储格式和数据来源。协议 3 仍有明确的公共边界：
对象编号与属性中的长度是 32 位，旧端点单条载荷最多 512 字节，共享通道最大 16 KiB（xv6 日志事务限制为 3 KiB）；每个连接串行处理请求，部分 VFS 操作仍持命名空间锁。
插件应按接口能力位声明实际支持的操作，不能把“独立加载”解释成已经支持任意外部文件系统的全部语义。
新增 xattr、ACL 或更大文件属性等接口需要演进公共协议；新增已有协议能够表达的后端无需改内核。
用户态默认栈只有 4 KiB，大型目录表、消息数组和恢复缓冲应放在堆上；监督模块通过编译器检查单帧大小。

测试 `fs_elf_plugin_{ext3,xv6}_{1,5}cpu` 从仓库外副本编译 C ELF，再离线安装到私有镜像，验证发现、参数、写源、故障恢复及重复挂载。
`fs_elf_root_{1,5}cpu` 在未格式化磁盘上运行新根插件，并验证不可变资产与描述符执行的权限边界；两组都检查原内核/引导包哈希不变。
