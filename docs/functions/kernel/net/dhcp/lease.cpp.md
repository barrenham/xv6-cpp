# `kernel/net/dhcp/lease.cpp`

## 函数导航

- [`xv::net::LeaseConfiguration::install`](lease.cpp.md#fn-585913184002ca08)
- [`xv::net::LeaseConfiguration::remove`](lease.cpp.md#fn-02f977e5647590a0)

<a id="fn-585913184002ca08"></a>

## `xv::net::LeaseConfiguration::install`

```doxygen
@fn int xv::net::LeaseConfiguration::install(NetworkInterface &interface, const DhcpLease &lease, bool &changed)
@brief 在配置锁内原子安装或替换本接口租约所属地址与路由。
@details 预先核对地址冲突、接口状态及容量，失败不留下部分配置；相同配置续租不重排路由或增加代次。默认路由以租约地址为首选源，metric 为 100+接口 ID。
@param interface 已发布且启用、未作为桥成员的接口。
@param lease 已完成协议字段验证的租约。
@param[out] changed 是否改变实际地址或路由，供锁外 ARP 清理使用。
@return 成功零，冲突、容量或状态错误为负 errno。
```

<a id="fn-02f977e5647590a0"></a>

## `xv::net::LeaseConfiguration::remove`

```doxygen
@fn bool xv::net::LeaseConfiguration::remove(NetworkInterface &interface)
@brief 只撤销指定接口租约所有的地址和路由。
@details 保留手工别名及手工路由，包括引用已失效租约源地址的静态路由；此类路由由查询时的有效源检查暂时禁用。配置变化会更新代次。
@return 是否确实移除了配置。
@note 调用者解锁后再清理邻居，不在此取得 ARP 锁。
```
