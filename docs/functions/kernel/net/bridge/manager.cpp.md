# `kernel/net/bridge/manager.cpp`

## 函数导航

- [`xv::net::BridgeManager::execute`](manager.cpp.md#fn-0497bea6af90ed3f)
- [`xv::net::BridgeManager::find`](manager.cpp.md#fn-d2dcd2b27caa0414)
- [`xv::net::BridgeManager::initialize`](manager.cpp.md#fn-063d16e712668e1c)
- [`xv::net::BridgeManager::membership`](manager.cpp.md#fn-9249cf80bd2b818c)
- [`xv::net::BridgeManager::unused`](manager.cpp.md#fn-c333fba347c24bcd)

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
@brief 校验管理 ABI 后串行处理创建、注销、成员变更和 FDB 查询/清空。
@details 配置锁内只检查与发布，驱动混杂切换和 ARP 清理在锁外完成；查询记录先清零。
@param operation NETCTL_BRIDGE_* 操作号。
@param[in,out] request 已复制到内核的版本一结构；source 字段在成员/FDB 操作中为端口 ID。
@return 查询一条为 1，结束/修改成功为 0，失败为负 errno。
@pre 不持配置锁、驱动锁或其他桥管理锁。
@see docs/network-bridge.md
```

<a id="fn-d2dcd2b27caa0414"></a>

## `xv::net::BridgeManager::find`

```doxygen
@fn BridgeInterface *xv::net::BridgeManager::find(unsigned id)
@brief 在固定槽中按 ID 查找已发布的桥；未创建或已注销返回 nullptr，调用者须持配置锁。
```

<a id="fn-9249cf80bd2b818c"></a>

## `xv::net::BridgeManager::membership`

```doxygen
@fn int xv::net::BridgeManager::membership(BridgeInterface &bridge, NetworkInterface &interface, bool add)
@brief 在管理锁保护下接入或移出二层端口。
@details 加入先校验空 IP/路由与单一归属，解锁启用混杂后重新核对并发布代次；退出先撤销归属和 FDB，再锁外恢复普通接收。
@param bridge 已创建的桥槽。
@param interface 未带 IPv4/路由的二层端口所属接口。
@param add true 加入，false 移出。
@return 成功零；忙、不支持、容量不足或未找到返回负 errno。
@pre 已持桥管理锁，调用本函数时不持配置锁。
```
<a id="fn-c333fba347c24bcd"></a>

## `xv::net::BridgeManager::unused`

```doxygen
@fn bool xv::net::BridgeManager::unused(NetworkInterface &interface) const
@brief 检查接口没有地址、路由归属或运行中的 DHCP 客户端。
@details DHCP 尚未取得地址也会占用接口，必须先 stop 才能加入成员或删除桥。
@pre 已持配置锁。
```
