# 用户态信号机制

支持默认处置、忽略、用户处理函数、屏蔽字、普通信号合并、嵌套投递和用户上下文
恢复。停止/继续、控制终端与 shell 作业控制见 [作业控制说明](job-control.md)。
接口按本项目 ABI 实现，语义参考 POSIX 的
[sigaction](https://pubs.opengroup.org/onlinepubs/9799919799/functions/sigaction.html)
和 [sigprocmask](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_sigmask.html)。

## 用户接口

包含 `user/runtime/user.h` 即可获得 `signal_abi.h` 中的结构和常量：

```cpp
static volatile sig_atomic_t interrupted;
static void on_interrupt(int signal) { interrupted = signal; }

struct sigaction action{on_interrupt, 0, 0};
sigaction(SIGINT, &action, nullptr);
sigset_t blocked = 0, old;
sigaddset(&blocked, SIGINT);
sigprocmask(SIG_BLOCK, &blocked, &old);
// 此时收到的 SIGINT 保留为待处理信号。
sigprocmask(SIG_SETMASK, &old, nullptr);
```

| 接口 | 行为 |
| --- | --- |
| `sigaction(signal, act, oldact)` | 安装/查询处置，空 act 只查询；成功返回 0 |
| `signal(signal, handler)` | 使用 SA_RESTART 安装处理函数，返回旧处理函数或 SIG_ERR |
| `sigprocmask(how, set, oldset)` | SIG_BLOCK、SIG_UNBLOCK、SIG_SETMASK；空 set 只查询 |
| `sigpending(set)` | 返回当前被屏蔽且待处理的信号 |
| `sigsuspend(set)` | 临时替换屏蔽字并原子等待；执行处理函数后恢复原屏蔽字，返回 -EINTR |
| `sigemptyset/sigfillset/sigaddset/sigdelset/sigismember` | 用户态信号集操作 |
| `sigsend(target, signal)` | 向进程或进程组投递；信号 0 只探测存在性 |
| `sigdisposition(signal, action)` | 保留旧默认/忽略接口，返回旧的默认/忽略分类；会替换已安装的处理函数 |

`sigset_t` 为 32 位，bit n 对应信号 n。支持 SIGHUP(1)、SIGINT(2)、SIGKILL(9)、
SIGUSR1(10)、SIGSEGV(11)、SIGUSR2(12)、SIGTERM(15)、SIGCHLD(17)、SIGCONT(18)、SIGSTOP(19)、
SIGTSTP(20)、SIGTTIN(21)。SIGCHLD 默认忽略，子状态通知与 shell 回收见 [SIGCHLD 说明](sigchld.md)。未知信号、未知集合位和未知 flags 返回 -1。
SIGKILL/SIGSTOP 的处置不可更改；屏蔽字自动移除这两位。

处理函数签名是 `void handler(int signal)`。`SIG_DFL` 为默认动作，`SIG_IGN` 为忽略。
SIGHUP/INT/KILL/USR1/SEGV/USR2/TERM 默认终止；STOP/TSTP/TTIN 默认停止；CONT 默认继续。
相同普通信号合并为一位；忽略信号时同时清除待处理位。

进入处理函数时，临时屏蔽字 = 当前屏蔽字 | sa_mask | 当前信号；SA_NODEFER
取消自动屏蔽当前信号。SA_RESETHAND 在进入前恢复默认处置。不同未屏蔽信号可以
嵌套，相同信号通过 SA_NODEFER 也可以嵌套。

SIGCONT 即使被屏蔽或忽略也继续停止的进程；有处理函数且被屏蔽时，处理函数
等到解除屏蔽才运行。生成 CONT 清除待处理停止信号，生成停止信号清除待处理 CONT。

`sigsend` 目标：正数为 PID，0 为当前进程组，小于 -1 为指定 PGID；-1 的全系统
广播不支持。无目标、INT_MIN 返回 -1。PID1 可被信号 0 探测，但拒绝显式信号和旧
kill，组投递跳过 PID1。Zombie 可探测，wait 回收后不可探测。

## 上下文与返回链

发送者只登记待处理位并唤醒可中断等待者。目标进程从自己的 trap 返回用户态前，
在 `ProcessManager::deliver_signals()` 中执行默认动作或准备处理函数。首次调度、
让出 CPU 后的返回路径也检查；无跨 CPU 重调度 IPI，其他核运行的目标在后续 trap 响应。

用户处理函数的调用链：

```text
返回用户态前 → enter_signal_handler → 用户栈 SignalFrame
           → handler(signal) → ret → __signal_return → ecall(sigreturn)
           → 恢复原 PC、31 个整数寄存器及屏蔽字 → 原程序继续
```

信号帧保持 16 字节栈对齐，保存原 PC、ra..t6 的全部 31 个整数寄存器及旧屏蔽字。
内核保存活动帧地址栈，sigreturn 只恢复最内层活动帧；检查帧标记、栈位置、恢复
PC 的用户可执行映射、SP 范围/对齐和屏蔽字。内核 SATP、内核栈、hart ID 与状态 CSR
从不从用户帧恢复。sigreturn 返回原 a0，系统调用分发器不会覆盖它。

处理函数返回使用用户运行时的汇编入口，不在用户栈放可执行指令。损坏的帧导致
sigreturn 返回 -1；自动返回入口随后 ebreak，按既有用户异常路径结束该进程。
无法写入信号帧、目标代码映射无效或超过 16 层时，以 SIGSEGV 结束该进程。
实际嵌套深度也受现有用户栈容量限制，不自动扩栈。

## 可中断等待与重启

原 `prepare_sleep` 保持磁盘、日志和内部锁协议完整。TTY、管道、pause、wait、
poll、sigsuspend 使用可中断等待。被屏蔽或忽略的信号不会打断这些等待；未屏蔽
的默认终止、停止或捕获信号使等待逐层释放资源，再在返回用户态前处理。
进程锁下再次检查可投递状态，关闭检查与真正睡眠之间的竞态窗口。poll 仍按对象
订阅，未恢复全局广播。组锁下只原子登记，释放组锁后遍历稳定进程对象并唤醒，
避免反转“进程锁 → 组锁”的顺序。

被捕获信号中断且无传输进展的操作返回 `-EINTR`（-4）；本项目仍直接返回负值，
不设置 libc errno。SA_RESTART 使 read/write/wait/waitpid 回退到 ecall，处理函数
返回后重新执行；有进展时返回实际传输量，绝不重放。poll 和 pause 不因
SA_RESTART 重启。默认停止仍沿用已有无进展操作的停止/继续重试规则。
sigsuspend 用临时屏蔽字等待，信号帧保存其调用前的屏蔽字。

`is_killed()` 沿用旧名字，现在用于查询“应中断当前操作”，包含可捕获信号；
`termination_status()` 只报告 legacy killed 与未屏蔽的默认终止动作。
普通 `wait` 保留旧退出状态：显式信号终止为 `128+signal`，旧 kill/用户异常为 -1。
`waitpid` 使用 [wait.h](../wait/wait.h) 的位编码。

## 生命周期

fork 复制处理函数、忽略处置、屏蔽字和活动信号帧链，待处理集为空；在处理函数
内部 fork 后，父子都能独立 sigreturn。exec 保留屏蔽字、忽略处置与待处理集，
重置捕获处置并清除旧映像的活动帧链。失败的 exec 不改变这些状态。reap 清理状态。

## 验证和边界

`signal_contract` 覆盖原子合并/并发投递、屏蔽、捕获分类、继承、exec 重置和停止/
继续。QEMU 的 `signaltest` 覆盖所有整数寄存器、PC、嵌套、自屏蔽、sa_mask、
SA_NODEFER/RESETHAND/RESTART、无进展中断和部分写、管道/TTY/poll/pause/wait、
sigsuspend 的已有待处理与实际睡眠、fork/exec、处理函数内 fork、损坏帧、无可写
栈、阻塞/忽略 CONT 的组唤醒，以及原有终止、作业控制与资源清理。

```sh
ctest --test-dir build/dev --output-on-failure -R '^(signal_contract|signals_[13]cpu)$'
```

目前没有实时信号、SA_SIGINFO、sigaltstack/SA_ONSTACK、信号队列载荷、用户凭据
权限检查或浮点/向量上下文支持。SIGSEGV 可以显式发送，也用于信号帧建立失败；
普通硬件用户异常仍沿用原有 -1 退出路径。SIGWINCH、SIGTTOU
及孤儿进程组规则仍未接入。处理函数内可调用原始系统调用，但 malloc/printf 等
用户运行时函数不保证异步信号安全；推荐设置 volatile sig_atomic_t 标志，或用 write。
