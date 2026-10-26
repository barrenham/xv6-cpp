# `kernel/process/lifecycle/process_manager.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/process_manager.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.print`, `xv.runtime`, `xv.fs`, `:manager`, `xv.memory`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

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
- [`xv::process::ProcessManager::wakeup`](../lifecycle.md#lifecycle-processmanager-wakeup)
