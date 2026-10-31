# `kernel/syscall/syscall.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/syscall.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:handlers`, `xv.process`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::DispatchFlow::invoke`](syscall.cpp.md#fn-309928c400ec4434)
- [`xv::syscall::DispatchFlow::known`](syscall.cpp.md#fn-afa627d44151bd89)
- [`xv::syscall::DispatchFlow::publish`](syscall.cpp.md#fn-d6698aa653d2d7f9)
- [`xv::syscall::DispatchFlow::restart`](syscall.cpp.md#fn-6fe0c1e68f7372b3)
- [`xv::syscall::DispatchFlow::rewind`](syscall.cpp.md#fn-b298ae538d36fc24)
- [`xv::syscall::DispatchFlow::unknown`](syscall.cpp.md#fn-ad4cc43da14969d6)
- [`xv::syscall::dispatch`](syscall.cpp.md#fn-b83b0bfa82c48ee9)
- [`xv::syscall::lookup_syscall`](syscall.cpp.md#fn-819d52ac7b54dd49)

<a id="fn-819d52ac7b54dd49"></a>

## `xv::syscall::lookup_syscall`

```doxygen
@fn static SyscallHandler xv::syscall::lookup_syscall(int number)
@brief 把共享 ABI 调用号映射到处理函数。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-b83b0bfa82c48ee9"></a>

## `xv::syscall::dispatch`

```doxygen
@fn void xv::syscall::dispatch()
@brief 用户参数校验、描述符取得及系统调用返回 ABI：按输入事件或系统调用编号选择处理路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details LAB_NET 下启用 bind/unbind/send/recv。recv 等待因停止信号返回 -1 时，和其他可重启阻塞调用一样恢复首参数并回退 PC，继续后重新执行。
@details 因停止信号中断的 accept/recvfrom/sendto 与 read/write/wait/waitpid 一样回退系统调用 PC。Socket 的 read/write 沿用原有分派、部分进度与信号处理。
@details 流程由 DispatchFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-309928c400ec4434"></a>

## `xv::syscall::DispatchFlow::invoke`

```doxygen
@fn static void xv::syscall::DispatchFlow::invoke(Context &c) noexcept
@brief 保存原参数并按 begin、一次 handler、finish 顺序执行系统调用。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-afa627d44151bd89"></a>

## `xv::syscall::DispatchFlow::known`

```doxygen
@fn static bool xv::syscall::DispatchFlow::known(Context &c) noexcept { return c.handler != nullptr; }
@brief 检查固定 ABI 查询是否找到处理器。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d6698aa653d2d7f9"></a>

## `xv::syscall::DispatchFlow::publish`

```doxygen
@fn static void xv::syscall::DispatchFlow::publish(Context &c) noexcept { c.process->registers()->a0 = c.result; }
@brief 发布经过 finish 处理的系统调用结果。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6fe0c1e68f7372b3"></a>

## `xv::syscall::DispatchFlow::restart`

```doxygen
@fn static bool xv::syscall::DispatchFlow::restart(Context &c) noexcept
@brief 检查完整失败、可重启 ABI 和原停止请求条件。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b298ae538d36fc24"></a>

## `xv::syscall::DispatchFlow::rewind`

```doxygen
@fn static void xv::syscall::DispatchFlow::rewind(Context &c) noexcept
@brief 回退系统调用指令并恢复原 a0 参数。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ad4cc43da14969d6"></a>

## `xv::syscall::DispatchFlow::unknown`

```doxygen
@fn static void xv::syscall::DispatchFlow::unknown(Context &c) noexcept
@brief 保留未知调用诊断并发布负一结果。
@details 保存 a0 后 begin，只调用一次 handler 并交给 finish；仅完整失败、可重启 ABI 且请求停止时回退 epc 并恢复原参数，部分传输不能重放。 下一节点由图策略声明，业务结果与控制信号分离。
```
