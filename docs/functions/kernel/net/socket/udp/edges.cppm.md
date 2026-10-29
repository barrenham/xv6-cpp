# `kernel/net/socket/udp/edges.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::UdpSocket::ConnectFlow::bind`](edges.cppm.md#fn-7f88e8be4bfe4a00)
- [`xv::net::UdpSocket::ConnectFlow::bound`](edges.cppm.md#fn-e44aa2cec275e28f)
- [`xv::net::UdpSocket::ConnectFlow::connect`](edges.cppm.md#fn-e1cbfc47ef300502)
- [`xv::net::UdpSocket::ConnectFlow::invalid`](edges.cppm.md#fn-1115a22a23def1db)
- [`xv::net::UdpSocket::ConnectFlow::locked`](edges.cppm.md#fn-eac3bc42c4f2b8e8)
- [`xv::net::UdpSocket::ConnectFlow::valid`](edges.cppm.md#fn-c8a6ec088506d0a8)
- [`xv::net::UdpSocket::DeliveryFlow::claim`](edges.cppm.md#fn-87d58b974995994b)
- [`xv::net::UdpSocket::DeliveryFlow::matches`](edges.cppm.md#fn-e8b7476f4ad730e9)
- [`xv::net::UdpSocket::DeliveryFlow::queue`](edges.cppm.md#fn-ae0692e75b3a3309)
- [`xv::net::UdpSocket::DeliveryFlow::space`](edges.cppm.md#fn-2943236044e5f2f8)

<a id="fn-7f88e8be4bfe4a00"></a>

## `xv::net::UdpSocket::ConnectFlow::bind`

```doxygen
@fn static inline void xv::net::UdpSocket::ConnectFlow::bind(Context &c) noexcept
@brief 完成绑定或进入租约已绑定状态；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e44aa2cec275e28f"></a>

## `xv::net::UdpSocket::ConnectFlow::bound`

```doxygen
@fn static inline bool xv::net::UdpSocket::ConnectFlow::bound(Context &c) noexcept
@brief 检查绑定步骤的业务结果是否成功；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e1cbfc47ef300502"></a>

## `xv::net::UdpSocket::ConnectFlow::connect`

```doxygen
@fn static inline void xv::net::UdpSocket::ConnectFlow::connect(Context &c) noexcept
@brief 提交 UDP 对端及按需选择的本地地址；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1115a22a23def1db"></a>

## `xv::net::UdpSocket::ConnectFlow::invalid`

```doxygen
@fn static inline void xv::net::UdpSocket::ConnectFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-eac3bc42c4f2b8e8"></a>

## `xv::net::UdpSocket::ConnectFlow::locked`

```doxygen
@fn static inline void xv::net::UdpSocket::ConnectFlow::locked(Context &c) noexcept
@brief 在原锁作用域内执行对应业务图；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-c8a6ec088506d0a8"></a>

## `xv::net::UdpSocket::ConnectFlow::valid`

```doxygen
@fn static inline bool xv::net::UdpSocket::ConnectFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::UdpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-87d58b974995994b"></a>

## `xv::net::UdpSocket::DeliveryFlow::claim`

```doxygen
@fn static inline void xv::net::UdpSocket::DeliveryFlow::claim(Context &c) noexcept
@brief 记录本次报文或对象已由当前服务接管处理；所属流程为 xv::net::UdpSocket::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e8b7476f4ad730e9"></a>

## `xv::net::UdpSocket::DeliveryFlow::matches`

```doxygen
@fn static inline bool xv::net::UdpSocket::DeliveryFlow::matches(Context &c) noexcept
@brief 检查对象的端点、接口或业务标识是否匹配；所属流程为 xv::net::UdpSocket::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ae0692e75b3a3309"></a>

## `xv::net::UdpSocket::DeliveryFlow::queue`

```doxygen
@fn static inline void xv::net::UdpSocket::DeliveryFlow::queue(Context &c) noexcept
@brief 将本次独占报文或值快照加入有界队列；所属流程为 xv::net::UdpSocket::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2943236044e5f2f8"></a>

## `xv::net::UdpSocket::DeliveryFlow::space`

```doxygen
@fn static inline bool xv::net::UdpSocket::DeliveryFlow::space(Context &c) noexcept
@brief 检查队列或发送缓冲是否仍有容量；所属流程为 xv::net::UdpSocket::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
