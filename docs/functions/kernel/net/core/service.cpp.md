# `kernel/net/core/service.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::InitializationFlow::bridge_interfaces`](service.cpp.md#fn-762a41f6a26260a5)
- [`xv::net::InitializationFlow::configure`](service.cpp.md#fn-40aad9a6fc19f325)
- [`xv::net::InitializationFlow::configured`](service.cpp.md#fn-48073031ffeada95)
- [`xv::net::InitializationFlow::default_setup`](service.cpp.md#fn-307336f08f509955)
- [`xv::net::InitializationFlow::ethernet_interfaces`](service.cpp.md#fn-67c82bf374b1571b)
- [`xv::net::InitializationFlow::invalid`](service.cpp.md#fn-ba8502913941fd7e)
- [`xv::net::InitializationFlow::local_interface`](service.cpp.md#fn-c370332b923d1588)
- [`xv::net::InitializationFlow::protocols`](service.cpp.md#fn-22b654916d8d5aea)
- [`xv::net::InitializationFlow::seal_interfaces`](service.cpp.md#fn-fbbfa3716d5c4b80)
- [`xv::net::InitializationFlow::seal_routes`](service.cpp.md#fn-5c368b48d457ef5a)
- [`xv::net::InitializationFlow::services`](service.cpp.md#fn-796ceaae8a86a70e)
- [`xv::net::InitializationFlow::start_dhcp`](service.cpp.md#fn-e26abf97379ecc3c)
- [`xv::net::TimerFlow::connections`](service.cpp.md#fn-ddf004ccd2490338)
- [`xv::net::TimerFlow::fragments`](service.cpp.md#fn-ecc604aa1a466c6f)
- [`xv::net::TimerFlow::interfaces`](service.cpp.md#fn-def5f3a5210caf00)
- [`xv::net::TimerFlow::leases`](service.cpp.md#fn-88a8859ac892c768)
- [`xv::net::attach_protocols`](service.cpp.md#fn-59448ed4d2996526)
- [`xv::net::configure_qemu_defaults`](service.cpp.md#fn-7b70fb13df020590)
- [`xv::net::control`](service.cpp.md#fn-6f0e1f01771f2e2f)
- [`xv::net::firewall_control`](service.cpp.md#fn-5f3ce21ae0aed65e)
- [`xv::net::initialize`](service.cpp.md#fn-cdd3b17385336e40)
- [`xv::net::initialize_ethernet_interfaces`](service.cpp.md#fn-2fd9097bce7b5e94)
- [`xv::net::initialize_loopback`](service.cpp.md#fn-f11df498114b312b)
- [`xv::net::interfaces`](service.cpp.md#fn-850f3daf0de0f624)
- [`xv::net::routing_table`](service.cpp.md#fn-d6092edb2c0b1b23)
- [`xv::net::socket_registry`](service.cpp.md#fn-a6dd2cdb97db1848)
- [`xv::net::start_default_dhcp`](service.cpp.md#fn-5429b3c8b100b5b5)
- [`xv::net::timer_tick`](service.cpp.md#fn-cbf196a5e94594a2)

<a id="fn-7b70fb13df020590"></a>

## `xv::net::configure_qemu_defaults`

```doxygen
@fn bool xv::net::configure_qemu_defaults()
@brief 按构建策略配置默认 QEMU 网络。
@details DHCP 构建不预装地址；关闭 XV_NET_BOOT_DHCP 时，为各以太接口配置静态地址、回环主机路由、直连路由及默认网关。任一步失败返回 false。
```

<a id="fn-f11df498114b312b"></a>

## `xv::net::initialize_loopback`

```doxygen
@fn void xv::net::initialize_loopback()
@brief 配置回环地址和接收入口，并注册回环接口及 127/8 路由；注册失败终止启动。
```

<a id="fn-2fd9097bce7b5e94"></a>

## `xv::net::initialize_ethernet_interfaces`

```doxygen
@fn void xv::net::initialize_ethernet_interfaces()
@brief 按驱动发现次序初始化以太接口并接入 IPv4，注册失败终止启动。
@details 保持驱动、接收回调、协议入口与注册表的原有装配次序。
```

<a id="fn-59448ed4d2996526"></a>

## `xv::net::attach_protocols`

```doxygen
@fn void xv::net::attach_protocols()
@brief 挂接 IPv4 协议号处理器，并把同一 socket 注册表分别注入 UDP 数据报与 TCP 段接收入口。
@details TCP 编解码对象只借用 IPv4；注册表构造时已显式借用同一个 TCP 发送对象。装配在发布收发服务前完成。
```

<a id="fn-5429b3c8b100b5b5"></a>

## `xv::net::start_default_dhcp`

```doxygen
@fn void xv::net::start_default_dhcp()
@brief DHCP 构建中为各以太接口启动默认客户端；失败终止启动。
@details 仅在默认配置完成且接口、路由已封存后调用；非 DHCP 构建为空操作。
```

<a id="fn-5f3ce21ae0aed65e"></a>

## `xv::net::firewall_control`

```doxygen
@fn int xv::net::firewall_control(unsigned operation, firewall_request &request)
@brief 将防火墙管理请求交给启动期组装的唯一 Firewall 服务。
```

<a id="fn-6f0e1f01771f2e2f"></a>

## `xv::net::control`

```doxygen
@fn int xv::net::control(unsigned operation, netctl_request &request)
@brief 把已经复制到内核的管理请求交给静态 NetworkManager。
@details 查询返回一或枚举结束零，修改成功零，失败负 errno；供系统调用与原生契约共用。
@param operation NETCTL_* 操作号。
@param[in,out] request 已复制至内核的 128 字节请求或查询结果。
@return 查询 1/0、修改成功 0，失败负 errno。
@see docs/network-management.md
```

<a id="fn-850f3daf0de0f624"></a>

## `xv::net::interfaces`

```doxygen
@fn const InterfaceRegistry &xv::net::interfaces()
@brief 借用已组装的只读接口注册表视图；对象身份固定，运行期配置通过管理事务修改。
```

<a id="fn-d6092edb2c0b1b23"></a>

## `xv::net::routing_table`

```doxygen
@fn const RoutingTable &xv::net::routing_table()
@brief 借用只读静态路由服务，用于按目标查询实际出口和源地址。
```

<a id="fn-a6dd2cdb97db1848"></a>

## `xv::net::socket_registry`

```doxygen
@fn SocketRegistry &xv::net::socket_registry()
@brief 借用启动时静态构造的唯一 SocketRegistry。
@details 借用启动时静态构造的唯一 SocketRegistry。
```

<a id="fn-cdd3b17385336e40"></a>

## `xv::net::initialize`

```doxygen
@fn void xv::net::initialize(NetworkSetup *setup)
@brief 按服务初始化、接口注册、协议挂接、启动配置和封存阶段组装生产网络。
@details 在 IPv4 发布前初始化并注入唯一 Firewall；显式 NetworkSetup 替代默认 QEMU 配置。接口与路由封存成功后，仅默认启动策略启动 DHCP。配置错误终止启动。 实现通过 InitializationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-cbf196a5e94594a2"></a>

## `xv::net::timer_tick`

```doxygen
@fn void xv::net::timer_tick()
@brief 用同一单调时刻维护接口、IP 重组、DHCP 租约和 socket，再排空回环。
@details DHCP 检查续租、重新绑定、过期和重试；发送及 ARP 清理均发生在配置锁外。 实现通过 TimerFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-762a41f6a26260a5"></a>

## `xv::net::InitializationFlow::bridge_interfaces`

```doxygen
@fn static void xv::net::InitializationFlow::bridge_interfaces(Context &) noexcept
@brief 初始化静态组装的网桥接口；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-40aad9a6fc19f325"></a>

## `xv::net::InitializationFlow::configure`

```doxygen
@fn static void xv::net::InitializationFlow::configure(Context &c) noexcept
@brief 执行启动配置或受保护的配置命令图；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-48073031ffeada95"></a>

## `xv::net::InitializationFlow::configured`

```doxygen
@fn static bool xv::net::InitializationFlow::configured(Context &c) noexcept
@brief 检查启动配置结果或执行锁内配置阶段；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-307336f08f509955"></a>

## `xv::net::InitializationFlow::default_setup`

```doxygen
@fn static bool xv::net::InitializationFlow::default_setup(Context &c) noexcept
@brief 检查启动是否采用默认配置；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-67c82bf374b1571b"></a>

## `xv::net::InitializationFlow::ethernet_interfaces`

```doxygen
@fn static void xv::net::InitializationFlow::ethernet_interfaces(Context &) noexcept
@brief 初始化并登记已发现的以太网接口；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ba8502913941fd7e"></a>

## `xv::net::InitializationFlow::invalid`

```doxygen
@fn static void xv::net::InitializationFlow::invalid(Context &) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c370332b923d1588"></a>

## `xv::net::InitializationFlow::local_interface`

```doxygen
@fn static void xv::net::InitializationFlow::local_interface(Context &) noexcept
@brief 初始化回环接口及其路由；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-22b654916d8d5aea"></a>

## `xv::net::InitializationFlow::protocols`

```doxygen
@fn static void xv::net::InitializationFlow::protocols(Context &) noexcept
@brief 挂接静态组装的网络协议与接收契约；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fbbfa3716d5c4b80"></a>

## `xv::net::InitializationFlow::seal_interfaces`

```doxygen
@fn static bool xv::net::InitializationFlow::seal_interfaces(Context &) noexcept
@brief 检查配置唯一性并封存接口表；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5c368b48d457ef5a"></a>

## `xv::net::InitializationFlow::seal_routes`

```doxygen
@fn static void xv::net::InitializationFlow::seal_routes(Context &) noexcept
@brief 封存启动路由配置；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-796ceaae8a86a70e"></a>

## `xv::net::InitializationFlow::services`

```doxygen
@fn static void xv::net::InitializationFlow::services(Context &) noexcept
@brief 初始化静态组装的各网络服务；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e26abf97379ecc3c"></a>

## `xv::net::InitializationFlow::start_dhcp`

```doxygen
@fn static void xv::net::InitializationFlow::start_dhcp(Context &) noexcept
@brief 启动默认配置的 DHCP 客户端；所属流程为 xv::net::InitializationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ddf004ccd2490338"></a>

## `xv::net::TimerFlow::connections`

```doxygen
@fn static void xv::net::TimerFlow::connections(Context &c) noexcept
@brief 对所属 socket 注册表执行定时处理；所属流程为 xv::net::TimerFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ecc604aa1a466c6f"></a>

## `xv::net::TimerFlow::fragments`

```doxygen
@fn static void xv::net::TimerFlow::fragments(Context &c) noexcept
@brief 执行 IP 重组槽的到期处理；所属流程为 xv::net::TimerFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-def5f3a5210caf00"></a>

## `xv::net::TimerFlow::interfaces`

```doxygen
@fn static void xv::net::TimerFlow::interfaces(Context &c) noexcept
@brief 按同一时间快照处理所有接口的定时任务；所属流程为 xv::net::TimerFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-88a8859ac892c768"></a>

## `xv::net::TimerFlow::leases`

```doxygen
@fn static void xv::net::TimerFlow::leases(Context &c) noexcept
@brief 按同一时间快照处理 DHCP 客户端定时任务；所属流程为 xv::net::TimerFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
