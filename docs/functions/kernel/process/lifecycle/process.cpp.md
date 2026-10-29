# `kernel/process/lifecycle/process.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/process.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.runtime`, `:manager`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::CommitFlow::image`](process.cpp.md#fn-54586d7458864c15)
- [`xv::process::Process::CommitFlow::linearize`](process.cpp.md#fn-44a4dcdfcc8dc701)
- [`xv::process::Process::CommitFlow::name`](process.cpp.md#fn-2435016428280ef8)
- [`xv::process::Process::CommitFlow::registers`](process.cpp.md#fn-332cf04c7cd9c846)
- [`xv::process::Process::CommitFlow::signals`](process.cpp.md#fn-c800fe6114b3568a)
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
@fn void xv::process::Process::commit_image(AddressSpace &image, unsigned long entry, unsigned long stack, unsigned long arguments, const char *path, unsigned long environment)
@brief 提交候选地址空间与用户入口寄存器。
@details 交换新旧地址空间，保存入口、栈、argv 到 a1 与 envp 到 a2，再销毁旧映像；argc 由系统调用返回路径写 a0。调用者已完成全部可失败操作。
@details 流程由 CommitFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-54586d7458864c15"></a>

## `xv::process::Process::CommitFlow::image`

```doxygen
@fn static void xv::process::Process::CommitFlow::image(Context &c) noexcept
@brief 交换候选映像与当前地址空间。
@details 加载已全部成功；先线性化 exec 与父进程 setpgid，再交换映像，重置信号执行态，发布用户入口和参数，最后设置进程名。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-44a4dcdfcc8dc701"></a>

## `xv::process::Process::CommitFlow::linearize`

```doxygen
@fn static void xv::process::Process::CommitFlow::linearize(Context &c) noexcept
@brief 通过进程管理器登记 exec 与 setpgid 的线性化边界。
@details 加载已全部成功；先线性化 exec 与父进程 setpgid，再交换映像，重置信号执行态，发布用户入口和参数，最后设置进程名。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2435016428280ef8"></a>

## `xv::process::Process::CommitFlow::name`

```doxygen
@fn static void xv::process::Process::CommitFlow::name(Context &c) noexcept
@brief 直接提取路径最后分量并按原加锁方法设置进程名。
@details 加载已全部成功；先线性化 exec 与父进程 setpgid，再交换映像，重置信号执行态，发布用户入口和参数，最后设置进程名。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-332cf04c7cd9c846"></a>

## `xv::process::Process::CommitFlow::registers`

```doxygen
@fn static void xv::process::Process::CommitFlow::registers(Context &c) noexcept
@brief 发布新入口、用户栈、参数和环境指针寄存器。
@details 加载已全部成功；先线性化 exec 与父进程 setpgid，再交换映像，重置信号执行态，发布用户入口和参数，最后设置进程名。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c800fe6114b3568a"></a>

## `xv::process::Process::CommitFlow::signals`

```doxygen
@fn static void xv::process::Process::CommitFlow::signals(Context &c) noexcept
@brief 应用 exec 信号规则并清除旧信号帧与挂起掩码状态。
@details 加载已全部成功；先线性化 exec 与父进程 setpgid，再交换映像，重置信号执行态，发布用户入口和参数，最后设置进程名。 下一节点由图策略声明，业务结果与控制信号分离。
```
