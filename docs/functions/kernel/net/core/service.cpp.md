# `kernel/net/core/service.cpp`

## 函数导航

- [`xv::net::initialize`](service.cpp.md#fn-cdd3b17385336e40)
- [`xv::net::interfaces`](service.cpp.md#fn-850f3daf0de0f624)
- [`xv::net::routing_table`](service.cpp.md#fn-d6092edb2c0b1b23)
- [`xv::net::socket_registry`](service.cpp.md#fn-a6dd2cdb97db1848)
- [`xv::net::tcp_protocol`](service.cpp.md#fn-955e03b182fca925)
- [`xv::net::timer_tick`](service.cpp.md#fn-cbf196a5e94594a2)
- [`xv::net::udp_protocol`](service.cpp.md#fn-75964e7f4e71a814)
- [`xv::net::qemu_defaults`](service.cpp.md#fn-2dfba446a5a4cb8b)

<a id="fn-75964e7f4e71a814"></a>

## `xv::net::udp_protocol`

```doxygen
@fn UdpProtocol &xv::net::udp_protocol()
@brief 借用已挂接 IPv4 protocol17 的唯一 UdpProtocol。。
@details 借用已挂接 IPv4 protocol17 的唯一 UdpProtocol。
```

<a id="fn-a6dd2cdb97db1848"></a>

## `xv::net::socket_registry`

```doxygen
@fn SocketRegistry &xv::net::socket_registry()
@brief 借用启动时静态构造的唯一 SocketRegistry。。
@details 借用启动时静态构造的唯一 SocketRegistry。
```

<a id="fn-955e03b182fca925"></a>

## `xv::net::tcp_protocol`

```doxygen
@fn TcpProtocol &xv::net::tcp_protocol()
@brief 借用已挂接 IPv4 protocol6 的唯一 TcpProtocol。。
@details 借用已挂接 IPv4 protocol6 的唯一 TcpProtocol。
```

<a id="fn-2dfba446a5a4cb8b"></a>

## `xv::net::qemu_defaults`

```doxygen
@fn bool xv::net::qemu_defaults()
@brief 仅作为默认 QEMU 启动策略，按发现顺序为 eth0..3 配置 10.0.2..5.15/24、对应 .2 网关、本机回环主机路由及递增默认路由 metric。
```

<a id="fn-850f3daf0de0f624"></a>

## `xv::net::interfaces`

```doxygen
@fn const InterfaceRegistry &xv::net::interfaces()
@brief 借用已组装的只读接口注册表；运行期配置已经封存。
```

<a id="fn-d6092edb2c0b1b23"></a>

## `xv::net::routing_table`

```doxygen
@fn const RoutingTable &xv::net::routing_table()
@brief 借用只读静态路由服务，用于按目标查询实际出口和源地址。
```

<a id="fn-cdd3b17385336e40"></a>

## `xv::net::initialize`

```doxygen
@fn void xv::net::initialize(NetworkSetup *setup)
@brief 一次性组装单份 IP/TCP/UDP/socket 栈、回环及发现的物理接口；调用自定义 NetworkSetup 或默认 QEMU 策略，然后核对地址冲突并封存。
@details 配置失败终止启动。
```

<a id="fn-cbf196a5e94594a2"></a>

## `xv::net::timer_tick`

```doxygen
@fn void xv::net::timer_tick()
@brief 使用同一单调时刻依次服务接口、IPv4 重组和 socket 定时器，再排空本轮生成的回环包；由 hart0 时钟服务调用。
```
