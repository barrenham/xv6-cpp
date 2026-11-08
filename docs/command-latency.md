# 普通命令输出延迟优化（2026-11-08）

## 原因与修改

公共运行库原先每输出一个字符调用一次 `write`。`ls` 每行属性和名称因此产生几十次系统调用，涉及终端锁、唤醒和调度。现在每次 `printf/fprintf/vprintf` 独立使用 256 字节栈缓冲，满时提交，调用返回前刷新。短行通常只需一次系统调用；无换行提示符仍立即刷新。

没有全局共享缓冲或额外分配。处理短写与 EINTR；零进展或终止错误停止提交，避免非阻塞目标上空转。保持现有格式化接口和列布局。多个调用之间不保证输出原子性。

## 测量

QEMU TCG、128 MiB，分别使用 1/5 CPU；每次启动使用独立测试盘，串口登录后按相同顺序运行两轮。使用宿主机 monotonic 时钟，记录提交命令到 shell 提示符返回的总耗时。测量期间没有并行构建。下表是两轮平均值，样本少，不能视为硬性性能保证。

| CPU | 命令 | 修改前（秒） | 修改后（秒） |
| --- | --- | ---: | ---: |
| 1 | `/bin/echo latency-probe` | 0.216 | 0.401 |
| 1 | `ls /bin > /dev/null` | 1.520 | 1.669 |
| 1 | `ls /bin` | 2.843 | 1.640 |
| 1 | `ls /usr/tests > /dev/null` | 3.304 | 2.890 |
| 1 | `ls /usr/tests` | 6.783 | 2.798 |
| 5 | `/bin/echo latency-probe` | 0.402 | 0.419 |
| 5 | `ls /bin > /dev/null` | 1.756 | 1.318 |
| 5 | `ls /bin` | 4.818 | 1.373 |
| 5 | `ls /usr/tests > /dev/null` | 3.185 | 2.383 |
| 5 | `ls /usr/tests` | 10.256 | 3.022 |

原始样本和修改后产物哈希见 [command-latency-results.json](command-latency-results.json)。控制台日志位于 `build/command-latency-{before,after}-{1,5}cpu.log`，每张测试盘正常 sync 后均通过只读 e2fsck。

`echo` 启动没有明显改善；单核重定向样本也有波动。此次优化主要降低可见列表的逐字符输出开销。目录逐项读取、逐文件属性查询和用户态 FS 请求仍有成本，不能据此声称命令启动或目录访问延迟已消除。

复测可使用 `tests/fs_elf_plugins.py` 的 Guest、`tests/console_login.py` 的 login_root，复制 `build/dev/fs-test.img`，依次执行 `/bin/echo latency-probe`、`ls /bin > /dev/null`、`ls /bin`、`ls /usr/tests > /dev/null`、`ls /usr/tests`，每种 CPU 数做两轮。不要让多个 QEMU 共用持久根盘。

## 验证

原生 `user_printf_contract` 覆盖原有格式、无换行提示符、长文本批量提交、短写、EINTR、EAGAIN、零进展和后续独立调用。客户机回归覆盖 workflow（1/5 CPU）、tty、pty、signals 和 pthread_runtime（5 CPU）。原生 1/1、客户机 6/6 全部通过；结果见 `build/command-printf-native-validation.log` 与 `build/command-output-guest-validation.log`。

持久根盘已更新 135 个系统文件。安装后的 ls、sh、drvctl、ps 与构建产物逐字节一致；服务配置和账号文件与自动备份一致，只读 e2fsck 通过。日志见 `build/command-output-rootfs-update.log` 和 `build/command-output-rootfs-integrity.log`。重新启动 QEMU 即可使用。
