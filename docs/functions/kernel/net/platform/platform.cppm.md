# `kernel/net/platform/platform.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::detail::allocate_page`](kernel_platform.cpp.md#fn-a39640ebe645b6ba)
- [`xv::net::detail::copy_in`](kernel_platform.cpp.md#fn-d96c58b6fbc16587)
- [`xv::net::detail::copy_out`](kernel_platform.cpp.md#fn-073e4679a830e329)
- [`xv::net::detail::fail`](kernel_platform.cpp.md#fn-5f2daef2aad08488)
- [`xv::net::detail::free_page`](kernel_platform.cpp.md#fn-9ef32a99d26fe40e)
- [`xv::net::detail::interrupted`](kernel_platform.cpp.md#fn-99f626cb5d17f655)
- [`xv::net::detail::milliseconds`](kernel_platform.cpp.md#fn-faf2eec702e52b21)
- [`xv::net::detail::prepare_sleep`](kernel_platform.cpp.md#fn-eeff613785c4e3e3)
- [`xv::net::detail::sleep`](kernel_platform.cpp.md#fn-b5609c0fae83d6c4)
- [`xv::net::detail::wake`](kernel_platform.cpp.md#fn-44622dca17b535c2)

<a id="fn-a7467d39781652f2"></a>

## `xv::net::detail::TimerChange::~TimerChange`

```doxygen
@fn xv::net::detail::TimerChange::~TimerChange()
@brief 在状态变更作用域结束时通知网络任务。
@details TimerChange 在作用域结束、局部状态与锁交接完成后通知网络定时任务；通知合并重复变化，不在调用者中执行协议超时。
```

- [`xv::net::detail::timers_changed`](kernel_platform.cpp.md#fn-3918e1d92132206c)

- [`xv::net::detail::TimerChange::~TimerChange`](platform.cppm.md#fn-a7467d39781652f2)
