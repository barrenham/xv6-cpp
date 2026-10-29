# `kernel/net/socket/registry/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::InetSocket::BindFlow::assign_source`](flows.cppm.md#fn-e6c55fd436d61c01)
- [`xv::net::InetSocket::BindFlow::auto_port`](flows.cppm.md#fn-b6f38232bcabab2f)
- [`xv::net::InetSocket::BindFlow::available`](flows.cppm.md#fn-b1d28a03ecf4ce03)
- [`xv::net::InetSocket::BindFlow::bind`](flows.cppm.md#fn-6396754b064e8171)
- [`xv::net::InetSocket::BindFlow::choose_port`](flows.cppm.md#fn-77fcd78200bcad14)
- [`xv::net::InetSocket::BindFlow::has_port`](flows.cppm.md#fn-406a86d78563f7ca)
- [`xv::net::InetSocket::BindFlow::invalid`](flows.cppm.md#fn-d488d12260511284)
- [`xv::net::InetSocket::BindFlow::routed`](flows.cppm.md#fn-ff71869f987f4a41)
- [`xv::net::InetSocket::BindFlow::select_source`](flows.cppm.md#fn-074483c510c70980)
- [`xv::net::InetSocket::BindFlow::unreachable`](flows.cppm.md#fn-ead6277794c83e38)
- [`xv::net::InetSocket::BindFlow::used`](flows.cppm.md#fn-6726a6bf2953c00b)
- [`xv::net::InetSocket::BindFlow::valid_bind`](flows.cppm.md#fn-8307f4a428170dca)
- [`xv::net::SocketRegistry::DeliveryFlow::advance`](flows.cppm.md#fn-6714060ab5135e33)
- [`xv::net::SocketRegistry::DeliveryFlow::allocate`](flows.cppm.md#fn-8231a8e1627ff935)
- [`xv::net::SocketRegistry::DeliveryFlow::allocated`](flows.cppm.md#fn-bb2a161bb01f2c67)
- [`xv::net::SocketRegistry::DeliveryFlow::backlog_space`](flows.cppm.md#fn-dfceae2f524f7a39)
- [`xv::net::SocketRegistry::DeliveryFlow::claim`](flows.cppm.md#fn-0e78b8848086f412)
- [`xv::net::SocketRegistry::DeliveryFlow::deliver`](flows.cppm.md#fn-576f2af49e3f6f51)
- [`xv::net::SocketRegistry::DeliveryFlow::inspect`](flows.cppm.md#fn-8a010459939d4344)
- [`xv::net::SocketRegistry::DeliveryFlow::listen_syn`](flows.cppm.md#fn-9373d5a954bad4e9)
- [`xv::net::SocketRegistry::DeliveryFlow::matches`](flows.cppm.md#fn-02cf56e2f1460fd1)
- [`xv::net::SocketRegistry::DeliveryFlow::more`](flows.cppm.md#fn-c5e5e2ddc63bd23a)
- [`xv::net::SocketRegistry::DeliveryFlow::passive`](flows.cppm.md#fn-3dfd978f591bf610)
- [`xv::net::SocketRegistry::DeliveryFlow::remember_listener`](flows.cppm.md#fn-9d2b39b088d2eef7)
- [`xv::net::SocketRegistry::DeliveryFlow::tcp`](flows.cppm.md#fn-b34751e913a14f35)

<a id="fn-e6c55fd436d61c01"></a>

## `xv::net::InetSocket::BindFlow::assign_source`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::assign_source(Context &c) noexcept
@brief 为 TCP 自动绑定保存已选择的源地址；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b6f38232bcabab2f"></a>

## `xv::net::InetSocket::BindFlow::auto_port`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::auto_port(Context &c) noexcept
@brief 为尚未绑定的 socket 按需分配临时本地端口；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b1d28a03ecf4ce03"></a>

## `xv::net::InetSocket::BindFlow::available`

```doxygen
@fn static inline bool xv::net::InetSocket::BindFlow::available(Context &c) noexcept
@brief 检查重传是否还有有效长度与 MSS；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6396754b064e8171"></a>

## `xv::net::InetSocket::BindFlow::bind`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::bind(Context &c) noexcept
@brief 完成绑定或进入租约已绑定状态；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-77fcd78200bcad14"></a>

## `xv::net::InetSocket::BindFlow::choose_port`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::choose_port(Context &c) noexcept
@brief 按需选择临时本地端口；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-406a86d78563f7ca"></a>

## `xv::net::InetSocket::BindFlow::has_port`

```doxygen
@fn static inline bool xv::net::InetSocket::BindFlow::has_port(Context &c) noexcept
@brief 检查本地端口是否已分配成功；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d488d12260511284"></a>

## `xv::net::InetSocket::BindFlow::invalid`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ff71869f987f4a41"></a>

## `xv::net::InetSocket::BindFlow::routed`

```doxygen
@fn static inline bool xv::net::InetSocket::BindFlow::routed(Context &c) noexcept
@brief 检查选路是否得到有效源地址；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-074483c510c70980"></a>

## `xv::net::InetSocket::BindFlow::select_source`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::select_source(Context &c) noexcept
@brief 为候选路由或传输发送选择源地址；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ead6277794c83e38"></a>

## `xv::net::InetSocket::BindFlow::unreachable`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6726a6bf2953c00b"></a>

## `xv::net::InetSocket::BindFlow::used`

```doxygen
@fn static inline void xv::net::InetSocket::BindFlow::used(Context &c) noexcept
@brief 记录本地端口已被占用的负 EADDRINUSE；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8307f4a428170dca"></a>

## `xv::net::InetSocket::BindFlow::valid_bind`

```doxygen
@fn static inline bool xv::net::InetSocket::BindFlow::valid_bind(Context &c) noexcept
@brief 检查本地绑定状态与指定地址归属；所属流程为 xv::net::InetSocket::BindFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6714060ab5135e33"></a>

## `xv::net::SocketRegistry::DeliveryFlow::advance`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8231a8e1627ff935"></a>

## `xv::net::SocketRegistry::DeliveryFlow::allocate`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bb2a161bb01f2c67"></a>

## `xv::net::SocketRegistry::DeliveryFlow::allocated`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dfceae2f524f7a39"></a>

## `xv::net::SocketRegistry::DeliveryFlow::backlog_space`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::backlog_space(Context &c) noexcept
@brief 检查监听待接受子连接是否未满；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0e78b8848086f412"></a>

## `xv::net::SocketRegistry::DeliveryFlow::claim`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::claim(Context &c) noexcept
@brief 记录本次报文或对象已由当前服务接管处理；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-576f2af49e3f6f51"></a>

## `xv::net::SocketRegistry::DeliveryFlow::deliver`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8a010459939d4344"></a>

## `xv::net::SocketRegistry::DeliveryFlow::inspect`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9373d5a954bad4e9"></a>

## `xv::net::SocketRegistry::DeliveryFlow::listen_syn`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::listen_syn(Context &c) noexcept
@brief 检查未匹配报文是否命中监听且为合法接入 SYN；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-02cf56e2f1460fd1"></a>

## `xv::net::SocketRegistry::DeliveryFlow::matches`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::matches(Context &c) noexcept
@brief 检查对象的端点、接口或业务标识是否匹配；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c5e5e2ddc63bd23a"></a>

## `xv::net::SocketRegistry::DeliveryFlow::more`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3dfd978f591bf610"></a>

## `xv::net::SocketRegistry::DeliveryFlow::passive`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::passive(Context &c) noexcept
@brief 初始化监听接入的被动子连接；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9d2b39b088d2eef7"></a>

## `xv::net::SocketRegistry::DeliveryFlow::remember_listener`

```doxygen
@fn static inline void xv::net::SocketRegistry::DeliveryFlow::remember_listener(Context &c) noexcept
@brief 在连接扫描中保留匹配的监听对象；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b34751e913a14f35"></a>

## `xv::net::SocketRegistry::DeliveryFlow::tcp`

```doxygen
@fn static inline bool xv::net::SocketRegistry::DeliveryFlow::tcp(Context &c) noexcept
@brief 检查当前协议或对象是否为 TCP；所属流程为 xv::net::SocketRegistry::DeliveryFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
