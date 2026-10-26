# `kernel/net/dhcp/client.cpp`

## 函数导航

- [`xv::net::DhcpManager::execute`](client.cpp.md#fn-d4e4da766bebabfb)
- [`xv::net::DhcpManager::find`](client.cpp.md#fn-8786da367c47a7df)
- [`xv::net::DhcpManager::initialize`](client.cpp.md#fn-19acae0e87259110)
- [`xv::net::DhcpManager::receive_bootstrap`](client.cpp.md#fn-46570f71ac652ac6)
- [`xv::net::DhcpManager::select`](client.cpp.md#fn-5497dbe215ae9a52)
- [`xv::net::DhcpManager::tick`](client.cpp.md#fn-40a1e0ee6745de59)
- [`xv::net::DhcpManager::transmit`](client.cpp.md#fn-f736e585f22f8be7)

<a id="fn-19acae0e87259110"></a>

## `xv::net::DhcpManager::initialize`

```doxygen
@fn void xv::net::DhcpManager::initialize()
@brief 启动期把本客户端注册为共享 IPv4 的启动协议接收器。
@details 不分配地址、不发送报文；各接口客户端由管理命令或默认启动策略单独启动。
```

<a id="fn-8786da367c47a7df"></a>

## `xv::net::DhcpManager::find`

```doxygen
@fn Client *xv::net::DhcpManager::find(NetworkInterface &interface)
@brief 按长期存活的接口对象查找其客户端槽。
@return 未登记返回 nullptr；调用者须持配置锁，不把返回指针交给异步代码。
```

<a id="fn-5497dbe215ae9a52"></a>

## `xv::net::DhcpManager::select`

```doxygen
@fn void xv::net::DhcpManager::select(Client &client,unsigned long now,unsigned delay = 0)
@brief 启动新的 Discover 事务并清空旧候选与租约计时。
@details 更新事务 ID 和串行号，重置重试次数，保留错误供查询；调用者必须先撤销旧租约配置。
@param now 单调毫秒。
@param delay 发送 Discover 前的等待毫秒。
@pre 已持配置锁。
```

<a id="fn-d4e4da766bebabfb"></a>

## `xv::net::DhcpManager::execute`

```doxygen
@fn int xv::net::DhcpManager::execute(unsigned operation,netctl_request &request)
@brief 处理 DHCP 启动、停止、状态和主动续租命令。
@details 启动要求已启用、无地址的以太或桥接口；停止先尽力发送 RELEASE，再只撤销租约所属配置，保留手工配置。接口被 DHCP 占用期间不能加入网桥。
@param operation NETCTL_DHCP_* 操作。
@param[in,out] request 固定版本管理结构，状态字段含阶段、地址、服务器、DNS、剩余租期及错误。
@return 状态成功 1，修改成功 0，失败负 errno。
@pre 不持配置或桥管理锁。
```

<a id="fn-f736e585f22f8be7"></a>

## `xv::net::DhcpManager::transmit`

```doxygen
@fn int xv::net::DhcpManager::transmit(const Client &snapshot,unsigned type = 0)
@brief 按客户端快照编码 Discover、Request、Decline 或 Release，并校验快照仍有效。
@details 初次获取和重新绑定使用指定接口广播入口，续租及 Release 使用正常选路并强制原接口。分配和发送不持配置锁，失败页面由 RAII 释放。
@param snapshot 客户端阶段、事务、MAC 和租约的值副本。
@param type 零按阶段选择消息，4 为 Decline，7 为 Release。
@return 零或负 errno。
```

<a id="fn-46570f71ac652ac6"></a>

## `xv::net::DhcpManager::receive_bootstrap`

```doxygen
@fn bool xv::net::DhcpManager::receive_bootstrap(NetworkInterface &interface,Packet &packet,const IpDatagram &datagram)
@brief 校验 DHCP UDP 回复并推进按接口隔离的事务状态机。
@details 核对端口、长度、可选 UDP 校验和、BOOTP 首部、MAC、事务 ID、目标和选中服务器；ACK 原子安装或续期，NAK 撤销租约并重新发现，本地地址冲突发送 Decline。
@param interface 实际三层入接口，桥上为 br0/br1。
@param packet 起始为 IP 首部的借用页面。
@param datagram 已通过 IPv4 基础校验的范围。
@return 已处理或丢弃当前 DHCP 回复为 true；不属于活动客户端为 false。
@note 不把服务器提供的配置直接写入未验证用户内存；解锁后才清空邻居或发送。
```

<a id="fn-40a1e0ee6745de59"></a>

## `xv::net::DhcpManager::tick`

```doxygen
@fn void xv::net::DhcpManager::tick(unsigned long now)
@brief 维护每接口的获取重试、T1 单播续租、T2 广播重新绑定和租约到期。
@details 首次事务最多六次退避发送，失败后等待六十秒重新尝试；链路不可用时暂停发送，但租约仍按时过期。无限租约不自动续租或过期。
@param now 单调毫秒，不依赖进程运行。
@note 配置锁内只改变状态和归属配置，解锁后分配/发送报文或清理 ARP。
```
