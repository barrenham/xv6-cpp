# `kernel/net/bridge/manager.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::BridgeManager::initialize`](manager.cpp.md#fn-063d16e712668e1c)
- [`xv::net::BridgeManager::find`](manager.cpp.md#fn-d2dcd2b27caa0414)
- [`xv::net::BridgeManager::unused`](manager.cpp.md#fn-c333fba347c24bcd)
- [`xv::net::BridgeManager::membership`](manager.cpp.md#fn-9249cf80bd2b818c)
- [`xv::net::BridgeManager::execute`](manager.cpp.md#fn-0497bea6af90ed3f)

<a id="fn-063d16e712668e1c"></a>

## `xv::net::BridgeManager::initialize`

```doxygen
@fn void xv::net::BridgeManager::initialize()
@brief 启动期初始化管理锁，从首个有效接口 MAC 派生各桥的稳定本地 MAC，然后初始化桥本机链路/ARP；不发布桥或自动加入成员。
```

<a id="fn-0497bea6af90ed3f"></a>

## `xv::net::BridgeManager::execute`

```doxygen
@fn int xv::net::BridgeManager::execute(unsigned operation, netctl_request &request)
@brief 校验桥管理请求，协调创建删除、成员关系、FDB 查询及清空。
@details 创建删除通过 InterfaceRegistry 发布或撤销稳定桥对象，不访问注册数组；active 与注册操作在同一配置锁内完成，成员驱动回调在锁外执行。
@return 查询成功一、枚举结束或修改成功零，失败负 errno。
```

<a id="fn-d2dcd2b27caa0414"></a>

## `xv::net::BridgeManager::find`

```doxygen
@fn BridgeInterface *xv::net::BridgeManager::find(unsigned id)
@brief 在固定槽中按 ID 查找已发布的桥；未创建或已注销返回 nullptr，调用者须持配置锁。
```

<a id="fn-c333fba347c24bcd"></a>

## `xv::net::BridgeManager::unused`

```doxygen
@fn bool xv::net::BridgeManager::unused(NetworkInterface &interface) const
@brief 判断接口是否没有地址、活动 DHCP 或路由引用，可用于加入或删除网桥。
@details 路由占用通过 RoutingTable::references 查询，不访问路由数组。
@pre 调用方已持配置锁。
```

<a id="fn-9249cf80bd2b818c"></a>

## `xv::net::BridgeManager::membership`

```doxygen
@fn int xv::net::BridgeManager::membership(BridgeInterface &bridge, NetworkInterface &interface, bool add)
@brief 在配置锁内协调网桥成员关系与接口拓扑代次，锁外切换驱动混杂模式和清理邻居。
@details 加入前后两次验证桥、端口和槽位状态；混杂模式失败或快照失效时保持未加入，并恢复混杂模式。退出清除成员与对应 FDB 记录，代次通过 topology_changed 推进。
```
