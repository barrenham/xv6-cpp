# SIGCHLD 与 shell 后台回收

SIGCHLD(17) 表示直接子进程的状态发生变化。默认不终止、不停止父进程。
子进程正常退出、被信号终止、实际停止以及从 Stopped 恢复时都会产生通知；
向正在运行的进程重复发送 SIGCONT 不产生新的继续事件。
接口语义参考 [POSIX sigaction](https://pubs.opengroup.org/onlinepubs/9799919799/functions/sigaction.html)。

## 通知和状态是两份信息

普通 SIGCHLD 合并为待处理位，不为每次事件排队。父进程安装处理函数后，应
循环调用 waitpid 收集状态。屏蔽 SIGCHLD 只延迟执行处理函数，不阻止子进程
退出，也不阻止父进程主动 waitpid。信号被处理或被清除，不会消耗 wait 状态。

本项目默认与显式 SIG_IGN 都保留现有 wait/reap 约定：退出的子进程仍需 wait
回收，退出状态仍可取得。尚未提供 Linux 的显式 SIG_IGN 自动回收语义，也未
提供 SA_NOCLDSTOP、SA_NOCLDWAIT 或 SA_SIGINFO。SIG_DFL 清除已待处理的 SIGCHLD；
默认处置下的未屏蔽信号在返回用户态前丢弃，不打断可中断等待。

## 内核路径与锁

exit/stop 在家庭锁和子进程锁下发布状态，然后向父进程原子登记 SIGCHLD。
家庭锁保护 parent 指针，使父进程不会在登记过程中被回收或改变父子关系。
登记时不获取父进程锁，也不重入进程登记表，避免子锁→父锁及子锁→存储锁的逆序。

登记后设置 signal_notifications_。RoundRobinScheduler 在 dispatch 返回并
释放刚调度进程的锁后，调用 service_signal_notifications 唤醒父进程。这样
停止/退出路径只带着自己的进程锁跨调度，即使没有其他就绪进程，也能唤醒
正在等待终端或 poll 的父进程。后续服务只查询标志，无事件时不扫描进程。

SIGCONT 的实际恢复发生在发送者持有目标进程锁时；这个位置不能再取得家庭锁。
因此另存 continue_notification_，释放进程/存储锁后，在家庭锁下取得稳定的
父子关系，再登记 SIGCHLD。该标志独立于 continued_event_，waitpid 提前消费
继续状态也不会丢失通知。原 child_events 等待通道继续唤醒 wait 调用者。
被 init 收养的 Zombie 也给新父进程登记 SIGCHLD；PID1 的外部信号保护不影响
这种内核产生的子状态通知。

## shell 的事件循环

shell 安装 SA_RESTART 的 SIGCHLD 处理函数。处理函数只设置
volatile sig_atomic_t 标志，并向非阻塞通知管道写一个字节，不调用 malloc、
printf、waitpid 或修改 Job 链表。管道满时已有可读通知，EAGAIN 不丢失唤醒。

读取命令时，shell 用 poll 同时等待 stdin 和通知管道。信号在 poll 睡眠前
发生时，管道仍可读；睡眠后发生时，信号或管道通知使 poll 返回。因此不依赖
“检查标志后立即阻塞 read”的竞态窗口，也不改变 stdin 的 O_NONBLOCK 状态。
SA_RESTART 保护 shell 普通输出及启动握手，不会自动重启 poll。

主循环排空管道后，对每个 Job 循环执行
waitpid(pid, ..., WNOHANG | WUNTRACED | WCONTINUED)。退出时报告 Done 并移除作业；
停止/继续更新状态。刷新前清除标志，新事件留下新的标志/管道字节。

前台等待也用“刷新后台→查询前台 WNOHANG→poll 通知管道”的顺序，因此前台
仍运行时也能回收后台进程；后台刷新跳过正在前台等待的 Job，防止偷走其状态。
命令启动时的 ready/gate 握手完成后才放行子进程。fork 子路径在执行协调代码
或 exec 前，重置 SIGCHLD 为默认并关闭通知管道，不把 shell 私有描述符传给程序。

异步通知后重画提示符。用户栈中的半行缓冲保持不变，TTY 中尚未提交的规范输入
也不清空。未实现完整终端光标重绘，规范输入的可视内容可能留在通知上方，
但之后提交的命令内容不丢失。

## 验证

signal_contract 检查默认处置、捕获、屏蔽、合并、替换处理函数保留 pending，
以及恢复默认时清除 pending。signaltest 检查退出、停止、继续、重复 CONT、
八个退出合并、每个退出状态保留、准确父进程选择、默认/忽略状态下的 wait。

sigchld_1cpu/sigchld_3cpu 检查空闲提示符自动 Done、规范/非规范半行输入保留、
前台运行时回收后台作业、快速后台退出、命令描述符清理以及 shell 不重启。
既有 jobs/PTY 检查改为等待自动通知，不再靠发送 jobs 命令触发状态刷新。

```sh
ctest --test-dir build/dev --output-on-failure -R '^(signal_contract|signals_[13]cpu|sigchld_[13]cpu)$'
```
