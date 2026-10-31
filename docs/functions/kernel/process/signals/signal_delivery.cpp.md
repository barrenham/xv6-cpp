# `kernel/process/signals/signal_delivery.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/signal_delivery.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessManager::DeliverSignalFlow::caught`](signal_delivery.cpp.md#fn-38ef1d6ef167ae85)
- [`xv::process::ProcessManager::DeliverSignalFlow::defaults`](signal_delivery.cpp.md#fn-044e80ec1a66b44c)
- [`xv::process::ProcessManager::DeliverSignalFlow::entered`](signal_delivery.cpp.md#fn-cb03b92572b8ea10)
- [`xv::process::ProcessManager::DeliverSignalFlow::fault`](signal_delivery.cpp.md#fn-757ba97fc7963478)
- [`xv::process::ProcessManager::DeliverSignalFlow::handler`](signal_delivery.cpp.md#fn-3aeaa1af8002dcb6)
- [`xv::process::ProcessManager::DeliverSignalFlow::prepare`](signal_delivery.cpp.md#fn-641bb66435ddf15a)
- [`xv::process::ProcessManager::DeliverSignalFlow::publish`](signal_delivery.cpp.md#fn-c876d4c97652ad8f)
- [`xv::process::ProcessManager::DeliverSignalFlow::retry`](signal_delivery.cpp.md#fn-307320d5e3bbd1e1)
- [`xv::process::ProcessManager::DeliverSignalFlow::status`](signal_delivery.cpp.md#fn-01a46cd178ffa830)
- [`xv::process::ProcessManager::DeliverSignalFlow::stopping`](signal_delivery.cpp.md#fn-f6ac4423a1b1c085)
- [`xv::process::ProcessManager::DeliverSignalFlow::suspend`](signal_delivery.cpp.md#fn-c417f4e9eae2749f)
- [`xv::process::ProcessManager::DeliverSignalFlow::terminate`](signal_delivery.cpp.md#fn-23a1d0ac5f5919ea)
- [`xv::process::ProcessManager::DeliverSignalFlow::terminating`](signal_delivery.cpp.md#fn-7f9e8b3931bcf7a4)
- [`xv::process::ProcessManager::DeliverSignalFlow::valid_stop`](signal_delivery.cpp.md#fn-aa9fc9fb3f9d4dcf)
- [`xv::process::ProcessManager::NotifyChildFlow::again`](signal_delivery.cpp.md#fn-b890898e036c42de)
- [`xv::process::ProcessManager::NotifyChildFlow::collect`](signal_delivery.cpp.md#fn-c549043117dd8daf)
- [`xv::process::ProcessManager::NotifyChildFlow::wake`](signal_delivery.cpp.md#fn-b044ef981295a2f1)
- [`xv::process::ProcessManager::SendGroupSignalFlow::matched`](signal_delivery.cpp.md#fn-2b220c97724945b3)
- [`xv::process::ProcessManager::SendGroupSignalFlow::signal`](signal_delivery.cpp.md#fn-287e9cc22f04e67e)
- [`xv::process::ProcessManager::SendGroupSignalFlow::success`](signal_delivery.cpp.md#fn-868e41f4a1334631)
- [`xv::process::ProcessManager::SendGroupSignalFlow::visit`](signal_delivery.cpp.md#fn-921c28a61bc8f3b1)
- [`xv::process::ProcessManager::SendGroupSignalFlow::wake`](signal_delivery.cpp.md#fn-1a04dd75c97b58d0)
- [`xv::process::ProcessManager::SendPidSignalFlow::find`](signal_delivery.cpp.md#fn-11058bd57e79e470)
- [`xv::process::ProcessManager::SendPidSignalFlow::found`](signal_delivery.cpp.md#fn-c612c08a5164604d)
- [`xv::process::ProcessManager::SendPidSignalFlow::notify`](signal_delivery.cpp.md#fn-68f2e16ccfc18896)
- [`xv::process::ProcessManager::SendPidSignalFlow::permitted`](signal_delivery.cpp.md#fn-5089273aeb49c304)
- [`xv::process::ProcessManager::SendPidSignalFlow::post`](signal_delivery.cpp.md#fn-5d0b53ba75d5ff99)
- [`xv::process::ProcessManager::SendPidSignalFlow::postable`](signal_delivery.cpp.md#fn-80487efaf58718c0)
- [`xv::process::ProcessManager::SendPidSignalFlow::unlock`](signal_delivery.cpp.md#fn-6cd36e183bb56c66)
- [`xv::process::ProcessManager::WakeSignalFlow::clear_wait`](signal_delivery.cpp.md#fn-43997b60aa5d28b3)
- [`xv::process::ProcessManager::WakeSignalFlow::continued`](signal_delivery.cpp.md#fn-8bc156188219c4d0)
- [`xv::process::ProcessManager::WakeSignalFlow::interrupt`](signal_delivery.cpp.md#fn-81ad5dd90ca9066d)
- [`xv::process::ProcessManager::WakeSignalFlow::resume`](signal_delivery.cpp.md#fn-9979a62b92679527)
- [`xv::process::ProcessManager::WakeSignalFlow::runnable`](signal_delivery.cpp.md#fn-251cc7a2578091ec)
- [`xv::process::ProcessManager::WakeSignalFlow::sleeping`](signal_delivery.cpp.md#fn-371ebec4c595b489)
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
@brief 正 PID 使用红黑树定位，进程组目标继续走组成员访问；保留信号零探测、权限及停止/继续语义。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-f31a3cd50df7bcf4"></a>

## `xv::process::ProcessManager::deliver_signals`

```doxygen
@fn void xv::process::ProcessManager::deliver_signals()
@brief 进程生命周期管理器：在安全返回边界执行默认动作或建立用户信号帧。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 DeliverSignalFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 WakeSignalFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@brief 调用方持 family 和子进程锁：先同步父进程事件队列，再原子发布 SIGCHLD；不获取父进程锁或等待桶锁。捕获信号的后续唤醒沿用原通知协议。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-8b264f5ebddc7cc5"></a>

## `xv::process::ProcessManager::notify_child_events`

```doxygen
@fn void xv::process::ProcessManager::notify_child_events()
@brief 进程生命周期管理器：在锁顺序允许的边界发布子进程状态变化。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 NotifyChildFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-38ef1d6ef167ae85"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::caught`

```doxygen
@fn static bool xv::process::ProcessManager::DeliverSignalFlow::caught(Context &c) noexcept
@brief 按原读取次序检查非停止状态与捕获信号。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-044e80ec1a66b44c"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::defaults`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::defaults(Context &c) noexcept
@brief 按原动作和掩码分别消费默认 CHLD 与 CONT。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cb03b92572b8ea10"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::entered`

```doxygen
@fn static bool xv::process::ProcessManager::DeliverSignalFlow::entered(Context &c) noexcept
@brief 检查用户处理器是否成功建立。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-757ba97fc7963478"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::fault`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::fault(Context &c) noexcept
@brief 按 SIGSEGV 结束无法建立用户信号帧的进程。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3aeaa1af8002dcb6"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::handler`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::handler(Context &c) noexcept
@brief 尝试构造并进入用户信号处理帧。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-641bb66435ddf15a"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::prepare`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::prepare(Context &c) noexcept
@brief 取消当前等待登记，取得 family 锁，在子进程锁之前只通知自己的父进程，然后锁住当前进程提取停止信号。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-c876d4c97652ad8f"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::publish`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::publish(Context &c) noexcept
@brief 发布停止状态与父通知然后释放家庭锁。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-307320d5e3bbd1e1"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::retry`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::retry(Context &c) noexcept
@brief 释放子锁再释放家庭锁以重新检查信号。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-01a46cd178ffa830"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::status`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::status(Context &c) noexcept
@brief 读取持锁保护的终止状态。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f6ac4423a1b1c085"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::stopping`

```doxygen
@fn static bool xv::process::ProcessManager::DeliverSignalFlow::stopping(Context &c) noexcept
@brief 检查是否仍请求停止。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c417f4e9eae2749f"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::suspend`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::suspend(Context &c) noexcept
@brief 持子进程锁切换调度，恢复后释放子锁。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-23a1d0ac5f5919ea"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::terminate`

```doxygen
@fn static void xv::process::ProcessManager::DeliverSignalFlow::terminate(Context &c) noexcept
@brief 再次读取终止信号并按原回退规则退出。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7f9e8b3931bcf7a4"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::terminating`

```doxygen
@fn static bool xv::process::ProcessManager::DeliverSignalFlow::terminating(Context &c) noexcept
@brief 检查是否应立即结束进程。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-aa9fc9fb3f9d4dcf"></a>

## `xv::process::ProcessManager::DeliverSignalFlow::valid_stop`

```doxygen
@fn static bool xv::process::ProcessManager::DeliverSignalFlow::valid_stop(Context &c) noexcept
@brief 检查停止信号有效且未被终止状态覆盖。
@details 保留终止、用户处理器、默认消费与停止优先次序；停止时先家庭锁后子锁，切换时持子锁，循环不递归。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b890898e036c42de"></a>

## `xv::process::ProcessManager::NotifyChildFlow::again`

```doxygen
@fn static bool xv::process::ProcessManager::NotifyChildFlow::again(Context &c) noexcept
@brief 原子取得并清除新通知以决定是否继续迭代。
@details 无通知快路径直接返回；每轮家庭锁保护 parent，访问器不获取父进程锁，家庭锁释放后才扫描唤醒。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c549043117dd8daf"></a>

## `xv::process::ProcessManager::NotifyChildFlow::collect`

```doxygen
@fn static void xv::process::ProcessManager::NotifyChildFlow::collect(Context &c) noexcept
@brief 委托有界存储的侵入式延迟通知队列收集，仅处理已登记的继续状态变化，不扫描全体进程。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-b044ef981295a2f1"></a>

## `xv::process::ProcessManager::NotifyChildFlow::wake`

```doxygen
@fn static void xv::process::ProcessManager::NotifyChildFlow::wake(Context &c) noexcept
@brief 释放家庭锁后唤醒信号等待者且不递归通知。
@details 无通知快路径直接返回；每轮家庭锁保护 parent，访问器不获取父进程锁，家庭锁释放后才扫描唤醒。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2b220c97724945b3"></a>

## `xv::process::ProcessManager::SendGroupSignalFlow::matched`

```doxygen
@fn static bool xv::process::ProcessManager::SendGroupSignalFlow::matched(Context &c) noexcept
@brief 检查是否至少匹配一个有权限的目标。
@details 组锁仅在 visit 回调期间固定 owner，原子投递不获取进程锁；访问完成后才扫描唤醒，不保留借用指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-287e9cc22f04e67e"></a>

## `xv::process::ProcessManager::SendGroupSignalFlow::signal`

```doxygen
@fn static bool xv::process::ProcessManager::SendGroupSignalFlow::signal(Context &c) noexcept
@brief 检查本次是否投递非零信号。
@details 组锁仅在 visit 回调期间固定 owner，原子投递不获取进程锁；访问完成后才扫描唤醒，不保留借用指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-868e41f4a1334631"></a>

## `xv::process::ProcessManager::SendGroupSignalFlow::success`

```doxygen
@fn static void xv::process::ProcessManager::SendGroupSignalFlow::success(Context &c) noexcept
@brief 记录零成功码。
@details 组锁仅在 visit 回调期间固定 owner，原子投递不获取进程锁；访问完成后才扫描唤醒，不保留借用指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-921c28a61bc8f3b1"></a>

## `xv::process::ProcessManager::SendGroupSignalFlow::visit`

```doxygen
@fn static void xv::process::ProcessManager::SendGroupSignalFlow::visit(Context &c) noexcept
@brief 枚举目标组并用原访问器检查权限和原子投递。
@details 组锁仅在 visit 回调期间固定 owner，原子投递不获取进程锁；访问完成后才扫描唤醒，不保留借用指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1a04dd75c97b58d0"></a>

## `xv::process::ProcessManager::SendGroupSignalFlow::wake`

```doxygen
@fn static void xv::process::ProcessManager::SendGroupSignalFlow::wake(Context &c) noexcept
@brief 在组锁释放后扫描可中断等待者。
@details 组锁仅在 visit 回调期间固定 owner，原子投递不获取进程锁；访问完成后才扫描唤醒，不保留借用指针。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-11058bd57e79e470"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::find`

```doxygen
@fn static void xv::process::ProcessManager::SendPidSignalFlow::find(Context &c) noexcept
@brief 按指定 PID 查询红黑树，成功取得已锁定且存活的进程描述符，后续权限检查和操作沿用原协议。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-c612c08a5164604d"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::found`

```doxygen
@fn static bool xv::process::ProcessManager::SendPidSignalFlow::found(Context &c) noexcept
@brief 检查目标是否存在。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-68f2e16ccfc18896"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::notify`

```doxygen
@fn static void xv::process::ProcessManager::SendPidSignalFlow::notify(Context &c) noexcept
@brief 处理子进程事件并记录零成功码。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5089273aeb49c304"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::permitted`

```doxygen
@fn static bool xv::process::ProcessManager::SendPidSignalFlow::permitted(Context &c) noexcept
@brief 检查初始进程限制与发送方目标方凭据权限。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5d0b53ba75d5ff99"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::post`

```doxygen
@fn static void xv::process::ProcessManager::SendPidSignalFlow::post(Context &c) noexcept
@brief 原子投递信号并处理目标唤醒。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-80487efaf58718c0"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::postable`

```doxygen
@fn static bool xv::process::ProcessManager::SendPidSignalFlow::postable(Context &c) noexcept
@brief 检查非零信号且目标尚非僵尸。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6cd36e183bb56c66"></a>

## `xv::process::ProcessManager::SendPidSignalFlow::unlock`

```doxygen
@fn static void xv::process::ProcessManager::SendPidSignalFlow::unlock(Context &c) noexcept
@brief 释放查找取得的目标进程锁。
@details 查找返回持锁进程；按原顺序验证初始进程限制与凭据，信号零只探测，解锁后才处理家庭事件。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-43997b60aa5d28b3"></a>

## `xv::process::ProcessManager::WakeSignalFlow::clear_wait`

```doxygen
@fn static void xv::process::ProcessManager::WakeSignalFlow::clear_wait(Context &c) noexcept
@brief 清除等待通道与可中断标志。
@details 调用者持进程锁；保留独立 pending 读取及停止、继续与可中断等待状态规则，共用最终 runnable 节点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8bc156188219c4d0"></a>

## `xv::process::ProcessManager::WakeSignalFlow::continued`

```doxygen
@fn static void xv::process::ProcessManager::WakeSignalFlow::continued(Context &c) noexcept
@brief 清除停止事件，按是否致命设置继续事件和独立通知状态，并加入去重延迟通知队列；此处仅持子进程锁，不反向获取 family 锁。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-81ad5dd90ca9066d"></a>

## `xv::process::ProcessManager::WakeSignalFlow::interrupt`

```doxygen
@fn static bool xv::process::ProcessManager::WakeSignalFlow::interrupt(Context &c) noexcept
@brief 检查当前等待是否可由待处理信号中断。
@details 调用者持进程锁；保留独立 pending 读取及停止、继续与可中断等待状态规则，共用最终 runnable 节点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9979a62b92679527"></a>

## `xv::process::ProcessManager::WakeSignalFlow::resume`

```doxygen
@fn static bool xv::process::ProcessManager::WakeSignalFlow::resume(Context &c) noexcept
@brief 检查已停止进程是否需要因 kill 或 continue 恢复。
@details 调用者持进程锁；保留独立 pending 读取及停止、继续与可中断等待状态规则，共用最终 runnable 节点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-251cc7a2578091ec"></a>

## `xv::process::ProcessManager::WakeSignalFlow::runnable`

```doxygen
@fn static void xv::process::ProcessManager::WakeSignalFlow::runnable(Context &c) noexcept
@brief 交由调度器使进程可运行。
@details 调用者持进程锁；保留独立 pending 读取及停止、继续与可中断等待状态规则，共用最终 runnable 节点。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-371ebec4c595b489"></a>

## `xv::process::ProcessManager::WakeSignalFlow::sleeping`

```doxygen
@fn static bool xv::process::ProcessManager::WakeSignalFlow::sleeping(Context &c) noexcept
@brief 检查进程是否处于睡眠状态。
@details 调用者持进程锁；保留独立 pending 读取及停止、继续与可中断等待状态规则，共用最终 runnable 节点。 下一节点由图策略声明，业务结果与控制信号分离。
```
