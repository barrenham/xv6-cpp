# 用户、组与权限

内核按数字 UID/GID 授权，用户态 `/etc/passwd` 负责名称映射。系统以 root shell 启动，root 可以创建账户并切换身份。

```sh
id
useradd alice
su alice
id
umask 027
echo hello > /tmp/alice-file
chmod 600 /tmp/alice-file
ls -l /tmp/alice-file
```

在 alice shell 按 Ctrl-D 回到 root。也可以执行 `su alice id`；只有 root 可以使用 su 切换账户。

## 账户与凭据

`Credentials` 属于 Process，独立自旋锁保护真实、有效、保存 UID/GID、最多 16 个附加组和 umask。ID 范围为 0..65534；0 是 root。fork 复制整份凭据，exec 保持凭据，不从环境变量 USER 判断权限。

| 接口 | 作用 |
| --- | --- |
| getuid/geteuid、getgid/getegid | 查询真实与有效身份 |
| setuid/setgid | root 同时设置真实、有效、保存身份；非 root 只能把有效身份切换到真实或保存值 |
| seteuid/setegid | 只修改有效身份，不能取得未经授权的 ID |
| getgroups/setgroups | 查询附加组；仅 root 可替换列表 |
| umask | 设置创建掩码，返回旧值，默认 0022 |

`init` 初始化 root 和 nobody 两个账户，创建 `/etc`、`/home` 和 `/root`。账户采用七字段 passwd 格式，密码字段为 `!`，文件权限 0644；`/root` 为 0700。`useradd NAME` 从 1000 分配 UID 和同号主组，创建 0750 的 `/home/NAME`，通过 `/etc/.users.lock` 串行化管理写入。

`su` 读取目标账户，清空环境并设置可信 PATH/HOME/USER/SHELL，关闭 3 及以上描述符，清空附加组，先 setgid 再 setuid，最后切换到目标主目录并执行 shell 或指定命令。标准输入输出继续使用已授予的终端描述符。

当前账户工具没有密码认证、登录服务、账户删除、独立组数据库或密码修改命令。账户追加不是崩溃原子更新；写入中断后管理员需要检查 passwd 和遗留锁目录。它提供本机 root 管理的多用户执行模型。

## 文件访问

inode 和 stat 都包含 uid/gid/mode。普通访问使用有效身份，从 owner/group/other 中选择权限，组判断包含有效主组和附加组。

| 操作 | 检查 |
| --- | --- |
| 路径遍历、chdir | 每个目录 X，chdir 最终目录也需要 X |
| open | 最终节点 R/W，与访问模式对应；O_TRUNC 额外需要 W |
| 创建、硬链接 | 父目录 W 和 X |
| unlink | 父目录 W/X；sticky 目录还要求 root、目录属主或目标属主 |
| exec | 普通文件 X；root 也要求文件存在某个执行位 |
| chmod | root 或属主，支持 rwx 与 sticky，即 0000..01777 |
| chown | root 可转移属主组；非 root 属主只能保留 UID，并改到自己所属的组；-1 表示不改 |
| access | 路径遍历与最终访问都使用真实 UID/GID |
| stat | 路径搜索权限；最终文件不需要 R |

root 绕过普通读写和目录搜索位，仍受只读后端及文件执行位规则约束。新文件模式为 0666、目录 0777，再扣除 umask；设备创建仅 root，初始 0600。已打开 fd 保留授予的读写能力，之后 chmod 或降权不会撤销 fd，这也适用于 fork 继承。

`/tmp` 为 root:root 01777。devfs 命名空间和属性只读，console 为 root:root 0600，null/zero/ptmx 为 0666，命名 pts 为创建者 uid/gid 的 0600。procfs 的 fd 目录为目标有效身份的 0500，fd 信息为 0400；描述符服务再次验证调用者是 root 或有效 UID 同时匹配目标真实和有效 UID，避免仅依赖缓存节点属性。

未实现 ACL、capabilities、setuid/setgid 可执行位和磁盘配额。

## 进程与网络

用户发送信号需要 root，或发送者真实/有效 UID 匹配目标真实/保存 UID。进程组发送逐目标授权。内核终端产生的信号由内核通知路径发送，不依赖中断发生时是否有当前进程。

网络状态查询允许普通用户，接口、地址、路由、邻居、网桥和 DHCP 等配置修改仅 root。绑定端口 1..1023 仅 root；端口 0 和普通高端口不受此特权限制。

## 磁盘格式与验证

磁盘 inode 从 64 字节扩为 128 字节，增加 uid/gid/mode，FSMAGIC 改为 0x10203041。旧镜像不能直接加载。构建会用新版 mkfs 生成 fs.img；保留旧数据前先备份镜像，再在新格式中导入文件。mkfs 按镜像安装清单设置属主与权限：普通目录与程序默认 root:root 0755，/root 为 0700，/tmp 为 01777，文档为 0644。默认账户 shell 为 /bin/sh。

`permissiontest` 覆盖磁盘/RAM 权限持久化、目录搜索、真实与有效身份、附加组、sticky 删除、PTY 属主、进程和网络授权、降权不可恢复与非法参数。permissions 单核/三核测试还覆盖 useradd、命令式 su 与交互式 su。原生 process_contract/fs_contract 覆盖凭据状态机和后端授权契约。
