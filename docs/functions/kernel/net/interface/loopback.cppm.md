# `kernel/net/interface/loopback.cppm`

## 函数导航

- [`xv::net::LoopbackInterface::LoopbackInterface`](loopback.cppm.md#fn-5f2bd08ab86d4933)
- [`xv::net::LoopbackInterface::accepts_destination`](loopback.cppm.md#fn-1a600c69d995581e)
- [`xv::net::LoopbackInterface::initialize`](loopback.cpp.md#fn-21e76ce52e463582)
- [`xv::net::LoopbackInterface::owns`](loopback.cpp.md#fn-19bc88c16594dcb2)
- [`xv::net::LoopbackInterface::tick`](loopback.cpp.md#fn-49f2bb3530d94f42)
- [`xv::net::LoopbackInterface::transmit`](loopback.cpp.md#fn-bb6135ec4e85a90a)

<a id="fn-5f2bd08ab86d4933"></a>

## `xv::net::LoopbackInterface::LoopbackInterface`

```doxygen
@fn xv::net::LoopbackInterface::LoopbackInterface(unsigned id = 1, const char *name = "lo")
@brief 构造对象并保存稳定身份或借用的层间依赖，初始化空状态；不执行网络 I/O，依赖与名称须比对象存活更久。
```

<a id="fn-1a600c69d995581e"></a>

## `xv::net::LoopbackInterface::accepts_destination`

```doxygen
@fn bool xv::net::LoopbackInterface::accepts_destination(unsigned value, const InterfaceRegistry &registry) const
@brief 允许发往任一已启用本机接口地址的本地回环包，配合主机 /32 路由。
```
