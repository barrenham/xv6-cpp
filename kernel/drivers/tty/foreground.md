# 控制终端、前台进程组与 Ctrl-C

本阶段将已有 TTY、进程组和最小信号层连接起来。接口参考
[tcgetpgrp/tcsetpgrp](https://man7.org/linux/man-pages/man3/tcsetpgrp.3.html)，
保留本项目返回 -1、不设置 errno 的约定。

## 所有权与接口

控制终端关系保存在 ProcessGroups 的 Session 对象中：终端身份是静态 Tty
对象的地址，前台身份是 PGID。TTY 不保存可能被释放的 Session/ProcessGroup
指针，所有查找和变更由组锁保护。

- `ioctl(fd, TIOCSCTTY, 0)`：会话首进程显式取得尚未被其他会话占用的终端。
  同一会话重复取得同一终端成功，不支持强制抢占，也不通过 open 自动取得。
- `tcgetpgrp(fd)`：返回调用者控制终端的前台 PGID，调用者不属于拥有该终端
  的会话时返回 -1。
- `tcsetpgrp(fd, pgid)`：目标必须是同一会话中已存在的非空组。允许 shell
  在命令退出后从后台收回终端；本阶段尚未实现 SIGTTOU/TOSTOP。
- 前台组清空时自动清除前台 PGID；会话首进程退出时释放终端归属。尚未增加
  SIGHUP、完整挂断通知或 devpts。

只限制调用者自身控制终端的后台读取：后台进程返回 -1、不消费输入，默认 SIGTTIN 会停止后台读取者；忽略 SIGTTIN 时返回 -1。setsid 后经继承描述符访问一个非控制终端，仍可按
普通字符设备访问。后台输出不受限制。

## Ctrl-C 的中断路径

```text
UART RX IRQ
  → Tty::receive
  → NttyDiscipline 处理输入转换与 literal-next
  → ISIG 且匹配非零 VINTR：清空输入，回显 ^C 和换行
  → 持组锁找到当前前台组，仅原子登记 SIGINT
  → UART 接收处理结束、驱动锁释放、PLIC complete
  → trap 平台调用 service_signal_notifications
  → 稳定遍历进程并唤醒可中断等待者
  → 目标在返回用户态前按既有 exit 路径终止
```

TTY 状态锁可以先于组锁，但组遍历回调不取得进程锁、不睡眠、不保存成员
借用地址。唤醒阶段放在外部中断路由返回之后，避免 TTY 锁与进程锁的逆序。
即使中断发生在空闲 CPU、没有当前进程，也可以按终端身份选择前台组。

默认 VINTR=3，ISIG 开启。关闭 ISIG 或使用 cfmakeraw 后，Ctrl-C 是普通字节；
Ctrl-V 引用的 Ctrl-C 也不会产生信号。Ctrl-Z 现由 VSUSP 转成 SIGTSTP；停止/继续与 fg/bg 见
[作业控制说明](../../process/signals/job-control.md)。

## shell 的交接流程

初始交互 shell 创建自己的会话并取得 console，保存一个独立的终端描述符，
避免命令重定向影响后续交接。输入来自管道时不启用这套交互流程。

每条前台命令使用双向管道握手：子进程先建立自己的进程组并恢复 SIGINT
默认动作，通知父进程就绪，再等待放行。父进程也设置子进程组，调用
tcsetpgrp 交出终端，最后放行。这样子进程不会在取得前台之前读取终端。
命令结束后 shell 等待并回收子进程，再切回自己的组并显示 `$`。

管道的协调进程忽略 SIGINT，各执行成员恢复默认动作并继承同一个 PGID。
Ctrl-C 终止执行成员，协调进程回收它们后退出，shell 才重新显示提示符。
命令列表传播被信号终止的状态，避免中断后继续执行列表右侧命令。
现有 `&` 分支把后台命令放到独立进程组，避免后续前台 Ctrl-C 波及它。

## 默认与忽略处置

新增 `sigdisposition(signal, action)`（系统调用 46），只接受 SIGINT/SIGTERM 的
SIGNAL_DEFAULT、SIGNAL_IGNORE，返回旧动作。SIGKILL 不允许更改处置，也不
支持自定义用户处理函数。忽略状态随 fork 继承、跨 exec 保留，子进程待处理
信号仍为空。shell 忽略 SIGINT，普通命令在放行前恢复默认动作。

待处理位和忽略位放在同一个 64 位原子状态中，以 CAS 同步修改；忽略信号时
同时清除对应待处理位，避免并发投递留下已忽略的信号。

提示符处 Ctrl-C 清空未提交的输入、回显换行，shell 保持原会话并继续等待。
尚无用户态信号处理器，因此这一场景不立即重新绘制提示符；前台命令被
Ctrl-C 终止后会正常重新显示 `$`。Ctrl-Z、停止/继续及 fg/bg 已由下一阶段接入。

## 针对性验证

```sh
cmake --build --preset dev
ctest --preset dev -j1 -R '^(process_groups_contract|signal_contract|driver_contract|foreground_[13]cpu|signals_[13]cpu|tty_[13]cpu|process_groups_[13]cpu)$'
```

原生契约检查终端独占、跨会话限制、前台组清空和首进程退出后的释放，
以及信号忽略/继承/默认恢复、VINTR、literal-next、关闭 ISIG。
单核和三核 QEMU 注入真实 Ctrl-C，覆盖运行态和读终端命令、管道两端、cat、
后台隔离、空闲输入取消，并确认 shell SID 不变、没有 init 重启 shell。
本阶段按用户要求不运行完整 usertests、RAM 矩阵或 freelist 验证。

在已运行的 QEMU 中不会自动替换内核；构建后重新启动 QEMU，再执行 `cat`
并按 Ctrl-C 可观察前台命令退出和提示符恢复。
