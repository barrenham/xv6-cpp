# 环境变量与 shell

环境属于用户进程。内核在 execve 时复制环境到新用户栈，用户运行库通过 `environ` 管理它。fork 复制地址空间，父子之后分别修改自己的环境；exec 保留凭据和 umask，替换程序及环境。

```sh
export MESSAGE="hello world"
echo "$MESSAGE"
env MESSAGE=child echo "$MESSAGE"
env -i PATH=/bin id
unset MESSAGE
```

第三行的 `$MESSAGE` 由当前 shell 先展开，传入子进程的环境值则为 `child`。可以用 `env MESSAGE=child env` 查看子环境。

## 接口与启动路径

`user/runtime/environment.cpp` 提供 `getenv`、`setenv`、`unsetenv`、`clearenv`、`execv`、`execvp` 和原有 `exec` 包装。`setenv` 深复制名字和值，替换时先准备完整的新块，失败保留旧环境；`getenv` 返回借用指针，修改环境后应重新查询。程序是单执行线程模型，没有并发环境修改协议。

`execve(path, argv, envp)` 显式指定环境，`envp == nullptr` 表示空环境。`exec` 与 `execv` 传入当前 `environ`；`execvp` 对不含斜杠的程序名依次搜索 PATH，PATH 未设置时使用 `/bin:/sbin:/usr/tests`，空路径组件表示当前目录。内核旧 SYS_exec 入口仍使用空环境，当前运行库不使用它继承环境。

ELF 装载器在独立候选地址空间准备参数和环境，成功后才提交。RISC-V 入口收到 a0=argc、a1=argv、a2=envp，`_user_start` 初始化 environ 后调用 main。参数或环境坏指针、超限、复制或 ELF 失败都不替换旧程序。

环境最多 31 项，运行库环境字符串总量最多 64 KiB；exec 的参数、环境字符串及两个指针向量还必须共同装入当前单页用户栈，因此执行边界小于环境编辑边界。参数也最多 31 项。

## shell 行为

支持 `$NAME`、`${NAME}`、单引号、双引号和反斜杠。单引号内不展开变量，双引号保留一个参数；没有字段拆分、通配符、命令替换或完整 POSIX shell 语法。

- `export NAME=value` 修改 shell 环境，`export` 列出环境。
- `unset NAME` 删除变量。
- `NAME=value command` 为执行该命令的子进程设置变量；单独赋值保留在当前 shell。
- `cd` 无参数时使用 HOME；`umask` 查看或设置八进制创建掩码。
- 命令执行使用 execvp；重定向路径也在执行时展开。

启动的 init 设置 PATH=/bin:/sbin:/usr/tests、HOME=/root、USER=root、SHELL=/bin/sh。内建命令在所属 shell 执行，管道和后台命令中的修改不会影响父 shell。输入行上限为 511 字节。

提示符显示为 `用户名:当前绝对路径$ `，例如 `root:/tmp$ `。用户名按有效 UID 查询账户库，不使用可修改的 USER 环境变量；查不到账户时显示数字 UID。路径每次从当前目录沿父目录查找，支持跨挂载点，嵌套启动 shell 也显示实际目录；目录已删除、父目录不可读或路径超过查找缓冲限制时显示 `?`。提示符通过一次 write 输出，作业通知后重绘使用相同格式。

`envtest` 和 environment 单核/三核冒烟测试覆盖复制、覆盖、删除、fork 隔离、显式/空环境、PATH、引号和失败回滚。

## 作业通知与终端输出

shell 的 Running、Stopped、Foreground 和 Done 消息先在局部缓冲中拼成完整一行，再通过一次 write 输出。TTY 的写锁串行化该调用，避免通知被其他进程输出逐字符插开；它不承诺多个独立 write 之间的整段输出原子性。

SIGCHLD 仍通过自管道唤醒 shell，前台作业运行期间也及时回收其他后台作业并报告状态。foreground 单核/三核测试连续六轮中断前台并清理后台，同时检查完整结果和唯一的 Done 通知。
