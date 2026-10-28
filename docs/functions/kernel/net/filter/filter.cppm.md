# `kernel/net/filter/filter.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Firewall::Firewall`](filter.cppm.md#fn-238e3ebf0d23e835)
- [`xv::net::Firewall::initialize`](filter.cpp.md#fn-e044e49987bca493)
- [`xv::net::Firewall::execute`](filter.cpp.md#fn-6c6726c5c280a782)
- [`xv::net::Firewall::accept`](filter.cpp.md#fn-b1ddf21079255883)

<a id="fn-238e3ebf0d23e835"></a>

## `xv::net::Firewall::Firewall`

```doxygen
@fn constexpr xv::net::Firewall::Firewall()
@brief 默认构造独立规则核心与尚未初始化的服务锁。
```
