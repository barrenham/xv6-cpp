# `kernel/net/socket/tcp/reliability.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpSocket::AcknowledgeFlow::advance`](reliability.cppm.md#fn-a12221894534ca13)
- [`xv::net::TcpSocket::AcknowledgeFlow::clock`](reliability.cppm.md#fn-ac1fc7b8f46f8d06)
- [`xv::net::TcpSocket::AcknowledgeFlow::duplicate_ack`](reliability.cppm.md#fn-c665c4cb6d577034)
- [`xv::net::TcpSocket::AcknowledgeFlow::fin_ack`](reliability.cppm.md#fn-22c698f63f58d05e)
- [`xv::net::TcpSocket::AcknowledgeFlow::finish`](reliability.cppm.md#fn-78823400ae805c8c)
- [`xv::net::TcpSocket::AcknowledgeFlow::handle_duplicate`](reliability.cppm.md#fn-ac9c7c05f0fe7951)
- [`xv::net::TcpSocket::AcknowledgeFlow::notify`](reliability.cppm.md#fn-4339e9a7ad4faffe)
- [`xv::net::TcpSocket::AcknowledgeFlow::partial_recovery`](reliability.cppm.md#fn-6e1ad8d4694b4937)
- [`xv::net::TcpSocket::AcknowledgeFlow::retransmit`](reliability.cppm.md#fn-459056c869b645c6)
- [`xv::net::TcpSocket::AcknowledgeFlow::valid`](reliability.cppm.md#fn-dccfa508d1abc120)
- [`xv::net::TcpSocket::FinishFlow::commit`](reliability.cppm.md#fn-05bb54ce41faec96)
- [`xv::net::TcpSocket::FinishFlow::credit`](reliability.cppm.md#fn-cb2a9988527a06ed)
- [`xv::net::TcpSocket::FinishFlow::eligible`](reliability.cppm.md#fn-8c657f360fd65398)
- [`xv::net::TcpSocket::FinishFlow::measure`](reliability.cppm.md#fn-60f00bc8eff76c0d)
- [`xv::net::TcpSocket::FinishFlow::send`](reliability.cppm.md#fn-5feb55b2074741bd)
- [`xv::net::TcpSocket::PersistFlow::data`](reliability.cppm.md#fn-668678409923c737)
- [`xv::net::TcpSocket::PersistFlow::due`](reliability.cppm.md#fn-174e07ab7b5cdf70)
- [`xv::net::TcpSocket::PersistFlow::eligible`](reliability.cppm.md#fn-9e5d89e660bc4b0b)
- [`xv::net::TcpSocket::PersistFlow::probe_ack`](reliability.cppm.md#fn-df2f019ed3539a55)
- [`xv::net::TcpSocket::PersistFlow::probe_data`](reliability.cppm.md#fn-8424cc447204af56)
- [`xv::net::TcpSocket::PersistFlow::reset`](reliability.cppm.md#fn-171279803e63badd)
- [`xv::net::TcpSocket::PersistFlow::schedule`](reliability.cppm.md#fn-b5674ab3c8f320a2)
- [`xv::net::TcpSocket::RetransmitFlow::available`](reliability.cppm.md#fn-0114b24f55a7752b)
- [`xv::net::TcpSocket::RetransmitFlow::data`](reliability.cppm.md#fn-c610c50f5ba5937e)
- [`xv::net::TcpSocket::RetransmitFlow::fin`](reliability.cppm.md#fn-390d562c118aeb0e)
- [`xv::net::TcpSocket::RetransmitFlow::measure`](reliability.cppm.md#fn-efb8b22d2fd73776)
- [`xv::net::TcpSocket::RetransmitFlow::send_data`](reliability.cppm.md#fn-4082d200bf7b03f6)
- [`xv::net::TcpSocket::RetransmitFlow::send_fin`](reliability.cppm.md#fn-9c8927c357ded64b)
- [`xv::net::TcpSocket::RetransmitFlow::send_syn`](reliability.cppm.md#fn-3a8ba6d71f2cf68e)
- [`xv::net::TcpSocket::RetransmitFlow::syn`](reliability.cppm.md#fn-0cd6e873189cb721)
- [`xv::net::TcpSocket::RetransmitFlow::uncertain`](reliability.cppm.md#fn-e692706eba52af43)

<a id="fn-a12221894534ca13"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::advance`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ac1fc7b8f46f8d06"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::clock`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::clock(Context &c) noexcept
@brief 读取并保存本次处理的时间快照；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c665c4cb6d577034"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::duplicate_ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcknowledgeFlow::duplicate_ack(Context &c) noexcept
@brief 检查确认号是否仍等于发送确认前沿；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-22c698f63f58d05e"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::fin_ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcknowledgeFlow::fin_ack(Context &c) noexcept
@brief 检查本次确认是否覆盖已发送 FIN；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-78823400ae805c8c"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::finish`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::finish(Context &c) noexcept
@brief 提交 FIN 或完成当前业务阶段；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ac9c7c05f0fe7951"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::handle_duplicate`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::handle_duplicate(Context &c) noexcept
@brief 按重复确认与恢复状态更新拥塞控制和重传计时；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4339e9a7ad4faffe"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::notify`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::notify(Context &c) noexcept
@brief 发布本阶段状态变化的等待唤醒；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6e1ad8d4694b4937"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::partial_recovery`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcknowledgeFlow::partial_recovery(Context &c) noexcept
@brief 根据新确认数据检查是否需要恢复重传；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-459056c869b645c6"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::retransmit`

```doxygen
@fn static inline void xv::net::TcpSocket::AcknowledgeFlow::retransmit(Context &c) noexcept
@brief 重发当前连接未确认的握手、数据或 FIN；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dccfa508d1abc120"></a>

## `xv::net::TcpSocket::AcknowledgeFlow::valid`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcknowledgeFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::TcpSocket::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-05bb54ce41faec96"></a>

## `xv::net::TcpSocket::FinishFlow::commit`

```doxygen
@fn static inline void xv::net::TcpSocket::FinishFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::TcpSocket::FinishFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cb2a9988527a06ed"></a>

## `xv::net::TcpSocket::FinishFlow::credit`

```doxygen
@fn static inline bool xv::net::TcpSocket::FinishFlow::credit(Context &c) noexcept
@brief 检查当前发送窗口是否还有额度；所属流程为 xv::net::TcpSocket::FinishFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8c657f360fd65398"></a>

## `xv::net::TcpSocket::FinishFlow::eligible`

```doxygen
@fn static inline bool xv::net::TcpSocket::FinishFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::TcpSocket::FinishFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-60f00bc8eff76c0d"></a>

## `xv::net::TcpSocket::FinishFlow::measure`

```doxygen
@fn static inline void xv::net::TcpSocket::FinishFlow::measure(Context &c) noexcept
@brief 计算本阶段发送量或计时样本；所属流程为 xv::net::TcpSocket::FinishFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5feb55b2074741bd"></a>

## `xv::net::TcpSocket::FinishFlow::send`

```doxygen
@fn static inline bool xv::net::TcpSocket::FinishFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::TcpSocket::FinishFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-668678409923c737"></a>

## `xv::net::TcpSocket::PersistFlow::data`

```doxygen
@fn static inline bool xv::net::TcpSocket::PersistFlow::data(Context &c) noexcept
@brief 检查是否还有可发送或可探测的数据；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-174e07ab7b5cdf70"></a>

## `xv::net::TcpSocket::PersistFlow::due`

```doxygen
@fn static inline bool xv::net::TcpSocket::PersistFlow::due(Context &c) noexcept
@brief 检查当前重试、探测或发送是否已经到期；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9e5d89e660bc4b0b"></a>

## `xv::net::TcpSocket::PersistFlow::eligible`

```doxygen
@fn static inline bool xv::net::TcpSocket::PersistFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-df2f019ed3539a55"></a>

## `xv::net::TcpSocket::PersistFlow::probe_ack`

```doxygen
@fn static inline void xv::net::TcpSocket::PersistFlow::probe_ack(Context &c) noexcept
@brief 发送无载荷的零窗口确认探测；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8424cc447204af56"></a>

## `xv::net::TcpSocket::PersistFlow::probe_data`

```doxygen
@fn static inline void xv::net::TcpSocket::PersistFlow::probe_data(Context &c) noexcept
@brief 发送一个数据字节的零窗口探测；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-171279803e63badd"></a>

## `xv::net::TcpSocket::PersistFlow::reset`

```doxygen
@fn static inline void xv::net::TcpSocket::PersistFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b5674ab3c8f320a2"></a>

## `xv::net::TcpSocket::PersistFlow::schedule`

```doxygen
@fn static inline void xv::net::TcpSocket::PersistFlow::schedule(Context &c) noexcept
@brief 安排下一次重试或探测并保存所需快照；所属流程为 xv::net::TcpSocket::PersistFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0114b24f55a7752b"></a>

## `xv::net::TcpSocket::RetransmitFlow::available`

```doxygen
@fn static inline bool xv::net::TcpSocket::RetransmitFlow::available(Context &c) noexcept
@brief 检查重传是否还有有效长度与 MSS；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c610c50f5ba5937e"></a>

## `xv::net::TcpSocket::RetransmitFlow::data`

```doxygen
@fn static inline bool xv::net::TcpSocket::RetransmitFlow::data(Context &c) noexcept
@brief 检查是否还有可发送或可探测的数据；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-390d562c118aeb0e"></a>

## `xv::net::TcpSocket::RetransmitFlow::fin`

```doxygen
@fn static inline bool xv::net::TcpSocket::RetransmitFlow::fin(Context &c) noexcept
@brief 检查是否还有未确认的 FIN；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-efb8b22d2fd73776"></a>

## `xv::net::TcpSocket::RetransmitFlow::measure`

```doxygen
@fn static inline void xv::net::TcpSocket::RetransmitFlow::measure(Context &c) noexcept
@brief 计算本阶段发送量或计时样本；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4082d200bf7b03f6"></a>

## `xv::net::TcpSocket::RetransmitFlow::send_data`

```doxygen
@fn static inline void xv::net::TcpSocket::RetransmitFlow::send_data(Context &c) noexcept
@brief 重传当前有界数据段；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9c8927c357ded64b"></a>

## `xv::net::TcpSocket::RetransmitFlow::send_fin`

```doxygen
@fn static inline void xv::net::TcpSocket::RetransmitFlow::send_fin(Context &c) noexcept
@brief 重传未确认的 FIN；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3a8ba6d71f2cf68e"></a>

## `xv::net::TcpSocket::RetransmitFlow::send_syn`

```doxygen
@fn static inline void xv::net::TcpSocket::RetransmitFlow::send_syn(Context &c) noexcept
@brief 重传当前握手序号的 SYN 或 SYN-ACK；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0cd6e873189cb721"></a>

## `xv::net::TcpSocket::RetransmitFlow::syn`

```doxygen
@fn static inline bool xv::net::TcpSocket::RetransmitFlow::syn(Context &c) noexcept
@brief 检查报文或待重传内容是否为 SYN；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e692706eba52af43"></a>

## `xv::net::TcpSocket::RetransmitFlow::uncertain`

```doxygen
@fn static inline void xv::net::TcpSocket::RetransmitFlow::uncertain(Context &c) noexcept
@brief 将重传样本标记为存在歧义；所属流程为 xv::net::TcpSocket::RetransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
