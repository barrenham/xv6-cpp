# 用户、组与权限

内核按数字 UID/GID 授权，用户态账户库负责名称映射。系统启动后进入登录界面。首次启动需要设置 root 密码，没有内置默认密码；登录 root 后可以创建账户并设置其他用户密码。

```sh
id
useradd alice
passwd alice
su alice
id
umask 027
echo hello > /tmp/alice-file
chmod 600 /tmp/alice-file
ls -l /tmp/alice-file
```

在 alice shell 按 Ctrl-D 回到 root。也可以执行 `su alice id`；只有 root 可以使用 su 切换账户。

## 账户与凭据

`Credentials` 属于 Process，独立自旋锁保护真实、有效、保存 UID/GID、最多 16 个附加组和 umask。ID 范围为 0..65534；0 是 root。fork 复制整份凭据，普通 exec 保持真实/有效 UID/GID 和附加组，并将保存身份设为新的有效身份；带 setuid/setgid 位的普通可执行文件按 inode 属主修改有效身份。不从环境变量 USER 判断权限。

| 接口 | 作用 |
| --- | --- |
| getuid/geteuid、getgid/getegid | 查询真实与有效身份 |
| setuid/setgid | root 同时设置真实、有效、保存身份；非 root 只能把有效身份切换到真实或保存值 |
| seteuid/setegid | 只修改有效身份，不能取得未经授权的 ID |
| getgroups/setgroups | 查询附加组；仅 root 可替换列表 |
| umask | 设置创建掩码，返回旧值，默认 0022 |

`init` 初始化 root 和 nobody 两个账户，创建账户目录和主目录。路径固定为 `/etc`、`/home` 和 `/root`，默认均存放在持久 rootfs 上，挂载额外磁盘不改变账户路径。账户采用七字段 passwd 格式，密码字段为 `!`，文件权限 0644；`/root` 为 0700。`useradd NAME` 从 1000 分配 UID 和同号主组，创建 0750 的 `/home/NAME`，通过账户目录中的 `.users.lock` 串行化管理写入。UID/GID 使用持久的 idnext 单调分配，账户或组删除后不会重新分配旧身份给新账户。

`su` 读取目标账户，清空环境并设置可信 PATH/HOME/USER/SHELL，关闭 3 及以上描述符，从组数据库装入附加组，先 setgid 再 setuid，最后切换到目标主目录并执行 shell 或指定命令。标准输入输出继续使用已授予的终端描述符。

## 登录与密码

`init` 在控制台运行 `/bin/login`，登录 shell 退出后重新显示登录提示。登录成功后清空环境，设置可信 PATH/HOME/USER/SHELL，从组数据库装入附加组，先设置 GID 再永久设置 UID，切换到账户主目录并执行账户 shell。用户名和密码验证失败均返回 `Login incorrect`，失败后等待 1 秒。密码输入关闭终端回显，退出输入过程恢复原设置；shell 退出后 init 也恢复控制台设置。

`passwd [USER]` 无参数修改当前真实 UID 对应账户；root 可指定其他账户，普通用户只可修改自己且必须验证当前密码。镜像中的 `/bin/passwd` 为 root:root 04755，程序入口关闭额外 fd、清空环境、设置 0077 umask，并按真实 UID 校验目标。获取数据库锁后再次核对账户身份，防止名称删除/重建期间改到另一个 UID。`useradd` 创建的账户默认没有密码记录，不能登录，root 执行 `passwd USER` 后才可使用；`su` 仍是仅 root 使用的身份切换工具。

密码为 8..63 字节，设置时需要重复输入。散列存于 root:root 0600 的 `/etc/shadow` 文件，`/etc/passwd` 继续只保存公开账户信息。记录为 `name:pbkdf2-sha256$100000$盐的十六进制$散列的十六进制`，盐长 16 字节，派生结果 32 字节；比较结果时合并全部字节差异。算法依据 [RFC 8018 的 PBKDF2](https://www.rfc-editor.org/rfc/rfc8018.html#section-5.2) 与 [RFC 6234 的 SHA-256/HMAC](https://www.rfc-editor.org/rfc/rfc6234.html)。宿主机契约测试用独立参考实现生成的向量验证散列、分块边界、HMAC 和多种迭代次数。

盐由单调硬件计时值、PID、启动 tick、账户名和旧数据库摘要生成，用于减少重复。当前没有内核 CSPRNG，盐不具备密码学随机性的保证；迭代参数面向 QEMU 教学环境，未实现生产认证所需的内存困难 KDF、持久限速策略或密码恢复服务。

密码与账户/组更新共用账户目录中的 `.users.lock` 串行化：先以 0600 写完整临时文件并 fsync，再通过同一磁盘日志事务中的 rename 原子替换 shadow，失败保留原数据库。崩溃可能遗留锁目录，管理员处理后重试。已有但无效的 root shadow 不触发首次注册流程，init 报错；没有 shadow 的旧镜像会在启动时进入首次设置。

## 账户与组管理

```sh
useradd alice
passwd alice
groupadd staff
usermod -aG staff alice
su alice id
usermod -G staff alice
groupdel staff
userdel alice
```

`groupadd`、`groupdel`、`usermod` 和 `userdel` 仅 root 可使用。group 文件采用 `name:!:gid:member,member`，权限 0644。useradd 创建同名私有主组；usermod -G 替换附加组列表，-aG 追加，最多 16 个。变更对下一次 login/su 会话生效，不修改已运行进程的凭据。不能删除 root 组或仍被某账户作为主组的组。

userdel 删除 passwd/shadow 中的账户记录并从附加组列表移除，保留主目录、文件和原私有组，不杀死现有进程。文件继续属于旧数字 UID；新账户不会取得被删除的 ID。重新使用同名账户前，管理员需处理遗留目录和同名私有组，不会自动 chown 旧数据。

每个数据库文件先写临时文件并 fsync，再原子 rename。多个数据库之间不是一个跨文件事务，崩溃仍可能留下待整理的组/目录或锁；保留单调 ID 分配记录避免因重用身份而取得旧文件访问权。

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

执行 setuid/setgid 位只在候选 ELF 成功提交后生效，失败 exec 不改变凭据。普通文件写入、截断和 chown 清除特殊执行位；非 root 设置 setgid 时需要属于文件的组。不支持目录的 setgid 继承语义，chmod/mkfs 会拒绝目录特殊执行位。只有 passwd 默认安装特权位，su 仍要求 root。

未实现 ACL、capabilities 和磁盘配额。

## 进程与网络

用户发送信号需要 root，或发送者真实/有效 UID 匹配目标真实/保存 UID。进程组发送逐目标授权。内核终端产生的信号由内核通知路径发送，不依赖中断发生时是否有当前进程。

网络状态查询允许普通用户，接口、地址、路由、邻居、网桥和 DHCP 等配置修改仅 root。绑定端口 1..1023 仅 root；端口 0 和普通高端口不受此特权限制。

## 磁盘格式与验证

磁盘 inode 从 64 字节扩为 128 字节，增加 uid/gid/mode，FSMAGIC 改为 0x10203041。旧镜像不能直接加载。构建会用新版 mkfs 生成 fs.img；保留旧数据前先备份镜像，再在新格式中导入文件。mkfs 按镜像安装清单设置属主与权限：普通目录与程序默认 root:root 0755，/root 为 0700，/tmp 为 01777，文档为 0644。默认账户 shell 为 /bin/sh。

`permissiontest` 覆盖磁盘/RAM 权限持久化、目录搜索、真实与有效身份、附加组、sticky 删除、PTY 属主、进程和网络授权、降权不可恢复与非法参数。permissions 单核/五核测试还覆盖 useradd、命令式 su 与交互式 su。原生 process_contract/fs_contract 覆盖凭据状态机和后端授权契约。
