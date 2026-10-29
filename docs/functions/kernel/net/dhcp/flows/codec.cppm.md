# `kernel/net/dhcp/flows/codec.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::DhcpCodec::DecodeFlow::describe`](codec.cppm.md#fn-cad3efc1319e0cd4)
- [`xv::net::DhcpCodec::DecodeFlow::file_options`](codec.cppm.md#fn-c94cb4ce55f7ec93)
- [`xv::net::DhcpCodec::DecodeFlow::file_overload`](codec.cppm.md#fn-bdcc357e25392d81)
- [`xv::net::DhcpCodec::DecodeFlow::header`](codec.cppm.md#fn-aacacdd2ce88fbbb)
- [`xv::net::DhcpCodec::DecodeFlow::main_options`](codec.cppm.md#fn-b67d0e34db5f9596)
- [`xv::net::DhcpCodec::DecodeFlow::reset`](codec.cppm.md#fn-4d218ceec317ccd5)
- [`xv::net::DhcpCodec::DecodeFlow::server_options`](codec.cppm.md#fn-e65344a4c501855a)
- [`xv::net::DhcpCodec::DecodeFlow::server_overload`](codec.cppm.md#fn-33be0010ec2e81a6)
- [`xv::net::DhcpCodec::DecodeFlow::success`](codec.cppm.md#fn-a66db185715c9321)
- [`xv::net::DhcpCodec::DecodeFlow::valid_reply`](codec.cppm.md#fn-d1b401e810840947)
- [`xv::net::DhcpCodec::EncodeFlow::capacity`](codec.cppm.md#fn-3acdcba45069eb72)
- [`xv::net::DhcpCodec::EncodeFlow::finish`](codec.cppm.md#fn-8bc5ce2110a243d7)
- [`xv::net::DhcpCodec::EncodeFlow::header`](codec.cppm.md#fn-e118f7cc29743e4a)
- [`xv::net::DhcpCodec::EncodeFlow::identity`](codec.cppm.md#fn-d53e8c1841e03b3a)
- [`xv::net::DhcpCodec::EncodeFlow::parameter_options`](codec.cppm.md#fn-5ec82c6ec50b8d21)
- [`xv::net::DhcpCodec::EncodeFlow::requested`](codec.cppm.md#fn-bdb875be2c9def0c)
- [`xv::net::DhcpCodec::EncodeFlow::requested_option`](codec.cppm.md#fn-4e28a7c33be15b56)
- [`xv::net::DhcpCodec::EncodeFlow::requests_configuration`](codec.cppm.md#fn-e5a6fcea48623d92)
- [`xv::net::DhcpCodec::EncodeFlow::server`](codec.cppm.md#fn-9e6f88c200c18f81)
- [`xv::net::DhcpCodec::EncodeFlow::server_option`](codec.cppm.md#fn-aeda02bd6ea933cb)
- [`xv::net::DhcpCodec::LeaseFlow::commit`](codec.cppm.md#fn-f435fb70b6991a0f)
- [`xv::net::DhcpCodec::LeaseFlow::gateway`](codec.cppm.md#fn-c7a6da845ad33e12)
- [`xv::net::DhcpCodec::LeaseFlow::host`](codec.cppm.md#fn-e4e69f9231ed86e4)
- [`xv::net::DhcpCodec::LeaseFlow::inspect`](codec.cppm.md#fn-91d7e8aee1d0bc80)
- [`xv::net::DhcpCodec::LeaseFlow::prefix`](codec.cppm.md#fn-939ea3b035d779b0)
- [`xv::net::DhcpCodec::LeaseFlow::valid_fields`](codec.cppm.md#fn-0a912571398a283a)

<a id="fn-cad3efc1319e0cd4"></a>

## `xv::net::DhcpCodec::DecodeFlow::describe`

```doxygen
@fn static inline void xv::net::DhcpCodec::DecodeFlow::describe(Context &c) noexcept
@brief 构造本阶段使用的具名值描述；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c94cb4ce55f7ec93"></a>

## `xv::net::DhcpCodec::DecodeFlow::file_options`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::file_options(Context &c) noexcept
@brief 解析 DHCP file 字段中复用的选项；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bdcc357e25392d81"></a>

## `xv::net::DhcpCodec::DecodeFlow::file_overload`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::file_overload(Context &c) noexcept
@brief 检查 DHCP 是否声明 file 字段选项复用；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aacacdd2ce88fbbb"></a>

## `xv::net::DhcpCodec::DecodeFlow::header`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::header(Context &c) noexcept
@brief 检查消息首部或编码固定消息首部；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b67d0e34db5f9596"></a>

## `xv::net::DhcpCodec::DecodeFlow::main_options`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::main_options(Context &c) noexcept
@brief 解析 DHCP 主选项区域；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4d218ceec317ccd5"></a>

## `xv::net::DhcpCodec::DecodeFlow::reset`

```doxygen
@fn static inline void xv::net::DhcpCodec::DecodeFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e65344a4c501855a"></a>

## `xv::net::DhcpCodec::DecodeFlow::server_options`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::server_options(Context &c) noexcept
@brief 解析 DHCP sname 字段中复用的选项；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-33be0010ec2e81a6"></a>

## `xv::net::DhcpCodec::DecodeFlow::server_overload`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::server_overload(Context &c) noexcept
@brief 检查 DHCP 是否声明 sname 字段选项复用；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a66db185715c9321"></a>

## `xv::net::DhcpCodec::DecodeFlow::success`

```doxygen
@fn static inline void xv::net::DhcpCodec::DecodeFlow::success(Context &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d1b401e810840947"></a>

## `xv::net::DhcpCodec::DecodeFlow::valid_reply`

```doxygen
@fn static inline bool xv::net::DhcpCodec::DecodeFlow::valid_reply(Context &c) noexcept
@brief 检查 DHCP 必需选项、服务器地址和回复类型；所属流程为 xv::net::DhcpCodec::DecodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3acdcba45069eb72"></a>

## `xv::net::DhcpCodec::EncodeFlow::capacity`

```doxygen
@fn static inline bool xv::net::DhcpCodec::EncodeFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8bc5ce2110a243d7"></a>

## `xv::net::DhcpCodec::EncodeFlow::finish`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::finish(Context &c) noexcept
@brief 提交 FIN 或完成当前业务阶段；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e118f7cc29743e4a"></a>

## `xv::net::DhcpCodec::EncodeFlow::header`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::header(Context &c) noexcept
@brief 检查消息首部或编码固定消息首部；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d53e8c1841e03b3a"></a>

## `xv::net::DhcpCodec::EncodeFlow::identity`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::identity(Context &c) noexcept
@brief 编码 DHCP 消息类型与客户端标识选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5ec82c6ec50b8d21"></a>

## `xv::net::DhcpCodec::EncodeFlow::parameter_options`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::parameter_options(Context &c) noexcept
@brief 编码参数请求列表与最大消息长度选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bdb875be2c9def0c"></a>

## `xv::net::DhcpCodec::EncodeFlow::requested`

```doxygen
@fn static inline bool xv::net::DhcpCodec::EncodeFlow::requested(Context &c) noexcept
@brief 检查 DHCP 请求是否携带请求地址；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4e28a7c33be15b56"></a>

## `xv::net::DhcpCodec::EncodeFlow::requested_option`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::requested_option(Context &c) noexcept
@brief 编码请求地址选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e5a6fcea48623d92"></a>

## `xv::net::DhcpCodec::EncodeFlow::requests_configuration`

```doxygen
@fn static inline bool xv::net::DhcpCodec::EncodeFlow::requests_configuration(Context &c) noexcept
@brief 检查 DHCP 消息是否应请求配置选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9e6f88c200c18f81"></a>

## `xv::net::DhcpCodec::EncodeFlow::server`

```doxygen
@fn static inline bool xv::net::DhcpCodec::EncodeFlow::server(Context &c) noexcept
@brief 检查是否应编码服务器标识选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-aeda02bd6ea933cb"></a>

## `xv::net::DhcpCodec::EncodeFlow::server_option`

```doxygen
@fn static inline void xv::net::DhcpCodec::EncodeFlow::server_option(Context &c) noexcept
@brief 编码 DHCP 服务器标识选项；所属流程为 xv::net::DhcpCodec::EncodeFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f435fb70b6991a0f"></a>

## `xv::net::DhcpCodec::LeaseFlow::commit`

```doxygen
@fn static inline void xv::net::DhcpCodec::LeaseFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c7a6da845ad33e12"></a>

## `xv::net::DhcpCodec::LeaseFlow::gateway`

```doxygen
@fn static inline bool xv::net::DhcpCodec::LeaseFlow::gateway(Context &c) noexcept
@brief 检查租约网关的地址和子网约束；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e4e69f9231ed86e4"></a>

## `xv::net::DhcpCodec::LeaseFlow::host`

```doxygen
@fn static inline bool xv::net::DhcpCodec::LeaseFlow::host(Context &c) noexcept
@brief 检查地址是否是允许的主机地址；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-91d7e8aee1d0bc80"></a>

## `xv::net::DhcpCodec::LeaseFlow::inspect`

```doxygen
@fn static inline void xv::net::DhcpCodec::LeaseFlow::inspect(Context &c) noexcept
@brief 读取本阶段后续条件需要的字段或快照；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-939ea3b035d779b0"></a>

## `xv::net::DhcpCodec::LeaseFlow::prefix`

```doxygen
@fn static inline void xv::net::DhcpCodec::LeaseFlow::prefix(Context &c) noexcept
@brief 根据连续掩码计算前缀长度；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0a912571398a283a"></a>

## `xv::net::DhcpCodec::LeaseFlow::valid_fields`

```doxygen
@fn static inline bool xv::net::DhcpCodec::LeaseFlow::valid_fields(Context &c) noexcept
@brief 检查租约必须字段、连续掩码和地址范围；所属流程为 xv::net::DhcpCodec::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
