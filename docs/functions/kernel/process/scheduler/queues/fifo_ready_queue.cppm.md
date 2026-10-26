# `kernel/process/scheduler/queues/fifo_ready_queue.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/queues/fifo_ready_queue.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:fifo_queue`。

直接模块依赖：`:ready_queue`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::FifoReadyQueue::FifoReadyQueue`](fifo_ready_queue.cppm.md#fn-06a2613ab71eea5f)
- [`xv::process::FifoReadyQueue::enqueue`](fifo_ready_queue.cpp.md#fn-67d6693a20eb9668)
- [`xv::process::FifoReadyQueue::initialize`](fifo_ready_queue.cpp.md#fn-b9d7ddcb94f3159c)
- [`xv::process::FifoReadyQueue::name`](fifo_ready_queue.cppm.md#fn-9de422aa5a554e68)
- [`xv::process::FifoReadyQueue::take`](fifo_ready_queue.cpp.md#fn-d75382703f1177bc)

<a id="fn-06a2613ab71eea5f"></a>

## `xv::process::FifoReadyQueue::FifoReadyQueue`

```doxygen
@fn constexpr xv::process::FifoReadyQueue::FifoReadyQueue()
@brief 构造FIFO 就绪队列，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9de422aa5a554e68"></a>

## `xv::process::FifoReadyQueue::name`

```doxygen
@fn const char *xv::process::FifoReadyQueue::name() const override
@brief FIFO 就绪队列：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
