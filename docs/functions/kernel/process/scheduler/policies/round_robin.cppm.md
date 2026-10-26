# `kernel/process/scheduler/policies/round_robin.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/round_robin.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:round_robin`。

直接模块依赖：`:scheduler`, `:ready_queue`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::RoundRobinScheduler::RoundRobinScheduler`](round_robin.cppm.md#fn-ae05d5851108cba7)
- [`xv::process::RoundRobinScheduler::make_runnable`](round_robin.cpp.md#fn-ef218500b96b3202)
- [`xv::process::RoundRobinScheduler::name`](round_robin.cppm.md#fn-4381b51eb1479f72)
- [`xv::process::RoundRobinScheduler::run`](round_robin.cpp.md#fn-c56b6a8213328e00)
- [`xv::process::RoundRobinScheduler::suspend`](round_robin.cpp.md#fn-ff847c806572406d)
- [`xv::process::RoundRobinScheduler::yield`](round_robin.cpp.md#fn-1f791894fd7a2945)

<a id="fn-ae05d5851108cba7"></a>

## `xv::process::RoundRobinScheduler::RoundRobinScheduler`

```doxygen
@fn constexpr explicit xv::process::RoundRobinScheduler::RoundRobinScheduler(ReadyQueue &ready)
@brief 构造轮转调度策略，建立本重载要求的成员初值和借用关系。
@param ready 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ready_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4381b51eb1479f72"></a>

## `xv::process::RoundRobinScheduler::name`

```doxygen
@fn const char *xv::process::RoundRobinScheduler::name() const override
@brief 轮转调度策略：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
