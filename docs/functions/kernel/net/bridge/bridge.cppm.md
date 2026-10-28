# `kernel/net/bridge/bridge.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::BridgeInterface::BridgeInterface`](bridge.cppm.md#fn-d7fb2410e3f68a0f)
- [`xv::net::BridgeInterface::initialize`](bridge.cpp.md#fn-ce4d08412e7d6ce2)
- [`xv::net::BridgeInterface::is_bridge`](bridge.cppm.md#fn-e0025dc638c74095)
- [`xv::net::BridgeInterface::carrier`](bridge.cpp.md#fn-c626d20e219e7087)
- [`xv::net::BridgeInterface::tick`](bridge.cpp.md#fn-ba92da7fc5981118)
- [`xv::net::BridgeInterface::bridge_id`](bridge.cppm.md#fn-f761b66eda4d25ec)
- [`xv::net::BridgeInterface::online`](bridge.cppm.md#fn-b73b01c488013607)
- [`xv::net::BridgeInterface::mac_address`](bridge.cpp.md#fn-e2285e8b355a3998)
- [`xv::net::BridgeInterface::transmit`](bridge.cpp.md#fn-26b371580e62fdc8)
- [`xv::net::BridgeInterface::ingress`](bridge.cpp.md#fn-84c2d9daab914b31)
- [`xv::net::BridgeInterface::forward`](bridge.cpp.md#fn-87db7052d871e63a)
- [`xv::net::BridgeManager::BridgeManager`](bridge.cppm.md#fn-514e98d3cfd6a78e)
- [`xv::net::BridgeManager::initialize`](manager.cpp.md#fn-063d16e712668e1c)
- [`xv::net::BridgeManager::execute`](manager.cpp.md#fn-0497bea6af90ed3f)
- [`xv::net::BridgeManager::find`](manager.cpp.md#fn-d2dcd2b27caa0414)
- [`xv::net::BridgeManager::unused`](manager.cpp.md#fn-c333fba347c24bcd)
- [`xv::net::BridgeManager::membership`](manager.cpp.md#fn-9249cf80bd2b818c)

<a id="fn-d7fb2410e3f68a0f"></a>

## `xv::net::BridgeInterface::BridgeInterface`

```doxygen
@fn constexpr xv::net::BridgeInterface::BridgeInterface(unsigned id, const char *name)
@brief 构造固定身份、空成员表和六十四项空 FDB；MAC 初值是本地回退地址，启动 initialize 前由管理器按设备 MAC 派生。
```

<a id="fn-e0025dc638c74095"></a>

## `xv::net::BridgeInterface::is_bridge`

```doxygen
@fn bool xv::net::BridgeInterface::is_bridge() const override
@brief 返回 true，标识本机网桥接口，管理器据此禁止把桥作为其他桥的成员。
```

<a id="fn-f761b66eda4d25ec"></a>

## `xv::net::BridgeInterface::bridge_id`

```doxygen
@fn unsigned xv::net::BridgeInterface::bridge_id() const override
@brief 返回桥槽的固定 NetworkInterface ID，供二层成员查询 master；不获取额外锁或访问用户地址。
```

<a id="fn-b73b01c488013607"></a>

## `xv::net::BridgeInterface::online`

```doxygen
@fn bool xv::net::BridgeInterface::online() const override
@brief 在配置锁下返回桥槽已发布且管理上启用；实际有无可用成员由 carrier 独立报告。
```

<a id="fn-514e98d3cfd6a78e"></a>

## `xv::net::BridgeManager::BridgeManager`

```doxygen
@fn constexpr xv::net::BridgeManager::BridgeManager(InterfaceRegistry &interfaces, RoutingTable &routes, IpReceiver &ip)
@brief 借用接口注册表、静态路由服务和共享 IP 接收器，构造 br0/br1 两个长期存活槽及管理锁；依赖必须比管理器存活更久。
```
