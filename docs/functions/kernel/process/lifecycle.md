# 进程生命周期与资源所有权

本文是独立的函数级开发说明；Doxygen 内容由工具写入 clangd 外部索引，不注入源码。

<a id="lifecycle-processmanager-initialize"></a>

## `xv::process::ProcessManager::initialize`

```doxygen
@fn void xv::process::ProcessManager::initialize()
@brief 初始化 PID 序号、亲属关系及延迟子进程通知队列的锁。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="lifecycle-processmanager-allocate-pid"></a>

## `xv::process::ProcessManager::allocate_pid`

```doxygen
@fn int xv::process::ProcessManager::allocate_pid()
@brief 在独立锁下为新进程取得单调递增的正 PID。
@return 可用正 PID；正 int 范围耗尽时返回 0。
@details 持 pid_lock_ 读取并递增 next_pid_，检查上界 0x7fffffff。不会回绕或复用旧编号，因此失败不可能意外指向历史进程。
@note PID 取得失败后，allocate 负责释放已取得的进程描述符。
```

<a id="lifecycle-processmanager-allocate"></a>

## `xv::process::ProcessManager::allocate`

```doxygen
@fn Process *xv::process::ProcessManager::allocate()
@brief 构造候选进程并把内核上下文指向预留栈顶。
@details 仍按顺序分配描述符、PID、trapframe、用户 PID 页和用户页表，失败逐项回滚；context.sp 设置为虚拟区域底加完整栈容量，context.ra 为首次返回入口。
@details 流程由 AllocateFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-reap"></a>

## `xv::process::ProcessManager::reap`

```doxygen
@fn void xv::process::ProcessManager::reap(Process *p)
@brief 销毁已退出或构造失败的进程资源，最终归还稳定描述符及内核栈。
@param p 需要回收的描述符，不能仍在 CPU 上执行或留在就绪队列中。
@return 无返回值；传入对象在调用结束后不可再使用。
@pre 调用者持有 p->lock。已发布子进程由 wait 路径在家庭锁下回收；未发布的分配失败对象由 allocate 回滚。
@details 先移除进程组成员，再归还 trapframe、用户地址空间和可选用户 PID 页，清空身份、信号和事件。已发布子进程在家庭锁下通过 detach_child 从父进程双向子链表常数时间移除，再清空父指针；未发布的分配失败对象没有父关系，脱链为空操作。最后把进程锁及对象所有权转交 store_.release。
@note 运行中进程的文件和工作目录由 exit_process 关闭；reap 不重复执行用户退出清理。store_.release 负责稳定登记、对象析构、内核栈和节点页释放。
@see ProcessManager::exit_process
@details 流程由 ReapFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-create-initial-process"></a>

## `xv::process::ProcessManager::create_initial_process`

```doxygen
@fn void xv::process::ProcessManager::create_initial_process(void)
@brief 建立 init 进程并取得无需进程上下文的初始根引用。
@details 初始 cwd 直接调用 filesystem().root()，不再经需要进程所有者的 VFS 命名空间睡眠锁。其余地址空间、进程组和调度发布流程保持原有约定。
@details 流程由 InitialFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-fork-process"></a>

## `xv::process::ProcessManager::fork_process`

```doxygen
@fn int xv::process::ProcessManager::fork_process(void)
@brief 复制当前进程的用户状态和资源引用，发布一个从 fork 返回 0 的子进程。
@return 父进程获得子 PID；失败返回 -1。子进程的 a0 被置零，稍后经首次返回路径进入用户态。
@pre 必须在有效当前进程的系统调用上下文执行。
@details allocate 创建持锁的新描述符。复制用户地址空间、处理函数与屏蔽字，但不复制待处理信号；活动信号帧地址链随已复制用户栈一起继承。复制 TrapFrame 并把 a0 置零，继承文件引用和工作目录引用，复制名字。
@details 释放新进程锁后取得家庭锁，继承组成员身份，通过 attach_child 设置 parent 并追加到父进程子链表。再取得新进程锁，发布 Runnable，最后释放锁返回 PID。父子关系先于可运行状态发布。
@note 文件描述符引用同一 File，所以文件偏移和 O_NONBLOCK 共享；用户地址空间通过 AddressSpace::copy_to 按其既有策略复制。
@details UDP/TCP socket 与其他描述符一起通过 DescriptorTable/File 引用继承共享，不再维护额外的 PID 绑定表或网络 fork 回调。
```

<a id="lifecycle-processmanager-reparent"></a>

## `xv::process::ProcessManager::reparent`

```doxygen
@fn void xv::process::ProcessManager::reparent(Process *p)
@brief 把退出进程仍拥有的子进程交给初始进程。
@param p 即将退出的旧父进程。
@return 无返回值。
@pre 调用者持有 wait_lock_，以稳定所有 parent 指针及回收关系。
@details 家庭锁保护的子链表包含尚未回收的 Zombie；空链表直接返回。只遍历该进程的直接子进程，逐个取得子进程锁并更新组层面的父关系，通过 detach_child/attach_child 将链表成员按创建顺序转移到 init，不扫描稳定存储中的无关进程。已为 Zombie 的孩子向新父进程登记 SIGCHLD；有收养时唤醒 child_events 等待者。
@note 重托管只原子登记父信号，不在子锁下再取得父进程锁。真正唤醒信号等待者延迟到无相关锁的服务边界。
@details 流程由 ReparentFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-exit-process"></a>

## `xv::process::ProcessManager::exit_process`

```doxygen
@fn void xv::process::ProcessManager::exit_process(int status, int signal)
@brief 结束当前进程的执行并发布 Zombie，等待父进程回收。
@param status 保存在 xstate 中的项目普通退出状态。
@param signal 显式终止信号编号；普通 exit 使用 0，供 waitpid 区分退出与信号终止。
@return 本函数不返回；Zombie 若意外恢复会 panic。
@pre 有效进程上下文，不能携带无关自旋锁进入；初始进程退出会 panic。
@details 先处理控制终端退出，撤销普通等待登记，关闭所有描述符，并在文件系统事务中清理工作目录。取得家庭锁，收养孩子并唤醒子事件等待者；再取得自身进程锁，写入状态和 Zombie，原子向父进程登记 SIGCHLD。
@details 释放家庭锁后，只持自身进程锁调用 scheduler_.suspend。调度器切回并释放该锁后服务信号通知，父进程可回收；本函数之后不再访问可被释放的描述符。
@note 用户地址空间、trapframe 与描述符本体由父进程的 wait/reap 释放；Zombie 保留可查询的退出状态。
@see ProcessManager::reap
@see ProcessManager::reparent
@details socket 经 descriptors_.close_all 与其他描述符统一关闭；最后一个 File 引用释放时调用具体 socket 后端回收排队页。正常 exit 和信号终止共用此路径。
@details 流程由 ExitFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-wakeup"></a>

## `xv::process::ProcessManager::wakeup`

```doxygen
@fn void xv::process::ProcessManager::wakeup(void *channel)
@brief 把对象通道唤醒请求转交给等待表。
@param channel 等待对象的完整地址。
@return 无返回值；通道上没有等待者也合法。
@details 调用 waits_.wake，同时覆盖普通等待与按对象订阅的 poll。它不遍历整个进程表，也不直接让被唤醒进程马上执行。
@see WaitChannels::wake
```

<a id="lifecycle-processmanager-kill"></a>

## `xv::process::ProcessManager::kill`

```doxygen
@fn int xv::process::ProcessManager::kill(int pid)
@brief 保留旧 kill ABI：设置目标 killed 标志并唤醒可中断等待。
@param pid 必须为正的单个进程 PID。
@return 找到可杀目标返回 0；PID 非正、目标不存在或目标为初始进程时返回 -1。
@details 在稳定存储中查找并取得进程锁，设置 killed=1，调用 wake_for_signal，然后释放目标锁并处理子状态通知。
@note 旧 kill 不使用显式信号的 128+signal 普通状态约定。目标在自己的安全返回路径中结束，发送者不会直接销毁它。
@see ProcessManager::exit_process
```

<a id="lifecycle-processmanager-dump"></a>

## `xv::process::ProcessManager::dump`

```doxygen
@fn void xv::process::ProcessManager::dump()
@brief 遍历稳定进程登记并输出 PID、调度状态与调试名字。
@return 无返回值。
@details ProcessStore::visit 在每个对象锁下调用临时 visitor，忽略 Unused，把 ProcessState 映射到短名称后通过 printk 输出。回调不保留指向进程的借用指针。
@note 调试输出并不构成整个进程集合在同一时刻的快照，也不改变调度状态。
```
