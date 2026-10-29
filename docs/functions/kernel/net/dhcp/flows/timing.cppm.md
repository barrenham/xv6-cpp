# `kernel/net/dhcp/flows/timing.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::DhcpManager::TickFlow::advance`](timing.cppm.md#fn-5aca053719678737)
- [`xv::net::DhcpManager::TickFlow::changed`](timing.cppm.md#fn-534b2f568607c315)
- [`xv::net::DhcpManager::TickFlow::eligible`](timing.cppm.md#fn-8a5cc62fcb9479b7)
- [`xv::net::DhcpManager::TickFlow::exhausted`](timing.cppm.md#fn-bac9dd0caee39d79)
- [`xv::net::DhcpManager::TickFlow::expire`](timing.cppm.md#fn-208fac5ffb238218)
- [`xv::net::DhcpManager::TickFlow::expired`](timing.cppm.md#fn-416abd970aeb91d2)
- [`xv::net::DhcpManager::TickFlow::fail`](timing.cppm.md#fn-6a042fbc2bab5f8d)
- [`xv::net::DhcpManager::TickFlow::inspect`](timing.cppm.md#fn-0da29518e28cde1c)
- [`xv::net::DhcpManager::TickFlow::invalidate`](timing.cppm.md#fn-5bfa6a063d531805)
- [`xv::net::DhcpManager::TickFlow::more`](timing.cppm.md#fn-25be9230fb8b4652)
- [`xv::net::DhcpManager::TickFlow::rebind`](timing.cppm.md#fn-95e510cafc4a8118)
- [`xv::net::DhcpManager::TickFlow::rebinding_due`](timing.cppm.md#fn-67d0d6ba48eb86b5)
- [`xv::net::DhcpManager::TickFlow::renew`](timing.cppm.md#fn-1e12772ee8813abb)
- [`xv::net::DhcpManager::TickFlow::renewal_due`](timing.cppm.md#fn-91fce2ee291297fd)
- [`xv::net::DhcpManager::TickFlow::retry_failed`](timing.cppm.md#fn-49784a23c2bd5c4f)
- [`xv::net::DhcpManager::TickFlow::schedule`](timing.cppm.md#fn-f53c6a2dcc18d8db)
- [`xv::net::DhcpManager::TickFlow::select`](timing.cppm.md#fn-aacf4105e6f4d4cc)
- [`xv::net::DhcpManager::TickFlow::should_send`](timing.cppm.md#fn-518551686899ffbf)
- [`xv::net::DhcpManager::TickFlow::snapshot`](timing.cppm.md#fn-1c51795d0a1cf5b8)
- [`xv::net::DhcpManager::TickFlow::transmission_due`](timing.cppm.md#fn-8932d65fb1806679)
- [`xv::net::DhcpManager::TickFlow::transmit`](timing.cppm.md#fn-2f99f91621a94363)
- [`xv::net::DhcpManager::TransmitFlow::allocate`](timing.cppm.md#fn-54e4ae64b2255058)
- [`xv::net::DhcpManager::TransmitFlow::allocated`](timing.cppm.md#fn-a89683570ac71338)
- [`xv::net::DhcpManager::TransmitFlow::bootstrap`](timing.cppm.md#fn-930a29b63d31e8ce)
- [`xv::net::DhcpManager::TransmitFlow::current`](timing.cppm.md#fn-9a0ba6a6df3cb9db)
- [`xv::net::DhcpManager::TransmitFlow::describe`](timing.cppm.md#fn-aa5212df17d92ac6)
- [`xv::net::DhcpManager::TransmitFlow::encode`](timing.cppm.md#fn-b28c79460e16bfdb)
- [`xv::net::DhcpManager::TransmitFlow::encoded`](timing.cppm.md#fn-5aab920c34ae1cba)
- [`xv::net::DhcpManager::TransmitFlow::invalid`](timing.cppm.md#fn-39962ead757f19c2)
- [`xv::net::DhcpManager::TransmitFlow::no_buffer`](timing.cppm.md#fn-6a0480c705a6bbe6)
- [`xv::net::DhcpManager::TransmitFlow::send`](timing.cppm.md#fn-2e190f5e4859c389)
- [`xv::net::DhcpManager::TransmitFlow::stale`](timing.cppm.md#fn-8fa10039dab713e5)
- [`xv::net::DhcpManager::TransmitFlow::udp_header`](timing.cppm.md#fn-758e50d9267acef7)
- [`xv::net::DhcpManager::TransmitFlow::unicast`](timing.cppm.md#fn-b986bd0891648882)

<a id="fn-5aca053719678737"></a>

## `xv::net::DhcpManager::TickFlow::advance`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-534b2f568607c315"></a>

## `xv::net::DhcpManager::TickFlow::changed`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::changed(Context &c) noexcept
@brief 记录或查询本次业务状态变化；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8a5cc62fcb9479b7"></a>

## `xv::net::DhcpManager::TickFlow::eligible`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bac9dd0caee39d79"></a>

## `xv::net::DhcpManager::TickFlow::exhausted`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::exhausted(Context &c) noexcept
@brief 检查当前重试次数是否耗尽；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-208fac5ffb238218"></a>

## `xv::net::DhcpManager::TickFlow::expire`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::expire(Context &c) noexcept
@brief 移除到期业务资源并准备后续重试；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-416abd970aeb91d2"></a>

## `xv::net::DhcpManager::TickFlow::expired`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::expired(Context &c) noexcept
@brief 检查当前租约是否已经到期；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6a042fbc2bab5f8d"></a>

## `xv::net::DhcpManager::TickFlow::fail`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::fail(Context &c) noexcept
@brief 记录 DHCP 重试耗尽并进入失败退避；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0da29518e28cde1c"></a>

## `xv::net::DhcpManager::TickFlow::inspect`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 ClientGraph，其 stop 结束该次内部 run。
```

<a id="fn-5bfa6a063d531805"></a>

## `xv::net::DhcpManager::TickFlow::invalidate`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-25be9230fb8b4652"></a>

## `xv::net::DhcpManager::TickFlow::more`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-95e510cafc4a8118"></a>

## `xv::net::DhcpManager::TickFlow::rebind`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::rebind(Context &c) noexcept
@brief 将 DHCP 续租客户端转入广播重绑定；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-67d0d6ba48eb86b5"></a>

## `xv::net::DhcpManager::TickFlow::rebinding_due`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::rebinding_due(Context &c) noexcept
@brief 检查 DHCP 重绑定时刻是否已经到达；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e12772ee8813abb"></a>

## `xv::net::DhcpManager::TickFlow::renew`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::renew(Context &c) noexcept
@brief 将 DHCP 已绑定客户端转为续租并推进代次；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-91fce2ee291297fd"></a>

## `xv::net::DhcpManager::TickFlow::renewal_due`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::renewal_due(Context &c) noexcept
@brief 检查有限租约续租时刻是否已经到达；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-49784a23c2bd5c4f"></a>

## `xv::net::DhcpManager::TickFlow::retry_failed`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::retry_failed(Context &c) noexcept
@brief 检查 DHCP 失败退避是否已经到期；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f53c6a2dcc18d8db"></a>

## `xv::net::DhcpManager::TickFlow::schedule`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::schedule(Context &c) noexcept
@brief 安排下一次重试或探测并保存所需快照；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aacf4105e6f4d4cc"></a>

## `xv::net::DhcpManager::TickFlow::select`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::select(Context &c) noexcept
@brief 重新开始 DHCP 地址选择；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-518551686899ffbf"></a>

## `xv::net::DhcpManager::TickFlow::should_send`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::should_send(Context &c) noexcept
@brief 检查客户端定时阶段是否要求发送；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1c51795d0a1cf5b8"></a>

## `xv::net::DhcpManager::TickFlow::snapshot`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::snapshot(Context &c) noexcept
@brief 保存锁内选定的客户端或缓冲边界值；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8932d65fb1806679"></a>

## `xv::net::DhcpManager::TickFlow::transmission_due`

```doxygen
@fn static inline bool xv::net::DhcpManager::TickFlow::transmission_due(Context &c) noexcept
@brief 检查客户端状态、时刻和接口是否允许发送；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2f99f91621a94363"></a>

## `xv::net::DhcpManager::TickFlow::transmit`

```doxygen
@fn static inline void xv::net::DhcpManager::TickFlow::transmit(Context &c) noexcept
@brief 通过本对象的发送方法提交已选择业务报文；所属流程为 xv::net::DhcpManager::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-54e4ae64b2255058"></a>

## `xv::net::DhcpManager::TransmitFlow::allocate`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-a89683570ac71338"></a>

## `xv::net::DhcpManager::TransmitFlow::allocated`

```doxygen
@fn static inline bool xv::net::DhcpManager::TransmitFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-930a29b63d31e8ce"></a>

## `xv::net::DhcpManager::TransmitFlow::bootstrap`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::bootstrap(Context &c) noexcept
@brief 通过 DHCP 引导发送入口提交报文；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9a0ba6a6df3cb9db"></a>

## `xv::net::DhcpManager::TransmitFlow::current`

```doxygen
@fn static inline bool xv::net::DhcpManager::TransmitFlow::current(Context &c) noexcept
@brief 检查请求或快照是否仍是当前有效状态；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-aa5212df17d92ac6"></a>

## `xv::net::DhcpManager::TransmitFlow::describe`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::describe(Context &c) noexcept
@brief 构造本阶段使用的具名值描述；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b28c79460e16bfdb"></a>

## `xv::net::DhcpManager::TransmitFlow::encode`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::encode(Context &c) noexcept
@brief 按已验证的业务值编码报文字段；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5aab920c34ae1cba"></a>

## `xv::net::DhcpManager::TransmitFlow::encoded`

```doxygen
@fn static inline bool xv::net::DhcpManager::TransmitFlow::encoded(Context &c) noexcept
@brief 检查消息编码是否产生有效长度；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-39962ead757f19c2"></a>

## `xv::net::DhcpManager::TransmitFlow::invalid`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6a0480c705a6bbe6"></a>

## `xv::net::DhcpManager::TransmitFlow::no_buffer`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::no_buffer(Context &c) noexcept
@brief 记录本次分配或容量不足的负错误；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2e190f5e4859c389"></a>

## `xv::net::DhcpManager::TransmitFlow::send`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8fa10039dab713e5"></a>

## `xv::net::DhcpManager::TransmitFlow::stale`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::stale(Context &c) noexcept
@brief 记录发送快照已经过期的负 EAGAIN；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-758e50d9267acef7"></a>

## `xv::net::DhcpManager::TransmitFlow::udp_header`

```doxygen
@fn static inline void xv::net::DhcpManager::TransmitFlow::udp_header(Context &c) noexcept
@brief 编码 DHCP 的 UDP 首部与地址伪首部校验和；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b986bd0891648882"></a>

## `xv::net::DhcpManager::TransmitFlow::unicast`

```doxygen
@fn static inline bool xv::net::DhcpManager::TransmitFlow::unicast(Context &c) noexcept
@brief 检查当前 DHCP 请求是否使用单播；所属流程为 xv::net::DhcpManager::TransmitFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
