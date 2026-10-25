# 最小信号机制

当前实现支持 SIGINT(2)、SIGKILL(9)、SIGTERM(15) 的默认终止动作，作为后续
TTY 前台进程组与 Ctrl-C 的基础。参考 [Linux 信号说明](https://man7.org/linux/man-pages/man7/signal.7.html)
及 [kill 的目标选择语义](https://man7.org/linux/man-pages/man2/kill.2.html)。
这不是完整 POSIX 信号接口：暂不支持用户处理器、屏蔽、实时信号、
sigreturn 和用户凭据权限检查。停止/继续与 Ctrl-Z、jobs、fg/bg 已接入，
详见 [作业控制与 waitpid](job-control.md)。

## 用户接口与返回状态

保留原来的 `kill(pid)` 系统调用和 -1 退出状态。新增 `sigsend(target, signal)`
（系统调用 45）显式指定信号，避免改变旧 xv6 用户程序 ABI。

| target | 选择范围 |
| --- | --- |
| 正 PID | 指定进程 |
| 0 | 调用者当前进程组 |
| 小于 -1 | PGID 为 -target 的组 |
| -1 | 本轮不支持全系统广播，返回 -1 |

信号 0 只探测目标是否存在，不设置待处理位。未知信号、无目标、INT_MIN
以及不支持的广播返回 -1。显式终止信号和旧 kill 都拒绝终止初始进程；组
投递跳过初始进程，只要仍有可投递成员就成功。尚未 wait 的 Zombie 可探测，
对它投递不会重新执行退出；wait 回收后 PID 不再可探测。

`wait(&status)` 继续返回本项目的普通退出状态。新信号终止时 status 为
`128 + signal`，例如 SIGINT=130、SIGKILL=137、SIGTERM=143；这不是 POSIX
wait 的位编码，不能当作 WIFSIGNALED/WTERMSIG 数据使用。

```cpp
#include "kernel/process/signals/signal.h"
// 加上项目的 types.h 和 user/runtime/user.h。
sigsend(pid, SIGINT);
sigsend(-pgid, SIGTERM);
```

shell 工具 `kill` 的默认调用仍走旧接口，显式选项走新接口：

```sh
kill 123
kill -INT 123
kill -TERM -123
kill -0 123
signaltest
```

在接入终端前台归属之前，shell 与后代默认仍可能属于同一组。需要定向整组
投递时，应先通过 setpgid 建立对应作业的组。

## 状态与生命周期

Process 拥有一个 `PendingSignals`，采用原子位集；相同普通信号重复发送会
合并，不排队计数。当前全部受支持信号都终止进程；同时待处理时优先选
SIGKILL，再选 SIGINT、SIGTERM。fork 构造空的子进程待处理集，不复制父集；
exec 保留当前待处理集；reap 在解除成员登记后清空状态。

发送信号只登记状态、唤醒可中断等待者。目标在自己的内核执行上下文中，
于 trap 返回用户态前调用 `ProcessManager::deliver_signals()`，通过既有
exit 路径关闭描述符、清理目录、转交子进程并成为 Zombie，最终由 wait 回收。
不会在发送者或硬件中断中直接销毁目标，也不会在任意持锁位置强行退出。
首次调度进入用户态前，以及让出 CPU 后重新准备返回用户态时同样检查。
SIGINT/SIGTERM 现在允许默认/忽略两种处置，详见 [前台终端说明](../../drivers/tty/foreground.md)。
无跨 CPU 重调度 IPI，正在另一核运行的目标最迟在后续 trap/返回检查响应。

旧 `killed` 标记仍用于 kill 和异常终止。`is_killed()` 同时检查它与待处理
终止信号，供可中断的 syscall 循环中止操作并逐层释放资源。

## 组投递和对象存活

`ProcessMembership` 保存所属 Process 的借用地址；只有持组锁的回调可使用。
组遍历在组锁内向成员原子登记信号，不取得进程锁。成员从组中移除之前
描述符不会释放，因此回调内的 owner 有效；回调结束后不保留 owner 指针。

放开组锁后，通过 ProcessStore 的稳定遍历取得各进程锁，唤醒仍带待处理
信号的可中断等待者。这样避免与回收路径的“进程锁 → 组锁”形成反向顺序。
这一阶段允许重复处理并发发送者已标记的进程；它不会重复入队 Runnable。
组投递的唤醒阶段目前是 O(进程数)，普通通道 wakeup 仍使用原来的桶索引，
没有退回全进程扫描。后续可用对象引用或带生命周期保护的快照优化信号路径。

## 可中断等待

原有 `prepare_sleep(channel)` 用于磁盘、日志和内部锁等必须完成协议的等待。
新增 `prepare_interruptible_sleep(channel)` 用于 TTY 输入、管道读写、时钟
pause 和父进程 wait。进程锁保护通道与 interruptible 标记：

1. 循环在条件锁下检查条件和终止状态。
2. 可中断登记再次检查终止状态，关闭检查与实际睡眠之间的竞态窗口。
3. 发送者在进程锁下清除通道；已睡眠时通过调度器发布 Runnable。
4. 被通知的桶链接暂时保留，之后由 cancel/prepare/exit 清理，禁止悬空回收。
5. 恢复的 syscall 重查终止状态并释放自己的锁、事务和局部资源。

`SleepLock::acquire_interruptible()` 可取消等待锁的操作；TTY 的读写串行化锁
使用这个入口，已取得所有权后交给 adopt_lock 守卫释放。磁盘与内部锁仍用
普通 acquire，信号不会让它们丢下进行中的 I/O。终端和文件的大块输出也在
批次之间检查终止状态。

## 验证

```sh
cmake --build --preset dev
ctest --preset dev -j1 -R '^(signal_contract|signals_[13]cpu|trap_contract|sync_contract|process_storage_contract)$'
```

原生测试覆盖原子信号合并/并发投递、优先级、可中断登记的提前通知、锁
取消与已持有锁的守卫释放，以及首次返回/让出 CPU 后的信号检查。
单核与三核 QEMU 的 signaltest 验证运行态、pause、管道读写、TTY 输入、排队
读锁和 wait 的终止与资源清理；检查组投递、组外进程隔离、组长回收后投递、
首次调度附近投递、signal 0、PID1 保护和旧 kill 兼容。随后运行 ttytest 和
shell 命令，验证终端锁与描述符没有被终止路径遗留。

TTY 现已启用 VINTR/ISIG 并按控制终端前台组投递 SIGINT；UART 中断只原子
登记，外部中断路由释放设备锁后再唤醒进程。用户态自定义处理器、Ctrl-Z
已支持停止/继续，仍不支持用户自定义信号处理器。
