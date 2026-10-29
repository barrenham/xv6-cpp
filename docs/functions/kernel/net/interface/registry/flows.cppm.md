# `kernel/net/interface/registry/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::InterfaceRegistry::RegistrationFlow::addresses_unique`](flows.cppm.md#fn-350a356d0b5f004d)
- [`xv::net::InterfaceRegistry::RegistrationFlow::advance`](flows.cppm.md#fn-7012389be16b0c5a)
- [`xv::net::InterfaceRegistry::RegistrationFlow::capacity`](flows.cppm.md#fn-379c300bad3b7fa5)
- [`xv::net::InterfaceRegistry::RegistrationFlow::exists`](flows.cppm.md#fn-695e0a4ea5e390b8)
- [`xv::net::InterfaceRegistry::RegistrationFlow::full`](flows.cppm.md#fn-f76f19f5417e58c4)
- [`xv::net::InterfaceRegistry::RegistrationFlow::identifier_unique`](flows.cppm.md#fn-a0b4bb2394e8fd33)
- [`xv::net::InterfaceRegistry::RegistrationFlow::invalid`](flows.cppm.md#fn-78e2b82102f715d0)
- [`xv::net::InterfaceRegistry::RegistrationFlow::more`](flows.cppm.md#fn-f7fb0310d575a8ac)
- [`xv::net::InterfaceRegistry::RegistrationFlow::name_unique`](flows.cppm.md#fn-b584db71e69828d5)
- [`xv::net::InterfaceRegistry::RegistrationFlow::valid`](flows.cppm.md#fn-4f9a03e5a3faed98)
- [`xv::net::NetworkInterface::DeliverFlow::count`](flows.cppm.md#fn-fe87458beaa6b281)
- [`xv::net::NetworkInterface::DeliverFlow::deliver`](flows.cppm.md#fn-5674d8c27f5fe77f)
- [`xv::net::NetworkInterface::DeliverFlow::drop`](flows.cppm.md#fn-8215cfad479c23a3)
- [`xv::net::NetworkInterface::DeliverFlow::eligible`](flows.cppm.md#fn-7b752eea161888bf)

<a id="fn-350a356d0b5f004d"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::addresses_unique`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::addresses_unique(Context &c) noexcept
@brief 检查地址是否与已有对象重复；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7012389be16b0c5a"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::advance`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::RegistrationFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-379c300bad3b7fa5"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::capacity`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-695e0a4ea5e390b8"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::exists`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::RegistrationFlow::exists(Context &c) noexcept
@brief 记录同名或同路径对象已存在的错误；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f76f19f5417e58c4"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::full`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::RegistrationFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a0b4bb2394e8fd33"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::identifier_unique`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::identifier_unique(Context &c) noexcept
@brief 检查接口数字标识是否未被登记；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-78e2b82102f715d0"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::invalid`

```doxygen
@fn static inline void xv::net::InterfaceRegistry::RegistrationFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f7fb0310d575a8ac"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::more`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b584db71e69828d5"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::name_unique`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::name_unique(Context &c) noexcept
@brief 检查接口名称是否与已有对象不同；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4f9a03e5a3faed98"></a>

## `xv::net::InterfaceRegistry::RegistrationFlow::valid`

```doxygen
@fn static inline bool xv::net::InterfaceRegistry::RegistrationFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::InterfaceRegistry::RegistrationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fe87458beaa6b281"></a>

## `xv::net::NetworkInterface::DeliverFlow::count`

```doxygen
@fn static inline void xv::net::NetworkInterface::DeliverFlow::count(Context &c) noexcept
@brief 更新当前报文或重复确认的计数；所属流程为 xv::net::NetworkInterface::DeliverFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5674d8c27f5fe77f"></a>

## `xv::net::NetworkInterface::DeliverFlow::deliver`

```doxygen
@fn static inline void xv::net::NetworkInterface::DeliverFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::NetworkInterface::DeliverFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8215cfad479c23a3"></a>

## `xv::net::NetworkInterface::DeliverFlow::drop`

```doxygen
@fn static inline void xv::net::NetworkInterface::DeliverFlow::drop(Context &c) noexcept
@brief 计入接口接收丢弃统计；所属流程为 xv::net::NetworkInterface::DeliverFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7b752eea161888bf"></a>

## `xv::net::NetworkInterface::DeliverFlow::eligible`

```doxygen
@fn static inline bool xv::net::NetworkInterface::DeliverFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::NetworkInterface::DeliverFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
