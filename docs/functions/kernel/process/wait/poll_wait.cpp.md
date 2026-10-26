# `kernel/process/wait/poll_wait.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/poll_wait.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:poll`, `:wait_channels`, `:poll_entry`, `:storage_platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::PollWait::allocate_page`](poll_wait.cpp.md#fn-a6b19002bbc8ed4b)
- [`xv::process::PollWait::notify`](poll_wait.cpp.md#fn-ac78ce960fa8587b)
- [`xv::process::PollWait::set_deadline`](poll_wait.cpp.md#fn-a963ef69e5d7511f)
- [`xv::process::PollWait::sleep`](poll_wait.cpp.md#fn-f79ddf34a080a9c4)
- [`xv::process::PollWait::table`](poll_wait.cpp.md#fn-ef1952a6c3f61b5f)
- [`xv::process::PollWait::triggered`](poll_wait.cpp.md#fn-58af055f0fb1e43c)
- [`xv::process::PollWait::watch`](poll_wait.cpp.md#fn-c0a508d830521c5d)
- [`xv::process::PollWait::~PollWait`](poll_wait.cpp.md#fn-000e85dd21110cb3)
- [`xv::process::PollWait::Page::Page`](poll_wait.cpp.md#fn-f4be9d404d94855a)
- [`xv::process::PollWait::Page::Table::Table`](poll_wait.cpp.md#fn-6646ab2c1e744b7a)
- [`xv::process::PollWait::Page::Table::watch`](poll_wait.cpp.md#fn-b7811a5bb7845e0b)
- [`xv::process::WaitChannels::arm_timer`](poll_wait.cpp.md#fn-3352c9d1a67a7dac)
- [`xv::process::WaitChannels::expire_timers`](poll_wait.cpp.md#fn-de20066532420bb1)
- [`xv::process::WaitChannels::remove_timer`](poll_wait.cpp.md#fn-e197f48a1ef8cac7)
- [`xv::process::WaitChannels::subscribe`](poll_wait.cpp.md#fn-1194a4f1b3678f15)
- [`xv::process::WaitChannels::unsubscribe`](poll_wait.cpp.md#fn-417e58d7ce31f1b6)

<a id="fn-6646ab2c1e744b7a"></a>

## `xv::process::PollWait::Page::Table::Table`

```doxygen
@fn explicit xv::process::PollWait::Page::Table::Table(PollWait &wait)
@brief 构造单次 poll 等待上下文，建立本重载要求的成员初值和借用关系。
@param wait 本次 poll 等待上下文。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`owner`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b7811a5bb7845e0b"></a>

## `xv::process::PollWait::Page::Table::watch`

```doxygen
@fn void xv::process::PollWait::Page::Table::watch(void *channel) override
@brief 单次 poll 等待上下文：登记一个影响就绪条件的对象通道。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`owner.watch`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f4be9d404d94855a"></a>

## `xv::process::PollWait::Page::Page`

```doxygen
@fn explicit xv::process::PollWait::Page::Page(PollWait &wait)
@brief 构造单次 poll 等待上下文，建立本重载要求的成员初值和借用关系。
@param wait 本次 poll 等待上下文。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`table`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a6b19002bbc8ed4b"></a>

## `xv::process::PollWait::allocate_page`

```doxygen
@fn PollWait::Page *xv::process::PollWait::allocate_page()
@brief 单次 poll 等待上下文：取得一个物理页或内部节点页。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::allocate_process_page` → `new` → `Page`。
@details 本函数可见的状态更新包括：`failed_ = true;`；`page->next = pages_;`；`pages_ = page;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!storage`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`nullptr`；`page`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ef1952a6c3f61b5f"></a>

## `xv::process::PollWait::table`

```doxygen
@fn xv::sync::PollTable *xv::process::PollWait::table()
@brief 单次 poll 等待上下文：读取或计算 `nullptr`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocate_page`。
@details 需要特别跟踪的条件：`!pages_ && !allocate_page()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`nullptr`；`&pages_->table`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c0a508d830521c5d"></a>

## `xv::process::PollWait::watch`

```doxygen
@fn void xv::process::PollWait::watch(void *channel)
@brief 单次 poll 等待上下文：登记一个影响就绪条件的对象通道。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocate_page` → `channels_.subscribe`。
@details 本函数可见的状态更新包括：`entry.owner = this;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!channel || failed_`；`page->entries[i].channel == channel`；`!pages_ || pages_->used == sizeof(pages_->entries) / sizeof(PollEntry)`；`!allocate_page()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-000e85dd21110cb3"></a>

## `xv::process::PollWait::~PollWait`

```doxygen
@fn xv::process::PollWait::~PollWait()
@brief 结束单次 poll 等待上下文的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`channels_.remove_timer` → `channels_.unsubscribe` → `Page` → `detail::free_process_page`。
@details 本函数可见的状态更新包括：`pages_ = page->next;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`pages_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-58af055f0fb1e43c"></a>

## `xv::process::PollWait::triggered`

```doxygen
@fn bool xv::process::PollWait::triggered() const
@brief 单次 poll 等待上下文：读取或计算 `triggered_`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@return 各分支实际返回：`triggered_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-ac78ce960fa8587b"></a>

## `xv::process::PollWait::notify`

```doxygen
@fn void xv::process::PollWait::notify()
@brief 单次 poll 等待上下文：通知。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `channels_.scheduler_.make_runnable`。
@details 本函数可见的状态更新包括：`triggered_ = true;`；`process_.chan = nullptr;`；`process_.interruptible_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`process_.chan == this`；`process_.state == ProcessState::Sleeping`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-a963ef69e5d7511f"></a>

## `xv::process::PollWait::set_deadline`

```doxygen
@fn void xv::process::PollWait::set_deadline(unsigned long milliseconds)
@brief 单次 poll 等待上下文：给本次等待设置截止时刻。
@param milliseconds 以毫秒表示的时刻或截止值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`channels_.arm_timer`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f79ddf34a080a9c4"></a>

## `xv::process::PollWait::sleep`

```doxygen
@fn void xv::process::PollWait::sleep()
@brief 单次 poll 等待上下文：在已登记的等待条件下挂起当前进程。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`channels_.cancel` → `process_.lock.acquire` → `process_.pending_signals_.interrupting` → `process_.pending_signals_.caught_signal` → `channels_.scheduler_.suspend` → `process_.lock.release`。
@details 本函数可见的状态更新包括：`process_.syscall_interrupted_ = true;`；`process_.chan = this;`；`process_.interruptible_ = true;`；`process_.state = ProcessState::Sleeping;`；`process_.chan = nullptr;`；`process_.interruptible_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`process_.pending_signals_.caught_signal()`；`!triggered_ && !interrupted`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`process_.lock.acquire` → `process_.lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1194a4f1b3678f15"></a>

## `xv::process::WaitChannels::subscribe`

```doxygen
@fn void xv::process::WaitChannels::subscribe(PollEntry &entry, void *channel)
@brief 对象等待通道表：把 poll 订阅节点链接到对象等待通道。
@param entry 需要链接或解析的条目。
@param channel 等待条件或通知对象的地址键；不能在未结束登记时销毁对应对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`index` → `guard`。
@details 本函数可见的状态更新包括：`entry.channel = channel;`；`entry.next = bucket.pollers;`；`bucket.pollers = &entry;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-417e58d7ce31f1b6"></a>

## `xv::process::WaitChannels::unsubscribe`

```doxygen
@fn void xv::process::WaitChannels::unsubscribe(PollEntry &entry)
@brief 对象等待通道表：撤销 poll 订阅并解除节点链接。
@param entry 需要链接或解析的条目。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`index` → `guard` → `detail::storage_fail`。
@details 需要特别跟踪的条件：`*link && *link != &entry`；`!*link`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3352c9d1a67a7dac"></a>

## `xv::process::WaitChannels::arm_timer`

```doxygen
@fn void xv::process::WaitChannels::arm_timer(PollWait &wait, unsigned long deadline)
@brief 对象等待通道表：设置下一次本 hart 的硬件定时截止值。
@param wait 本次 poll 等待上下文。
@param deadline 本次操作使用的 deadline 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::storage_fail`。
@details 本函数可见的状态更新包括：`wait.deadline_ = deadline;`；`wait.timer_next_ = timers_;`；`wait.timer_linked_ = true;`；`timers_ = &wait;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`wait.timer_linked_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e197f48a1ef8cac7"></a>

## `xv::process::WaitChannels::remove_timer`

```doxygen
@fn void xv::process::WaitChannels::remove_timer(PollWait &wait)
@brief 对象等待通道表：撤销等待上下文的超时登记。
@param wait 本次 poll 等待上下文。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `detail::storage_fail`。
@details 本函数可见的状态更新包括：`wait.timer_linked_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!wait.timer_linked_`；`*link && *link != &wait`；`!*link`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-de20066532420bb1"></a>

## `xv::process::WaitChannels::expire_timers`

```doxygen
@fn void xv::process::WaitChannels::expire_timers(unsigned long milliseconds)
@brief 对象等待通道表：通知并移除已经到期的等待项。
@param milliseconds 以毫秒表示的时刻或截止值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `wait->notify`。
@details 本函数可见的状态更新包括：`wait->timer_linked_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`*link`；`milliseconds >= wait->deadline_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
