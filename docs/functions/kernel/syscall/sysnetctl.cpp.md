# `kernel/syscall/sysnetctl.cpp`

## 函数导航

- [`xv::syscall::FirewallControlFlow::bad_copy`](sysnetctl.cpp.md#fn-d3f9d5d8d6c4c212)
- [`xv::syscall::FirewallControlFlow::copy_request`](sysnetctl.cpp.md#fn-dd17ad190800b984)
- [`xv::syscall::FirewallControlFlow::copy_result`](sysnetctl.cpp.md#fn-e87435fc68620c81)
- [`xv::syscall::FirewallControlFlow::execute`](sysnetctl.cpp.md#fn-7b613ec691726e6c)
- [`xv::syscall::FirewallControlFlow::has_output`](sysnetctl.cpp.md#fn-0bfaac638870e63b)
- [`xv::syscall::FirewallControlFlow::permitted`](sysnetctl.cpp.md#fn-f19c8e9e04600d47)
- [`xv::syscall::NetworkControlFlow::bad_copy`](sysnetctl.cpp.md#fn-36feb34095eab72d)
- [`xv::syscall::NetworkControlFlow::copy_request`](sysnetctl.cpp.md#fn-f8c66715b16c8b6a)
- [`xv::syscall::NetworkControlFlow::copy_result`](sysnetctl.cpp.md#fn-5b0c82a43ab724ae)
- [`xv::syscall::NetworkControlFlow::execute`](sysnetctl.cpp.md#fn-8ba7414ea4b296cb)
- [`xv::syscall::NetworkControlFlow::has_output`](sysnetctl.cpp.md#fn-31d129d608fb382c)
- [`xv::syscall::NetworkControlFlow::permitted`](sysnetctl.cpp.md#fn-e77ca2f79e18746a)
- [`xv::syscall::sys_netctl`](sysnetctl.cpp.md#fn-470182ff56d32e47)

<a id="fn-470182ff56d32e47"></a>

## `xv::syscall::sys_netctl`

```doxygen
@fn unsigned long xv::syscall::sys_netctl()
@brief 处理网络查询与管理员配置请求。
@details 先识别操作类别，接口/地址/路由/邻居/网桥及 DHCP 状态查询可读；修改配置要求有效 UID 为 0。仍验证请求结构及具体管理器约束，不信任用户缓冲。
@details 64 至 73 为防火墙专属操作范围，复制独立的 128 字节 firewall_request；仅链与规则查询允许普通用户，全部修改要求有效 UID 为 0。用户复制始终位于 Firewall 锁外。
@details 流程由 FirewallControlFlow::Graph, NetworkControlFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-d3f9d5d8d6c4c212"></a>

## `xv::syscall::FirewallControlFlow::bad_copy`

```doxygen
@fn static void xv::syscall::FirewallControlFlow::bad_copy(Context &c) noexcept { c.result = -EFAULT; }
@brief 记录用户复制失败的负 EFAULT。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dd17ad190800b984"></a>

## `xv::syscall::FirewallControlFlow::copy_request`

```doxygen
@fn static bool xv::syscall::FirewallControlFlow::copy_request(Context &c) noexcept
@brief 复制本次请求并报告用户内存读取结果。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e87435fc68620c81"></a>

## `xv::syscall::FirewallControlFlow::copy_result`

```doxygen
@fn static bool xv::syscall::FirewallControlFlow::copy_result(Context &c) noexcept
@brief 把业务修改后的请求复制回原用户地址。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-7b613ec691726e6c"></a>

## `xv::syscall::FirewallControlFlow::execute`

```doxygen
@fn static void xv::syscall::FirewallControlFlow::execute(Context &c) noexcept
@brief 调用对应网络管理业务并保存有符号返回值。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0bfaac638870e63b"></a>

## `xv::syscall::FirewallControlFlow::has_output`

```doxygen
@fn static bool xv::syscall::FirewallControlFlow::has_output(Context &c) noexcept { return c.result > 0; }
@brief 检查业务结果是否为需要复制输出的正值。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f19c8e9e04600d47"></a>

## `xv::syscall::FirewallControlFlow::permitted`

```doxygen
@fn static bool xv::syscall::FirewallControlFlow::permitted(Context &c) noexcept
@brief 按原查询集合与当前有效 UID 检查本次操作权限。
@details 防火墙先检查查询或管理权限，再复制请求；仅正返回值复制输出，复制失败返回 EFAULT。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-36feb34095eab72d"></a>

## `xv::syscall::NetworkControlFlow::bad_copy`

```doxygen
@fn static void xv::syscall::NetworkControlFlow::bad_copy(Context &c) noexcept { c.result = -EFAULT; }
@brief 记录用户复制失败的负 EFAULT。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f8c66715b16c8b6a"></a>

## `xv::syscall::NetworkControlFlow::copy_request`

```doxygen
@fn static bool xv::syscall::NetworkControlFlow::copy_request(Context &c) noexcept
@brief 复制本次请求并报告用户内存读取结果。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5b0c82a43ab724ae"></a>

## `xv::syscall::NetworkControlFlow::copy_result`

```doxygen
@fn static bool xv::syscall::NetworkControlFlow::copy_result(Context &c) noexcept
@brief 把业务修改后的请求复制回原用户地址。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8ba7414ea4b296cb"></a>

## `xv::syscall::NetworkControlFlow::execute`

```doxygen
@fn static void xv::syscall::NetworkControlFlow::execute(Context &c) noexcept
@brief 调用对应网络管理业务并保存有符号返回值。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-31d129d608fb382c"></a>

## `xv::syscall::NetworkControlFlow::has_output`

```doxygen
@fn static bool xv::syscall::NetworkControlFlow::has_output(Context &c) noexcept { return c.result > 0; }
@brief 检查业务结果是否为需要复制输出的正值。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e77ca2f79e18746a"></a>

## `xv::syscall::NetworkControlFlow::permitted`

```doxygen
@fn static bool xv::syscall::NetworkControlFlow::permitted(Context &c) noexcept
@brief 按原查询集合与当前有效 UID 检查本次操作权限。
@details 普通网络管理先复制请求，再判断查询或管理权限；仅正返回值复制输出，保留检查顺序。 下一节点由图策略声明，业务结果与控制信号分离。
```
