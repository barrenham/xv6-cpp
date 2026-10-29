# `kernel/net/socket/tcp/incoming.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpSocket::IncomingFlow::acceptable`](incoming.cppm.md#fn-2b30f6c4010baa62)
- [`xv::net::TcpSocket::IncomingFlow::ack`](incoming.cppm.md#fn-81305e410890a935)
- [`xv::net::TcpSocket::IncomingFlow::acknowledge`](incoming.cppm.md#fn-2a4db99db41a49b5)
- [`xv::net::TcpSocket::IncomingFlow::acknowledges_syn`](incoming.cppm.md#fn-ebcd4e5ecc682265)
- [`xv::net::TcpSocket::IncomingFlow::activate_peer`](incoming.cppm.md#fn-d61a1d60f6cb3a0e)
- [`xv::net::TcpSocket::IncomingFlow::active_open`](incoming.cppm.md#fn-b94db57bdb3433de)
- [`xv::net::TcpSocket::IncomingFlow::apply_fin_state`](incoming.cppm.md#fn-aec7e777cf58224e)
- [`xv::net::TcpSocket::IncomingFlow::bad_syn_ack`](incoming.cppm.md#fn-b1e1c415ada27eae)
- [`xv::net::TcpSocket::IncomingFlow::closed`](incoming.cppm.md#fn-18f70d68a3cdab3f)
- [`xv::net::TcpSocket::IncomingFlow::connection_reset`](incoming.cppm.md#fn-8537b2099f4b868d)
- [`xv::net::TcpSocket::IncomingFlow::consume_syn`](incoming.cppm.md#fn-3a038605e828c135)
- [`xv::net::TcpSocket::IncomingFlow::decode`](incoming.cppm.md#fn-bd201c5f53294211)
- [`xv::net::TcpSocket::IncomingFlow::emit_ack`](incoming.cppm.md#fn-0b041d2a684fb269)
- [`xv::net::TcpSocket::IncomingFlow::empty_handshake`](incoming.cppm.md#fn-457117c8b6b74aa8)
- [`xv::net::TcpSocket::IncomingFlow::establish_active`](incoming.cppm.md#fn-8603450f946fbd6d)
- [`xv::net::TcpSocket::IncomingFlow::exact_reset`](incoming.cppm.md#fn-418041376f17bb68)
- [`xv::net::TcpSocket::IncomingFlow::future_ack`](incoming.cppm.md#fn-0126dfe46c817732)
- [`xv::net::TcpSocket::IncomingFlow::initialize_peer`](incoming.cppm.md#fn-d831f5693e4963d3)
- [`xv::net::TcpSocket::IncomingFlow::insert_payload`](incoming.cppm.md#fn-6d919aab64046837)
- [`xv::net::TcpSocket::IncomingFlow::new_fin`](incoming.cppm.md#fn-4cebe2f0a6fce718)
- [`xv::net::TcpSocket::IncomingFlow::notify`](incoming.cppm.md#fn-25a2ea31c242d490)
- [`xv::net::TcpSocket::IncomingFlow::notify_needed`](incoming.cppm.md#fn-d7147ab85d276fc7)
- [`xv::net::TcpSocket::IncomingFlow::parse_mss`](incoming.cppm.md#fn-15d082c027f80a24)
- [`xv::net::TcpSocket::IncomingFlow::pump`](incoming.cppm.md#fn-9e163e00a3846cd8)
- [`xv::net::TcpSocket::IncomingFlow::refresh_time_wait`](incoming.cppm.md#fn-07c3174b8c35ddeb)
- [`xv::net::TcpSocket::IncomingFlow::refused`](incoming.cppm.md#fn-ee4c09f2a0a47745)
- [`xv::net::TcpSocket::IncomingFlow::reject_syn_ack`](incoming.cppm.md#fn-fb6f39fe174ba1ec)
- [`xv::net::TcpSocket::IncomingFlow::repeated_syn`](incoming.cppm.md#fn-e995d811dfe34e00)
- [`xv::net::TcpSocket::IncomingFlow::reply_needed`](incoming.cppm.md#fn-b83fa2445dedb213)
- [`xv::net::TcpSocket::IncomingFlow::reset`](incoming.cppm.md#fn-2ea79b9d86b10652)
- [`xv::net::TcpSocket::IncomingFlow::retransmit`](incoming.cppm.md#fn-a3e3abc801f4e2be)
- [`xv::net::TcpSocket::IncomingFlow::simultaneous_open`](incoming.cppm.md#fn-3ad0eae86d6966d3)
- [`xv::net::TcpSocket::IncomingFlow::syn`](incoming.cppm.md#fn-d7653bd22bc1353a)
- [`xv::net::TcpSocket::IncomingFlow::time_wait`](incoming.cppm.md#fn-460c2cf8b501f19d)
- [`xv::net::TcpSocket::IncomingFlow::valid_final_ack`](incoming.cppm.md#fn-81686ebd7faf7675)
- [`xv::net::TcpSocket::IncomingFlow::valid_mss`](incoming.cppm.md#fn-a07540e7c4913d69)

<a id="fn-2b30f6c4010baa62"></a>

## `xv::net::TcpSocket::IncomingFlow::acceptable`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::acceptable(Context &c) noexcept
@brief 检查接收序号与占用长度是否落在窗口内；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-81305e410890a935"></a>

## `xv::net::TcpSocket::IncomingFlow::ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::ack(Context &c) noexcept
@brief 检查报文的 ACK 标志；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2a4db99db41a49b5"></a>

## `xv::net::TcpSocket::IncomingFlow::acknowledge`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::acknowledge(Context &c) noexcept
@brief 处理累计确认并更新确认相关状态；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ebcd4e5ecc682265"></a>

## `xv::net::TcpSocket::IncomingFlow::acknowledges_syn`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::acknowledges_syn(Context &c) noexcept
@brief 检查确认号是否等于本地发送前沿；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d61a1d60f6cb3a0e"></a>

## `xv::net::TcpSocket::IncomingFlow::activate_peer`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::activate_peer(Context &c) noexcept
@brief 处理窗口重开及握手转入已建立状态；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b94db57bdb3433de"></a>

## `xv::net::TcpSocket::IncomingFlow::active_open`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::active_open(Context &c) noexcept
@brief 检查是否处于主动握手等待状态；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aec7e777cf58224e"></a>

## `xv::net::TcpSocket::IncomingFlow::apply_fin_state`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::apply_fin_state(Context &c) noexcept
@brief 根据当前 TCP 状态应用新到达 FIN 的状态转换；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b1e1c415ada27eae"></a>

## `xv::net::TcpSocket::IncomingFlow::bad_syn_ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::bad_syn_ack(Context &c) noexcept
@brief 检查主动握手的 ACK 是否不匹配发送前沿；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-18f70d68a3cdab3f"></a>

## `xv::net::TcpSocket::IncomingFlow::closed`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::closed(Context &c) noexcept
@brief 检查连接是否已经关闭；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8537b2099f4b868d"></a>

## `xv::net::TcpSocket::IncomingFlow::connection_reset`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::connection_reset(Context &c) noexcept
@brief 按协议规则将连接复位为 ECONNRESET；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3a038605e828c135"></a>

## `xv::net::TcpSocket::IncomingFlow::consume_syn`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::consume_syn(Context &c) noexcept
@brief 消耗同时打开的 SYN 序号并去除 SYN 标记；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bd201c5f53294211"></a>

## `xv::net::TcpSocket::IncomingFlow::decode`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::decode(Context &c) noexcept
@brief 按已验证格式解码本阶段字段；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0b041d2a684fb269"></a>

## `xv::net::TcpSocket::IncomingFlow::emit_ack`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::emit_ack(Context &c) noexcept
@brief 发送当前连接的确认响应；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-457117c8b6b74aa8"></a>

## `xv::net::TcpSocket::IncomingFlow::empty_handshake`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::empty_handshake(Context &c) noexcept
@brief 检查握手之后是否没有载荷与 FIN；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8603450f946fbd6d"></a>

## `xv::net::TcpSocket::IncomingFlow::establish_active`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::establish_active(Context &c) noexcept
@brief 完成主动握手并消耗对端 SYN 序号；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-418041376f17bb68"></a>

## `xv::net::TcpSocket::IncomingFlow::exact_reset`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::exact_reset(Context &c) noexcept
@brief 检查 RST 序号是否精确等于接收前沿；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0126dfe46c817732"></a>

## `xv::net::TcpSocket::IncomingFlow::future_ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::future_ack(Context &c) noexcept
@brief 检查对端是否确认了尚未发送的数据；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d831f5693e4963d3"></a>

## `xv::net::TcpSocket::IncomingFlow::initialize_peer`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::initialize_peer(Context &c) noexcept
@brief 应用对端握手序号、窗口与 MSS；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6d919aab64046837"></a>

## `xv::net::TcpSocket::IncomingFlow::insert_payload`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::insert_payload(Context &c) noexcept
@brief 将已接受序号的载荷与 FIN 插入接收流；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4cebe2f0a6fce718"></a>

## `xv::net::TcpSocket::IncomingFlow::new_fin`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::new_fin(Context &c) noexcept
@brief 检查本次插入是否使接收流首次完成 FIN；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-25a2ea31c242d490"></a>

## `xv::net::TcpSocket::IncomingFlow::notify`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::notify(Context &c) noexcept
@brief 发布本阶段状态变化的等待唤醒；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d7147ab85d276fc7"></a>

## `xv::net::TcpSocket::IncomingFlow::notify_needed`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::notify_needed(Context &c) noexcept
@brief 检查接收进展或窗口变化是否需要通知；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-15d082c027f80a24"></a>

## `xv::net::TcpSocket::IncomingFlow::parse_mss`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::parse_mss(Context &c) noexcept
@brief 通过 TCP 选项解析取得对端 MSS；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9e163e00a3846cd8"></a>

## `xv::net::TcpSocket::IncomingFlow::pump`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::pump(Context &c) noexcept
@brief 按当前状态与窗口推进连接发送；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-07c3174b8c35ddeb"></a>

## `xv::net::TcpSocket::IncomingFlow::refresh_time_wait`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::refresh_time_wait(Context &c) noexcept
@brief 对重复 FIN 回 ACK 并刷新 TIME_WAIT；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ee4c09f2a0a47745"></a>

## `xv::net::TcpSocket::IncomingFlow::refused`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::refused(Context &c) noexcept
@brief 记录主动连接被拒绝的 ECONNREFUSED；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fb6f39fe174ba1ec"></a>

## `xv::net::TcpSocket::IncomingFlow::reject_syn_ack`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::reject_syn_ack(Context &c) noexcept
@brief 向主动握手中的错误 ACK 发送 RST；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e995d811dfe34e00"></a>

## `xv::net::TcpSocket::IncomingFlow::repeated_syn`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::repeated_syn(Context &c) noexcept
@brief 检查被动握手是否再次收到同序号 SYN；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b83fa2445dedb213"></a>

## `xv::net::TcpSocket::IncomingFlow::reply_needed`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::reply_needed(Context &c) noexcept
@brief 检查载荷、FIN 或同时打开是否需要 ACK；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2ea79b9d86b10652"></a>

## `xv::net::TcpSocket::IncomingFlow::reset`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a3e3abc801f4e2be"></a>

## `xv::net::TcpSocket::IncomingFlow::retransmit`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::retransmit(Context &c) noexcept
@brief 重发当前连接未确认的握手、数据或 FIN；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3ad0eae86d6966d3"></a>

## `xv::net::TcpSocket::IncomingFlow::simultaneous_open`

```doxygen
@fn static inline void xv::net::TcpSocket::IncomingFlow::simultaneous_open(Context &c) noexcept
@brief 转入同时打开的 SYN_RECEIVED 并重传握手；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d7653bd22bc1353a"></a>

## `xv::net::TcpSocket::IncomingFlow::syn`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::syn(Context &c) noexcept
@brief 检查报文或待重传内容是否为 SYN；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-460c2cf8b501f19d"></a>

## `xv::net::TcpSocket::IncomingFlow::time_wait`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::time_wait(Context &c) noexcept
@brief 检查连接是否处于 TIME_WAIT；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-81686ebd7faf7675"></a>

## `xv::net::TcpSocket::IncomingFlow::valid_final_ack`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::valid_final_ack(Context &c) noexcept
@brief 检查 SYN_RECEIVED 最终确认的序号和确认号；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a07540e7c4913d69"></a>

## `xv::net::TcpSocket::IncomingFlow::valid_mss`

```doxygen
@fn static inline bool xv::net::TcpSocket::IncomingFlow::valid_mss(Context &c) noexcept
@brief 检查解析得到的对端 MSS 是否非零；所属流程为 xv::net::TcpSocket::IncomingFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
