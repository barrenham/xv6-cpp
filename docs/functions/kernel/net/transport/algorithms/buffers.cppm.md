# `kernel/net/transport/algorithms/buffers.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpPageBuffer::InitializeFlow::advance`](buffers.cppm.md#fn-a85675829dc8e4cd)
- [`xv::net::TcpPageBuffer::InitializeFlow::complete`](buffers.cppm.md#fn-1a45cbfdf79f2c38)
- [`xv::net::TcpPageBuffer::InitializeFlow::ensure`](buffers.cppm.md#fn-2bf2085fed38702b)
- [`xv::net::TcpPageBuffer::InitializeFlow::more`](buffers.cppm.md#fn-55acd5cb5cd52c4b)
- [`xv::net::TcpReceiveBuffer::InsertFlow::advance_frontier`](buffers.cppm.md#fn-09d2c654bd5fdd8d)
- [`xv::net::TcpReceiveBuffer::InsertFlow::finished`](buffers.cppm.md#fn-91e612295ec5b859)
- [`xv::net::TcpReceiveBuffer::InsertFlow::insert_bytes`](buffers.cppm.md#fn-e6d8bfdbb6441a68)
- [`xv::net::TcpReceiveBuffer::InsertFlow::progress`](buffers.cppm.md#fn-45fdce0c3d1cb83d)
- [`xv::net::TcpReceiveBuffer::InsertFlow::record_fin`](buffers.cppm.md#fn-f2d2470dd0a6bc6f)
- [`xv::net::TcpReceiveBuffer::InsertFlow::snapshot`](buffers.cppm.md#fn-8018624bc9f03e5a)
- [`xv::net::TcpReceiveBuffer::InsertFlow::valid_fin`](buffers.cppm.md#fn-834ca735e2049937)
- [`xv::net::TcpSendBuffer::AppendFlow::clamp`](buffers.cppm.md#fn-b782fe2adcfa2691)
- [`xv::net::TcpSendBuffer::AppendFlow::copy`](buffers.cppm.md#fn-aae4e978f77ae4d5)

<a id="fn-a85675829dc8e4cd"></a>

## `xv::net::TcpPageBuffer::InitializeFlow::advance`

```doxygen
@fn static inline void xv::net::TcpPageBuffer::InitializeFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::TcpPageBuffer::InitializeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1a45cbfdf79f2c38"></a>

## `xv::net::TcpPageBuffer::InitializeFlow::complete`

```doxygen
@fn static inline void xv::net::TcpPageBuffer::InitializeFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::TcpPageBuffer::InitializeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2bf2085fed38702b"></a>

## `xv::net::TcpPageBuffer::InitializeFlow::ensure`

```doxygen
@fn static inline bool xv::net::TcpPageBuffer::InitializeFlow::ensure(Context &c) noexcept
@brief 按需为当前缓冲槽分配物理页；所属流程为 xv::net::TcpPageBuffer::InitializeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-55acd5cb5cd52c4b"></a>

## `xv::net::TcpPageBuffer::InitializeFlow::more`

```doxygen
@fn static inline bool xv::net::TcpPageBuffer::InitializeFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::TcpPageBuffer::InitializeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-09d2c654bd5fdd8d"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::advance_frontier`

```doxygen
@fn static inline void xv::net::TcpReceiveBuffer::InsertFlow::advance_frontier(Context &c) noexcept
@brief 按接收位图推进连续字节前沿并处理 FIN；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-91e612295ec5b859"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::finished`

```doxygen
@fn static inline bool xv::net::TcpReceiveBuffer::InsertFlow::finished(Context &c) noexcept
@brief 检查接收流是否已经结束；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e6d8bfdbb6441a68"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::insert_bytes`

```doxygen
@fn static inline void xv::net::TcpReceiveBuffer::InsertFlow::insert_bytes(Context &c) noexcept
@brief 按首次到达策略将窗口内字节写入接收位图与缓冲；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-45fdce0c3d1cb83d"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::progress`

```doxygen
@fn static inline void xv::net::TcpReceiveBuffer::InsertFlow::progress(Context &c) noexcept
@brief 生成本次连续接收前沿推进量；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f2d2470dd0a6bc6f"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::record_fin`

```doxygen
@fn static inline void xv::net::TcpReceiveBuffer::InsertFlow::record_fin(Context &c) noexcept
@brief 保存窗口内最早待完成的 FIN 序号；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8018624bc9f03e5a"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::snapshot`

```doxygen
@fn static inline void xv::net::TcpReceiveBuffer::InsertFlow::snapshot(Context &c) noexcept
@brief 保存锁内选定的客户端或缓冲边界值；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-834ca735e2049937"></a>

## `xv::net::TcpReceiveBuffer::InsertFlow::valid_fin`

```doxygen
@fn static inline bool xv::net::TcpReceiveBuffer::InsertFlow::valid_fin(Context &c) noexcept
@brief 检查 FIN 序号是否在接收窗口内且早于待处理 FIN；所属流程为 xv::net::TcpReceiveBuffer::InsertFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b782fe2adcfa2691"></a>

## `xv::net::TcpSendBuffer::AppendFlow::clamp`

```doxygen
@fn static inline void xv::net::TcpSendBuffer::AppendFlow::clamp(Context &c) noexcept
@brief 按本组件容量或协议上限约束数值；所属流程为 xv::net::TcpSendBuffer::AppendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aae4e978f77ae4d5"></a>

## `xv::net::TcpSendBuffer::AppendFlow::copy`

```doxygen
@fn static inline void xv::net::TcpSendBuffer::AppendFlow::copy(Context &c) noexcept
@brief 复制本阶段已限定长度的业务字节；所属流程为 xv::net::TcpSendBuffer::AppendFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
