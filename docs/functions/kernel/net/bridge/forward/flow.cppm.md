# `kernel/net/bridge/forward/flow.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::BridgeInterface::ForwardFlow::advance`](flow.cppm.md#fn-9c9efe6d8cc9abef)
- [`xv::net::BridgeInterface::ForwardFlow::classify`](flow.cppm.md#fn-1e65ffa77f5b2420)
- [`xv::net::BridgeInterface::ForwardFlow::clone`](flow.cppm.md#fn-7eaf21bb36e057a0)
- [`xv::net::BridgeInterface::ForwardFlow::cloned`](flow.cppm.md#fn-245c27ad5e72b625)
- [`xv::net::BridgeInterface::ForwardFlow::collect`](flow.cppm.md#fn-07a7c3b8eef969ed)
- [`xv::net::BridgeInterface::ForwardFlow::deliver`](flow.cppm.md#fn-d3b20622fc3f9dbf)
- [`xv::net::BridgeInterface::ForwardFlow::find_ingress`](flow.cppm.md#fn-097180101e399567)
- [`xv::net::BridgeInterface::ForwardFlow::forwarded`](flow.cppm.md#fn-583f812dadddcffd)
- [`xv::net::BridgeInterface::ForwardFlow::inspect`](flow.cppm.md#fn-e86d9e7eedf2ce58)
- [`xv::net::BridgeInterface::ForwardFlow::learn`](flow.cppm.md#fn-164e4b4c828d38c2)
- [`xv::net::BridgeInterface::ForwardFlow::local`](flow.cppm.md#fn-7b4e7587adcc88cb)
- [`xv::net::BridgeInterface::ForwardFlow::more_targets`](flow.cppm.md#fn-8fc40f039e610fda)
- [`xv::net::BridgeInterface::ForwardFlow::online`](flow.cppm.md#fn-4e3c7a8d1836843a)
- [`xv::net::BridgeInterface::ForwardFlow::own_source`](flow.cppm.md#fn-d686b3118c13e40c)
- [`xv::net::BridgeInterface::ForwardFlow::release_copy`](flow.cppm.md#fn-6a4f5a51ec288250)
- [`xv::net::BridgeInterface::ForwardFlow::select_targets`](flow.cppm.md#fn-817b5b15535f4ce7)
- [`xv::net::BridgeInterface::ForwardFlow::selected`](flow.cppm.md#fn-e11723f3db9fcffb)
- [`xv::net::BridgeInterface::ForwardFlow::send`](flow.cppm.md#fn-8b73bb1135ee1a75)
- [`xv::net::BridgeInterface::ForwardFlow::still_valid`](flow.cppm.md#fn-87f2cb06502d7347)
- [`xv::net::BridgeInterface::ForwardFlow::valid_frame`](flow.cppm.md#fn-d15fbd4cfe157a32)
- [`xv::net::BridgeInterface::ForwardFlow::valid_ingress`](flow.cppm.md#fn-5e607ac423b98f32)
- [`xv::net::BridgeInterface::ForwardFlow::valid_size`](flow.cppm.md#fn-28166d66c8e52ef0)

<a id="fn-9c9efe6d8cc9abef"></a>

## `xv::net::BridgeInterface::ForwardFlow::advance`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e65ffa77f5b2420"></a>

## `xv::net::BridgeInterface::ForwardFlow::classify`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::classify(Context &c) noexcept
@brief 判断网桥目的类型并查找已学习出口；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7eaf21bb36e057a0"></a>

## `xv::net::BridgeInterface::ForwardFlow::clone`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::clone(Context &c) noexcept
@brief 为当前网桥出口克隆独占报文页；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-245c27ad5e72b625"></a>

## `xv::net::BridgeInterface::ForwardFlow::cloned`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::cloned(Context &c) noexcept
@brief 检查出口报文克隆是否成功；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-07a7c3b8eef969ed"></a>

## `xv::net::BridgeInterface::ForwardFlow::collect`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::collect(Context &c) noexcept
@brief 收集锁内选定的网桥出口快照；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d3b20622fc3f9dbf"></a>

## `xv::net::BridgeInterface::ForwardFlow::deliver`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::deliver(Context &c) noexcept
@brief 向对应接收器或应用队列交付借用报文；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-097180101e399567"></a>

## `xv::net::BridgeInterface::ForwardFlow::find_ingress`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::find_ingress(Context &c) noexcept
@brief 查找入口端口所属的网桥成员快照；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-583f812dadddcffd"></a>

## `xv::net::BridgeInterface::ForwardFlow::forwarded`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::forwarded(Context &c) noexcept
@brief 根据实际出口发送情况生成转发结果；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e86d9e7eedf2ce58"></a>

## `xv::net::BridgeInterface::ForwardFlow::inspect`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-164e4b4c828d38c2"></a>

## `xv::net::BridgeInterface::ForwardFlow::learn`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::learn(Context &c) noexcept
@brief 学习入口源 MAC 或 ARP 邻居及其有效期；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7b4e7587adcc88cb"></a>

## `xv::net::BridgeInterface::ForwardFlow::local`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::local(Context &c) noexcept
@brief 检查报文是否应向本地协议路径交付；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8fc40f039e610fda"></a>

## `xv::net::BridgeInterface::ForwardFlow::more_targets`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::more_targets(Context &c) noexcept
@brief 检查网桥是否还有待发送出口快照；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4e3c7a8d1836843a"></a>

## `xv::net::BridgeInterface::ForwardFlow::online`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::online(Context &c) noexcept
@brief 检查本阶段需要的链路和服务是否在线；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d686b3118c13e40c"></a>

## `xv::net::BridgeInterface::ForwardFlow::own_source`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::own_source(Context &c) noexcept
@brief 检查网桥入口源是否错误使用自身 MAC；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6a4f5a51ec288250"></a>

## `xv::net::BridgeInterface::ForwardFlow::release_copy`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::release_copy(Context &c) noexcept
@brief 在推进网桥出口之前析构当前克隆包；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 Packet 成员在已有存储中原位重建，页面所有权由成员析构维持。
```

<a id="fn-817b5b15535f4ce7"></a>

## `xv::net::BridgeInterface::ForwardFlow::select_targets`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::select_targets(Context &c) noexcept
@brief 在配置锁内选择网桥出口快照；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 SelectionGraph，其 stop 结束该次内部 run。
```

<a id="fn-e11723f3db9fcffb"></a>

## `xv::net::BridgeInterface::ForwardFlow::selected`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::selected(Context &c) noexcept
@brief 检查网桥出口选择阶段是否有效完成；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8b73bb1135ee1a75"></a>

## `xv::net::BridgeInterface::ForwardFlow::send`

```doxygen
@fn static inline void xv::net::BridgeInterface::ForwardFlow::send(Context &c) noexcept
@brief 通过本对象所属下层服务发送并保存业务结果；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-87f2cb06502d7347"></a>

## `xv::net::BridgeInterface::ForwardFlow::still_valid`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::still_valid(Context &c) noexcept
@brief 在锁外发送之前复查网桥出口身份与代次；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-d15fbd4cfe157a32"></a>

## `xv::net::BridgeInterface::ForwardFlow::valid_frame`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::valid_frame(Context &c) noexcept
@brief 检查网桥帧源、类型与保留目的地址；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5e607ac423b98f32"></a>

## `xv::net::BridgeInterface::ForwardFlow::valid_ingress`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::valid_ingress(Context &c) noexcept
@brief 按对象身份、活动状态和代次检查网桥入口；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-28166d66c8e52ef0"></a>

## `xv::net::BridgeInterface::ForwardFlow::valid_size`

```doxygen
@fn static inline bool xv::net::BridgeInterface::ForwardFlow::valid_size(Context &c) noexcept
@brief 检查当前帧有效性、最短长度与 MTU；所属流程为 xv::net::BridgeInterface::ForwardFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
