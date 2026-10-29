# `kernel/process/scheduler/policies/scheduling_policy.h`

源码：[打开实际文件](../../../../../../kernel/process/scheduler/policies/scheduling_policy.h)。

接口、生命周期与验证见 [设计说明](../../../../../scheduling.md)。

## 函数导航

- [`xv::scheduling::RunQueue::Earlier::operator()`](scheduling_policy.h.md#fn-0adc23d8f52d4090)
- [`xv::scheduling::RunQueue::RunQueue`](scheduling_policy.h.md#fn-91ebb1c614e95878)
- [`xv::scheduling::RunQueue::RunQueue`](scheduling_policy.h.md#fn-93875b1406a5b311)
- [`xv::scheduling::RunQueue::add`](scheduling_policy.h.md#fn-8403eb9ef360a447)
- [`xv::scheduling::RunQueue::boost`](scheduling_policy.h.md#fn-cf64396a6b83d1ca)
- [`xv::scheduling::RunQueue::enqueue`](scheduling_policy.h.md#fn-929445650da076f3)
- [`xv::scheduling::RunQueue::kind`](scheduling_policy.h.md#fn-7a397223ac4d28de)
- [`xv::scheduling::RunQueue::minimum_runtime`](scheduling_policy.h.md#fn-0710796e74fc6451)
- [`xv::scheduling::RunQueue::operator=`](scheduling_policy.h.md#fn-373faab75888c926)
- [`xv::scheduling::RunQueue::quantum`](scheduling_policy.h.md#fn-71fa5470bb8bb789)
- [`xv::scheduling::RunQueue::refresh`](scheduling_policy.h.md#fn-a662593e0026eadd)
- [`xv::scheduling::RunQueue::scaled`](scheduling_policy.h.md#fn-4bfff853915d0675)
- [`xv::scheduling::RunQueue::size`](scheduling_policy.h.md#fn-d17b6c73011deab9)
- [`xv::scheduling::RunQueue::start`](scheduling_policy.h.md#fn-f448b6a829faddcf)
- [`xv::scheduling::RunQueue::stop`](scheduling_policy.h.md#fn-34b2aa91c49e8900)
- [`xv::scheduling::RunQueue::take`](scheduling_policy.h.md#fn-fb695d1787880b09)
- [`xv::scheduling::RunQueue::tick`](scheduling_policy.h.md#fn-09e5e21424c77e3b)
- [`xv::scheduling::RunQueue::update_floor`](scheduling_policy.h.md#fn-b7106efdb8ac656a)
- [`xv::scheduling::RunQueue::valid`](scheduling_policy.h.md#fn-785ffe01ed19a053)

<a id="fn-93875b1406a5b311"></a>

## `xv::scheduling::RunQueue::RunQueue`

```doxygen
@fn constexpr explicit xv::scheduling::RunQueue::RunQueue(Kind kind, Time quantum = DefaultQuantum)
@brief 构造所选策略和基础时间单位的空调度队列。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-91ebb1c614e95878"></a>

## `xv::scheduling::RunQueue::RunQueue`

```doxygen
@fn xv::scheduling::RunQueue::RunQueue(const RunQueue &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-373faab75888c926"></a>

## `xv::scheduling::RunQueue::operator=`

```doxygen
@fn RunQueue &xv::scheduling::RunQueue::operator=(const RunQueue &)
@brief 编译期禁止复制或赋值，防止共享侵入式链接与可变容器状态。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-785ffe01ed19a053"></a>

## `xv::scheduling::RunQueue::valid`

```doxygen
@fn bool xv::scheduling::RunQueue::valid() const
@brief 验证策略枚举和不会使预算乘法溢出的正基础时间单位。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-d17b6c73011deab9"></a>

## `xv::scheduling::RunQueue::size`

```doxygen
@fn unsigned long xv::scheduling::RunQueue::size() const
@brief 返回排队实体数，不包含运行中的实体。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-7a397223ac4d28de"></a>

## `xv::scheduling::RunQueue::kind`

```doxygen
@fn Kind xv::scheduling::RunQueue::kind() const
@brief 返回构造时选定的调度策略。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-0710796e74fc6451"></a>

## `xv::scheduling::RunQueue::minimum_runtime`

```doxygen
@fn Time xv::scheduling::RunQueue::minimum_runtime() const
@brief 返回公平策略单调的最小虚拟运行时间。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-929445650da076f3"></a>

## `xv::scheduling::RunQueue::enqueue`

```doxygen
@fn bool xv::scheduling::RunQueue::enqueue(Entity &entity, Time now)
@brief 先记账仍在运行的实体，再按公平树或 MLFQ/FIFO 入队；新激活实体按最小虚拟时间放置，重复入队失败。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-fb695d1787880b09"></a>

## `xv::scheduling::RunQueue::take`

```doxygen
@fn Entity *xv::scheduling::RunQueue::take(Time now, Eligible eligible)
@brief 按策略顺序选择满足 CPU 资格的实体，移除其队列身份；不取得进程锁。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-f448b6a829faddcf"></a>

## `xv::scheduling::RunQueue::start`

```doxygen
@fn bool xv::scheduling::RunQueue::start(Entity &entity, Time now)
@brief 将已出队实体登记到运行集合，设置实际运行/额度起点和策略片段预算；拒绝重复运行。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-34b2aa91c49e8900"></a>

## `xv::scheduling::RunQueue::stop`

```doxygen
@fn void xv::scheduling::RunQueue::stop(Entity &entity, Time now)
@brief 累计实际与加权虚拟时间及分数余量，退出运行集合；MLFQ 跨主动让出累计额度并降级。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-09e5e21424c77e3b"></a>

## `xv::scheduling::RunQueue::tick`

```doxygen
@fn bool xv::scheduling::RunQueue::tick(Entity &entity, Time now)
@brief 根据等待工作、片段或 MLFQ 额度判断抢占；轮转保留无积压时的时钟让出。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-0adc23d8f52d4090"></a>

## `xv::scheduling::RunQueue::Earlier::operator()`

```doxygen
@fn bool xv::scheduling::RunQueue::Earlier::operator()(const Entity &a, const Entity &b) const
@brief 按虚拟运行时间、入队序号和稳定节点地址构造严格顺序。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-8403eb9ef360a447"></a>

## `xv::scheduling::RunQueue::add`

```doxygen
@fn static Time xv::scheduling::RunQueue::add(Time a, Time b)
@brief 使用饱和加法累计时间，避免回绕破坏排序。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-4bfff853915d0675"></a>

## `xv::scheduling::RunQueue::scaled`

```doxygen
@fn static Time xv::scheduling::RunQueue::scaled(Time elapsed, unsigned weight, Time fractional)
@brief 按权重计算虚拟运行时间增量，携带上次分数余量并防止乘法溢出。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-71fa5470bb8bb789"></a>

## `xv::scheduling::RunQueue::quantum`

```doxygen
@fn Time xv::scheduling::RunQueue::quantum(unsigned level) const
@brief 按策略和 MLFQ 层级返回基础单位的 1/2/4 倍预算。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-a662593e0026eadd"></a>

## `xv::scheduling::RunQueue::refresh`

```doxygen
@fn void xv::scheduling::RunQueue::refresh(Entity &entity, Time now)
@brief 观察新的 MLFQ 提升代次，重置层级和额度，运行实体同时更新额度起点。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-cf64396a6b83d1ca"></a>

## `xv::scheduling::RunQueue::boost`

```doxygen
@fn void xv::scheduling::RunQueue::boost(Time now)
@brief 每 20 个基础单位提升就绪和运行实体；睡眠实体在下次激活时观察代次。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```

<a id="fn-b7106efdb8ac656a"></a>

## `xv::scheduling::RunQueue::update_floor`

```doxygen
@fn void xv::scheduling::RunQueue::update_floor(Time now, const Entity *pending = nullptr)
@brief 取就绪树、各 hart 实际运行投影及待重新入队实体的最小值，单调推进公平基准。
@details 算法只修改调用方提供的调度实体，队列操作需外部排他；不依赖 Process、寄存器或分配器。
```
