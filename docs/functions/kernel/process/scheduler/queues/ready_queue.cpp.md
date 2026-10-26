# `kernel/process/scheduler/queues/ready_queue.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/queues/ready_queue.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:ready_queue`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::ReadyQueue::eligible`](ready_queue.cpp.md#fn-68b75aa7cc18acd6)
- [`xv::process::ReadyQueue::next`](ready_queue.cpp.md#fn-67e0dc0257fbe5a8)
- [`xv::process::ReadyQueue::process_locked`](ready_queue.cpp.md#fn-70010f7d4b60b9e1)
- [`xv::process::ReadyQueue::queued`](ready_queue.cpp.md#fn-493eb25f1b781417)

<a id="fn-70010f7d4b60b9e1"></a>

## `xv::process::ReadyQueue::process_locked`

```doxygen
@fn bool xv::process::ReadyQueue::process_locked(const Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process.lock.holding()`，返回当前查询结果。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process.lock.holding`。
@return 各分支实际返回：`process.lock.holding()`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-68b75aa7cc18acd6"></a>

## `xv::process::ReadyQueue::eligible`

```doxygen
@fn bool xv::process::ReadyQueue::eligible(const Process &process, const CpuState &cpu)
@brief 进程对象生命周期、组身份、调度、等待与信号：判断进程的 CPU 亲和性是否允许当前 hart 运行。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param cpu 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`__atomic_load_n`。
@note 此实现包含原子操作或内联汇编；其内存序、特权级与屏障语义属于接口的一部分，不能用普通字段读写替换。
@return 各分支实际返回：`!affinity || affinity == &cpu`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-67e0dc0257fbe5a8"></a>

## `xv::process::ReadyQueue::next`

```doxygen
@fn Process *&xv::process::ReadyQueue::next(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process.ready_next_`，返回当前查询结果。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@return 各分支实际返回：`process.ready_next_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-493eb25f1b781417"></a>

## `xv::process::ReadyQueue::queued`

```doxygen
@fn bool &xv::process::ReadyQueue::queued(Process &process)
@brief 进程对象生命周期、组身份、调度、等待与信号：读取或计算 `process.queued_`，返回当前查询结果。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@return 各分支实际返回：`process.queued_`。应结合上面的条件检查区分正常值、空值和错误值。
```
