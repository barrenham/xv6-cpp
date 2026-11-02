# 通用文件系统桥接插件

内核提供统一的 userfs 桥接插件和运行时服务注册表。独立用户态 FS 提交版本、类型名、根属性、操作能力与只读标志；内核将具名类型接入 VFS，应用继续使用普通文件接口。新增实现只需编译、安装用户态程序，不需要为新类型修改内核分派、增加静态后端对象或重新链接内核。

注册、挂载和实例寿命分开：每个服务端点代表一个待挂载实例；同一类型可注册多个实例。`mount -t 类型 目录` 选择该类型尚未挂载的实例，一条连接只能被挂载一次。FS 程序如何启动、如何打开存储或网络资源由用户态决定，内核不包含 memfsd/tarfsd 的实现。

## 使用已有插件

镜像包含 SDK 驱动的可写内存后端 memfsd、磁盘归档后端 tarfsd，以及可直接使用的 USTAR 样本。以内存后端为例，先注册：

```sh
memfsd --register my-memory &
cat /proc/fsplugins
```

看到该类型处于 `registered` 后挂载：

```sh
mkdir /tmp/memory
mount -t my-memory /tmp/memory
echo hello > /tmp/memory/message
cat /tmp/memory/message
umount /tmp/memory
```

归档后端从已经打开的磁盘文件读取实际内容，不把整个归档加载到用户进程内存：

```sh
tarfsd /usr/share/doc/xv6/plugin.tar my-archive &
cat /proc/fsplugins
```

看到 `my-archive` 已注册后：

```sh
mkdir /tmp/archive
mount -t my-archive /tmp/archive
cat /tmp/archive/hello.txt
ls /tmp/archive/docs
umount /tmp/archive
```

tarfsd 为只读后端；归档源文件在服务退出后仍然保留。重新启动服务并注册另一个类型名，也能挂载同一归档。服务中的目录索引是内存数据，文件内容按需从源描述符读取。示例支持有界 USTAR 普通文件、目录和符号链接；不支持 PAX/GNU 扩展、硬链接头或压缩归档。

`cat /proc/filesystems` 查看内核及动态注册类型；`cat /proc/fsplugins` 查看每个服务的类型、PID、注册/挂载/断开状态、协议版本、只读标志及实例设备号。`mount` 和 `/proc/mounts` 展示具体类型名，同类型实例有独立的设备号、缓存和内容。

## 接入新文件系统

[用户态 SDK](../user/runtime/userfs/server.h) 提供 `userfs::Backend` 操作接口；[公共服务循环](../user/runtime/userfs/server.cpp) 负责消息接收、校验、操作分派和回复。只读后端通常实现 root、lookup、getattr、readdir、read，并声明 ReadOnly/ReadOperations；可写后端实现 create/write/resize/setattr/link/unlink/rename/sync。存储算法不需要理解内核 inode 指针或 VFS 挂载表。

服务通过 `userfs::register_backend(type, backend)` 注册，取得描述符后调用 `userfs::serve(fd, backend)`。开发者也可直接使用 `fsregister(Registration*)` 和 read/write/poll 实现其他语言的等价协议客户端。具体开发与独立构建说明见 [插件开发接口](fs-plugin-authoring.md)。全部系统 FS 的迁移进度和独立引导路径见 [FS 服务迁移](fs-service-migration.md)。

类型名是以小写字母开头、最长 31 字节的 ASCII 名称，后续允许小写字母、数字、短横线、下划线。旧启动方式禁止覆盖内建类型名；独立服务引导方式允许系统服务注册这些类型。注册仅允许有效 uid 0；结构长度、版本、标志、根节点和必需操作能力必须通过内核校验。每条消息的版本、请求编号、操作、数据长度也会检查。协议是本项目的 64 位本地 ABI，不是 Linux FUSE 二进制协议。

每实例的 `max_payload` 可在 1..512 字节之间协商，较大读写由桥接层拆分；未声明的操作在内核直接失败。只读标志同时约束 VFS 写权限和桥接层可发送的修改操作，不依靠服务自觉拒绝写入。

## 内核分工和实例管理

[registry.cpp](../kernel/fs/userfs/registry.cpp) 是通用插件注册、选择和回收入口；[userfs.cpp](../kernel/fs/userfs/userfs.cpp) 是 VFS 与协议的适配器。两者不包含具体插件类型分派。实例及 inode 页按需分配，当前注册表容量为 16；同名服务共享类型名称，但使用独立实例。一次挂载尝试会先固定选中的实例，失败后释放该固定，使后续挂载可以重试。

注册表锁仅用于选择、快照和摘除条目，不持锁执行路径查找或设备 I/O。结束挂载和关闭最后一个服务描述符分别改变实例状态；两者都完成且无挂载尝试占用时，实例才可回收。后续注册批量摘除退役条目，在锁外释放页，避免旧描述符访问新实例及在自旋锁内释放内存。

内核负责路径路由、挂载、描述符、传统 uid/gid/mode 权限、用户缓冲区复制和 inode 生命周期。服务负责目录、属性、内容与持久化。每个编号可以被多次 Lookup/Create 返回，所有本地引用归零时只发送一次聚合 Forget；服务可据此回收已删除对象。Getattr、Readdir 不增加这种引用。若内核 inode 缓存已满，桥接层撤回不能接纳的远端引用。

stat/fstat 向服务刷新属性；chmod/chown 由服务确认后才更新本地属性；fsync 把服务 Sync 失败传回应用。持久化由插件定义，Sync 成功必须兑现该插件的持久化承诺。桥接层不会替插件记录磁盘日志。

每次请求最多等待 5 秒；关闭、被杀、中断或超时都会使等待者返回错误，超时和中断会停用连接，避免迟到回复影响后续请求。打开的文件或工作目录引用会阻止卸载。卸载优先匹配已登记路径，因此父服务断开后仍能先清理子挂载；相对路径或其他别名继续使用原路径解析。busy 检查还等待最后一次引用归还的 Forget 完成，再允许回收实例。正常卸载通知服务 EOF，服务关闭描述符后，类型在没有其他活动实例时消失。断开的挂载需显式卸载，不自动恢复旧句柄或重放可能已生效的写操作。

## 边界与验证

当前每个实例只有一条在途请求，每实例最多 64 个活跃缓存 inode，VFS 总挂载数仍为 8。桥接框架没有共享数据缓冲区、零复制或并行请求队列。原有 VFS namespace 锁仍可能在路径操作等待服务期间延迟其他路径查找；服务应在注册前打开后端文件/设备，并在请求处理中使用已有描述符，避免重入路径查找。

memfsd 是一个 SDK 后端，文件内容随服务退出消失；它最多同时保留 128 个对象、256 个目录项，每文件 1MiB，最后引用关闭后可回收。tarfsd 是另一个 SDK 后端，最多索引 128 个对象，按需读取持久源文件。两种后端限制不等于桥接框架只支持这两种文件系统。

`fsplugins_1cpu`、`fsplugins_5cpu` 用真实用户进程验证：任意长类型名注册、同类型多实例、统一 mount 工具、非内存归档读取、只读执行、退出后重新注册挂载、能力/版本/权限校验、注册表占满及回收。`userfs_1cpu`、`userfs_5cpu` 保留完整读写、错误回复、杀死、超时和 160 轮对象回收验证。独立构建还重新编译、链接 tarfsd，并核对内核 ELF 的 SHA256 保持不变。

完整源码/二进制指纹、独立构建证明、16 项回归、最终协商与样本用例、分配失败注入、ASan/UBSan 和安装记录见 [插件框架验收](fs-plugins-acceptance.json)。持久镜像已备份并更新服务、挂载工具、测试程序与样本，账户及用户目录逐项核对未改变。

源码/二进制指纹、独立构建证明、最终 16 项回归、分配失败注入、ASan/UBSan 与安装记录见 [插件框架验收](fs-plugins-acceptance.json)。服务、挂载工具、测试程序和样本已安装到备份后的持久镜像，账户及用户目录内容和属性逐项核对未改变。
