# `kernel/net/link/arp/request.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::ArpCache::RequestFlow::allocate`](request.cppm.md#fn-4bf19da863b27564)
- [`xv::net::ArpCache::RequestFlow::allocated`](request.cppm.md#fn-99ad0015ba387bf2)
- [`xv::net::ArpCache::RequestFlow::encode`](request.cppm.md#fn-2ce92a58a79cc122)
- [`xv::net::ArpCache::RequestFlow::routed`](request.cppm.md#fn-fcae7e1a874f3762)
- [`xv::net::ArpCache::RequestFlow::send`](request.cppm.md#fn-0f3910cab2a3f24a)
- [`xv::net::ArpCache::RequestFlow::source`](request.cppm.md#fn-0d44724da98ad150)

<a id="fn-4bf19da863b27564"></a>

## `xv::net::ArpCache::RequestFlow::allocate`

```doxygen
@fn static inline void xv::net::ArpCache::RequestFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-99ad0015ba387bf2"></a>

## `xv::net::ArpCache::RequestFlow::allocated`

```doxygen
@fn static inline bool xv::net::ArpCache::RequestFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2ce92a58a79cc122"></a>

## `xv::net::ArpCache::RequestFlow::encode`

```doxygen
@fn static inline void xv::net::ArpCache::RequestFlow::encode(Context &c) noexcept
@brief 按已验证的业务值编码报文字段；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fcae7e1a874f3762"></a>

## `xv::net::ArpCache::RequestFlow::routed`

```doxygen
@fn static inline bool xv::net::ArpCache::RequestFlow::routed(Context &c) noexcept
@brief 检查选路是否得到有效源地址；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0f3910cab2a3f24a"></a>

## `xv::net::ArpCache::RequestFlow::send`

```doxygen
@fn static inline void xv::net::ArpCache::RequestFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0d44724da98ad150"></a>

## `xv::net::ArpCache::RequestFlow::source`

```doxygen
@fn static inline void xv::net::ArpCache::RequestFlow::source(Context &c) noexcept
@brief 为 ARP 请求读取可用源地址或检查源选择结果；所属流程为 xv::net::ArpCache::RequestFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```
