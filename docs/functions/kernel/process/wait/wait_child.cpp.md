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
@brief 按编码状态清除停止或继续事件，并释放子锁。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
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
@brief 按原 PID/组和事件筛选顺序直接扫描子链，保留命中子锁并编码状态。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
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
@brief 在锁内登记可中断等待，解锁睡眠，再重新取得 family 锁。
@details 入口持 family 锁；只直接扫描自己的子链，候选事件保留子锁。复制状态成功后才消费事件或回收，reap 后仅使用提前保存的 PID。睡眠在 family 锁内登记，解锁调度后重新持锁并重扫。 下一节点由图策略声明，业务结果与控制信号分离。
```
