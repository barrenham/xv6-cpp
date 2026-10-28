# `kernel/net/interface/tunnel.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::IpInIpInterface::IpInIpInterface`](tunnel.cppm.md#fn-0f4903b461079f38)
- [`xv::net::IpInIpInterface::initialize`](tunnel.cpp.md#fn-414d9b2164bde204)
- [`xv::net::IpInIpInterface::accepts_destination`](tunnel.cppm.md#fn-981dbfd84fe417af)
- [`xv::net::IpInIpInterface::is_tunnel`](tunnel.cppm.md#fn-2fcbe5ac1b8a96b6)
- [`xv::net::IpInIpInterface::carrier`](tunnel.cppm.md#fn-a02d4d873452dbc8)
- [`xv::net::IpInIpInterface::transmit`](tunnel.cpp.md#fn-6e1ec61ddfefeb21)
- [`xv::net::IpInIpInterface::receive_outer`](tunnel.cpp.md#fn-9e6c6c02ccc93ae4)
- [`xv::net::IpInIpProtocol::add`](tunnel.cpp.md#fn-c1358ee430c56716)
- [`xv::net::IpInIpProtocol::receive`](tunnel.cpp.md#fn-0bbf7c9d366e4c64)
- [`xv::net::NetworkSetup::configure`](tunnel.cppm.md#fn-c156bec475eea836)
- [`xv::net::NetworkSetup::~NetworkSetup`](tunnel.cppm.md#fn-8608e7764fd35f85)

<a id="fn-0f4903b461079f38"></a>

## `xv::net::IpInIpInterface::IpInIpInterface`

```doxygen
@fn constexpr xv::net::IpInIpInterface::IpInIpInterface(unsigned id, const char *name, unsigned mtu = 1480)
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-981dbfd84fe417af"></a>

## `xv::net::IpInIpInterface::accepts_destination`

```doxygen
@fn bool xv::net::IpInIpInterface::accepts_destination(unsigned value, const InterfaceRegistry &registry) const override
@brief 允许内层目标为任一已启用本机地址；因此隧道自身可以不配置 IPv4 地址。
```

<a id="fn-2fcbe5ac1b8a96b6"></a>

## `xv::net::IpInIpInterface::is_tunnel`

```doxygen
@fn bool xv::net::IpInIpInterface::is_tunnel() const override
@brief 返回 true，向配置代码标识隧道能力。
```

<a id="fn-a02d4d873452dbc8"></a>

## `xv::net::IpInIpInterface::carrier`

```doxygen
@fn bool xv::net::IpInIpInterface::carrier() const override
@brief 仅在下层启用、仍拥有配置的外层本地地址且链路可用时返回 true；删除外层本地地址会使隧道选路失效。
```

<a id="fn-c156bec475eea836"></a>

## `xv::net::NetworkSetup::configure`

```doxygen
@fn bool xv::net::NetworkSetup::configure(InterfaceRegistry &interfaces, RoutingTable &routes, Ipv4Layer &ip, IpInIpProtocol &tunnels)
@brief 启动期自定义配置契约：为已发现的物理接口设置地址/路由，或注册长期存活的虚拟接口和隧道。
@details 返回 false 会终止启动；服务随后封存启动期 API，运行期地址和路由交 NetworkManager 管理。
@param interfaces 已含回环和发现的以太接口，可增加自有虚拟接口。
@param routes 启动期可写的路由表，回环前缀路由已存在。
@param ip 各接口共用的纯 IPv4 接收器和发送服务。
@param tunnels 协议号 4 的隧道分派器。
@return 完整配置成功返回 true，任何必需步骤失败返回 false。
@pre 仅启动期调用；回调内创建的已注册对象不能是栈上临时变量。
```

<a id="fn-8608e7764fd35f85"></a>

## `xv::net::NetworkSetup::~NetworkSetup`

```doxygen
@fn xv::net::NetworkSetup::~NetworkSetup()
@brief 结束接口基类生命周期；protected 非虚析构禁止通过接口 delete，具体对象由启动期长期存储管理。
```
