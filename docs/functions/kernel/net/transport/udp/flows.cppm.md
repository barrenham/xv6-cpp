# `kernel/net/transport/udp/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::UdpProtocol::ReceiveFlow::deliver`](flows.cppm.md#fn-1095fbda5721c4d3)
- [`xv::net::UdpProtocol::ReceiveFlow::handled`](flows.cppm.md#fn-922fa4c4bf5c4c69)
- [`xv::net::UdpProtocol::ReceiveFlow::has_header`](flows.cppm.md#fn-2632fb4ee9095f1f)
- [`xv::net::UdpProtocol::ReceiveFlow::inspect`](flows.cppm.md#fn-2c7f7d4f9c107abe)
- [`xv::net::UdpProtocol::ReceiveFlow::unreachable`](flows.cppm.md#fn-67b62501bba3702f)
- [`xv::net::UdpProtocol::ReceiveFlow::valid_checksum`](flows.cppm.md#fn-ca9b39ec3713294f)
- [`xv::net::UdpProtocol::ReceiveFlow::valid_header`](flows.cppm.md#fn-22d07bb5a3615238)
- [`xv::net::UdpProtocol::SendFlow::allocate`](flows.cppm.md#fn-ce3c5dd3b90dc11e)
- [`xv::net::UdpProtocol::SendFlow::allocated`](flows.cppm.md#fn-0af2bf2f1a2472db)
- [`xv::net::UdpProtocol::SendFlow::checksum`](flows.cppm.md#fn-e939cd88fd196070)
- [`xv::net::UdpProtocol::SendFlow::encode`](flows.cppm.md#fn-d7e3e19951127955)
- [`xv::net::UdpProtocol::SendFlow::fits`](flows.cppm.md#fn-8c13f7bad706c085)
- [`xv::net::UdpProtocol::SendFlow::has_source`](flows.cppm.md#fn-dfae4960d1d3c356)
- [`xv::net::UdpProtocol::SendFlow::no_buffer`](flows.cppm.md#fn-8860cfd4959bafd7)
- [`xv::net::UdpProtocol::SendFlow::select_source`](flows.cppm.md#fn-e54a16aea1be99aa)
- [`xv::net::UdpProtocol::SendFlow::send`](flows.cppm.md#fn-a7ea079006ab468a)
- [`xv::net::UdpProtocol::SendFlow::too_large`](flows.cppm.md#fn-0fcb2923c3939467)
- [`xv::net::UdpProtocol::SendFlow::unreachable`](flows.cppm.md#fn-ae109c709afe1852)

<a id="fn-1095fbda5721c4d3"></a>

## `xv::net::UdpProtocol::ReceiveFlow::deliver`

```doxygen
@fn static inline void xv::net::UdpProtocol::ReceiveFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-922fa4c4bf5c4c69"></a>

## `xv::net::UdpProtocol::ReceiveFlow::handled`

```doxygen
@fn static inline bool xv::net::UdpProtocol::ReceiveFlow::handled(Context &c) noexcept
@brief 检查接收器是否已经处理当前报文；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2632fb4ee9095f1f"></a>

## `xv::net::UdpProtocol::ReceiveFlow::has_header`

```doxygen
@fn static inline bool xv::net::UdpProtocol::ReceiveFlow::has_header(Context &c) noexcept
@brief 检查报文是否包含协议最短首部；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2c7f7d4f9c107abe"></a>

## `xv::net::UdpProtocol::ReceiveFlow::inspect`

```doxygen
@fn static inline void xv::net::UdpProtocol::ReceiveFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-67b62501bba3702f"></a>

## `xv::net::UdpProtocol::ReceiveFlow::unreachable`

```doxygen
@fn static inline void xv::net::UdpProtocol::ReceiveFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ca9b39ec3713294f"></a>

## `xv::net::UdpProtocol::ReceiveFlow::valid_checksum`

```doxygen
@fn static inline bool xv::net::UdpProtocol::ReceiveFlow::valid_checksum(Context &c) noexcept
@brief 检查协议规定的伪首部校验和；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-22d07bb5a3615238"></a>

## `xv::net::UdpProtocol::ReceiveFlow::valid_header`

```doxygen
@fn static inline bool xv::net::UdpProtocol::ReceiveFlow::valid_header(Context &c) noexcept
@brief 检查本协议固定首部与长度约束；所属流程为 xv::net::UdpProtocol::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ce3c5dd3b90dc11e"></a>

## `xv::net::UdpProtocol::SendFlow::allocate`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-0af2bf2f1a2472db"></a>

## `xv::net::UdpProtocol::SendFlow::allocated`

```doxygen
@fn static inline bool xv::net::UdpProtocol::SendFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e939cd88fd196070"></a>

## `xv::net::UdpProtocol::SendFlow::checksum`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::checksum(Context &c) noexcept
@brief 计算并写入报文校验和；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d7e3e19951127955"></a>

## `xv::net::UdpProtocol::SendFlow::encode`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::encode(Context &c) noexcept
@brief 按已验证的业务值编码报文字段；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8c13f7bad706c085"></a>

## `xv::net::UdpProtocol::SendFlow::fits`

```doxygen
@fn static inline bool xv::net::UdpProtocol::SendFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dfae4960d1d3c356"></a>

## `xv::net::UdpProtocol::SendFlow::has_source`

```doxygen
@fn static inline bool xv::net::UdpProtocol::SendFlow::has_source(Context &c) noexcept
@brief 检查已选路由是否有有效源地址；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8860cfd4959bafd7"></a>

## `xv::net::UdpProtocol::SendFlow::no_buffer`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::no_buffer(Context &c) noexcept
@brief 记录本次分配或容量不足的负错误；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e54a16aea1be99aa"></a>

## `xv::net::UdpProtocol::SendFlow::select_source`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::select_source(Context &c) noexcept
@brief 为候选路由或传输发送选择源地址；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a7ea079006ab468a"></a>

## `xv::net::UdpProtocol::SendFlow::send`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0fcb2923c3939467"></a>

## `xv::net::UdpProtocol::SendFlow::too_large`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::too_large(Context &c) noexcept
@brief 记录长度超限的负 EMSGSIZE；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ae109c709afe1852"></a>

## `xv::net::UdpProtocol::SendFlow::unreachable`

```doxygen
@fn static inline void xv::net::UdpProtocol::SendFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::UdpProtocol::SendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
