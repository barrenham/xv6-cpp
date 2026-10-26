# `kernel/sync/platform/kernel_platform.cpp`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/platform/kernel_platform.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync`。

直接模块依赖：`xv.arch`, `xv.print`, `xv.process`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::pop_interrupts_off`](kernel_platform.cpp.md#fn-225d9917867056a8)
- [`xv::sync::push_interrupts_off`](kernel_platform.cpp.md#fn-b33b7a25f520fd67)

<a id="fn-b33b7a25f520fd67"></a>

## `xv::sync::push_interrupts_off`

```doxygen
@fn void xv::sync::push_interrupts_off()
@brief 锁所有权、屏蔽中断与对象等待登记：压入interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::arch::rc_sstatus` → `xv::process::current_cpu`。
@details 需要特别跟踪的条件：`xv::process::current_cpu()->noff == 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-225d9917867056a8"></a>

## `xv::sync::pop_interrupts_off`

```doxygen
@fn void xv::sync::pop_interrupts_off()
@brief 锁所有权、屏蔽中断与对象等待登记：弹出interrupts关闭。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`xv::process::current_cpu` → `xv::arch::intr_get` → `xv::print::panic` → `xv::arch::intr_on`。
@details 本函数可见的状态更新包括：`c->noff -= 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`xv::arch::intr_get()`；`c->noff < 1`；`c->noff == 0 && c->intena`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
