# `kernel/net/interface/configuration.cppm`

## 函数导航

- [`xv::net::configuration_lock`](configuration.cpp.md#fn-557bd6206ad9f26a)
- [`xv::net::ConfigurationGuard::ConfigurationGuard`](configuration.cppm.md#fn-ceb30f08fd73447e)
- [`xv::net::ConfigurationGuard::ConfigurationGuard`](configuration.cppm.md#fn-137bf23101d30d90)
- [`xv::net::ConfigurationGuard::operator=`](configuration.cppm.md#fn-35da480158e4f367)
- [`xv::net::ConfigurationGuard::~ConfigurationGuard`](configuration.cppm.md#fn-be2b3f12d7b2d373)

<a id="fn-ceb30f08fd73447e"></a>

## `xv::net::ConfigurationGuard::ConfigurationGuard`

```doxygen
@fn xv::net::ConfigurationGuard::ConfigurationGuard()
@brief 进入中断安全配置临界区；同一调用链已持锁时复用外层锁，只有最外层 guard 负责获取和释放。
@pre 临界区只读写配置，不进行协议回调或获取 ARP/设备锁。
```

<a id="fn-be2b3f12d7b2d373"></a>

## `xv::net::ConfigurationGuard::~ConfigurationGuard`

```doxygen
@fn xv::net::ConfigurationGuard::~ConfigurationGuard()
@brief 离开配置临界区，只有实际获取锁的最外层 guard 解锁并恢复其对应中断嵌套层级。
```

<a id="fn-137bf23101d30d90"></a>

## `xv::net::ConfigurationGuard::ConfigurationGuard`

```doxygen
@fn xv::net::ConfigurationGuard::ConfigurationGuard(const ConfigurationGuard &)
@brief 禁止复制或赋值配置作用域守卫，避免重复解锁或丢失最外层锁的释放责任。
```

<a id="fn-35da480158e4f367"></a>

## `xv::net::ConfigurationGuard::operator=`

```doxygen
@fn ConfigurationGuard &xv::net::ConfigurationGuard::operator=(const ConfigurationGuard &)
@brief 禁止复制或赋值配置作用域守卫，避免重复解锁或丢失最外层锁的释放责任。
```
