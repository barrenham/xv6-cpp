# `kernel/process/lifecycle/process_manager.cpp`

## 函数导航

- [`xv::process::ProcessManager::allocate`](../lifecycle.md#lifecycle-processmanager-allocate)
- [`xv::process::ProcessManager::allocate_pid`](../lifecycle.md#lifecycle-processmanager-allocate-pid)
- [`xv::process::ProcessManager::create_initial_process`](../lifecycle.md#lifecycle-processmanager-create-initial-process)
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
@details 地址空间复制自然包含 environ 的堆/栈状态，Credentials::inherit 独立复制全部身份、组及 umask；fd 保持共享 File。分配或复制失败回收未发布子进程。
```

<a id="lifecycle-processmanager-kill"></a>

## `xv::process::ProcessManager::kill`

```doxygen
@fn int xv::process::ProcessManager::kill(int pid)
@brief 按用户身份授权后终止指定 PID。
@details 只接受正 PID，定位目标并比较发送者与目标凭据；root 或 UID 匹配才设置终止状态并唤醒可中断睡眠，失败返回 -1。
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
```
