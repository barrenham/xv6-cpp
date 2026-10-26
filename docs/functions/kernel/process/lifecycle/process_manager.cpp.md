# `kernel/process/lifecycle/process_manager.cpp`

## 函数导航

- [`xv::process::ProcessManager::allocate`](../lifecycle.md#lifecycle-processmanager-allocate)
- [`xv::process::ProcessManager::allocate_pid`](../lifecycle.md#lifecycle-processmanager-allocate-pid)
- [`xv::process::ProcessManager::create_initial_process`](../lifecycle.md#lifecycle-processmanager-create-initial-process)
- [`xv::process::ProcessManager::dump`](../lifecycle.md#lifecycle-processmanager-dump)
- [`xv::process::ProcessManager::exit_process`](../lifecycle.md#lifecycle-processmanager-exit-process)
- [`xv::process::ProcessManager::fork_process`](../lifecycle.md#lifecycle-processmanager-fork-process)
- [`xv::process::ProcessManager::initialize`](../lifecycle.md#lifecycle-processmanager-initialize)
- [`xv::process::ProcessManager::kill`](../lifecycle.md#lifecycle-processmanager-kill)
- [`xv::process::ProcessManager::reap`](../lifecycle.md#lifecycle-processmanager-reap)
- [`xv::process::ProcessManager::reparent`](../lifecycle.md#lifecycle-processmanager-reparent)
- [`xv::process::ProcessManager::snapshot_after`](process_manager.cpp.md#fn-bbc335370e273170)
- [`xv::process::ProcessManager::wakeup`](../lifecycle.md#lifecycle-processmanager-wakeup)

<a id="fn-bbc335370e273170"></a>

## `xv::process::ProcessManager::snapshot_after`

```doxygen
@fn bool xv::process::ProcessManager::snapshot_after(int after, ProcessInfo &result)
@brief 复制 PID 大于给定值的最小活动进程信息。
@details 先取得家族锁，再由存储遍历逐进程加锁，以保护父指针、状态与名称。组/会话经组锁查询，地址空间大小读取独立原子发布值；回调不睡眠，不保留 Process 指针。返回 false 表示没有后续 PID。
```
