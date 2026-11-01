# `kernel/support/deadline_queue.h`


<a id="fn-14c2f7cc3dd55d4d"></a>

## `xv::time::Earlier::operator()`

```doxygen
@fn bool xv::time::Earlier::operator()(const Deadline &a, const Deadline &b) const
@brief 按截止时间和节点地址确定全序。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```

- [`xv::time::DeadlineQueue::cancel`](deadline_queue.h.md#fn-74f5c0466039541b)

- [`xv::time::DeadlineQueue::empty`](deadline_queue.h.md#fn-19d4a9600de079af)

- [`xv::time::DeadlineQueue::expire`](deadline_queue.h.md#fn-fe52b600db5c1c90)

- [`xv::time::DeadlineQueue::next`](deadline_queue.h.md#fn-6589a3fb75875e5e)

- [`xv::time::DeadlineQueue::reset`](deadline_queue.h.md#fn-ba6c468d266a666e)

- [`xv::time::Earlier::operator()`](deadline_queue.h.md#fn-14c2f7cc3dd55d4d)

<a id="fn-74f5c0466039541b"></a>

## `xv::time::DeadlineQueue::cancel`

```doxygen
@fn void xv::time::DeadlineQueue::cancel(Deadline &node)
@brief 幂等撤销仍在树中的截止时间节点。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```

<a id="fn-19d4a9600de079af"></a>

## `xv::time::DeadlineQueue::empty`

```doxygen
@fn bool xv::time::DeadlineQueue::empty() const
@brief 只读树根检查是否无节点，不访问节点所在栈。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```

<a id="fn-fe52b600db5c1c90"></a>

## `xv::time::DeadlineQueue::expire`

```doxygen
@fn unsigned xv::time::DeadlineQueue::expire(unsigned long now, unsigned budget = 64)
@brief 按截止时间顺序摘除到期节点并投递至多 budget 次通知。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```

<a id="fn-6589a3fb75875e5e"></a>

## `xv::time::DeadlineQueue::next`

```doxygen
@fn unsigned long xv::time::DeadlineQueue::next() const
@brief 读取最早登记的绝对毫秒时间，空队列返回 ~0UL。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```

<a id="fn-ba6c468d266a666e"></a>

## `xv::time::DeadlineQueue::reset`

```doxygen
@fn void xv::time::DeadlineQueue::reset(Deadline &node, unsigned long at, void (*notify)(void *), void *owner)
@brief 撤销旧登记并按新截止时间和通知所有者重新插入。
@details 侵入式截止时间红黑树，不分配节点；调用者提供互斥并保持节点地址稳定。reset 撤销旧登记，cancel 幂等，expire 先摘除再通知且最多处理预算数；通知只允许有界唤醒，不得睡眠、分配或重入队列。
```
