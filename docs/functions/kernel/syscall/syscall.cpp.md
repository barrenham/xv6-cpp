# `kernel/syscall/syscall.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/syscall.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:handlers`, `xv.process`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::dispatch`](syscall.cpp.md#fn-b83b0bfa82c48ee9)
- [`xv::syscall::lookup_syscall`](syscall.cpp.md#fn-819d52ac7b54dd49)

<a id="fn-819d52ac7b54dd49"></a>

## `xv::syscall::lookup_syscall`

```doxygen
@fn static SyscallHandler xv::syscall::lookup_syscall(int number)
@brief 用户参数校验、描述符取得及系统调用返回 ABI：把系统调用编号映射到内部处理函数。
@param number 本次操作使用的 编号 值。
@return 各分支实际返回：`sys_fork`；`sys_exit`；`sys_wait`；`sys_waitpid`；`sys_openpty`；`sys_poll`；`sys_fcntl`；`sys_sigaction`；`sys_sigprocmask`；`sys_sigpending`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-b83b0bfa82c48ee9"></a>

## `xv::syscall::dispatch`

```doxygen
@fn void xv::syscall::dispatch()
@brief 用户参数校验、描述符取得及系统调用返回 ABI：按输入事件或系统调用编号选择处理路径。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current` → `p->registers` → `lookup_syscall` → `p->begin_system_call` → `p->finish_system_call` → `handler` → `p->stop_requested` → `xv::print::printk` → `p->id` → `p->debug_name`。
@details 需要特别跟踪的条件：`handler`；`result == ~0UL && restartable && p->stop_requested()`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details LAB_NET 下启用 bind/unbind/send/recv。recv 等待因停止信号返回 -1 时，和其他可重启阻塞调用一样恢复首参数并回退 PC，继续后重新执行。
```
