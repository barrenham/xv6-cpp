# `kernel/net/interface/address_configuration.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::AddressConfiguration::add_static`](address_configuration.cpp.md#fn-61f8d692dbb019bd)
- [`xv::net::AddressConfiguration::install_lease`](address_configuration.cpp.md#fn-ec6332fe77a9c444)
- [`xv::net::AddressConfiguration::remove_lease`](address_configuration.cpp.md#fn-c8c67bede08a808c)
- [`xv::net::AddressConfiguration::remove_static`](address_configuration.cpp.md#fn-a4bbc7a46ce7f664)

<a id="fn-61f8d692dbb019bd"></a>

## `xv::net::AddressConfiguration::add_static`

```doxygen
@fn int xv::net::AddressConfiguration::add_static(NetworkInterface &interface, Ipv4Address address)
@brief 在同一配置锁内校验并提交手工地址及本机、直连自动路由。
@details 核对注册对象身份、回环保护、桥成员、IPv4 字段、地址与路由容量、全局冲突和子网边界。提交时清除调用方传入的归属值，手工地址固定为 owner=0。失败无部分修改，成功推进接口代次。 实现通过 StaticFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 成功零，身份、字段、容量、冲突等错误为负 errno。
@note 不做邻居清理；调用方解锁后 invalidate。
```

<a id="fn-a4bbc7a46ce7f664"></a>

## `xv::net::AddressConfiguration::remove_static`

```doxygen
@fn int xv::net::AddressConfiguration::remove_static(NetworkInterface &interface, Ipv4Address address)
@brief 在同一配置锁内删除指定手工地址及相关自动、首选源路由。
@details 校验接口注册身份、回环保护、桥成员、地址字段及存在性，租约地址返回 -EBUSY。成功保持其他地址次序并推进接口代次。 实现通过 StaticFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 成功零，身份、字段、缺失或归属错误为负 errno。
@note 调用方在解锁后清理邻居和待发队列。
```

<a id="fn-ec6332fe77a9c444"></a>

## `xv::net::AddressConfiguration::install_lease`

```doxygen
@fn int xv::net::AddressConfiguration::install_lease(NetworkInterface &interface, Ipv4Address address, unsigned gateway, unsigned metric, bool &changed)
@brief 在同一配置锁内原子安装或替换接口租约所属地址与路由。
@details 核对注册身份、接口状态、全局与本接口冲突、可复用地址槽位，再委托路由表预检容量并整组替换路由。只有路由替换成功才提交地址并推进代次；相同配置不改变代次或路由顺序。归属固定为接口 ID。 实现通过 LeaseFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@param gateway 已由上层租约语义校验的可选网关。
@param metric 上层指定的默认路由优先级。
@param[out] changed 是否实际改变地址或路由，失败及相同配置均为 false。
@return 成功零，字段、接口状态、冲突或容量错误为负 errno。
@note 不依赖 DHCP 报文、租期或管理 ABI；解锁后由调用方处理邻居失效。
```

<a id="fn-c8c67bede08a808c"></a>

## `xv::net::AddressConfiguration::remove_lease`

```doxygen
@fn bool xv::net::AddressConfiguration::remove_lease(NetworkInterface &interface)
@brief 在同一配置锁内仅移除注册接口租约所属的地址及路由。
@details 按对象身份拒绝未注册接口；保留手工别名和引用旧源的手工路由，实际移除配置时推进接口代次。 实现通过 RemoveLeaseFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@return 是否确实移除了配置。
@note 调用方解锁后清理邻居，本层不调用驱动或发送报文。
```
