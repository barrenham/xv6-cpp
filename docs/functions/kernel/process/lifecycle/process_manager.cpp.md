# `kernel/process/lifecycle/process_manager.cpp`

## 函数导航

- [`xv::process::ProcessManager::AllocateFlow::allocated`](process_manager.cpp.md#fn-a88d497d2d723a56)
- [`xv::process::ProcessManager::AllocateFlow::descriptor`](process_manager.cpp.md#fn-992a7aa604af9bf4)
- [`xv::process::ProcessManager::AllocateFlow::has_trapframe`](process_manager.cpp.md#fn-2a8067a3b4d5899d)
- [`xv::process::ProcessManager::AllocateFlow::has_user_syscall`](process_manager.cpp.md#fn-6cfa98e381f42eef)
- [`xv::process::ProcessManager::AllocateFlow::identified`](process_manager.cpp.md#fn-c519294fd10654cf)
- [`xv::process::ProcessManager::AllocateFlow::identity`](process_manager.cpp.md#fn-d01b50a125f4bd06)
- [`xv::process::ProcessManager::AllocateFlow::image`](process_manager.cpp.md#fn-9ea5649ddaa8d70c)
- [`xv::process::ProcessManager::AllocateFlow::prepare_return`](process_manager.cpp.md#fn-a409244c0ce60c56)
- [`xv::process::ProcessManager::AllocateFlow::ready`](process_manager.cpp.md#fn-809f7d761c99578f)
- [`xv::process::ProcessManager::AllocateFlow::rollback`](process_manager.cpp.md#fn-feb2726ba9db30f4)
- [`xv::process::ProcessManager::AllocateFlow::trapframe`](process_manager.cpp.md#fn-17bba1d8b2bed469)
- [`xv::process::ProcessManager::AllocateFlow::user_syscall`](process_manager.cpp.md#fn-d7f11cf7376e38b0)
- [`xv::process::ProcessManager::ExitFlow::cancel_wait`](process_manager.cpp.md#fn-333104b0f995ec7e)
- [`xv::process::ProcessManager::ExitFlow::close_files`](process_manager.cpp.md#fn-5b170934752a884f)
- [`xv::process::ProcessManager::ExitFlow::directory`](process_manager.cpp.md#fn-e066b0c405c31107)
- [`xv::process::ProcessManager::ExitFlow::family`](process_manager.cpp.md#fn-f93801cb3cda2fec)
- [`xv::process::ProcessManager::ExitFlow::publish`](process_manager.cpp.md#fn-b4a8db086931ec68)
- [`xv::process::ProcessManager::ExitFlow::suspend`](process_manager.cpp.md#fn-1c272eb841d55c67)
- [`xv::process::ProcessManager::ExitFlow::terminal`](process_manager.cpp.md#fn-fb8d54149eae0e11)
- [`xv::process::ProcessManager::ForkFlow::allocate`](process_manager.cpp.md#fn-dfa62a8c54ac2c08)
- [`xv::process::ProcessManager::ForkFlow::allocated`](process_manager.cpp.md#fn-7f6457ae88a6c5e8)
- [`xv::process::ProcessManager::ForkFlow::copied`](process_manager.cpp.md#fn-7d69741f2b81e7c3)
- [`xv::process::ProcessManager::ForkFlow::family`](process_manager.cpp.md#fn-df942201b3e8a27f)
- [`xv::process::ProcessManager::ForkFlow::image`](process_manager.cpp.md#fn-d07eb866698ad467)
- [`xv::process::ProcessManager::ForkFlow::publish`](process_manager.cpp.md#fn-157f8c594dddea29)
- [`xv::process::ProcessManager::ForkFlow::registers`](process_manager.cpp.md#fn-080ad399ab2df598)
- [`xv::process::ProcessManager::ForkFlow::resources`](process_manager.cpp.md#fn-806fc4dcf560c320)
- [`xv::process::ProcessManager::ForkFlow::rollback`](process_manager.cpp.md#fn-f1027c67449a661e)
- [`xv::process::ProcessManager::ForkFlow::signal_state`](process_manager.cpp.md#fn-289b8202a4bc8ab0)
- [`xv::process::ProcessManager::InitialFlow::allocate`](process_manager.cpp.md#fn-7e4dbd07ae294c29)
- [`xv::process::ProcessManager::InitialFlow::allocated`](process_manager.cpp.md#fn-fe08e6b39fac243f)
- [`xv::process::ProcessManager::InitialFlow::directory`](process_manager.cpp.md#fn-b9b3200789e6e1ee)
- [`xv::process::ProcessManager::InitialFlow::group`](process_manager.cpp.md#fn-33605f16debf7bd3)
- [`xv::process::ProcessManager::InitialFlow::grouped`](process_manager.cpp.md#fn-ae313aa67ea5471f)
- [`xv::process::ProcessManager::InitialFlow::no_session`](process_manager.cpp.md#fn-c574a4529a9f24aa)
- [`xv::process::ProcessManager::InitialFlow::no_slot`](process_manager.cpp.md#fn-0f5d45a7ee2c1423)
- [`xv::process::ProcessManager::InitialFlow::publish`](process_manager.cpp.md#fn-c58964ca5af4446e)
- [`xv::process::ProcessManager::InspectFlow::credentials`](process_manager.cpp.md#fn-d824cfabdc71584f)
- [`xv::process::ProcessManager::InspectFlow::find`](process_manager.cpp.md#fn-38c35c5d91a24af3)
- [`xv::process::ProcessManager::InspectFlow::found`](process_manager.cpp.md#fn-f26da8f4f898fb8c)
- [`xv::process::ProcessManager::InspectFlow::inspect`](process_manager.cpp.md#fn-2457750a0b4d4f6c)
- [`xv::process::ProcessManager::InspectFlow::permitted`](process_manager.cpp.md#fn-1012f4475c3bf528)
- [`xv::process::ProcessManager::InspectFlow::unlock`](process_manager.cpp.md#fn-7265a30e83f26c24)
- [`xv::process::ProcessManager::KillFlow::find`](process_manager.cpp.md#fn-04ab2fe30cd6f672)
- [`xv::process::ProcessManager::KillFlow::found`](process_manager.cpp.md#fn-6f5431af81044331)
- [`xv::process::ProcessManager::KillFlow::mark`](process_manager.cpp.md#fn-dca64f0bfe80e384)
- [`xv::process::ProcessManager::KillFlow::notify`](process_manager.cpp.md#fn-e785322c78fb0ace)
- [`xv::process::ProcessManager::KillFlow::permitted`](process_manager.cpp.md#fn-a038aa028866e983)
- [`xv::process::ProcessManager::KillFlow::unlock`](process_manager.cpp.md#fn-8fae86f2accfe852)
- [`xv::process::ProcessManager::ReapFlow::detach`](process_manager.cpp.md#fn-bb187f0ce4341d7e)
- [`xv::process::ProcessManager::ReapFlow::free_trapframe`](process_manager.cpp.md#fn-bf822ce40d0fa1f4)
- [`xv::process::ProcessManager::ReapFlow::free_user_syscall`](process_manager.cpp.md#fn-9f5a40cf4edef515)
- [`xv::process::ProcessManager::ReapFlow::group`](process_manager.cpp.md#fn-35c1f95f52977af7)
- [`xv::process::ProcessManager::ReapFlow::has_trapframe`](process_manager.cpp.md#fn-1ca7c277c4c59d6a)
- [`xv::process::ProcessManager::ReapFlow::has_user_syscall`](process_manager.cpp.md#fn-afd8ff98fbd21227)
- [`xv::process::ProcessManager::ReapFlow::image`](process_manager.cpp.md#fn-c0fb13b8faf5c4c8)
- [`xv::process::ProcessManager::ReapFlow::release`](process_manager.cpp.md#fn-8f26aeb956dc6234)
- [`xv::process::ProcessManager::ReapFlow::reset_identity`](process_manager.cpp.md#fn-71757f2f981da05c)
- [`xv::process::ProcessManager::ReparentFlow::adopt`](process_manager.cpp.md#fn-11e8da61c2c46eb8)
- [`xv::process::ProcessManager::ReparentFlow::advance`](process_manager.cpp.md#fn-7a797ce9c7fe1c13)
- [`xv::process::ProcessManager::ReparentFlow::more`](process_manager.cpp.md#fn-29663f1abd11fa8a)
- [`xv::process::ProcessManager::ReparentFlow::notify`](process_manager.cpp.md#fn-2ceb3f6a6607b2d6)
- [`xv::process::ProcessManager::ReparentFlow::wake`](process_manager.cpp.md#fn-8388ff9b200fdb74)
- [`xv::process::ProcessManager::ReparentFlow::zombie`](process_manager.cpp.md#fn-0fc82682334f3a9b)
- [`xv::process::ProcessManager::allocate`](../lifecycle.md#lifecycle-processmanager-allocate)
- [`xv::process::ProcessManager::allocate_pid`](../lifecycle.md#lifecycle-processmanager-allocate-pid)
- [`xv::process::ProcessManager::attach_child`](process_manager.cpp.md#fn-a16d407c78d63bf0)
- [`xv::process::ProcessManager::create_initial_process`](../lifecycle.md#lifecycle-processmanager-create-initial-process)
- [`xv::process::ProcessManager::detach_child`](process_manager.cpp.md#fn-1a675ddf12132ca0)
- [`xv::process::ProcessManager::dump`](../lifecycle.md#lifecycle-processmanager-dump)
- [`xv::process::ProcessManager::exit_process`](../lifecycle.md#lifecycle-processmanager-exit-process)
- [`xv::process::ProcessManager::fork_process`](process_manager.cpp.md#lifecycle-processmanager-fork-process)
- [`xv::process::ProcessManager::initialize`](../lifecycle.md#lifecycle-processmanager-initialize)
- [`xv::process::ProcessManager::inspect_files`](process_manager.cpp.md#fn-eb5e5ec736ff1777)
- [`xv::process::ProcessManager::kill`](process_manager.cpp.md#lifecycle-processmanager-kill)
- [`xv::process::ProcessManager::reap`](../lifecycle.md#lifecycle-processmanager-reap)
- [`xv::process::ProcessManager::reparent`](../lifecycle.md#lifecycle-processmanager-reparent)
- [`xv::process::ProcessManager::snapshot_after`](process_manager.cpp.md#fn-bbc335370e273170)
- [`xv::process::ProcessManager::wakeup`](../lifecycle.md#lifecycle-processmanager-wakeup)

<a id="lifecycle-processmanager-fork-process"></a>

## `xv::process::ProcessManager::fork_process`

```doxygen
@fn int xv::process::ProcessManager::fork_process(void)
@brief 复制进程资源并继承身份与环境内存。
@details 地址空间复制自然包含 environ 的堆/栈状态，Credentials::inherit 独立复制全部身份、组及 umask；fd 保持共享 File。分配或复制失败回收未发布子进程。成功路径在家庭锁下建立 parent 并追加到父进程的双向子链表，先于 Runnable 发布；回收或重托管时常数时间脱链。
@details 流程由 ForkFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="lifecycle-processmanager-kill"></a>

## `xv::process::ProcessManager::kill`

```doxygen
@fn int xv::process::ProcessManager::kill(int pid)
@brief 按用户身份授权后终止指定 PID。
@details 只接受正 PID，定位目标并比较发送者与目标凭据；root 或 UID 匹配才设置终止状态并唤醒可中断睡眠，失败返回 -1。
@details 流程由 KillFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-bbc335370e273170"></a>

## `xv::process::ProcessManager::snapshot_after`

```doxygen
@fn bool xv::process::ProcessManager::snapshot_after(int after, ProcessInfo &result)
@brief 复制 PID 后继的公开进程信息。
@details 存储访问与进程锁保护对象寿命，复制真实/有效 UID/GID 与原调度信息；凭据通过独立快照读取，不把 Process 指针交给 procfs。
```

<a id="fn-eb5e5ec736ff1777"></a>

## `xv::process::ProcessManager::inspect_files`

```doxygen
@fn int xv::process::ProcessManager::inspect_files(int pid, xv::fs::FileInfo *entries, unsigned capacity)
@brief 按凭据限制描述符元数据观测。
@details root 或调用者有效 UID 同时匹配目标真实与有效 UID 才可检查。进程锁内确认身份，再在 fd 表短锁内复制已发布的不可变 FileInfo；不调用后端、不关闭引用，不睡眠。
@details 流程由 InspectFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-a16d407c78d63bf0"></a>

## `xv::process::ProcessManager::attach_child`

```doxygen
@fn void xv::process::ProcessManager::attach_child(Process &parent, Process &child)
@brief 将未关联子进程追加到父进程子链表。
@pre 已关联对象的操作由家庭锁保护；未关联构造失败对象由调用者独占。
@details 设置 parent、兄弟双向链接与首尾指针，追加顺序为创建或收养顺序。父子关系先于 Runnable 发布。
```

<a id="fn-1a675ddf12132ca0"></a>

## `xv::process::ProcessManager::detach_child`

```doxygen
@fn void xv::process::ProcessManager::detach_child(Process &child)
@brief 将已关联子进程从父进程子链表常数时间移除。
@pre 已关联对象的操作由家庭锁保护；未关联构造失败对象由调用者独占。
@details 更新相邻兄弟及父进程首尾指针，再清空子进程父指针和兄弟链接；从未关联的构造失败对象直接返回。
```

<a id="fn-a88d497d2d723a56"></a>

## `xv::process::ProcessManager::AllocateFlow::allocated`

```doxygen
@fn static bool xv::process::ProcessManager::AllocateFlow::allocated(Context &c) noexcept
@brief 检查是否已取得进程描述符。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-992a7aa604af9bf4"></a>

## `xv::process::ProcessManager::AllocateFlow::descriptor`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::descriptor(Context &c) noexcept
@brief 尝试取得带锁的稳定进程描述符。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2a8067a3b4d5899d"></a>

## `xv::process::ProcessManager::AllocateFlow::has_trapframe`

```doxygen
@fn static bool xv::process::ProcessManager::AllocateFlow::has_trapframe(Context &c) noexcept
@brief 检查陷阱帧页是否已取得。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6cfa98e381f42eef"></a>

## `xv::process::ProcessManager::AllocateFlow::has_user_syscall`

```doxygen
@fn static bool xv::process::ProcessManager::AllocateFlow::has_user_syscall(Context &c) noexcept
@brief 检查 LAB_PGTBL 系统调用共享页是否已取得。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c519294fd10654cf"></a>

## `xv::process::ProcessManager::AllocateFlow::identified`

```doxygen
@fn static bool xv::process::ProcessManager::AllocateFlow::identified(Context &c) noexcept
@brief 检查进程号空间尚未耗尽。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d01b50a125f4bd06"></a>

## `xv::process::ProcessManager::AllocateFlow::identity`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::identity(Context &c) noexcept
@brief 分配并保存进程号。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9ea5649ddaa8d70c"></a>

## `xv::process::ProcessManager::AllocateFlow::image`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::image(Context &c) noexcept
@brief 创建空用户映像并记录是否成功。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a409244c0ce60c56"></a>

## `xv::process::ProcessManager::AllocateFlow::prepare_return`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::prepare_return(Context &c) noexcept
@brief 初始化 first_return 上下文与内核栈顶，保存带锁描述符结果。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-809f7d761c99578f"></a>

## `xv::process::ProcessManager::AllocateFlow::ready`

```doxygen
@fn static bool xv::process::ProcessManager::AllocateFlow::ready(Context &c) noexcept
@brief 检查空用户映像是否创建成功。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-feb2726ba9db30f4"></a>

## `xv::process::ProcessManager::AllocateFlow::rollback`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::rollback(Context &c) noexcept
@brief 回收失败候选及其锁，之后不再访问可能已经释放的描述符。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-17bba1d8b2bed469"></a>

## `xv::process::ProcessManager::AllocateFlow::trapframe`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::trapframe(Context &c) noexcept
@brief 申请陷阱帧页，LAB_LOCK 下仅在成功后清除绑定 CPU。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d7f11cf7376e38b0"></a>

## `xv::process::ProcessManager::AllocateFlow::user_syscall`

```doxygen
@fn static void xv::process::ProcessManager::AllocateFlow::user_syscall(Context &c) noexcept
@brief 在 LAB_PGTBL 下申请、清零系统调用共享页并设置 PID。
@details ProcessStore 返回稳定描述符及其持有的进程锁；失败统一经 reap 回收全部已取得资源并结束本图，成功仍把进程锁转交调用者。条件编译的 LAB_PGTBL 和 LAB_LOCK 顺序保留。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-333104b0f995ec7e"></a>

## `xv::process::ProcessManager::ExitFlow::cancel_wait`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::cancel_wait(Context &c) noexcept
@brief 取消当前进程已有等待依赖。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5b170934752a884f"></a>

## `xv::process::ProcessManager::ExitFlow::close_files`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::close_files(Context &c) noexcept
@brief 按原描述符顺序归还所有打开文件引用。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e066b0c405c31107"></a>

## `xv::process::ProcessManager::ExitFlow::directory`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::directory(Context &c) noexcept
@brief 在局部文件系统事务内归还当前目录引用。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f93801cb3cda2fec"></a>

## `xv::process::ProcessManager::ExitFlow::family`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::family(Context &c) noexcept
@brief 取得 family 锁，收养当前子进程并唤醒父等待事件。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b4a8db086931ec68"></a>

## `xv::process::ProcessManager::ExitFlow::publish`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::publish(Context &c) noexcept
@brief 取得当前进程锁，发布退出状态与 Zombie，投递子事件信号后释放 family 锁。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1c272eb841d55c67"></a>

## `xv::process::ProcessManager::ExitFlow::suspend`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::suspend(Context &c) noexcept
@brief 带进程锁进入调度器，异常返回时诊断 zombie exit。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fb8d54149eae0e11"></a>

## `xv::process::ProcessManager::ExitFlow::terminal`

```doxygen
@fn static void xv::process::ProcessManager::ExitFlow::terminal(Context &c) noexcept
@brief 退出控制终端成员关系并执行原组通知协议。
@details 初始进程退出在入口拒绝。目录引用清理使用动作局部 Transaction；变为 Zombie 后释放 family 锁，但继续持进程锁进入调度器。调度器异常返回保留原 panic 诊断。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dfa62a8c54ac2c08"></a>

## `xv::process::ProcessManager::ForkFlow::allocate`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::allocate(Context &c) noexcept
@brief 分配带锁子进程及空映像。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7f6457ae88a6c5e8"></a>

## `xv::process::ProcessManager::ForkFlow::allocated`

```doxygen
@fn static bool xv::process::ProcessManager::ForkFlow::allocated(Context &c) noexcept
@brief 检查子进程描述符是否已取得。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7d69741f2b81e7c3"></a>

## `xv::process::ProcessManager::ForkFlow::copied`

```doxygen
@fn static bool xv::process::ProcessManager::ForkFlow::copied(Context &c) noexcept
@brief 检查映像克隆是否成功。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-df942201b3e8a27f"></a>

## `xv::process::ProcessManager::ForkFlow::family`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::family(Context &c) noexcept
@brief 在 family 锁内继承组成员关系并把子节点挂到父子链尾。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d07eb866698ad467"></a>

## `xv::process::ProcessManager::ForkFlow::image`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::image(Context &c) noexcept
@brief 将父映像复制到子映像并保存成功标记。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-157f8c594dddea29"></a>

## `xv::process::ProcessManager::ForkFlow::publish`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::publish(Context &c) noexcept
@brief 重新锁住子进程，发布 Runnable 后解锁。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-080ad399ab2df598"></a>

## `xv::process::ProcessManager::ForkFlow::registers`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::registers(Context &c) noexcept
@brief 复制保存的用户寄存器并设置子进程 fork 返回值为零。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-806fc4dcf560c320"></a>

## `xv::process::ProcessManager::ForkFlow::resources`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::resources(Context &c) noexcept
@brief 按描述符、目录、凭据次序继承资源，复制名称，保存 PID 并释放子锁。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f1027c67449a661e"></a>

## `xv::process::ProcessManager::ForkFlow::rollback`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::rollback(Context &c) noexcept
@brief 回收失败子候选并结束图，不再读取其描述符。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-289b8202a4bc8ab0"></a>

## `xv::process::ProcessManager::ForkFlow::signal_state`

```doxygen
@fn static void xv::process::ProcessManager::ForkFlow::signal_state(Context &c) noexcept
@brief 继承信号 disposition、处理深度和按原顺序复制的帧数组。
@details 候选先持子进程锁复制映像和资源，再释放子锁，在 family 锁内继承组并挂接父子链，随后重新取得子锁发布 Runnable。返回 PID 在发布前保存，发布后不依赖子对象仍然存活。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7e4dbd07ae294c29"></a>

## `xv::process::ProcessManager::InitialFlow::allocate`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::allocate(Context &c) noexcept
@brief 分配首个进程的带锁描述符及映像。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fe08e6b39fac243f"></a>

## `xv::process::ProcessManager::InitialFlow::allocated`

```doxygen
@fn static bool xv::process::ProcessManager::InitialFlow::allocated(Context &c) noexcept
@brief 检查初始进程是否成功分配。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b9b3200789e6e1ee"></a>

## `xv::process::ProcessManager::InitialFlow::directory`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::directory(Context &c) noexcept
@brief 取得文件系统根引用并安装为初始当前目录。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-33605f16debf7bd3"></a>

## `xv::process::ProcessManager::InitialFlow::group`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::group(Context &c) noexcept
@brief 发布 initial 指针并尝试创建初始进程组和会话。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ae313aa67ea5471f"></a>

## `xv::process::ProcessManager::InitialFlow::grouped`

```doxygen
@fn static bool xv::process::ProcessManager::InitialFlow::grouped(Context &c) noexcept
@brief 检查初始会话创建是否成功。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c574a4529a9f24aa"></a>

## `xv::process::ProcessManager::InitialFlow::no_session`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::no_session(Context &) noexcept
@brief 诊断初始会话内存不足。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0f5d45a7ee2c1423"></a>

## `xv::process::ProcessManager::InitialFlow::no_slot`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::no_slot(Context &) noexcept
@brief 诊断没有可用的初始进程描述符。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c58964ca5af4446e"></a>

## `xv::process::ProcessManager::InitialFlow::publish`

```doxygen
@fn static void xv::process::ProcessManager::InitialFlow::publish(Context &c) noexcept
@brief 标记初始进程可运行，再释放其进程锁。
@details 启动分配或会话创建失败保留原 panic 语义；取得根目录引用后发布 Runnable，最后释放新进程锁。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d824cfabdc71584f"></a>

## `xv::process::ProcessManager::InspectFlow::credentials`

```doxygen
@fn static void xv::process::ProcessManager::InspectFlow::credentials(Context &c) noexcept
@brief 按调用者、目标次序读取凭据并计算读取文件表的权限。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-38c35c5d91a24af3"></a>

## `xv::process::ProcessManager::InspectFlow::find`

```doxygen
@fn static void xv::process::ProcessManager::InspectFlow::find(Context &c) noexcept
@brief 按原 PID 谓词查找并锁住目标进程。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f26da8f4f898fb8c"></a>

## `xv::process::ProcessManager::InspectFlow::found`

```doxygen
@fn static bool xv::process::ProcessManager::InspectFlow::found(Context &c) noexcept
@brief 检查目标进程是否存在。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2457750a0b4d4f6c"></a>

## `xv::process::ProcessManager::InspectFlow::inspect`

```doxygen
@fn static void xv::process::ProcessManager::InspectFlow::inspect(Context &c) noexcept
@brief 在目标进程锁内读取文件表快照并保存原返回值。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1012f4475c3bf528"></a>

## `xv::process::ProcessManager::InspectFlow::permitted`

```doxygen
@fn static bool xv::process::ProcessManager::InspectFlow::permitted(Context &c) noexcept
@brief 检查本次文件表查询权限是否通过。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7265a30e83f26c24"></a>

## `xv::process::ProcessManager::InspectFlow::unlock`

```doxygen
@fn static void xv::process::ProcessManager::InspectFlow::unlock(Context &c) noexcept
@brief 释放目标进程锁。
@details 入口验证 PID 和容量。查找后持目标进程锁；先读取调用者和目标两份凭据快照，即使调用者为 root 也不合并或省略读取，再按原权限判断决定是否检查文件表。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-04ab2fe30cd6f672"></a>

## `xv::process::ProcessManager::KillFlow::find`

```doxygen
@fn static void xv::process::ProcessManager::KillFlow::find(Context &c) noexcept
@brief 按原 PID 谓词查找并锁住目标进程。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6f5431af81044331"></a>

## `xv::process::ProcessManager::KillFlow::found`

```doxygen
@fn static bool xv::process::ProcessManager::KillFlow::found(Context &c) noexcept
@brief 检查是否找到目标进程。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dca64f0bfe80e384"></a>

## `xv::process::ProcessManager::KillFlow::mark`

```doxygen
@fn static void xv::process::ProcessManager::KillFlow::mark(Context &c) noexcept
@brief 标记 killed 并执行目标的信号唤醒协议。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e785322c78fb0ace"></a>

## `xv::process::ProcessManager::KillFlow::notify`

```doxygen
@fn static void xv::process::ProcessManager::KillFlow::notify(Context &c) noexcept
@brief 发出子事件通知并记录零成功码。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a038aa028866e983"></a>

## `xv::process::ProcessManager::KillFlow::permitted`

```doxygen
@fn static bool xv::process::ProcessManager::KillFlow::permitted(Context &c) noexcept
@brief 拒绝 init，再按原调用表达式检查信号权限。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8fae86f2accfe852"></a>

## `xv::process::ProcessManager::KillFlow::unlock`

```doxygen
@fn static void xv::process::ProcessManager::KillFlow::unlock(Context &c) noexcept
@brief 释放查找转交的进程锁。
@details PID 参数在入口验证，ProcessStore 查找返回带锁对象；保护 init 并保留原凭据快照调用，先标记和唤醒，再解进程锁并发送子事件通知。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bb187f0ce4341d7e"></a>

## `xv::process::ProcessManager::ReapFlow::detach`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::detach(Context &c) noexcept
@brief 从父进程的侵入式子链移除节点，未挂接候选为空操作。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bf822ce40d0fa1f4"></a>

## `xv::process::ProcessManager::ReapFlow::free_trapframe`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::free_trapframe(Context &c) noexcept
@brief 归还已取得的陷阱帧页。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9f5a40cf4edef515"></a>

## `xv::process::ProcessManager::ReapFlow::free_user_syscall`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::free_user_syscall(Context &c) noexcept
@brief 归还 LAB_PGTBL 系统调用共享页。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-35c1f95f52977af7"></a>

## `xv::process::ProcessManager::ReapFlow::group`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::group(Context &c) noexcept
@brief 归还进程组与会话成员关系。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1ca7c277c4c59d6a"></a>

## `xv::process::ProcessManager::ReapFlow::has_trapframe`

```doxygen
@fn static bool xv::process::ProcessManager::ReapFlow::has_trapframe(Context &c) noexcept
@brief 检查陷阱帧页是否存在。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-afd8ff98fbd21227"></a>

## `xv::process::ProcessManager::ReapFlow::has_user_syscall`

```doxygen
@fn static bool xv::process::ProcessManager::ReapFlow::has_user_syscall(Context &c) noexcept
@brief 检查 LAB_PGTBL 系统调用共享页是否存在。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c0fb13b8faf5c4c8"></a>

## `xv::process::ProcessManager::ReapFlow::image`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::image(Context &c) noexcept
@brief 清空陷阱帧指针并重置拥有的用户映像。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8f26aeb956dc6234"></a>

## `xv::process::ProcessManager::ReapFlow::release`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::release(Context &c) noexcept
@brief 归还描述符及其持有的锁，允许底层释放该对象。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-71757f2f981da05c"></a>

## `xv::process::ProcessManager::ReapFlow::reset_identity`

```doxygen
@fn static void xv::process::ProcessManager::ReapFlow::reset_identity(Context &c) noexcept
@brief 按原顺序清空 PID、名称、等待、信号与退出事件状态。
@details 入口持目标进程锁；有父进程时还由调用者持 family 锁，未发布候选允许没有父节点。描述符归还必须是最后一步，其后图不再访问该进程。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-11e8da61c2c46eb8"></a>

## `xv::process::ProcessManager::ReparentFlow::adopt`

```doxygen
@fn static void xv::process::ProcessManager::ReparentFlow::adopt(Context &c) noexcept
@brief 取得当前子锁，转移组父关系并把节点从原子链移到 init 子链尾。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7a797ce9c7fe1c13"></a>

## `xv::process::ProcessManager::ReparentFlow::advance`

```doxygen
@fn static void xv::process::ProcessManager::ReparentFlow::advance(Context &c) noexcept
@brief 释放当前子锁并重新选取原父子链的首节点。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-29663f1abd11fa8a"></a>

## `xv::process::ProcessManager::ReparentFlow::more`

```doxygen
@fn static bool xv::process::ProcessManager::ReparentFlow::more(Context &c) noexcept
@brief 检查是否还有子节点待收养。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2ceb3f6a6607b2d6"></a>

## `xv::process::ProcessManager::ReparentFlow::notify`

```doxygen
@fn static void xv::process::ProcessManager::ReparentFlow::notify(Context &c) noexcept
@brief 为被收养僵尸按原 disposition 规则投递子事件信号。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8388ff9b200fdb74"></a>

## `xv::process::ProcessManager::ReparentFlow::wake`

```doxygen
@fn static void xv::process::ProcessManager::ReparentFlow::wake(Context &c) noexcept
@brief 完成全部收养后唤醒子事件等待者。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0fc82682334f3a9b"></a>

## `xv::process::ProcessManager::ReparentFlow::zombie`

```doxygen
@fn static bool xv::process::ProcessManager::ReparentFlow::zombie(Context &c) noexcept
@brief 检查被收养节点是否已为僵尸。
@details 调用者持 family 锁且已排除空子链；仅按创建顺序遍历自己的子节点，包含未回收僵尸。每轮都释放子锁后再选择下一个节点，循环不增加 C++ 调用栈。 下一节点由图策略声明，业务结果与控制信号分离。
```
