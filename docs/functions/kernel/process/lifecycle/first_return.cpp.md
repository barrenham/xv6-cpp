# `kernel/process/lifecycle/first_return.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/first_return.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.fs`, `xv.trap`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::FirstReturnFlow::execute_init`](first_return.cpp.md#fn-130a681a8486da10)
- [`xv::process::Process::FirstReturnFlow::fail`](first_return.cpp.md#fn-cdd848d8b9145137)
- [`xv::process::Process::FirstReturnFlow::first`](first_return.cpp.md#fn-361c66ca104b12e6)
- [`xv::process::Process::FirstReturnFlow::initialize`](first_return.cpp.md#fn-c35fa07ba0b59592)
- [`xv::process::Process::FirstReturnFlow::loaded`](first_return.cpp.md#fn-7fe729c767e95db2)
- [`xv::process::Process::FirstReturnFlow::publish`](first_return.cpp.md#fn-3898a8ca7f09eb2a)
- [`xv::process::Process::FirstReturnFlow::unlock`](first_return.cpp.md#fn-979da37c1d0a05e0)
- [`xv::process::Process::FirstReturnFlow::user`](first_return.cpp.md#fn-058b79f50941ba2f)
- [`xv::process::first_return`](first_return.cpp.md#fn-f06f00214daff1d9)

<a id="fn-f06f00214daff1d9"></a>

## `xv::process::first_return`

```doxygen
@fn void xv::process::first_return()
@brief 在进程首次调度时准备文件系统并进入用户态。
@pre 调度器持有当前进程锁，本函数先释放该锁，再进行可能睡眠的文件系统初始化。
@details 初始进程恢复并挂载文件系统，然后从 XV_INIT_PATH 指定的 /sbin/init 装载首个用户程序。参数首项使用相同路径，装载结果写入 trapframe.a0；失败触发 panic。其他进程跳过首轮初始化并按自己的已准备上下文返回用户态。
@note first 标记用 acquire/release 发布初始化完成状态，确保其他核心观察到已就绪的系统服务。
@return 经 return_to_user 进入用户态，不普通返回到调度调用点。
@details 流程由 FirstReturnFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-130a681a8486da10"></a>

## `xv::process::Process::FirstReturnFlow::execute_init`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::execute_init(Context &c) noexcept
@brief 按原初始化参数执行 init 并保存 argc 返回寄存器。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-cdd848d8b9145137"></a>

## `xv::process::Process::FirstReturnFlow::fail`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::fail(Context &) noexcept
@brief 按原 exec 失败诊断终止。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-361c66ca104b12e6"></a>

## `xv::process::Process::FirstReturnFlow::first`

```doxygen
@fn static bool xv::process::Process::FirstReturnFlow::first(Context &c) noexcept
@brief 以 acquire 检查是否仍需首次文件系统初始化。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c35fa07ba0b59592"></a>

## `xv::process::Process::FirstReturnFlow::initialize`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::initialize(Context &) noexcept
@brief 在普通进程上下文初始化根文件系统。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7fe729c767e95db2"></a>

## `xv::process::Process::FirstReturnFlow::loaded`

```doxygen
@fn static bool xv::process::Process::FirstReturnFlow::loaded(Context &c) noexcept
@brief 检查 init 是否执行成功。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3898a8ca7f09eb2a"></a>

## `xv::process::Process::FirstReturnFlow::publish`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::publish(Context &c) noexcept
@brief 以 release 发布首次初始化完成。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-979da37c1d0a05e0"></a>

## `xv::process::Process::FirstReturnFlow::unlock`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::unlock(Context &c) noexcept
@brief 释放调度器交接的当前进程锁。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-058b79f50941ba2f"></a>

## `xv::process::Process::FirstReturnFlow::user`

```doxygen
@fn static void xv::process::Process::FirstReturnFlow::user(Context &) noexcept
@brief 调用原陷阱返回路径进入用户态。
@details swtch 首次进入时持进程锁，先释放；首次文件系统初始化必须在可睡眠进程上下文执行，release 发布完成后才执行 init，最后进入用户态。 下一节点由图策略声明，业务结果与控制信号分离。
```
