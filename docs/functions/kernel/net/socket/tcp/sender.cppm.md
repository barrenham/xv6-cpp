# `kernel/net/socket/tcp/sender.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpSocket::PumpFlow::active`](sender.cppm.md#fn-f07f635ccaab0fa0)
- [`xv::net::TcpSocket::PumpFlow::advance`](sender.cppm.md#fn-9a46dbb84fe25aed)
- [`xv::net::TcpSocket::PumpFlow::calculate_credit`](sender.cppm.md#fn-d28c3efe8365037b)
- [`xv::net::TcpSocket::PumpFlow::finish`](sender.cppm.md#fn-2b8f8d35a65be0d2)
- [`xv::net::TcpSocket::PumpFlow::has_credit`](sender.cppm.md#fn-92e5c81f57348036)
- [`xv::net::TcpSocket::PumpFlow::maximum`](sender.cppm.md#fn-3375676d3451c802)
- [`xv::net::TcpSocket::PumpFlow::pending`](sender.cppm.md#fn-20a3461f3adf739f)
- [`xv::net::TcpSocket::PumpFlow::routable`](sender.cppm.md#fn-5db418feb4ba108a)
- [`xv::net::TcpSocket::PumpFlow::send_segment`](sender.cppm.md#fn-4bd91367f456b2f0)
- [`xv::net::TcpSocket::PumpFlow::sent`](sender.cppm.md#fn-973825d1c28bc4b7)
- [`xv::net::TcpSocket::TickFlow::active`](sender.cppm.md#fn-33d03a51cea48a19)
- [`xv::net::TcpSocket::TickFlow::close_idle`](sender.cppm.md#fn-8f4d734d8eb94535)
- [`xv::net::TcpSocket::TickFlow::count_retry`](sender.cppm.md#fn-129448402d56ae26)
- [`xv::net::TcpSocket::TickFlow::destroy`](sender.cppm.md#fn-4960eed140982b6b)
- [`xv::net::TcpSocket::TickFlow::exhausted`](sender.cppm.md#fn-a581545d65795acc)
- [`xv::net::TcpSocket::TickFlow::idle_expired`](sender.cppm.md#fn-f588e13944aa4ab5)
- [`xv::net::TcpSocket::TickFlow::persist`](sender.cppm.md#fn-81691710d2a3406e)
- [`xv::net::TcpSocket::TickFlow::pump`](sender.cppm.md#fn-4bcc0b8d00c979d8)
- [`xv::net::TcpSocket::TickFlow::retransmission_due`](sender.cppm.md#fn-dee978e812dfb8ca)
- [`xv::net::TcpSocket::TickFlow::retransmit`](sender.cppm.md#fn-fb765c4b89e17ac9)
- [`xv::net::TcpSocket::TickFlow::stalled`](sender.cppm.md#fn-2cebe49471e56c9e)
- [`xv::net::TcpSocket::TickFlow::timeout`](sender.cppm.md#fn-acc490fa3aab9d8d)
- [`xv::net::TcpSocket::TickFlow::unowned_closed`](sender.cppm.md#fn-4dbe7ac17c2c80cd)

<a id="fn-f07f635ccaab0fa0"></a>

## `xv::net::TcpSocket::PumpFlow::active`

```doxygen
@fn static inline bool xv::net::TcpSocket::PumpFlow::active(Context &c) noexcept
@brief 检查对象是否处于允许处理的活动状态；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9a46dbb84fe25aed"></a>

## `xv::net::TcpSocket::PumpFlow::advance`

```doxygen
@fn static inline void xv::net::TcpSocket::PumpFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d28c3efe8365037b"></a>

## `xv::net::TcpSocket::PumpFlow::calculate_credit`

```doxygen
@fn static inline void xv::net::TcpSocket::PumpFlow::calculate_credit(Context &c) noexcept
@brief 计算发送在途字节与可用窗口额度；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2b8f8d35a65be0d2"></a>

## `xv::net::TcpSocket::PumpFlow::finish`

```doxygen
@fn static inline void xv::net::TcpSocket::PumpFlow::finish(Context &c) noexcept
@brief 提交 FIN 或完成当前业务阶段；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-92e5c81f57348036"></a>

## `xv::net::TcpSocket::PumpFlow::has_credit`

```doxygen
@fn static inline bool xv::net::TcpSocket::PumpFlow::has_credit(Context &c) noexcept
@brief 检查发送在途量是否小于本次发送额度；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3375676d3451c802"></a>

## `xv::net::TcpSocket::PumpFlow::maximum`

```doxygen
@fn static inline void xv::net::TcpSocket::PumpFlow::maximum(Context &c) noexcept
@brief 取得当前发送可用的 MSS 上限；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-20a3461f3adf739f"></a>

## `xv::net::TcpSocket::PumpFlow::pending`

```doxygen
@fn static inline bool xv::net::TcpSocket::PumpFlow::pending(Context &c) noexcept
@brief 检查发送前沿之前是否还有待发送数据；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5db418feb4ba108a"></a>

## `xv::net::TcpSocket::PumpFlow::routable`

```doxygen
@fn static inline bool xv::net::TcpSocket::PumpFlow::routable(Context &c) noexcept
@brief 检查当前路由是否能提供有效 MSS；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4bd91367f456b2f0"></a>

## `xv::net::TcpSocket::PumpFlow::send_segment`

```doxygen
@fn static inline void xv::net::TcpSocket::PumpFlow::send_segment(Context &c) noexcept
@brief 根据已计算窗口额度提交一个 TCP 数据段；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-973825d1c28bc4b7"></a>

## `xv::net::TcpSocket::PumpFlow::sent`

```doxygen
@fn static inline bool xv::net::TcpSocket::PumpFlow::sent(Context &c) noexcept
@brief 检查当前数据段发送是否成功；所属流程为 xv::net::TcpSocket::PumpFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-33d03a51cea48a19"></a>

## `xv::net::TcpSocket::TickFlow::active`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::active(Context &c) noexcept
@brief 检查对象是否处于允许处理的活动状态；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8f4d734d8eb94535"></a>

## `xv::net::TcpSocket::TickFlow::close_idle`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::close_idle(Context &c) noexcept
@brief 将到期的空闲连接转为关闭并通知；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-129448402d56ae26"></a>

## `xv::net::TcpSocket::TickFlow::count_retry`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::count_retry(Context &c) noexcept
@brief 推进重试次数并记录是否达到上限；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4960eed140982b6b"></a>

## `xv::net::TcpSocket::TickFlow::destroy`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::destroy(Context &c) noexcept
@brief 销毁已经不再被业务持有的对象；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 销毁阶段为相应路径终点，不再使用已销毁对象。
```

<a id="fn-a581545d65795acc"></a>

## `xv::net::TcpSocket::TickFlow::exhausted`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::exhausted(Context &c) noexcept
@brief 检查当前重试次数是否耗尽；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f588e13944aa4ab5"></a>

## `xv::net::TcpSocket::TickFlow::idle_expired`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::idle_expired(Context &c) noexcept
@brief 检查 TIME_WAIT 或 FIN_WAIT_2 是否到期；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-81691710d2a3406e"></a>

## `xv::net::TcpSocket::TickFlow::persist`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::persist(Context &c) noexcept
@brief 按当前时间推进零窗口探测；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4bcc0b8d00c979d8"></a>

## `xv::net::TcpSocket::TickFlow::pump`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::pump(Context &c) noexcept
@brief 按当前状态与窗口推进连接发送；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dee978e812dfb8ca"></a>

## `xv::net::TcpSocket::TickFlow::retransmission_due`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::retransmission_due(Context &c) noexcept
@brief 检查在途数据是否达到重传时间；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fb765c4b89e17ac9"></a>

## `xv::net::TcpSocket::TickFlow::retransmit`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::retransmit(Context &c) noexcept
@brief 重发当前连接未确认的握手、数据或 FIN；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2cebe49471e56c9e"></a>

## `xv::net::TcpSocket::TickFlow::stalled`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::stalled(Context &c) noexcept
@brief 检查活动连接的业务发送是否长期无进展；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-acc490fa3aab9d8d"></a>

## `xv::net::TcpSocket::TickFlow::timeout`

```doxygen
@fn static inline void xv::net::TcpSocket::TickFlow::timeout(Context &c) noexcept
@brief 发送复位并将连接置为 ETIMEDOUT；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4dbe7ac17c2c80cd"></a>

## `xv::net::TcpSocket::TickFlow::unowned_closed`

```doxygen
@fn static inline bool xv::net::TcpSocket::TickFlow::unowned_closed(Context &c) noexcept
@brief 检查关闭连接是否已没有应用所有者；所属流程为 xv::net::TcpSocket::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
