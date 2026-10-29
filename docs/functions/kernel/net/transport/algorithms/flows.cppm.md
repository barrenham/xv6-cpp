# `kernel/net/transport/algorithms/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::RetransmissionTimer::AcknowledgeFlow::all_acknowledged`](flows.cppm.md#fn-1a5a2b955ee15a3b)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::clear_ambiguity`](flows.cppm.md#fn-74f01d138275667b)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::first_sample`](flows.cppm.md#fn-90c1d41573619c32)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::initialize`](flows.cppm.md#fn-fe27b2296724ab04)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::interval`](flows.cppm.md#fn-5040d1923bead400)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::measure`](flows.cppm.md#fn-6e3501f7e33538aa)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::sampled`](flows.cppm.md#fn-3d7c3ece3325ec03)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::smooth`](flows.cppm.md#fn-77f9647802805559)
- [`xv::net::RetransmissionTimer::AcknowledgeFlow::start`](flows.cppm.md#fn-730928cbd13b1da2)
- [`xv::net::TcpCongestion::AcknowledgeFlow::accumulate`](flows.cppm.md#fn-b749ee5106eb3ddd)
- [`xv::net::TcpCongestion::AcknowledgeFlow::acknowledge_barrier`](flows.cppm.md#fn-424aae69cd104340)
- [`xv::net::TcpCongestion::AcknowledgeFlow::bytes`](flows.cppm.md#fn-f6238dffe3b3b652)
- [`xv::net::TcpCongestion::AcknowledgeFlow::clamp`](flows.cppm.md#fn-3070cb9f2c5123e2)
- [`xv::net::TcpCongestion::AcknowledgeFlow::grow`](flows.cppm.md#fn-0851b3d72213d164)
- [`xv::net::TcpCongestion::AcknowledgeFlow::leave_recovery`](flows.cppm.md#fn-02410d8241ef0fc8)
- [`xv::net::TcpCongestion::AcknowledgeFlow::partial`](flows.cppm.md#fn-34dfff1f02fb465a)
- [`xv::net::TcpCongestion::AcknowledgeFlow::recovery`](flows.cppm.md#fn-35c884f982165f82)
- [`xv::net::TcpCongestion::AcknowledgeFlow::repair`](flows.cppm.md#fn-d2f0bb77a877d60f)
- [`xv::net::TcpCongestion::AcknowledgeFlow::slow_start`](flows.cppm.md#fn-b44a993d3a842029)
- [`xv::net::TcpCongestion::DuplicateFlow::can_inflate`](flows.cppm.md#fn-bd80429e3bf2e10c)
- [`xv::net::TcpCongestion::DuplicateFlow::count`](flows.cppm.md#fn-cf43b511143bb03b)
- [`xv::net::TcpCongestion::DuplicateFlow::eligible`](flows.cppm.md#fn-c6d950b14a8543cc)
- [`xv::net::TcpCongestion::DuplicateFlow::inflate`](flows.cppm.md#fn-c1f3c29405280463)
- [`xv::net::TcpCongestion::DuplicateFlow::recover`](flows.cppm.md#fn-9fcb537ad88d82f2)
- [`xv::net::TcpCongestion::DuplicateFlow::third`](flows.cppm.md#fn-279a6da12308e7bb)

<a id="fn-1a5a2b955ee15a3b"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::all_acknowledged`

```doxygen
@fn static inline bool xv::net::RetransmissionTimer::AcknowledgeFlow::all_acknowledged(Context &c) noexcept
@brief 检查是否已没有未确认的数据；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-74f01d138275667b"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::clear_ambiguity`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::clear_ambiguity(Context &c) noexcept
@brief 清除重传采样的歧义标志；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-90c1d41573619c32"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::first_sample`

```doxygen
@fn static inline bool xv::net::RetransmissionTimer::AcknowledgeFlow::first_sample(Context &c) noexcept
@brief 检查计时器是否尚无平滑 RTT 样本；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fe27b2296724ab04"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::initialize`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::initialize(Context &c) noexcept
@brief 初始化当前业务阶段状态；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5040d1923bead400"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::interval`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::interval(Context &c) noexcept
@brief 根据 RTT 平滑值与偏差更新有界 RTO；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6e3501f7e33538aa"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::measure`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::measure(Context &c) noexcept
@brief 计算本阶段发送量或计时样本；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3d7c3ece3325ec03"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::sampled`

```doxygen
@fn static inline bool xv::net::RetransmissionTimer::AcknowledgeFlow::sampled(Context &c) noexcept
@brief 检查本次累计确认是否覆盖有效 RTT 样本；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-77f9647802805559"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::smooth`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::smooth(Context &c) noexcept
@brief 根据新 RTT 样本更新平滑值与偏差；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-730928cbd13b1da2"></a>

## `xv::net::RetransmissionTimer::AcknowledgeFlow::start`

```doxygen
@fn static inline void xv::net::RetransmissionTimer::AcknowledgeFlow::start(Context &c) noexcept
@brief 开始监听后的计时或开始 DHCP 选择；所属流程为 xv::net::RetransmissionTimer::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b749ee5106eb3ddd"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::accumulate`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::accumulate(Context &c) noexcept
@brief 累积拥塞避免的确认字节额度；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-424aae69cd104340"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::acknowledge_barrier`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::acknowledge_barrier(Context &c) noexcept
@brief 清除重复确认计数并按恢复边界更新屏障；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f6238dffe3b3b652"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::bytes`

```doxygen
@fn static inline bool xv::net::TcpCongestion::AcknowledgeFlow::bytes(Context &c) noexcept
@brief 检查本次是否确认了非零数据字节；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3070cb9f2c5123e2"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::clamp`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::clamp(Context &c) noexcept
@brief 按本组件容量或协议上限约束数值；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0851b3d72213d164"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::grow`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::grow(Context &c) noexcept
@brief 按慢启动规则增加拥塞窗口；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-02410d8241ef0fc8"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::leave_recovery`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::leave_recovery(Context &c) noexcept
@brief 结束快速恢复并还原门限窗口与计数；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-34dfff1f02fb465a"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::partial`

```doxygen
@fn static inline bool xv::net::TcpCongestion::AcknowledgeFlow::partial(Context &c) noexcept
@brief 检查本次确认是否尚未到达快速恢复边界；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-35c884f982165f82"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::recovery`

```doxygen
@fn static inline bool xv::net::TcpCongestion::AcknowledgeFlow::recovery(Context &c) noexcept
@brief 检查拥塞控制是否处于快速恢复；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d2f0bb77a877d60f"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::repair`

```doxygen
@fn static inline void xv::net::TcpCongestion::AcknowledgeFlow::repair(Context &c) noexcept
@brief 修复重组首部或快速恢复中的窗口状态；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b44a993d3a842029"></a>

## `xv::net::TcpCongestion::AcknowledgeFlow::slow_start`

```doxygen
@fn static inline bool xv::net::TcpCongestion::AcknowledgeFlow::slow_start(Context &c) noexcept
@brief 检查拥塞窗口是否仍处于慢启动区间；所属流程为 xv::net::TcpCongestion::AcknowledgeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bd80429e3bf2e10c"></a>

## `xv::net::TcpCongestion::DuplicateFlow::can_inflate`

```doxygen
@fn static inline bool xv::net::TcpCongestion::DuplicateFlow::can_inflate(Context &c) noexcept
@brief 检查重复确认是否允许扩大恢复窗口；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf43b511143bb03b"></a>

## `xv::net::TcpCongestion::DuplicateFlow::count`

```doxygen
@fn static inline void xv::net::TcpCongestion::DuplicateFlow::count(Context &c) noexcept
@brief 更新当前报文或重复确认的计数；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c6d950b14a8543cc"></a>

## `xv::net::TcpCongestion::DuplicateFlow::eligible`

```doxygen
@fn static inline bool xv::net::TcpCongestion::DuplicateFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1f3c29405280463"></a>

## `xv::net::TcpCongestion::DuplicateFlow::inflate`

```doxygen
@fn static inline void xv::net::TcpCongestion::DuplicateFlow::inflate(Context &c) noexcept
@brief 按恢复规则扩展拥塞窗口；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9fcb537ad88d82f2"></a>

## `xv::net::TcpCongestion::DuplicateFlow::recover`

```doxygen
@fn static inline void xv::net::TcpCongestion::DuplicateFlow::recover(Context &c) noexcept
@brief 进入 TCP 快速恢复并请求重传；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-279a6da12308e7bb"></a>

## `xv::net::TcpCongestion::DuplicateFlow::third`

```doxygen
@fn static inline bool xv::net::TcpCongestion::DuplicateFlow::third(Context &c) noexcept
@brief 检查是否到达第三个重复确认；所属流程为 xv::net::TcpCongestion::DuplicateFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
