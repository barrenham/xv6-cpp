# `kernel/process/signals/process_signals.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/signals/process_signals.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

## 函数导航

- [`xv::process::Process::is_killed`](process_signals.cpp.md#fn-f4265352809f85a0)
- [`xv::process::Process::request_kill`](process_signals.cpp.md#fn-d56b75df7731853a)
- [`xv::process::Process::termination_status`](process_signals.cpp.md#fn-5e63eeb4721e2fa0)

<a id="fn-f4265352809f85a0"></a>

## `xv::process::Process::is_killed`

```doxygen
@fn bool xv::process::Process::is_killed() const
@brief 进程描述符：读取或计算 `killed != 0 || pending_signals_.interrupting()`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `pending_signals_.caught_signal` → `pending_signals_.interrupting`。
@details 本函数可见的状态更新包括：`syscall_interrupted_ = true;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`pending_signals_.caught_signal()`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`killed != 0 || pending_signals_.interrupting()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d56b75df7731853a"></a>

## `xv::process::Process::request_kill`

```doxygen
@fn void xv::process::Process::request_kill()
@brief 进程描述符：请求终止请求。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5e63eeb4721e2fa0"></a>

## `xv::process::Process::termination_status`

```doxygen
@fn int xv::process::Process::termination_status() const
@brief 进程描述符：读取或计算 `signal ? SIGNAL_EXIT_STATUS(signal) : (killed ? -1 : 0)`，返回当前查询结果。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `pending_signals_.terminating_signal` → `SIGNAL_EXIT_STATUS`。
@return 各分支实际返回：`signal ? SIGNAL_EXIT_STATUS(signal) : (killed ? -1 : 0)`。应结合上面的条件检查区分正常值、空值和错误值。
```
