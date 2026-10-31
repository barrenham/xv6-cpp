# `kernel/process/wait/wait_child.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/wait_child.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ProcessManager::WaitFlow::consume`](wait_child.cpp.md#fn-70e6b74367f3ab46)
- [`xv::process::ProcessManager::WaitFlow::copy_status`](wait_child.cpp.md#fn-b90d19e2c4ba9321)
- [`xv::process::ProcessManager::WaitFlow::eligible`](wait_child.cpp.md#fn-9691227e1429a54e)
- [`xv::process::ProcessManager::WaitFlow::exited`](wait_child.cpp.md#fn-a8b902dade37e750)
- [`xv::process::ProcessManager::WaitFlow::failure`](wait_child.cpp.md#fn-d6f033ab1d60f3ff)
- [`xv::process::ProcessManager::WaitFlow::found`](wait_child.cpp.md#fn-1ca469afdb597e38)
- [`xv::process::ProcessManager::WaitFlow::nohang`](wait_child.cpp.md#fn-393219f772f37871)
- [`xv::process::ProcessManager::WaitFlow::nonblocking`](wait_child.cpp.md#fn-2c9884b222a9f8d8)
- [`xv::process::ProcessManager::WaitFlow::reap`](wait_child.cpp.md#fn-401253c81cfead64)
- [`xv::process::ProcessManager::WaitFlow::select_event`](wait_child.cpp.md#fn-7b2b7f4364b5b489)
- [`xv::process::ProcessManager::WaitFlow::success`](wait_child.cpp.md#fn-99647bf06191326c)
- [`xv::process::ProcessManager::WaitFlow::unlock_child`](wait_child.cpp.md#fn-d6f89fd1d2506785)
- [`xv::process::ProcessManager::WaitFlow::wait`](wait_child.cpp.md#fn-2561b054e7c8bb31)
- [`xv::process::ProcessManager::wait`](wait_child.cpp.md#fn-15b2744ee3edc5d8)
- [`xv::process::ProcessManager::wait_child`](wait_child.cpp.md#fn-d7ca44eff96231ec)
- [`xv::process::ProcessManager::waitpid`](wait_child.cpp.md#fn-3d4e53d945a90f89)

- [`xv::process::ProcessManager::update_child_event`](wait_child.cpp.md#fn-b6cdc964d2b78f2e)

- [`xv::process::ProcessManager::refresh_child_events`](wait_child.cpp.md#fn-03da042b7d74a1e6)

- [`xv::process::ProcessManager::remove_child_events`](wait_child.cpp.md#fn-3f9e1ed58704500c)

- [`xv::process::ProcessManager::queue_child_notification`](wait_child.cpp.md#fn-934632d76503279c)

- [`xv::process::ProcessManager::cancel_child_notification`](wait_child.cpp.md#fn-06ba42f93fa856c3)

- [`xv::process::ProcessManager::collect_child_notifications`](wait_child.cpp.md#fn-7406f868096667c7)

<a id="fn-15b2744ee3edc5d8"></a>

## `xv::process::ProcessManager::wait`

```doxygen
@fn int xv::process::ProcessManager::wait(unsigned long address)
@brief 进程生命周期管理器：读取或计算 `wait_child(-1, address, 0, false)`，返回当前查询结果。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`wait_child`。
@return 各分支实际返回：`wait_child(-1, address, 0, false)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3d4e53d945a90f89"></a>

## `xv::process::ProcessManager::waitpid`

```doxygen
@fn int xv::process::ProcessManager::waitpid(int pid, unsigned long address, int options)
@brief 进程生命周期管理器：读取或计算 `-1`，返回当前查询结果。
@param pid 进程选择编号；特殊值语义由对应进程接口决定。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param options 调用选项位组合。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`wait_child`。
@details 需要特别跟踪的条件：`(options & ~(WNOHANG | WUNTRACED | WCONTINUED)) || pid < -0x7fffffff`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`wait_child(pid, address, options, true)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d7ca44eff96231ec"></a>

## `xv::process::ProcessManager::wait_child`

```doxygen
@fn int xv::process::ProcessManager::wait_child(int selector, unsigned long address, int options, bool encoded)
@brief 按 PID 或进程组选择当前进程的可等待子事件。
@param selector 本次操作使用的 selector 值。
@param address 地址参数；用户地址必须通过内存复制/映射校验路径访问。
@param options 调用选项位组合。
@param encoded 本次操作使用的 encoded 值。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 返回发生事件的子 PID；WNOHANG 且有匹配子进程但无事件时返回 0；无匹配子进程、终止或用户状态复制失败时返回 -1。
@details 家庭锁稳定 parent->first_child_ 与兄弟链接，直接遍历自己的子进程并逐个取得进程锁；不取得登记表锁，不检查无关进程。PID/组筛选、Zombie、停止和继续事件选择保持原语义。用户状态复制失败不回收或消费事件；成功回收通过 reap 脱链，停止/继续事件仅清除对应通知。
@details 流程由 WaitFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-70e6b74367f3ab46"></a>

## `xv::process::ProcessManager::WaitFlow::consume`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::consume(Context &c) noexcept
@brief 状态复制成功后清除已报告的停止或继续标志，并在 family 与子锁内同步事件队列，最后释放子进程锁。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-b90d19e2c4ba9321"></a>

## `xv::process::ProcessManager::WaitFlow::copy_status`

```doxygen
@fn static bool xv::process::ProcessManager::WaitFlow::copy_status(Context &c) noexcept
@brief 仅在地址非零时复制已编码状态，报告是否成功。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9691227e1429a54e"></a>

## `xv::process::ProcessManager::WaitFlow::eligible`

```doxygen
@fn static bool xv::process::ProcessManager::WaitFlow::eligible(Context &c) noexcept
@brief 检查存在匹配子进程且父进程尚未被终止。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a8b902dade37e750"></a>

## `xv::process::ProcessManager::WaitFlow::exited`

```doxygen
@fn static bool xv::process::ProcessManager::WaitFlow::exited(Context &c) noexcept
@brief 检查选中事件是否为子进程退出。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d6f033ab1d60f3ff"></a>

## `xv::process::ProcessManager::WaitFlow::failure`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::failure(Context &c) noexcept
@brief 释放 family 锁并保留负一结果。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1ca469afdb597e38"></a>

## `xv::process::ProcessManager::WaitFlow::found`

```doxygen
@fn static bool xv::process::ProcessManager::WaitFlow::found(Context &c) noexcept
@brief 检查是否选中了一个待报告子事件。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-393219f772f37871"></a>

## `xv::process::ProcessManager::WaitFlow::nohang`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::nohang(Context &c) noexcept
@brief 释放 family 锁并记录零返回值，不消费事件。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2c9884b222a9f8d8"></a>

## `xv::process::ProcessManager::WaitFlow::nonblocking`

```doxygen
@fn static bool xv::process::ProcessManager::WaitFlow::nonblocking(Context &c) noexcept
@brief 检查是否指定 WNOHANG。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-401253c81cfead64"></a>

## `xv::process::ProcessManager::WaitFlow::reap`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::reap(Context &c) noexcept
@brief 在 family 和子锁协议下回收僵尸，之后不再访问子描述符。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7b2b7f4364b5b489"></a>

## `xv::process::ProcessManager::WaitFlow::select_event`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::select_event(Context &c) noexcept
@brief 正 PID 经索引定位并核对直接亲属关系；任意子进程等待只访问请求种类的待处理事件队列，并在子锁内复核延迟继续状态。按组等待在必要时扫描直接子进程判断是否仍有匹配成员。成功保留子锁，退出优先，状态复制失败不消费。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-99647bf06191326c"></a>

## `xv::process::ProcessManager::WaitFlow::success`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::success(Context &c) noexcept
@brief 释放 family 锁并返回提前保存的 PID。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d6f89fd1d2506785"></a>

## `xv::process::ProcessManager::WaitFlow::unlock_child`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::unlock_child(Context &c) noexcept
@brief 在状态复制失败时释放子锁，保留事件供后续等待。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2561b054e7c8bb31"></a>

## `xv::process::ProcessManager::WaitFlow::wait`

```doxygen
@fn static void xv::process::ProcessManager::WaitFlow::wait(Context &c) noexcept
@brief 仍持 family 锁时登记到当前父进程独立的子事件通道，再释放 family 锁并睡眠；唤醒后重新取得 family 锁检查事件，保持提前通知不会丢失。
@note 锁交接、复杂度与验证边界见 docs/process-index-wait.md。
```

<a id="fn-b6cdc964d2b78f2e"></a>

## `xv::process::ProcessManager::update_child_event`

```doxygen
@fn void xv::process::ProcessManager::update_child_event(Process &child, unsigned kind, bool pending)
@brief 在 family 锁内以常数时间链接或摘除一种子进程事件；每种事件使用独立的侵入式双向链接。调用方持子进程锁，父进程由亲属关系固定；不申请内存。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-03da042b7d74a1e6"></a>

## `xv::process::ProcessManager::refresh_child_events`

```doxygen
@fn void xv::process::ProcessManager::refresh_child_events(Process &child)
@brief 在 family 与子进程锁内依据 Zombie、stop_event 和 continued_event 同步三个待报告队列。退出优先并移除过时停止/继续记录，已入队事件不重复插入。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-3f9e1ed58704500c"></a>

## `xv::process::ProcessManager::remove_child_events`

```doxygen
@fn void xv::process::ProcessManager::remove_child_events(Process &child)
@brief 在解除父子关系前，从旧父进程的三个事件队列移除该子进程；不消费业务状态，重新认领时可按现有状态重新登记。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-934632d76503279c"></a>

## `xv::process::ProcessManager::queue_child_notification`

```doxygen
@fn void xv::process::ProcessManager::queue_child_notification(Process &child)
@brief 持子进程锁，将 SIGCONT 的延迟亲属通知去重加入独立队列，并以 release 发布通知标志。队列锁内不获取进程锁或 family 锁。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-06ba42f93fa856c3"></a>

## `xv::process::ProcessManager::cancel_child_notification`

```doxygen
@fn void xv::process::ProcessManager::cancel_child_notification(Process &child)
@brief 持子进程锁，在回收之前从延迟通知队列常数时间摘除该进程。已发布子进程的回收还须持 family 锁，避免消费者借用悬空对象。
@note 详细锁协议见 docs/process-index-wait.md。
```

<a id="fn-7406f868096667c7"></a>

## `xv::process::ProcessManager::collect_child_notifications`

```doxygen
@fn void xv::process::ProcessManager::collect_child_notifications()
@brief 持 family 锁逐个取出待处理的继续通知；先释放通知队列锁再取得子进程锁，发布事件与 SIGCHLD，释放子进程锁后只唤醒其父进程。family 锁固定出队对象与父对象，收集不扫描进程注册表。
@note 详细锁协议见 docs/process-index-wait.md。
```
