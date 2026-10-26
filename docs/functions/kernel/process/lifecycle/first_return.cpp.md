# `kernel/process/lifecycle/first_return.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/first_return.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.fs`, `xv.trap`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::first_return`](first_return.cpp.md#fn-f06f00214daff1d9)

<a id="fn-f06f00214daff1d9"></a>

## `xv::process::first_return`

```doxygen
@fn void xv::process::first_return()
@brief 进程对象生命周期、组身份、调度、等待与信号：首项返回。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`current` → `p->lock.release` → `__atomic_load_n` → `xv::fs::filesystem` → `initialize` → `__atomic_store_n` → `execute` → `xv::print::panic` → `xv::trap::return_to_user`。
@details 本函数可见的状态更新包括：`p->trapframe->a0 = execute("/init", arguments);`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`__atomic_load_n(&first, __ATOMIC_ACQUIRE)`；`p->trapframe->a0 == -1`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`p->lock.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`_to_user()`。应结合上面的条件检查区分正常值、空值和错误值。
```
