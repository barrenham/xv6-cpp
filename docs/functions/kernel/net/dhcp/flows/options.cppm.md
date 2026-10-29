# `kernel/net/dhcp/flows/options.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::DhcpCodec::OptionsFlow::advance`](options.cppm.md#fn-e89d14df128faf1a)
- [`xv::net::DhcpCodec::OptionsFlow::code`](options.cppm.md#fn-ace79744dad820db)
- [`xv::net::DhcpCodec::OptionsFlow::complete`](options.cppm.md#fn-15f212e109749d75)
- [`xv::net::DhcpCodec::OptionsFlow::decode`](options.cppm.md#fn-66cec6e0d6a57a13)
- [`xv::net::DhcpCodec::OptionsFlow::end`](options.cppm.md#fn-ab514604abb414de)
- [`xv::net::DhcpCodec::OptionsFlow::fits`](options.cppm.md#fn-5fc84044c7256a9f)
- [`xv::net::DhcpCodec::OptionsFlow::has_length`](options.cppm.md#fn-d31dc502decd5b09)
- [`xv::net::DhcpCodec::OptionsFlow::length`](options.cppm.md#fn-9761faad17dcf95f)
- [`xv::net::DhcpCodec::OptionsFlow::more`](options.cppm.md#fn-bc1f3f2b0af060de)
- [`xv::net::DhcpCodec::OptionsFlow::padding`](options.cppm.md#fn-c833c8b6c2c9ff9e)
- [`xv::net::DhcpCodec::OptionsFlow::unique`](options.cppm.md#fn-981f6c198fc25f33)
- [`xv::net::DhcpCodec::OptionsFlow::valid_option`](options.cppm.md#fn-c69bee12d42a157a)

<a id="fn-e89d14df128faf1a"></a>

## `xv::net::DhcpCodec::OptionsFlow::advance`

```doxygen
@fn static inline void xv::net::DhcpCodec::OptionsFlow::advance(Context &c) noexcept
@brief 推进本次扫描或发送的游标；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ace79744dad820db"></a>

## `xv::net::DhcpCodec::OptionsFlow::code`

```doxygen
@fn static inline void xv::net::DhcpCodec::OptionsFlow::code(Context &c) noexcept
@brief 读取一个 DHCP 选项编号并推进字节游标；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-15f212e109749d75"></a>

## `xv::net::DhcpCodec::OptionsFlow::complete`

```doxygen
@fn static inline void xv::net::DhcpCodec::OptionsFlow::complete(Context &c) noexcept
@brief 记录本阶段已完成或检查同步阶段是否结束；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-66cec6e0d6a57a13"></a>

## `xv::net::DhcpCodec::OptionsFlow::decode`

```doxygen
@fn static inline void xv::net::DhcpCodec::OptionsFlow::decode(Context &c) noexcept
@brief 按已验证格式解码本阶段字段；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ab514604abb414de"></a>

## `xv::net::DhcpCodec::OptionsFlow::end`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::end(Context &c) noexcept
@brief 检查选项迭代是否遇到结束标记；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5fc84044c7256a9f"></a>

## `xv::net::DhcpCodec::OptionsFlow::fits`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::fits(Context &c) noexcept
@brief 检查长度是否能放入当前协议或存储容量；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d31dc502decd5b09"></a>

## `xv::net::DhcpCodec::OptionsFlow::has_length`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::has_length(Context &c) noexcept
@brief 检查选项编号之后是否仍有长度字节；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9761faad17dcf95f"></a>

## `xv::net::DhcpCodec::OptionsFlow::length`

```doxygen
@fn static inline void xv::net::DhcpCodec::OptionsFlow::length(Context &c) noexcept
@brief 读取当前选项长度并推进字节游标；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bc1f3f2b0af060de"></a>

## `xv::net::DhcpCodec::OptionsFlow::more`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::more(Context &c) noexcept
@brief 检查当前有界扫描是否还有候选项；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c833c8b6c2c9ff9e"></a>

## `xv::net::DhcpCodec::OptionsFlow::padding`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::padding(Context &c) noexcept
@brief 检查当前选项是否为填充字节；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-981f6c198fc25f33"></a>

## `xv::net::DhcpCodec::OptionsFlow::unique`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::unique(Context &c) noexcept
@brief 检查当前对象、选项或地址没有冲突；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c69bee12d42a157a"></a>

## `xv::net::DhcpCodec::OptionsFlow::valid_option`

```doxygen
@fn static inline bool xv::net::DhcpCodec::OptionsFlow::valid_option(Context &c) noexcept
@brief 检查一个 DHCP 选项的类型长度与 overload 约束；所属流程为 xv::net::DhcpCodec::OptionsFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
