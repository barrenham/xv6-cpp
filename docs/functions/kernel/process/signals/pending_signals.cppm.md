# `kernel/process/signals/pending_signals.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/pending_signals.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:signals`。

## 函数导航

- [`xv::process::PendingSignals::PendingSignals`](pending_signals.cppm.md#fn-4b98f3f11386c457)
- [`xv::process::PendingSignals::PendingSignals`](pending_signals.cppm.md#fn-d3c879de37373f0c)
- [`xv::process::PendingSignals::action`](pending_signals.cppm.md#fn-424641a20d1b2a92)
- [`xv::process::PendingSignals::caught_signal`](pending_signals.cppm.md#fn-fe25ebc3fb856492)
- [`xv::process::PendingSignals::consume`](pending_signals.cppm.md#fn-bc6784b1f197250e)
- [`xv::process::PendingSignals::consume_continue`](pending_signals.cppm.md#fn-13fda9b2f7f5dd81)
- [`xv::process::PendingSignals::continuing`](pending_signals.cppm.md#fn-2937be41959f4896)
- [`xv::process::PendingSignals::deliverable`](pending_signals.cppm.md#fn-11a787a9b65e1b18)
- [`xv::process::PendingSignals::disposition`](pending_signals.cppm.md#fn-62c82a545a28e5c7)
- [`xv::process::PendingSignals::handled`](pending_signals.cppm.md#fn-40d9c697b275333b)
- [`xv::process::PendingSignals::inherit_dispositions`](pending_signals.cppm.md#fn-43e4f50ccbf29835)
- [`xv::process::PendingSignals::interrupting`](pending_signals.cppm.md#fn-f8da99019c33940b)
- [`xv::process::PendingSignals::mask`](pending_signals.cppm.md#fn-501023c87de27086)
- [`xv::process::PendingSignals::on_exec`](pending_signals.cppm.md#fn-9dce8b637d2bf6ad)
- [`xv::process::PendingSignals::operator=`](pending_signals.cppm.md#fn-3f6793e2e84f5c87)
- [`xv::process::PendingSignals::pending`](pending_signals.cppm.md#fn-3cdb3278b53f0228)
- [`xv::process::PendingSignals::post`](pending_signals.cppm.md#fn-d6f433243e974db7)
- [`xv::process::PendingSignals::reset`](pending_signals.cppm.md#fn-15444ef3bbd98d38)
- [`xv::process::PendingSignals::set_action`](pending_signals.cppm.md#fn-4da3940f9cfe6e6c)
- [`xv::process::PendingSignals::set_mask`](pending_signals.cppm.md#fn-889286c91713bbc7)
- [`xv::process::PendingSignals::stopping_signal`](pending_signals.cppm.md#fn-434b21a505b88509)
- [`xv::process::PendingSignals::take_stop`](pending_signals.cppm.md#fn-164df2530296d0ae)
- [`xv::process::PendingSignals::terminating_signal`](pending_signals.cppm.md#fn-5a25b62a6479f685)
- [`xv::process::supported_signal`](pending_signals.cppm.md#fn-61ff6553596cbb52)

<a id="fn-61ff6553596cbb52"></a>

## `xv::process::supported_signal`

```doxygen
@fn inline constexpr bool xv::process::supported_signal(int signal)
@brief 进程对象生命周期、组身份、调度、等待与信号：受支持信号。
@param signal 待查询、投递或处理的信号编号。
@return 返回类型为 `bool (int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-4b98f3f11386c457"></a>

## `xv::process::PendingSignals::PendingSignals`

```doxygen
@fn constexpr xv::process::PendingSignals::PendingSignals()
@brief 构造进程信号状态，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d3c879de37373f0c"></a>

## `xv::process::PendingSignals::PendingSignals`

```doxygen
@fn xv::process::PendingSignals::PendingSignals(const PendingSignals &)
@brief 进程信号状态的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-3f6793e2e84f5c87"></a>

## `xv::process::PendingSignals::operator=`

```doxygen
@fn PendingSignals &xv::process::PendingSignals::operator=(const PendingSignals &)
@brief 进程信号状态的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-d6f433243e974db7"></a>

## `xv::process::PendingSignals::post`

```doxygen
@fn inline bool xv::process::PendingSignals::post(int signal)
@brief 进程信号状态：合并登记一个标准信号。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supported_signal` → `__atomic_load_n` → `__atomic_compare_exchange_n`。
@details 需要特别跟踪的条件：`!supported_signal(signal)`；`signal == SIGCONT`；`signal == SIGSTOP || signal == SIGTSTP || signal == SIGTTIN`；`__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-3cdb3278b53f0228"></a>

## `xv::process::PendingSignals::pending`

```doxygen
@fn inline unsigned xv::process::PendingSignals::pending() const
@brief 进程信号状态：读取或计算 `static_cast<unsigned>(value) & ~static_cast<unsigned>(value >> 32)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`static_cast<unsigned>(value) & ~static_cast<unsigned>(value >> 32)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-62c82a545a28e5c7"></a>

## `xv::process::PendingSignals::disposition`

```doxygen
@fn inline int xv::process::PendingSignals::disposition(int signal, int action)
@brief 进程信号状态：处置。
@param signal 待查询、投递或处理的信号编号。
@param action 本次操作使用的 动作 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`supported_signal` → `__atomic_load_n` → `__atomic_compare_exchange_n` → `__atomic_fetch_and`。
@details 本函数可见的状态更新包括：`actions_[signal] = {};`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!supported_signal(signal) || signal == SIGKILL || signal == SIGSTOP || (action != SIGNAL_DEFAULT && action != SIGNAL_IGNORE)`；`signal == SIGCHLD && action == SIGNAL_DEFAULT`；`__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`-1`；`((old >> 32) & bit) ? SIGNAL_IGNORE : SIGNAL_DEFAULT`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-43e4f50ccbf29835"></a>

## `xv::process::PendingSignals::inherit_dispositions`

```doxygen
@fn inline void xv::process::PendingSignals::inherit_dispositions(const PendingSignals &parent)
@brief 进程信号状态：继承处置与屏蔽字，并清空子进程待处理信号。
@param parent 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `__atomic_store_n` → `set_mask` → `parent.mask` → `parent.handled`。
@details 本函数可见的状态更新包括：`actions_[i] = parent.actions_[i];`。局部临时变量与对象字段应按作用域区分。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-501023c87de27086"></a>

## `xv::process::PendingSignals::mask`

```doxygen
@fn inline unsigned xv::process::PendingSignals::mask() const
@brief 进程信号状态：读取或计算 `__atomic_load_n(&blocked_, __ATOMIC_ACQUIRE)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&blocked_, __ATOMIC_ACQUIRE)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-40d9c697b275333b"></a>

## `xv::process::PendingSignals::handled`

```doxygen
@fn inline unsigned xv::process::PendingSignals::handled() const
@brief 进程信号状态：读取或计算 `__atomic_load_n(&handled_, __ATOMIC_ACQUIRE)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&handled_, __ATOMIC_ACQUIRE)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-889286c91713bbc7"></a>

## `xv::process::PendingSignals::set_mask`

```doxygen
@fn inline void xv::process::PendingSignals::set_mask(unsigned value)
@brief 进程信号状态：更新屏蔽字并去除不可屏蔽信号。
@param value 本次操作使用的 value 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-11a787a9b65e1b18"></a>

## `xv::process::PendingSignals::deliverable`

```doxygen
@fn inline unsigned xv::process::PendingSignals::deliverable() const
@brief 进程信号状态：读取或计算 `pending() & ~mask()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`pending` → `mask`。
@return 各分支实际返回：`pending() & ~mask()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f8da99019c33940b"></a>

## `xv::process::PendingSignals::interrupting`

```doxygen
@fn inline bool xv::process::PendingSignals::interrupting() const
@brief 进程信号状态：判断待处理状态是否应中断当前可中断操作。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`terminating_signal` → `stopping_signal` → `caught_signal`。
@return 各分支实际返回：`terminating_signal() || stopping_signal() || caught_signal()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-fe25ebc3fb856492"></a>

## `xv::process::PendingSignals::caught_signal`

```doxygen
@fn inline int xv::process::PendingSignals::caught_signal() const
@brief 进程信号状态：选择当前可投递的捕获信号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`deliverable` → `handled` → `__builtin_ctz`。
@return 各分支实际返回：`bits ? __builtin_ctz(bits) : 0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-2937be41959f4896"></a>

## `xv::process::PendingSignals::continuing`

```doxygen
@fn inline bool xv::process::PendingSignals::continuing() const
@brief 进程信号状态：读取或计算 `__atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << SIGCONT)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`__atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << SIGCONT)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bc6784b1f197250e"></a>

## `xv::process::PendingSignals::consume`

```doxygen
@fn inline void xv::process::PendingSignals::consume(int signal)
@brief 进程信号状态：消费当前缓冲项或待处理信号。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_fetch_and`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-424641a20d1b2a92"></a>

## `xv::process::PendingSignals::action`

```doxygen
@fn inline Action xv::process::PendingSignals::action(int signal) const
@brief 进程信号状态：读取或计算 `result`，返回当前查询结果。
@param signal 待查询、投递或处理的信号编号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@details 本函数可见的状态更新包括：`result.handler = SIGNAL_IGNORE;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!(__atomic_load_n(&state_, __ATOMIC_ACQUIRE) & (1UL << (signal + 32)))`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`result`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-4da3940f9cfe6e6c"></a>

## `xv::process::PendingSignals::set_action`

```doxygen
@fn inline void xv::process::PendingSignals::set_action(int signal, Action value)
@brief 进程信号状态：更新指定信号的处置和处理函数信息。
@param signal 待查询、投递或处理的信号编号。
@param value 本次操作使用的 value 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`disposition` → `__atomic_fetch_and` → `__atomic_fetch_or`。
@details 本函数可见的状态更新包括：`actions_[signal] = value;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`value.handler <= SIGNAL_IGNORE`；`value.handler > SIGNAL_IGNORE`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9dce8b637d2bf6ad"></a>

## `xv::process::PendingSignals::on_exec`

```doxygen
@fn inline void xv::process::PendingSignals::on_exec()
@brief 进程信号状态：按成功 exec 的规则重置捕获处置。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`handled` → `set_action`。
@details 需要特别跟踪的条件：`handled() & (1U << i)`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5a25b62a6479f685"></a>

## `xv::process::PendingSignals::terminating_signal`

```doxygen
@fn inline int xv::process::PendingSignals::terminating_signal() const
@brief 进程信号状态：选择未屏蔽且采用默认终止动作的信号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`deliverable` → `handled`。
@details 需要特别跟踪的条件：`bits & (1U << SIGKILL)`；`bits & (1U << SIGINT)`；`bits & (1U << SIGTERM)`；`bits & (1U << SIGHUP)`；`bits & (1U << SIGUSR1)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`SIGKILL`；`SIGINT`；`SIGTERM`；`SIGHUP`；`SIGUSR1`；`SIGUSR2`；`SIGSEGV`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-434b21a505b88509"></a>

## `xv::process::PendingSignals::stopping_signal`

```doxygen
@fn inline int xv::process::PendingSignals::stopping_signal() const
@brief 进程信号状态：选择未屏蔽且采用默认停止动作的信号。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`deliverable` → `handled`。
@details 需要特别跟踪的条件：`bits & (1U << SIGSTOP)`；`bits & (1U << SIGTSTP)`；`bits & (1U << SIGTTIN)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`SIGSTOP`；`SIGTSTP`；`SIGTTIN`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-13fda9b2f7f5dd81"></a>

## `xv::process::PendingSignals::consume_continue`

```doxygen
@fn inline void xv::process::PendingSignals::consume_continue()
@brief 进程信号状态：消费继续。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_fetch_and`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-164df2530296d0ae"></a>

## `xv::process::PendingSignals::take_stop`

```doxygen
@fn inline int xv::process::PendingSignals::take_stop()
@brief 进程信号状态：取出停止。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n` → `mask` → `handled` → `__atomic_compare_exchange_n`。
@details 需要特别跟踪的条件：`!signal`；`__atomic_compare_exchange_n(&state_, &old, next, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`0`；`signal`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-15444ef3bbd98d38"></a>

## `xv::process::PendingSignals::reset`

```doxygen
@fn inline void xv::process::PendingSignals::reset()
@brief 进程信号状态：清理当前持有状态并恢复空对象。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_store_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
