# 停止、继续与 shell 作业控制

支持 Ctrl-Z、jobs、fg、bg，以及 SIGSTOP/SIGTSTP/SIGCONT/SIGTTIN。
接口规则参考 [waitpid](https://man7.org/linux/man-pages/man2/waitpid.2.html)
和 [signal](https://man7.org/linux/man-pages/man7/signal.7.html)。本实现保留
单线程进程模型和无 errno 的错误返回方式，不是完整 POSIX shell。

## 用户操作

```sh
cat
# Ctrl-Z 暂停，shell 显示 [编号] Stopped cat
jobs
bg
# cat 尝试后台读取控制终端时被 SIGTTIN 再次暂停
fg
# 从暂停位置继续读入终端；Ctrl-C 终止并返回 shell
```

`fg %编号`、`bg %编号` 可以选择其他作业，省略参数选择最近登记的作业。
顶层 `命令 &` 和 `管道 &` 直接登记为 shell 的后台子作业。每个作业记录
根 PID/PGID、状态、命令字符串及暂停时的 termios；作业表在用户堆动态分配，
完成后回收。作业编号单调递增，不要求第一个暂停作业编号为 1。

管道的各进程和协调进程属于同一个 PGID。Ctrl-Z 投递到整组，协调进程也
暂停并保留自己的 wait 执行位置；继续后仍负责回收管道成员。
shell 忽略 SIGTSTP/SIGTTIN，放行命令前恢复默认动作。

## 内核状态和信号

`ProcessState::Stopped` 表示保留内核栈、用户地址空间、描述符和上下文，但
不在就绪队列。停止只发生在返回用户态的安全边界，不在任意持锁位置切走。
SIGSTOP 不可忽略；SIGTSTP、SIGTTIN 可以使用 sigdisposition 设置默认/忽略。

SIGCONT 原子清除待处理的停止信号，停止信号清除待处理的 SIGCONT。
继续投递到 Stopped 进程时，在进程锁下将其发布 Runnable，一次转移只入队
一次。SIGKILL 和旧 kill 也能唤醒停止进程以执行退出；普通 SIGINT/SIGTERM
在停止期间保持待处理，收到 SIGCONT 后再执行默认动作。

TTY 默认 VSUSP=26。在 ISIG 开启且字符没有被 literal-next 引用时，Ctrl-Z
向前台组登记 SIGTSTP。后台读取自己的控制终端时产生 SIGTTIN；忽略 SIGTTIN
的读取返回 -1。SIGTTOU/TOSTOP 和孤儿进程组的 SIGHUP/SIGCONT 规则尚未实现。

## 等待事件与 ABI

新增 waitpid（系统调用 47），支持正 PID、-1 全部子进程、0 调用者当前组、
小于 -1 的指定组；只观察调用者自己的子进程。支持 WNOHANG、WUNTRACED、
WCONTINUED，状态宏位于 `kernel/process/wait/wait.h`：

| 状态 | 编码与判定 |
| --- | --- |
| 正常退出 | 退出值低 8 位左移 8；WIFEXITED / WEXITSTATUS |
| 信号退出 | 信号编号；WIFSIGNALED / WTERMSIG |
| 停止 | 信号编号左移 8，再加 0x7f；WIFSTOPPED / WSTOPSIG |
| 继续 | 0xffff；WIFCONTINUED |

旧 wait 继续返回普通退出状态，不报告停止/继续，兼容既有 xv6 程序。
错误状态指针不会消费事件，WNOHANG 无事件返回 0 且不改状态。停止/继续事件
采用每进程标志，快速相反转换可以合并；Zombie 退出事件优先，只有退出才回收。

家庭锁保护父子关系和等待登记；状态事件在进程锁下改变。所有子状态变化
使用共同的 child_events 通道。继续/杀死停止进程时先完成进程锁下的操作，
释放存储锁和进程锁后再取得家庭锁通知，避免逆序和丢失唤醒。

## 可中断 syscall 与终端模式

停止请求使可中断读取、写入、pause 和等待逐层退出临界区。若 syscall
未传输任何数据、返回 -1 且仍有停止请求，分发器将用户 PC 回退到 ecall，
保留原始参数；继续后重新执行该调用。部分传输返回正字节数，不重放已完成
的写入。pause 重新执行原请求的 tick 数，不提供精确的剩余时间恢复。
磁盘/日志等不可中断协议先正常完成，再在安全边界停止。

因此停止的 cat 不持有 TTY 读锁，shell 可以接收 jobs/fg/bg；cat 继续后重试
读取，而不是因暂停期间的 -1 退出。shell 在放行作业前保存自身 termios，
作业停止时保存作业 termios 并恢复 shell 模式；fg 先交还终端、恢复作业模式，
再发送 SIGCONT。信号终止时也恢复 shell 模式，普通成功退出仍保留 stty 改动。

后台完成与状态更新在下一次命令处理或 jobs 时通过非阻塞 waitpid 刷新，
尚不实现 SIGCHLD 异步提示。只有顶层作业由交互 shell 跟踪；嵌套语法内部的
后台分支沿用旧执行方式。退出 shell 时向记录中的作业组发送 SIGKILL，防止
遗留无法操作的停止作业。

## 针对性验证

本阶段只运行 signal_contract、driver_contract、process_storage_contract 和
单核/三核 jobs 测试，不运行完整 usertests、RAM 矩阵或 freelist。
jobtest contracts 验证 waitpid 选择/选项、状态编码、坏指针保留事件、停止/继续
各报告一次、读取恢复、SIGKILL 唤醒停止进程，以及停止期间 SIGTERM 待处理。
交互检查覆盖 Ctrl-Z→jobs→bg→fg、后台 cat 再暂停、多进程管道、终端模式恢复、
多作业选择、后台完成回收和同一 shell 持续运行。
