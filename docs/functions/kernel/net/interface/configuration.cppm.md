# `kernel/net/interface/configuration.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::ConfigurationGuard::ConfigurationGuard`](configuration.cppm.md#fn-ceb30f08fd73447e)
- [`xv::net::ConfigurationGuard::ConfigurationGuard`](configuration.cppm.md#fn-137bf23101d30d90)
- [`xv::net::ConfigurationGuard::operator=`](configuration.cppm.md#fn-35da480158e4f367)
- [`xv::net::ConfigurationGuard::~ConfigurationGuard`](configuration.cppm.md#fn-be2b3f12d7b2d373)
- [`xv::net::configuration_lock`](configuration.cpp.md#fn-557bd6206ad9f26a)

<a id="fn-ceb30f08fd73447e"></a>

## `xv::net::ConfigurationGuard::ConfigurationGuard`

```doxygen
@fn xv::net::ConfigurationGuard::ConfigurationGuard()
@brief 先关闭本地中断固定 CPU 身份，再检查归属并进入配置临界区；已持锁时复用调用方的锁。
@details 每个 Guard 先配对取得一层关中断计数，然后调用 holding；只有检查为未持有配置锁时才 acquire。SpinLock 的获取与释放另有独立的关中断层级，因此未持锁入口等待其他 CPU 的锁时也不会发生归属检查期间的 CPU 迁移。
@pre 临界区只读写配置，不睡眠、不进行协议回调或获取 ARP/设备锁。Guard 必须在同一执行上下文按作用域析构，不得手工提前释放其取得的锁。
@note 支持中断原本开启、原本关闭以及调用方已持配置锁的入口；不改变调用方已有的锁或关中断层级。
```

<a id="fn-be2b3f12d7b2d373"></a>

## `xv::net::ConfigurationGuard::~ConfigurationGuard`

```doxygen
@fn xv::net::ConfigurationGuard::~ConfigurationGuard()
@brief 离开配置临界区；仅释放本 Guard 实际取得的锁，并始终归还本 Guard 自己的关中断计数。
@details acquired_ 为 true 时先 release 配置锁，由 SpinLock 归还其自己的关中断计数；随后本 Guard 无条件 pop_interrupts_off。嵌套或借用已有锁的 Guard 不解锁，也不会提前打开调用方的中断。
@post 退出时恢复构造前的中断嵌套深度；提前 return 同样经 RAII 执行本路径。
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
