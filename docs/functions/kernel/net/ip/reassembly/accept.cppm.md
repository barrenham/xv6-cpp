# `kernel/net/ip/reassembly/accept.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::Ipv4Reassembly::AcceptFlow::assemble`](accept.cppm.md#fn-a8b60218cd268904)
- [`xv::net::Ipv4Reassembly::AcceptFlow::complete`](accept.cppm.md#fn-1f50173eebb2aafd)
- [`xv::net::Ipv4Reassembly::AcceptFlow::detach`](accept.cppm.md#fn-81bf8dd46b90926a)
- [`xv::net::Ipv4Reassembly::AcceptFlow::detached`](accept.cppm.md#fn-2086398444dad265)
- [`xv::net::Ipv4Reassembly::AcceptFlow::discard`](accept.cppm.md#fn-a28a333de2d4d70e)
- [`xv::net::Ipv4Reassembly::AcceptFlow::found`](accept.cppm.md#fn-e878680017d82d99)
- [`xv::net::Ipv4Reassembly::AcceptFlow::insert`](accept.cppm.md#fn-7d9e60e7067dd507)
- [`xv::net::Ipv4Reassembly::AcceptFlow::inspect`](accept.cppm.md#fn-5dbc04000a66b27b)
- [`xv::net::Ipv4Reassembly::AcceptFlow::locate`](accept.cppm.md#fn-7fc4602197f9b68b)
- [`xv::net::Ipv4Reassembly::AcceptFlow::repair`](accept.cppm.md#fn-102e7c466ee7d0c1)
- [`xv::net::Ipv4Reassembly::AcceptFlow::valid_fragment`](accept.cppm.md#fn-a7de8cd0848860cd)
- [`xv::net::Ipv4Reassembly::AcceptFlow::valid_input`](accept.cppm.md#fn-cf57f8d3819f9c15)
- [`xv::net::Ipv4Reassembly::AcceptFlow::waiting`](accept.cppm.md#fn-7125e4c687b92804)

<a id="fn-a8b60218cd268904"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::assemble`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::assemble(Context &c) noexcept
@brief 在重组锁下执行槽选择和片段插入图；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-1f50173eebb2aafd"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::complete`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-81bf8dd46b90926a"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::detach`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::detach(Context &c) noexcept
@brief 从重组槽分离完成或无效报文并清空槽；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2086398444dad265"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::detached`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::detached(Context &c) noexcept
@brief 检查是否有报文页从重组槽分离；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a28a333de2d4d70e"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::discard`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::discard(Context &c) noexcept
@brief 析构被拒绝的独占报文并清空上下文所有权；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e878680017d82d99"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::found`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7d9e60e7067dd507"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::insert`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::insert(Context &c) noexcept
@brief 向重组槽或规则链插入已验证数据；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5dbc04000a66b27b"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::inspect`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7fc4602197f9b68b"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::locate`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::locate(Context &c) noexcept
@brief 查找地址位置或选择重组槽；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-102e7c466ee7d0c1"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::repair`

```doxygen
@fn static inline void xv::net::Ipv4Reassembly::AcceptFlow::repair(Context &c) noexcept
@brief 修复重组首部或快速恢复中的窗口状态；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a7de8cd0848860cd"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::valid_fragment`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::valid_fragment(Context &c) noexcept
@brief 检查分片长度、八字节对齐与最大重组容量；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf57f8d3819f9c15"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::valid_input`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::valid_input(Context &c) noexcept
@brief 检查重组入口 Packet 与数据报范围；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7125e4c687b92804"></a>

## `xv::net::Ipv4Reassembly::AcceptFlow::waiting`

```doxygen
@fn static inline bool xv::net::Ipv4Reassembly::AcceptFlow::waiting(Context &c) noexcept
@brief 检查主动连接是否仍在握手等待；所属流程为 xv::net::Ipv4Reassembly::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
