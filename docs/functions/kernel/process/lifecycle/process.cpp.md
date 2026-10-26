# `kernel/process/lifecycle/process.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/process.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.runtime`, `:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::commit_image`](process.cpp.md#fn-97213cc0f22d399e)
- [`xv::process::Process::create_image`](process.cpp.md#fn-9d91050686d2abd0)
- [`xv::process::Process::set_name`](process.cpp.md#fn-7970dbfee688fb06)

<a id="fn-7970dbfee688fb06"></a>

## `xv::process::Process::set_name`

```doxygen
@fn void xv::process::Process::set_name(const char *label)
@brief 在进程锁内更新调试名称。
@details exec 发布名称时与进程快照读取互斥，复制结果以零终止。调用者不能已持有本进程锁；当前调用来自成功 exec 的拥有线程。
```

<a id="fn-9d91050686d2abd0"></a>

## `xv::process::Process::create_image`

```doxygen
@fn bool xv::process::Process::create_image(AddressSpace &image)
@brief 进程描述符：建立引用当前 trapframe 和用户信息页的空页表。
@param image 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`image.create`。
@return 各分支实际返回：`image.create(trapframe, usyscall)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-97213cc0f22d399e"></a>

## `xv::process::Process::commit_image`

```doxygen
@fn void xv::process::Process::commit_image(AddressSpace &image, unsigned long entry, unsigned long stack, unsigned long arguments, const char *path)
@brief 进程描述符：提交已经完成全部可失败步骤的新用户映像。
@param image 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param entry 需要链接或解析的条目。
@param stack 本次操作使用的 栈 值。
@param arguments 本次操作使用的 arguments 值。
@param path 待解析的零结尾路径字符串。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_manager.did_exec` → `memory_.swap` → `pending_signals_.on_exec` → `set_name`。
@details 本函数可见的状态更新包括：`signal_depth_ = 0;`；`suspended_mask_ = suspend_restarted_ = false;`；`suspend_restarted_ = false;`；`trapframe->epc = entry;`；`trapframe->sp = stack;`；`trapframe->a1 = arguments;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`*c == '/'`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
