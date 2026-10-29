# `kernel/net/dhcp/flows/command.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::DhcpManager::CommandFlow::already`](command.cppm.md#fn-b399f8a5e3722e06)
- [`xv::net::DhcpManager::CommandFlow::busy`](command.cppm.md#fn-1d0df963d06c3402)
- [`xv::net::DhcpManager::CommandFlow::cleanup`](command.cppm.md#fn-9c3a08ecd7df62ad)
- [`xv::net::DhcpManager::CommandFlow::clock`](command.cppm.md#fn-a3d6c773086f4137)
- [`xv::net::DhcpManager::CommandFlow::complete`](command.cppm.md#fn-65b08ad9eabe2846)
- [`xv::net::DhcpManager::CommandFlow::configure`](command.cppm.md#fn-1740743f7934bbc3)
- [`xv::net::DhcpManager::CommandFlow::disable`](command.cppm.md#fn-f0493c73a40c6b03)
- [`xv::net::DhcpManager::CommandFlow::find_slot`](command.cppm.md#fn-89470e8827e07fdd)
- [`xv::net::DhcpManager::CommandFlow::found`](command.cppm.md#fn-df7f82367da836a4)
- [`xv::net::DhcpManager::CommandFlow::has_slot`](command.cppm.md#fn-d8ed3c9f0fac1478)
- [`xv::net::DhcpManager::CommandFlow::inactive`](command.cppm.md#fn-7a34fa9deacc3bb3)
- [`xv::net::DhcpManager::CommandFlow::invalid`](command.cppm.md#fn-c612f020bb5af99e)
- [`xv::net::DhcpManager::CommandFlow::invalidate`](command.cppm.md#fn-9e52c08f43b468e8)
- [`xv::net::DhcpManager::CommandFlow::lookup_client`](command.cppm.md#fn-84ac7559b3357198)
- [`xv::net::DhcpManager::CommandFlow::lookup_interface`](command.cppm.md#fn-065d2144147f7097)
- [`xv::net::DhcpManager::CommandFlow::mark_stopping`](command.cppm.md#fn-84e2d5f477e48048)
- [`xv::net::DhcpManager::CommandFlow::no_buffer`](command.cppm.md#fn-06a722adb734a47e)
- [`xv::net::DhcpManager::CommandFlow::no_device`](command.cppm.md#fn-8171974d51df422e)
- [`xv::net::DhcpManager::CommandFlow::not_connected`](command.cppm.md#fn-ca577b5661e82ce9)
- [`xv::net::DhcpManager::CommandFlow::operation`](command.cppm.md#fn-d9f65cfff03c7d2c)
- [`xv::net::DhcpManager::CommandFlow::ready`](command.cppm.md#fn-71c52b856604645d)
- [`xv::net::DhcpManager::CommandFlow::release`](command.cppm.md#fn-29899358502f4ad9)
- [`xv::net::DhcpManager::CommandFlow::renew`](command.cppm.md#fn-c7381865c4587350)
- [`xv::net::DhcpManager::CommandFlow::renewable`](command.cppm.md#fn-65316ae8be7e658f)
- [`xv::net::DhcpManager::CommandFlow::running`](command.cppm.md#fn-cf39082278df69e9)
- [`xv::net::DhcpManager::CommandFlow::send_release`](command.cppm.md#fn-b305df4256973e9c)
- [`xv::net::DhcpManager::CommandFlow::start`](command.cppm.md#fn-856130ff4a463a85)
- [`xv::net::DhcpManager::CommandFlow::status`](command.cppm.md#fn-0c12eddb89d8c696)
- [`xv::net::DhcpManager::CommandFlow::stopping`](command.cppm.md#fn-a4ef89ab5bba8c99)
- [`xv::net::DhcpManager::CommandFlow::supported`](command.cppm.md#fn-64382a5d6eb126f6)
- [`xv::net::DhcpManager::CommandFlow::unsupported`](command.cppm.md#fn-9d5bf3cb0711c0b9)
- [`xv::net::DhcpManager::CommandFlow::valid`](command.cppm.md#fn-b7bdfb64da184a16)
- [`xv::net::DhcpManager::CommandFlow::valid_mac`](command.cppm.md#fn-dfab753a30ae7950)

<a id="fn-b399f8a5e3722e06"></a>

## `xv::net::DhcpManager::CommandFlow::already`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::already(Context &c) noexcept
@brief 记录 DHCP 已经运行的错误；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1d0df963d06c3402"></a>

## `xv::net::DhcpManager::CommandFlow::busy`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9c3a08ecd7df62ad"></a>

## `xv::net::DhcpManager::CommandFlow::cleanup`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::cleanup(Context &c) noexcept
@brief 按客户端代次清理已停止的租约状态；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。
```

<a id="fn-a3d6c773086f4137"></a>

## `xv::net::DhcpManager::CommandFlow::clock`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::clock(Context &c) noexcept
@brief 读取并保存本次处理的时间快照；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-65b08ad9eabe2846"></a>

## `xv::net::DhcpManager::CommandFlow::complete`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1740743f7934bbc3"></a>

## `xv::net::DhcpManager::CommandFlow::configure`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::configure(Context &c) noexcept
@brief 执行启动配置或受保护的配置命令图；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 ConfigurationGraph，其 stop 结束该次内部 run。
```

<a id="fn-f0493c73a40c6b03"></a>

## `xv::net::DhcpManager::CommandFlow::disable`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::disable(Context &c) noexcept
@brief 关闭接口 DHCP 活动标志；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-89470e8827e07fdd"></a>

## `xv::net::DhcpManager::CommandFlow::find_slot`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::find_slot(Context &c) noexcept
@brief 为 DHCP 启动选择已有客户端或空闲槽；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-df7f82367da836a4"></a>

## `xv::net::DhcpManager::CommandFlow::found`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d8ed3c9f0fac1478"></a>

## `xv::net::DhcpManager::CommandFlow::has_slot`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::has_slot(Context &c) noexcept
@brief 检查 DHCP 客户端槽是否选择成功；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7a34fa9deacc3bb3"></a>

## `xv::net::DhcpManager::CommandFlow::inactive`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::inactive(Context &c) noexcept
@brief 检查 DHCP 客户端是否不存在或处于 OFF；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c612f020bb5af99e"></a>

## `xv::net::DhcpManager::CommandFlow::invalid`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9e52c08f43b468e8"></a>

## `xv::net::DhcpManager::CommandFlow::invalidate`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::invalidate(Context &c) noexcept
@brief 在相应锁外使接口的缓存状态失效；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-84ac7559b3357198"></a>

## `xv::net::DhcpManager::CommandFlow::lookup_client`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::lookup_client(Context &c) noexcept
@brief 查找接口对应的 DHCP 客户端；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-065d2144147f7097"></a>

## `xv::net::DhcpManager::CommandFlow::lookup_interface`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::lookup_interface(Context &c) noexcept
@brief 查找请求所指的接口对象；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-84e2d5f477e48048"></a>

## `xv::net::DhcpManager::CommandFlow::mark_stopping`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::mark_stopping(Context &c) noexcept
@brief 推进客户端代次并取得停止阶段的值快照；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-06a722adb734a47e"></a>

## `xv::net::DhcpManager::CommandFlow::no_buffer`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::no_buffer(Context &c) noexcept
@brief 记录本次分配或容量不足的负错误；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8171974d51df422e"></a>

## `xv::net::DhcpManager::CommandFlow::no_device`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::no_device(Context &c) noexcept
@brief 记录管理目标接口不存在的负 ENODEV；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ca577b5661e82ce9"></a>

## `xv::net::DhcpManager::CommandFlow::not_connected`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::not_connected(Context &c) noexcept
@brief 记录尚未连接的负 ENOTCONN；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-71c52b856604645d"></a>

## `xv::net::DhcpManager::CommandFlow::ready`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::ready(Context &c) noexcept
@brief 检查 DHCP 启动所需的接口状态；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-29899358502f4ad9"></a>

## `xv::net::DhcpManager::CommandFlow::release`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::release(Context &c) noexcept
@brief 检查停止的 DHCP 租约是否需要发送 Release；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c7381865c4587350"></a>

## `xv::net::DhcpManager::CommandFlow::renew`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::renew(Context &c) noexcept
@brief 将 DHCP 已绑定客户端转为续租并推进代次；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-65316ae8be7e658f"></a>

## `xv::net::DhcpManager::CommandFlow::renewable`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::renewable(Context &c) noexcept
@brief 检查客户端是否允许手工触发续租；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf39082278df69e9"></a>

## `xv::net::DhcpManager::CommandFlow::running`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::running(Context &c) noexcept
@brief 检查 DHCP 客户端是否已经运行；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b305df4256973e9c"></a>

## `xv::net::DhcpManager::CommandFlow::send_release`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::send_release(Context &c) noexcept
@brief 在配置锁外发送停止租约的 Release；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-856130ff4a463a85"></a>

## `xv::net::DhcpManager::CommandFlow::start`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::start(Context &c) noexcept
@brief 开始监听后的计时或开始 DHCP 选择；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0c12eddb89d8c696"></a>

## `xv::net::DhcpManager::CommandFlow::status`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::status(Context &c) noexcept
@brief 将 DHCP 客户端与租约时限序列化为查询结果；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a4ef89ab5bba8c99"></a>

## `xv::net::DhcpManager::CommandFlow::stopping`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::stopping(Context &c) noexcept
@brief 检查 DHCP 客户端是否已在停止过程中；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-64382a5d6eb126f6"></a>

## `xv::net::DhcpManager::CommandFlow::supported`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::supported(Context &c) noexcept
@brief 检查接口是否支持所请求的网络能力；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9d5bf3cb0711c0b9"></a>

## `xv::net::DhcpManager::CommandFlow::unsupported`

```doxygen
@fn static inline void xv::net::DhcpManager::CommandFlow::unsupported(Context &c) noexcept
@brief 记录操作不支持的业务错误；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b7bdfb64da184a16"></a>

## `xv::net::DhcpManager::CommandFlow::valid`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dfab753a30ae7950"></a>

## `xv::net::DhcpManager::CommandFlow::valid_mac`

```doxygen
@fn static inline bool xv::net::DhcpManager::CommandFlow::valid_mac(Context &c) noexcept
@brief 检查 DHCP 客户端 MAC 非零且为单播；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d9f65cfff03c7d2c"></a>

## `xv::net::DhcpManager::CommandFlow::operation`

```doxygen
@fn static bool xv::net::DhcpManager::CommandFlow::operation(Context &c) noexcept
@brief 检查命令编号是否等于模板指定编号；所属流程为 xv::net::DhcpManager::CommandFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
