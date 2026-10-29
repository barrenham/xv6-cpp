# `kernel/net/dhcp/flows/receive.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::DhcpManager::ReceiveFlow::ack`](receive.cppm.md#fn-a2cadaf47c279cfe)
- [`xv::net::DhcpManager::ReceiveFlow::addressed`](receive.cppm.md#fn-d62efa0170078883)
- [`xv::net::DhcpManager::ReceiveFlow::apply`](receive.cppm.md#fn-f8ab4e8eb1097b7a)
- [`xv::net::DhcpManager::ReceiveFlow::bind`](receive.cppm.md#fn-c7a05d7035277f12)
- [`xv::net::DhcpManager::ReceiveFlow::changed`](receive.cppm.md#fn-fdfd65befd90852e)
- [`xv::net::DhcpManager::ReceiveFlow::claim`](receive.cppm.md#fn-ab062d7af399bad2)
- [`xv::net::DhcpManager::ReceiveFlow::classify_state`](receive.cppm.md#fn-3a20adba4489147a)
- [`xv::net::DhcpManager::ReceiveFlow::clock`](receive.cppm.md#fn-81f90aab195677ae)
- [`xv::net::DhcpManager::ReceiveFlow::correct_ports`](receive.cppm.md#fn-b2c885769d8eaddd)
- [`xv::net::DhcpManager::ReceiveFlow::correct_server`](receive.cppm.md#fn-b5665c0be1bde5b4)
- [`xv::net::DhcpManager::ReceiveFlow::current`](receive.cppm.md#fn-e74735f3be355618)
- [`xv::net::DhcpManager::ReceiveFlow::decline`](receive.cppm.md#fn-f100754f5fc25c33)
- [`xv::net::DhcpManager::ReceiveFlow::decode`](receive.cppm.md#fn-e6dc0ef959922909)
- [`xv::net::DhcpManager::ReceiveFlow::enabled_client`](receive.cppm.md#fn-d301022f9ff46c36)
- [`xv::net::DhcpManager::ReceiveFlow::expecting_reply`](receive.cppm.md#fn-05d272d26c537da5)
- [`xv::net::DhcpManager::ReceiveFlow::find_client`](receive.cppm.md#fn-1fec7b0873de69d5)
- [`xv::net::DhcpManager::ReceiveFlow::has_header`](receive.cppm.md#fn-ba8a1c5d9d7e06f0)
- [`xv::net::DhcpManager::ReceiveFlow::inherit_options`](receive.cppm.md#fn-6a4f346d7a87e619)
- [`xv::net::DhcpManager::ReceiveFlow::inspect`](receive.cppm.md#fn-3ac1dbc93f74bb6f)
- [`xv::net::DhcpManager::ReceiveFlow::install`](receive.cppm.md#fn-e5b2a7e25add6c99)
- [`xv::net::DhcpManager::ReceiveFlow::installed`](receive.cppm.md#fn-71b99ec88be13f95)
- [`xv::net::DhcpManager::ReceiveFlow::invalidate`](receive.cppm.md#fn-f3f14fe0f688af72)
- [`xv::net::DhcpManager::ReceiveFlow::nack`](receive.cppm.md#fn-bf6aef6d0ac60d21)
- [`xv::net::DhcpManager::ReceiveFlow::normalize_address`](receive.cppm.md#fn-4a2569ab9ec0f704)
- [`xv::net::DhcpManager::ReceiveFlow::offer`](receive.cppm.md#fn-93c1b8fab62962e1)
- [`xv::net::DhcpManager::ReceiveFlow::reject_lease`](receive.cppm.md#fn-5b19db91a67416d6)
- [`xv::net::DhcpManager::ReceiveFlow::request_offer`](receive.cppm.md#fn-84fd8f7cd36a9214)
- [`xv::net::DhcpManager::ReceiveFlow::rollback`](receive.cppm.md#fn-6b4f4a007471b0c0)
- [`xv::net::DhcpManager::ReceiveFlow::same_address`](receive.cppm.md#fn-d48feeb827403a4e)
- [`xv::net::DhcpManager::ReceiveFlow::send_decline`](receive.cppm.md#fn-36f79969efc141d6)
- [`xv::net::DhcpManager::ReceiveFlow::valid_lease`](receive.cppm.md#fn-23ca66ce058dbb3c)
- [`xv::net::DhcpManager::ReceiveFlow::valid_offer`](receive.cppm.md#fn-a0e940dcdc0ddd48)
- [`xv::net::DhcpManager::ReceiveFlow::valid_udp`](receive.cppm.md#fn-9ffc6358f7c528c0)

<a id="fn-a2cadaf47c279cfe"></a>

## `xv::net::DhcpManager::ReceiveFlow::ack`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::ack(Context &c) noexcept
@brief 检查报文的 ACK 标志；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d62efa0170078883"></a>

## `xv::net::DhcpManager::ReceiveFlow::addressed`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::addressed(Context &c) noexcept
@brief 检查报文目的是否匹配本次客户端或租约；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f8ab4e8eb1097b7a"></a>

## `xv::net::DhcpManager::ReceiveFlow::apply`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::apply(Context &c) noexcept
@brief 在配置锁内执行 DHCP 回复应用图；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 ApplyGraph，其 stop 结束该次内部 run。
```

<a id="fn-c7a05d7035277f12"></a>

## `xv::net::DhcpManager::ReceiveFlow::bind`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::bind(Context &c) noexcept
@brief 完成绑定或进入租约已绑定状态；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fdfd65befd90852e"></a>

## `xv::net::DhcpManager::ReceiveFlow::changed`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::changed(Context &c) noexcept
@brief 记录或查询本次业务状态变化；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ab062d7af399bad2"></a>

## `xv::net::DhcpManager::ReceiveFlow::claim`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::claim(Context &c) noexcept
@brief 记录本次报文或对象已由当前服务接管处理；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3a20adba4489147a"></a>

## `xv::net::DhcpManager::ReceiveFlow::classify_state`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::classify_state(Context &c) noexcept
@brief 记录客户端当前续租和重绑定阶段；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-81f90aab195677ae"></a>

## `xv::net::DhcpManager::ReceiveFlow::clock`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::clock(Context &c) noexcept
@brief 读取并保存本次处理的时间快照；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b2c885769d8eaddd"></a>

## `xv::net::DhcpManager::ReceiveFlow::correct_ports`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::correct_ports(Context &c) noexcept
@brief 检查 DHCP 服务器与客户端端口；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b5665c0be1bde5b4"></a>

## `xv::net::DhcpManager::ReceiveFlow::correct_server`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::correct_server(Context &c) noexcept
@brief 检查租约回复的服务器身份；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e74735f3be355618"></a>

## `xv::net::DhcpManager::ReceiveFlow::current`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::current(Context &c) noexcept
@brief 检查请求或快照是否仍是当前有效状态；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f100754f5fc25c33"></a>

## `xv::net::DhcpManager::ReceiveFlow::decline`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::decline(Context &c) noexcept
@brief 检查是否需要发送 DHCP Decline；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e6dc0ef959922909"></a>

## `xv::net::DhcpManager::ReceiveFlow::decode`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::decode(Context &c) noexcept
@brief 按已验证格式解码本阶段字段；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d301022f9ff46c36"></a>

## `xv::net::DhcpManager::ReceiveFlow::enabled_client`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::enabled_client(Context &c) noexcept
@brief 在配置锁下检查 DHCP 客户端是否启用；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-05d272d26c537da5"></a>

## `xv::net::DhcpManager::ReceiveFlow::expecting_reply`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::expecting_reply(Context &c) noexcept
@brief 检查 DHCP 客户端是否正在等待租约回复；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1fec7b0873de69d5"></a>

## `xv::net::DhcpManager::ReceiveFlow::find_client`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::find_client(Context &c) noexcept
@brief 按入口接口查找当前 DHCP 客户端；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ba8a1c5d9d7e06f0"></a>

## `xv::net::DhcpManager::ReceiveFlow::has_header`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::has_header(Context &c) noexcept
@brief 检查报文是否包含协议最短首部；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6a4f346d7a87e619"></a>

## `xv::net::DhcpManager::ReceiveFlow::inherit_options`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::inherit_options(Context &c) noexcept
@brief 在续租或重绑定回复中补全既有配置选项；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3ac1dbc93f74bb6f"></a>

## `xv::net::DhcpManager::ReceiveFlow::inspect`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e5b2a7e25add6c99"></a>

## `xv::net::DhcpManager::ReceiveFlow::install`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::install(Context &c) noexcept
@brief 安装已验证的租约配置或隧道登记记录；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-71b99ec88be13f95"></a>

## `xv::net::DhcpManager::ReceiveFlow::installed`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::installed(Context &c) noexcept
@brief 检查租约配置提交结果是否成功；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f3f14fe0f688af72"></a>

## `xv::net::DhcpManager::ReceiveFlow::invalidate`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bf6aef6d0ac60d21"></a>

## `xv::net::DhcpManager::ReceiveFlow::nack`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::nack(Context &c) noexcept
@brief 检查 DHCP 回复是否为 NAK；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4a2569ab9ec0f704"></a>

## `xv::net::DhcpManager::ReceiveFlow::normalize_address`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::normalize_address(Context &c) noexcept
@brief 在续租与重绑定 ACK 中补全省略的租约地址；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-93c1b8fab62962e1"></a>

## `xv::net::DhcpManager::ReceiveFlow::offer`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::offer(Context &c) noexcept
@brief 检查当前回复是否是选择阶段的 DHCP Offer；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5b19db91a67416d6"></a>

## `xv::net::DhcpManager::ReceiveFlow::reject_lease`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::reject_lease(Context &c) noexcept
@brief 移除被 NAK 拒绝的租约并重新开始选择；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-84fd8f7cd36a9214"></a>

## `xv::net::DhcpManager::ReceiveFlow::request_offer`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::request_offer(Context &c) noexcept
@brief 保存 Offer 并转入 DHCP Requesting；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6b4f4a007471b0c0"></a>

## `xv::net::DhcpManager::ReceiveFlow::rollback`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::rollback(Context &c) noexcept
@brief 撤销未能提交的成员或租约变更；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d48feeb827403a4e"></a>

## `xv::net::DhcpManager::ReceiveFlow::same_address`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::same_address(Context &c) noexcept
@brief 检查 ACK 的地址是否与所选租约一致；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-36f79969efc141d6"></a>

## `xv::net::DhcpManager::ReceiveFlow::send_decline`

```doxygen
@fn static inline void xv::net::DhcpManager::ReceiveFlow::send_decline(Context &c) noexcept
@brief 在配置锁外发送地址冲突 Decline；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-23ca66ce058dbb3c"></a>

## `xv::net::DhcpManager::ReceiveFlow::valid_lease`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::valid_lease(Context &c) noexcept
@brief 将回复转换为已经验证的租约值；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a0e940dcdc0ddd48"></a>

## `xv::net::DhcpManager::ReceiveFlow::valid_offer`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::valid_offer(Context &c) noexcept
@brief 检查 Offer 提供的地址是否可作为单播主机地址；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9ffc6358f7c528c0"></a>

## `xv::net::DhcpManager::ReceiveFlow::valid_udp`

```doxygen
@fn static inline bool xv::net::DhcpManager::ReceiveFlow::valid_udp(Context &c) noexcept
@brief 检查 DHCP UDP 长度与可选校验和；所属流程为 xv::net::DhcpManager::ReceiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
