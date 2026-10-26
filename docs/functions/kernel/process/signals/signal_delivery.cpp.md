# `kernel/process/signals/signal_delivery.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/signal_delivery.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessManager::deliver_signals`](signal_delivery.cpp.md#fn-f31a3cd50df7bcf4)
- [`xv::process::ProcessManager::notify_child_events`](signal_delivery.cpp.md#fn-8b264f5ebddc7cc5)
- [`xv::process::ProcessManager::post_child_signal`](signal_delivery.cpp.md#fn-53b39d608263bc67)
- [`xv::process::ProcessManager::post_terminal_interrupt`](signal_delivery.cpp.md#fn-0373c5a31ed35b56)
- [`xv::process::ProcessManager::send_signal`](signal_delivery.cpp.md#fn-acc6667b4a47dba0)
- [`xv::process::ProcessManager::service_signal_notifications`](signal_delivery.cpp.md#fn-2ed2e57fbe59b75f)
- [`xv::process::ProcessManager::set_signal_disposition`](signal_delivery.cpp.md#fn-51bd3bad2bb3bae7)
- [`xv::process::ProcessManager::wake_for_signal`](signal_delivery.cpp.md#fn-5a8dcf9b719eaa90)
- [`xv::process::ProcessManager::wake_signal_waiters`](signal_delivery.cpp.md#fn-e54dae23e0a9982b)

<a id="fn-acc6667b4a47dba0"></a>

## `xv::process::ProcessManager::send_signal`

```doxygen
@fn int xv::process::ProcessManager::send_signal(int target, int signal)
@brief 进程生命周期管理器：按 PID 或进程组选择目标并登记信号。
@param target 本次操作使用的 target 值。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supported_signal` → `Pid` → `id_` → `matches` → `key` → `store_.find` → `process->lock.release` → `process->pending_signals_.post` → `wake_for_signal` → `notify_child_events` → `Post` → `initial_`。
@details 需要特别跟踪的条件：`(signal != 0 && !supported_signal(signal)) || target == -1 || target < -0x7fffffff`；`target > 0`；`!process`；`process == initial_ && signal != 0`；`signal && process->state != ProcessState::Zombie`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`process->lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 各分支实际返回：`-1`；`process.state != ProcessState::Unused && process.pid == id_`；`0`；无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f31a3cd50df7bcf4"></a>

## `xv::process::ProcessManager::deliver_signals`

```doxygen
@fn void xv::process::ProcessManager::deliver_signals()
@brief 进程生命周期管理器：在安全返回边界执行默认动作或建立用户信号帧。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `process->termination_status` → `process->pending_signals_.terminating_signal` → `exit_process` → `process->stop_requested` → `process->pending_signals_.caught_signal` → `process->enter_signal_handler` → `SIGNAL_EXIT_STATUS` → `process->pending_signals_.handled` → `process->pending_signals_.mask` → `process->pending_signals_.consume` → `process->pending_signals_.consume_continue`。
@details 本函数可见的状态更新包括：`process->stop_signal_ = signal;`；`process->stop_event_ = true;`；`process->continued_event_ = false;`；`process->state = ProcessState::Stopped;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`status != 0`；`!process->stop_requested() && process->pending_signals_.caught_signal()`；`!process->enter_signal_handler()`；`!(process->pending_signals_.handled() & (1U << SIGCHLD)) && !(process->pending_signals_.mask() & (1U << SIGCHLD))`；`!(process->pending_signals_.handled() & (1U << SIGCONT))`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`wait_lock_.acquire` → `process->lock.acquire` → `process->lock.release` → `wait_lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-51bd3bad2bb3bae7"></a>

## `xv::process::ProcessManager::set_signal_disposition`

```doxygen
@fn int xv::process::ProcessManager::set_signal_disposition(int signal, int action)
@brief 进程生命周期管理器：安装兼容的默认或忽略信号处置。
@param signal 待查询、投递或处理的信号编号。
@param action 本次操作使用的 动作 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `guard` → `process->pending_signals_.disposition`。
@return 各分支实际返回：`process->pending_signals_.disposition(signal, action)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0373c5a31ed35b56"></a>

## `xv::process::ProcessManager::post_terminal_interrupt`

```doxygen
@fn void xv::process::ProcessManager::post_terminal_interrupt(void *terminal, int signal)
@brief 进程生命周期管理器：登记终端中断。
@param terminal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Post` → `initial_` → `signal_` → `visit` → `member.owner` → `process->pending_signals_.post` → `post` → `groups_.visit_foreground` → `__atomic_store_n`。
@details 需要特别跟踪的条件：`process && process != initial_`；`signal_ == SIGHUP`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2ed2e57fbe59b75f"></a>

## `xv::process::ProcessManager::service_signal_notifications`

```doxygen
@fn void xv::process::ProcessManager::service_signal_notifications()
@brief 进程生命周期管理器：处理信号notifications。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`notify_child_events` → `__atomic_exchange_n` → `wake_signal_waiters`。
@details 需要特别跟踪的条件：`__atomic_exchange_n(&signal_notifications_, false, __ATOMIC_ACQ_REL)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5a8dcf9b719eaa90"></a>

## `xv::process::ProcessManager::wake_for_signal`

```doxygen
@fn void xv::process::ProcessManager::wake_for_signal(Process &process)
@brief 进程生命周期管理器：根据目标状态唤醒可中断或停止的进程。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.pending_signals_.pending` → `process.pending_signals_.terminating_signal` → `process.pending_signals_.continuing` → `__atomic_store_n` → `scheduler_.make_runnable` → `process.pending_signals_.interrupting`。
@details 本函数可见的状态更新包括：`process.stop_event_ = false;`；`process.continued_event_ = !fatal;`；`process.continue_notification_ = process.continue_notification_ || !fatal;`；`process.chan = nullptr;`；`process.interruptible_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`process.state == ProcessState::Stopped && (process.killed || (bits & (1U << SIGKILL)) || process.pending_signals_.continuing())`；`!process.interruptible_ || (!process.killed && !process.pending_signals_.interrupting())`；`process.state == ProcessState::Sleeping`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e54dae23e0a9982b"></a>

## `xv::process::ProcessManager::wake_signal_waiters`

```doxygen
@fn void xv::process::ProcessManager::wake_signal_waiters(bool notify_children)
@brief 进程生命周期管理器：wake信号waiters。
@param notify_children 本次操作使用的 通知children 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Wake` → `manager_` → `visit` → `process.pending_signals_.pending` → `process.pending_signals_.continuing` → `manager_.wake_for_signal` → `wake` → `store_.visit` → `notify_child_events`。
@details 需要特别跟踪的条件：`process.state != ProcessState::Unused && process.state != ProcessState::Zombie && (process.pending_signals_.pending() || process.pending_signals_.continuing())`；`notify_children`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-53b39d608263bc67"></a>

## `xv::process::ProcessManager::post_child_signal`

```doxygen
@fn void xv::process::ProcessManager::post_child_signal(Process &child)
@brief 进程生命周期管理器：向真实父进程登记子状态通知。
@param child 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`child.parent->pending_signals_.post` → `__atomic_store_n`。
@details 需要特别跟踪的条件：`child.parent`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8b264f5ebddc7cc5"></a>

## `xv::process::ProcessManager::notify_child_events`

```doxygen
@fn void xv::process::ProcessManager::notify_child_events()
@brief 进程生命周期管理器：在锁顺序允许的边界发布子进程状态变化。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_exchange_n` → `family` → `Continued` → `manager_` → `visit` → `manager_.post_child_signal` → `continued` → `store_.visit` → `wakeup` → `wake_signal_waiters`。
@details 本函数可见的状态更新包括：`child.continue_notification_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`__atomic_exchange_n(&child_notifications_, false, __ATOMIC_ACQ_REL)`；`!child.continue_notification_`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
