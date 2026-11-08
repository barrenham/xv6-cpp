# 用户程序终端样式

用户可见的帮助、错误、表头、状态和结果采用统一的 ANSI 配色。配色由连接串口的终端模拟器显示；客户端决定背景、字体和调色板。

| 信息 | 样式 |
| --- | --- |
| 表头、输入提示 | 默认前景色加粗 |
| 普通说明、详细信息 | 默认前景色 |
| 成功、运行中、已绑定 | 绿色 |
| 用法、等待、未绑定、警告 | 黄色 |
| 错误、失败、不可用 | 亮红色 |
| 辅助说明、停止状态 | 灰色 |

## 全部应用的输出约定

已按 `user/apps` 应用目录检查全部 53 个应用：

| 分类 | 应用 |
| --- | --- |
| accounts | `groupadd`, `groupdel`, `id`, `login`, `passwd`, `su`, `useradd`, `userdel`, `usermod` |
| coreutils | `cat`, `echo`, `env`, `grep`, `wc` |
| examples | `cppdemo`, `zombie` |
| filesystem | `chmod`, `chown`, `cp`, `fsboot`, `fsctl`, `ln`, `ls`, `memfsd`, `mkdir`, `mount`, `mv`, `pwd`, `ramfsd`, `readlink`, `rm`, `sync`, `tarfsd`, `umount` |
| network | `curl`, `iptables`, `netctl`, `nslookup`, `ping` |
| shell | `sh` |
| system | `drvctl`, `free`, `init`, `kill`, `ps`, `servicectl`, `serviced` |
| terminal | `clear`, `fbdemo`, `fbterm`, `pty`, `stty`, `vim` |

管理工具保留已有列名、字段含义和对齐：`ps`、`free`、`drvctl`、`fsctl`、`servicectl`、`mount` 和 `iptables` 着色表头/信息；驱动绑定和服务状态按状态着色。账户操作、文件操作、网络和终端工具的错误与用法同样采用统一样式。登录与密码输入提示支持配色，密码回显和认证流程保持原有设置。

对数据工具，输出内容本身就是接口：`echo`、`cat`、`grep` 匹配结果、`wc` 数字、`env`、`pwd`、`readlink`、`curl` 响应体及 `-w` 自定义格式保持原样；它们的错误/用法使用统一样式。`clear` 的控制序列、PTY/终端代理的转发数据和 fbterm 性能记录也保持原样。成功时原本不输出的 `sync`、文件操作和 `zombie` 不额外打印提示。

`user/tests` 的测试协议、断言、基准数据继续使用原有 printf/write；不通过全局格式化器猜测字符串含义或给所有字节强行加颜色。FS 引导、服务管理和插件进程面向人的状态/错误使用相同样式；写入日志文件时自动退回纯文本。

## 使用

默认根据每条消息的目标描述符检测终端，stdout 和 stderr 独立处理。重定向 stdout 不妨碍仍输出到终端的 stderr 错误配色。`--raw` 模式的正常输出继续保持纯文本。

```sh
ps
free
drvctl devices
drvctl status 0
fsctl /
servicectl list
mount
iptables -L

export NO_COLOR=1  # 关闭当前环境中的自动配色
unset NO_COLOR    # 恢复
```

`TERM=dumb` 也禁用自动配色；没有设置 TERM 的串口终端仍启用。主动清理环境的登录/权限切换程序遵循原有环境清理策略，新环境中的颜色按新环境判断。`ls` 额外支持 `--color=auto|always|never`，显式 always 可强制输出颜色。

## 实现

公共接口为 `user/runtime/ui.h` 的 `heading/info/success/warning/error/usage/muted` 和 `print(fd, Tone, ...)`。输出语义由调用处明确选择，不修改通用 printf/write 的数据语义。终端检测保留 errno；颜色前缀、格式化正文和重置序列共用 printf 的 256 字节栈缓冲，短消息一次 write，保留短写和 EINTR 处理。没有动态分配或全局可变颜色状态。

新应用应使用 ui 输出人类提示，用 printf/write 输出数据；不要对含颜色码的字符串直接做列宽计算。shell 继续分别保存提示符的显示序列和可见字符宽度。

## 验证

`user_printf_contract` 覆盖未着色格式化、样式/正文/重置一次提交、stdout/stderr 独立检测、errno、NO_COLOR、TERM=dumb、原样模式，以及样式跨缓冲边界时的短写和中断。扩展的 `serial_colors_1cpu/5cpu` 在真实 QEMU UART 上检查系统/FS/网络/账户工具、raw 输出、管道、重定向和跨 80 列编辑。客户机回归还覆盖 session、workflow、PTY、HTTP、DNS、防火墙和服务管理。

构建和验证日志：`build/user-ui-build.log`、`build/user-ui-native-validation.log`、`build/user-ui-guest-validation.log`。

验证结果：原生 1/1 通过；累计 10 个不同的客户机用例均已通过。首轮发现的 DNS 标签/地址间颜色码问题已改为整行输出；解析 ps 的综合测试显式请求 NO_COLOR，并按实际表头同步服务恢复时的异步 UART 输出。DNS/网桥复测见 `build/user-ui-final-validation.log`，服务综合复测见 `build/user-ui-service-validation.log`。

安装状态：最新配色与 `/bin/vim` 已安装；145 个系统文件与构建产物一致，服务和账号配置保留，只读 e2fsck 通过。最新日志见 `build/vim-rootfs-update.log`、`build/vim-rootfs-integrity.log`。

配色调整：普通信息恢复默认文字色，表头只加粗，root 提示符改为暖黄色，符号链接改用紫色。仅状态、告警和文件分类保留少量强调色，详见 [简易编辑器](vim.md)。
