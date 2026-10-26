# `kernel/process/lifecycle/process_manager.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/process_manager.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:manager`。

直接模块依赖：`:process`, `:store`, `:scheduler`, `:wait_channels`, `:groups`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::first_return`](first_return.cpp.md#fn-f06f00214daff1d9)
- [`xv::process::ProcessManager::ProcessManager`](process_manager.cppm.md#fn-b0cae405c6f752ad)
- [`xv::process::ProcessManager::allocate`](../lifecycle.md#lifecycle-processmanager-allocate)
- [`xv::process::ProcessManager::allocate_pid`](../lifecycle.md#lifecycle-processmanager-allocate-pid)
- [`xv::process::ProcessManager::claim_terminal`](../groups/group_operations.cpp.md#fn-dc2905888d371e3b)
- [`xv::process::ProcessManager::create_initial_process`](../lifecycle.md#lifecycle-processmanager-create-initial-process)
- [`xv::process::ProcessManager::create_session`](../groups/group_operations.cpp.md#fn-ac54851324d72c43)
- [`xv::process::ProcessManager::deliver_signals`](../signals/signal_delivery.cpp.md#fn-f31a3cd50df7bcf4)
- [`xv::process::ProcessManager::detach_terminal`](../groups/group_operations.cpp.md#fn-82dd9a0dcde32e3a)
- [`xv::process::ProcessManager::did_exec`](../groups/group_operations.cpp.md#fn-026cc66345c86e17)
- [`xv::process::ProcessManager::dump`](../lifecycle.md#lifecycle-processmanager-dump)
- [`xv::process::ProcessManager::exit_process`](../lifecycle.md#lifecycle-processmanager-exit-process)
- [`xv::process::ProcessManager::fork_process`](../lifecycle.md#lifecycle-processmanager-fork-process)
- [`xv::process::ProcessManager::get_process_group`](../groups/group_operations.cpp.md#fn-5273c2c990423d82)
- [`xv::process::ProcessManager::get_session`](../groups/group_operations.cpp.md#fn-4d575d2c1c9debae)
- [`xv::process::ProcessManager::initialize`](../lifecycle.md#lifecycle-processmanager-initialize)
- [`xv::process::ProcessManager::kill`](../lifecycle.md#lifecycle-processmanager-kill)
- [`xv::process::ProcessManager::may_read_terminal`](../groups/group_operations.cpp.md#fn-6ef8f28b8535c2e1)
- [`xv::process::ProcessManager::notify_child_events`](../signals/signal_delivery.cpp.md#fn-8b264f5ebddc7cc5)
- [`xv::process::ProcessManager::post_child_signal`](../signals/signal_delivery.cpp.md#fn-53b39d608263bc67)
- [`xv::process::ProcessManager::post_terminal_interrupt`](../signals/signal_delivery.cpp.md#fn-0373c5a31ed35b56)
- [`xv::process::ProcessManager::reap`](../lifecycle.md#lifecycle-processmanager-reap)
- [`xv::process::ProcessManager::reparent`](../lifecycle.md#lifecycle-processmanager-reparent)
- [`xv::process::ProcessManager::send_signal`](../signals/signal_delivery.cpp.md#fn-acc6667b4a47dba0)
- [`xv::process::ProcessManager::service_signal_notifications`](../signals/signal_delivery.cpp.md#fn-2ed2e57fbe59b75f)
- [`xv::process::ProcessManager::set_process_group`](../groups/group_operations.cpp.md#fn-da222928090ae0f5)
- [`xv::process::ProcessManager::set_signal_disposition`](../signals/signal_delivery.cpp.md#fn-51bd3bad2bb3bae7)
- [`xv::process::ProcessManager::set_terminal_foreground`](../groups/group_operations.cpp.md#fn-855c9b132f03c0ea)
- [`xv::process::ProcessManager::terminal_foreground`](../groups/group_operations.cpp.md#fn-58daa43bb91047a3)
- [`xv::process::ProcessManager::wait`](../wait/wait_child.cpp.md#fn-15b2744ee3edc5d8)
- [`xv::process::ProcessManager::wait_child`](../wait/wait_child.cpp.md#fn-d7ca44eff96231ec)
- [`xv::process::ProcessManager::waitpid`](../wait/wait_child.cpp.md#fn-3d4e53d945a90f89)
- [`xv::process::ProcessManager::wake_for_signal`](../signals/signal_delivery.cpp.md#fn-5a8dcf9b719eaa90)
- [`xv::process::ProcessManager::wake_signal_waiters`](../signals/signal_delivery.cpp.md#fn-e54dae23e0a9982b)
- [`xv::process::ProcessManager::wakeup`](../lifecycle.md#lifecycle-processmanager-wakeup)

<a id="fn-b0cae405c6f752ad"></a>

## `xv::process::ProcessManager::ProcessManager`

```doxygen
@fn constexpr xv::process::ProcessManager::ProcessManager(ProcessStore &store, Scheduler &scheduler, WaitChannels &waits, ProcessGroups &groups)
@brief 构造进程生命周期管理器，建立本重载要求的成员初值和借用关系。
@param store 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param scheduler 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param waits 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param groups 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`store_` → `scheduler_` → `waits_` → `groups_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
