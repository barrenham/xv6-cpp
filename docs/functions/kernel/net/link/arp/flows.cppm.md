# `kernel/net/link/arp/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::ArpCache::ReceiveFlow::advance`](flows.cppm.md#fn-2c339c566208a7fc)
- [`xv::net::ArpCache::ReceiveFlow::eligible`](flows.cppm.md#fn-d3790e7867a1e87c)
- [`xv::net::ArpCache::ReceiveFlow::flush_one`](flows.cppm.md#fn-5b8ba921804a8824)
- [`xv::net::ArpCache::ReceiveFlow::inspect`](flows.cppm.md#fn-10994cb31dade2e3)
- [`xv::net::ArpCache::ReceiveFlow::learn`](flows.cppm.md#fn-7e38fa88b39bdda7)
- [`xv::net::ArpCache::ReceiveFlow::more_pending`](flows.cppm.md#fn-3c0394a9e1dc57cf)
- [`xv::net::ArpCache::ReceiveFlow::reply`](flows.cppm.md#fn-d90efaa99aeffed7)
- [`xv::net::ArpCache::ReceiveFlow::request`](flows.cppm.md#fn-3fc5605707e7dd7b)
- [`xv::net::ArpCache::ReceiveFlow::valid_header`](flows.cppm.md#fn-9ac0652c32e3ed6c)
- [`xv::net::ArpCache::ReceiveFlow::valid_source`](flows.cppm.md#fn-afa10980355a1efa)
- [`xv::net::ArpCache::SendFlow::cached`](flows.cppm.md#fn-6accbe14483ec6c9)
- [`xv::net::ArpCache::SendFlow::find_neighbor`](flows.cppm.md#fn-374c6a5133e8fa0f)
- [`xv::net::ArpCache::SendFlow::full`](flows.cppm.md#fn-03184f243469a1d4)
- [`xv::net::ArpCache::SendFlow::queue`](flows.cppm.md#fn-1dbbe25851c5b0fd)
- [`xv::net::ArpCache::SendFlow::queued`](flows.cppm.md#fn-cf974ad54c894d89)
- [`xv::net::ArpCache::SendFlow::request`](flows.cppm.md#fn-9c70b2cfeac48d87)
- [`xv::net::ArpCache::SendFlow::resolve`](flows.cppm.md#fn-150a4cdd9aef6dad)
- [`xv::net::ArpCache::SendFlow::send`](flows.cppm.md#fn-fcb4c75720aa6fdb)
- [`xv::net::ArpCache::SendFlow::unreachable`](flows.cppm.md#fn-905acedaec883334)
- [`xv::net::ArpCache::SendFlow::valid_interface`](flows.cppm.md#fn-628cdf6f1c9d09a4)
- [`xv::net::ArpCache::TickFlow::advance`](flows.cppm.md#fn-4a12a3474c299646)
- [`xv::net::ArpCache::TickFlow::count_retry`](flows.cppm.md#fn-444f53b03cfc61dc)
- [`xv::net::ArpCache::TickFlow::due`](flows.cppm.md#fn-fb7c6d5642d0cba3)
- [`xv::net::ArpCache::TickFlow::exhausted`](flows.cppm.md#fn-c50e09215c6ffa7b)
- [`xv::net::ArpCache::TickFlow::expire`](flows.cppm.md#fn-a990f9a2dafb1f4e)
- [`xv::net::ArpCache::TickFlow::inspect`](flows.cppm.md#fn-7136cbfef2837887)
- [`xv::net::ArpCache::TickFlow::invalidate`](flows.cppm.md#fn-68934b2aab35d6a8)
- [`xv::net::ArpCache::TickFlow::more`](flows.cppm.md#fn-9486b003a58d110f)
- [`xv::net::ArpCache::TickFlow::request`](flows.cppm.md#fn-e83cedf9308a9396)
- [`xv::net::ArpCache::TickFlow::retry`](flows.cppm.md#fn-fef90ab6a207c272)
- [`xv::net::ArpCache::TickFlow::schedule`](flows.cppm.md#fn-7dad91a4a79fa2e1)

<a id="fn-2c339c566208a7fc"></a>

## `xv::net::ArpCache::ReceiveFlow::advance`

```doxygen
@fn static inline void xv::net::ArpCache::ReceiveFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d3790e7867a1e87c"></a>

## `xv::net::ArpCache::ReceiveFlow::eligible`

```doxygen
@fn static inline bool xv::net::ArpCache::ReceiveFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5b8ba921804a8824"></a>

## `xv::net::ArpCache::ReceiveFlow::flush_one`

```doxygen
@fn static inline void xv::net::ArpCache::ReceiveFlow::flush_one(Context &c) noexcept
@brief 在锁内分离一条 ARP 待解析记录并在锁外发送；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-10994cb31dade2e3"></a>

## `xv::net::ArpCache::ReceiveFlow::inspect`

```doxygen
@fn static inline void xv::net::ArpCache::ReceiveFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7e38fa88b39bdda7"></a>

## `xv::net::ArpCache::ReceiveFlow::learn`

```doxygen
@fn static inline void xv::net::ArpCache::ReceiveFlow::learn(Context &c) noexcept
@brief 学习入口源 MAC 或 ARP 邻居及其有效期；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-3c0394a9e1dc57cf"></a>

## `xv::net::ArpCache::ReceiveFlow::more_pending`

```doxygen
@fn static inline bool xv::net::ArpCache::ReceiveFlow::more_pending(Context &c) noexcept
@brief 检查 ARP 扫描是否还有待解析记录；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d90efaa99aeffed7"></a>

## `xv::net::ArpCache::ReceiveFlow::reply`

```doxygen
@fn static inline void xv::net::ArpCache::ReceiveFlow::reply(Context &c) noexcept
@brief 编码并发送 ARP 回复；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3fc5605707e7dd7b"></a>

## `xv::net::ArpCache::ReceiveFlow::request`

```doxygen
@fn static inline bool xv::net::ArpCache::ReceiveFlow::request(Context &c) noexcept
@brief 发送邻居解析请求或检查 ARP 请求类型；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9ac0652c32e3ed6c"></a>

## `xv::net::ArpCache::ReceiveFlow::valid_header`

```doxygen
@fn static inline bool xv::net::ArpCache::ReceiveFlow::valid_header(Context &c) noexcept
@brief 检查本协议固定首部与长度约束；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-afa10980355a1efa"></a>

## `xv::net::ArpCache::ReceiveFlow::valid_source`

```doxygen
@fn static inline bool xv::net::ArpCache::ReceiveFlow::valid_source(Context &c) noexcept
@brief 检查 ARP 源地址不是本机、零或全局广播；所属流程为 xv::net::ArpCache::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6accbe14483ec6c9"></a>

## `xv::net::ArpCache::SendFlow::cached`

```doxygen
@fn static inline bool xv::net::ArpCache::SendFlow::cached(Context &c) noexcept
@brief 检查邻居解析是否命中缓存；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-374c6a5133e8fa0f"></a>

## `xv::net::ArpCache::SendFlow::find_neighbor`

```doxygen
@fn static inline bool xv::net::ArpCache::SendFlow::find_neighbor(Context &c) noexcept
@brief 查找仍未过期的下一跳邻居缓存；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-03184f243469a1d4"></a>

## `xv::net::ArpCache::SendFlow::full`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1dbbe25851c5b0fd"></a>

## `xv::net::ArpCache::SendFlow::queue`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::queue(Context &c) noexcept
@brief 将本次独占报文或值快照加入有界队列；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf974ad54c894d89"></a>

## `xv::net::ArpCache::SendFlow::queued`

```doxygen
@fn static inline bool xv::net::ArpCache::SendFlow::queued(Context &c) noexcept
@brief 检查未解析报文是否已经成功排队；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9c70b2cfeac48d87"></a>

## `xv::net::ArpCache::SendFlow::request`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::request(Context &c) noexcept
@brief 发送邻居解析请求或检查 ARP 请求类型；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-150a4cdd9aef6dad"></a>

## `xv::net::ArpCache::SendFlow::resolve`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::resolve(Context &c) noexcept
@brief 在邻居锁下执行缓存或排队解析图；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 ResolutionGraph，其 stop 结束该次内部 run。
```

<a id="fn-fcb4c75720aa6fdb"></a>

## `xv::net::ArpCache::SendFlow::send`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-905acedaec883334"></a>

## `xv::net::ArpCache::SendFlow::unreachable`

```doxygen
@fn static inline void xv::net::ArpCache::SendFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-628cdf6f1c9d09a4"></a>

## `xv::net::ArpCache::SendFlow::valid_interface`

```doxygen
@fn static inline bool xv::net::ArpCache::SendFlow::valid_interface(Context &c) noexcept
@brief 在配置锁下检查发送接口及源地址归属；所属流程为 xv::net::ArpCache::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-4a12a3474c299646"></a>

## `xv::net::ArpCache::TickFlow::advance`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-444f53b03cfc61dc"></a>

## `xv::net::ArpCache::TickFlow::count_retry`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::count_retry(Context &c) noexcept
@brief 推进重试次数并记录是否达到上限；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fb7c6d5642d0cba3"></a>

## `xv::net::ArpCache::TickFlow::due`

```doxygen
@fn static inline bool xv::net::ArpCache::TickFlow::due(Context &c) noexcept
@brief 检查当前重试、探测或发送是否已经到期；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c50e09215c6ffa7b"></a>

## `xv::net::ArpCache::TickFlow::exhausted`

```doxygen
@fn static inline bool xv::net::ArpCache::TickFlow::exhausted(Context &c) noexcept
@brief 检查当前重试次数是否耗尽；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a990f9a2dafb1f4e"></a>

## `xv::net::ArpCache::TickFlow::expire`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::expire(Context &c) noexcept
@brief 移除到期业务资源并准备后续重试；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7136cbfef2837887"></a>

## `xv::net::ArpCache::TickFlow::inspect`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 PendingGraph，其 stop 结束该次内部 run。
```

<a id="fn-68934b2aab35d6a8"></a>

## `xv::net::ArpCache::TickFlow::invalidate`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9486b003a58d110f"></a>

## `xv::net::ArpCache::TickFlow::more`

```doxygen
@fn static inline bool xv::net::ArpCache::TickFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e83cedf9308a9396"></a>

## `xv::net::ArpCache::TickFlow::request`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::request(Context &c) noexcept
@brief 发送邻居解析请求或检查 ARP 请求类型；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fef90ab6a207c272"></a>

## `xv::net::ArpCache::TickFlow::retry`

```doxygen
@fn static inline bool xv::net::ArpCache::TickFlow::retry(Context &c) noexcept
@brief 检查当前 ARP 记录是否需要锁外重试；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7dad91a4a79fa2e1"></a>

## `xv::net::ArpCache::TickFlow::schedule`

```doxygen
@fn static inline void xv::net::ArpCache::TickFlow::schedule(Context &c) noexcept
@brief 安排下一次重试或探测并保存所需快照；所属流程为 xv::net::ArpCache::TickFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
