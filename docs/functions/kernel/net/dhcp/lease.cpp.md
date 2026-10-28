# `kernel/net/dhcp/lease.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::LeaseConfiguration::install`](lease.cpp.md#fn-585913184002ca08)
- [`xv::net::LeaseConfiguration::remove`](lease.cpp.md#fn-02f977e5647590a0)

<a id="fn-585913184002ca08"></a>

## `xv::net::LeaseConfiguration::install`

```doxygen
@fn int xv::net::LeaseConfiguration::install(NetworkInterface &interface, const DhcpLease &lease, bool &changed)
@brief 校验租约地址、前缀和非零租期，将配置字段转换给 AddressConfiguration。
@details 租约默认路由 metric 为 100+接口 ID；地址冲突、容量预检及联合提交由共用地址事务服务完成。
@param[out] changed 配置内容是否实际变化；无效租约、失败及相同配置均为 false。
@return 成功零，租约无效 -EINVAL，其他失败为地址事务返回的负 errno。
```

<a id="fn-02f977e5647590a0"></a>

## `xv::net::LeaseConfiguration::remove`

```doxygen
@fn bool xv::net::LeaseConfiguration::remove(NetworkInterface &interface)
@brief 委托共用地址事务服务撤销指定接口租约配置。
@return 是否确实移除了地址或路由。
@note 邻居清理由调用方在配置锁外执行。
```
