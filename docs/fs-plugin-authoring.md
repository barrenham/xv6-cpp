# 开发独立用户态 FS 插件

FS 插件是一个用户态可执行程序。内核只安装一次通用桥接层；插件按公共 ABI 注册自己，文件操作通过服务描述符交换。内核不动态加载插件代码到自己的地址空间。

## 最小服务结构

包含 `user/runtime/userfs/server.h`，继承 `userfs::Backend`。只读插件至少需要实现：

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

本版接入普通文件、目录和符号链接；设备、socket 等内核对象继续由现有内核机制管理。

服务入口的公共部分如下：

```cpp
MyBackend backend(source_descriptor);
int service = userfs::register_backend(type_from_arguments, backend);
if (service < 0) exit(1);
int result = userfs::serve(service, backend);
close(service);
exit(result < 0);
```

后端文件、块设备或网络连接应在注册前准备好。客户端在挂载后立即发起请求也应能处理；不能依赖在第一次请求后才通过路径查找打开后端，否则可能重入 VFS 的命名空间锁。

完整的独立只读后端见 [tarfsd](../user/apps/tarfsd/tarfsd.cpp)，它只实现 Backend 回调，SDK 处理所有协议分派；[memfsd](../user/apps/memfsd/memfsd.cpp) 演示可写操作和对象回收。两个后端没有对应的内核专用分支。

## 独立构建

在 `user/CMakeLists.txt` 中向 programs 列表加入程序源文件，或给自己的用户态目标链接项目的 user_runtime、user_fs、xv_options 及用户链接脚本。`user_fs` 是静态 SDK 库，只有引用它的程序才会提取服务循环代码，不给所有普通程序增加服务代码。

已有后端的独立构建命令：

```sh
cmake --build build/dev --target user_tarfsd
```

该目标只构建用户态依赖和 tarfsd，不依赖 kernel 目标。新增算法或改动插件代码无需改变注册表、VFS、系统调用分派或重新链接内核。为放入新安装种子可再构建 image；更新正在使用的持久 rootfs 必须使用离线更新流程并保留账户目录。

## 注册契约

低层 `xv::userfs::Registration` 定义在 [protocol.h](../kernel/fs/userfs/protocol.h)：

- size/version 标识完整结构及公共协议版本；不匹配则注册失败。
- type 是唯一的类型名称标识，可以存在多个同名实例，注册不自动挂载。
- root 提供初始根属性，根对象编号必须为 1 且为目录。
- flags 当前支持 ReadOnly；内核独立执行只读策略。
- operations 声明服务支持的操作位；Lookup/Getattr/Read/Readdir/Forget/Sync 是基础集合。
- max_payload 在 1..512 范围内，决定该实例每条读写请求的最大数据载荷。

挂载使用 `mount -t type /existing/directory`。不同服务进程可以使用同一个可执行文件、不同参数和不同存储源注册同一类型的多个实例。挂载器会选择尚未挂载的实例；已经挂载、停止或失去连接的实例不会再被选择。内核不会代替用户态进程管理器启动 FS 程序。

读取 `/proc/fsplugins` 可以观察 registered、mounting、mounted、disconnected、stopped 状态。注册失败、挂载失败、进程退出和资源容量耗尽都是可观察的错误，不能用无限等待处理。服务注册后应立即进入接收循环，不使用终端输入作为文件系统请求通道。

## 引用和存储一致性

编号必须在仍被内核引用时稳定。多次 Lookup/Create 可能对应同一编号；内核合并本地引用，最终一次释放只发送一个 Forget。服务可以维护“内核仍引用该编号”的标志。链接计数为零的对象仍可能被打开，必须在最终 Forget 后才回收并复用编号。

Readdir 只是枚举，不建立引用。Getattr 查询已有编号，不建立新引用。本地缓存不足时内核会撤回不能接纳的借用。卸载是整个连接生命周期的终止，SDK 接收到 EOF 返回；重新注册建立新连接，不继承旧句柄。

用户态意味着插件可以独立演进和退出，不代表任意插件都天然具备持久化、ACL、崩溃恢复或高吞吐。具体存储语义由插件实现；当前桥接版本仍有串行请求和 VFS 命名空间锁的限制。

## 用户态系统后端迁移中的 ABI 变更

当前源码的协议版本为 **2**。`Attributes` 在原有字段后增加 `major`、`minor`；
桥接层据此将字符设备、块设备节点绑定到内核设备机制。普通文件与目录应将这两个字段置零。
版本 1 的服务需要用当前头文件及 SDK 重新编译，不可把旧消息布局直接用于版本 2。
用户态根服务的引导及验证进度见 [FS 服务迁移](fs-service-migration.md)。
