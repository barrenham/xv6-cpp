# `kernel/net/interface/configuration.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::configuration_lock`](configuration.cpp.md#fn-557bd6206ad9f26a)

<a id="fn-557bd6206ad9f26a"></a>

## `xv::net::configuration_lock`

```doxygen
@fn xv::sync::SpinLock &xv::net::configuration_lock()
@brief 借用全局配置自旋锁。
@details 只用于短暂读取或修改地址、状态和路由，不允许持锁睡眠、用户复制或调用协议/设备回调。
```
