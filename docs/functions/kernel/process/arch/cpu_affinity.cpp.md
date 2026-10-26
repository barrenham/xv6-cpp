# `kernel/process/arch/cpu_affinity.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/arch/cpu_affinity.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:cpu`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::Process::pin_cpu`](cpu_affinity.cpp.md#fn-7cb6f1f52058eb7d)

<a id="fn-7cb6f1f52058eb7d"></a>

## `xv::process::Process::pin_cpu`

```doxygen
@fn int xv::process::Process::pin_cpu(int index)
@brief 进程描述符：设置进程可运行的目标 hart。
@param index 数组、CPU 或页面的索引。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `__atomic_store_n` → `cpu_at`。
@details 需要特别跟踪的条件：`index < 0 || index >= NCPU`。这些条件决定检查、重试或提前返回路径。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```
