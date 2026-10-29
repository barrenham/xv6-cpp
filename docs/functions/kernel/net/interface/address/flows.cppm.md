# `kernel/net/interface/address/flows.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::AddressConfiguration::LeaseFlow::assign_owner`](flows.cppm.md#fn-48344cb30cacc253)
- [`xv::net::AddressConfiguration::LeaseFlow::capacity`](flows.cppm.md#fn-8ceb87df6fb59295)
- [`xv::net::AddressConfiguration::LeaseFlow::commit`](flows.cppm.md#fn-111974d281a35373)
- [`xv::net::AddressConfiguration::LeaseFlow::conflict`](flows.cppm.md#fn-83a9738259d1d3ea)
- [`xv::net::AddressConfiguration::LeaseFlow::full`](flows.cppm.md#fn-d740abf87311e5d1)
- [`xv::net::AddressConfiguration::LeaseFlow::invalid`](flows.cppm.md#fn-c3748f1b5789c122)
- [`xv::net::AddressConfiguration::LeaseFlow::locate`](flows.cppm.md#fn-ea6e907f39e1cf86)
- [`xv::net::AddressConfiguration::LeaseFlow::reachable`](flows.cppm.md#fn-fc2d9219a6a733e7)
- [`xv::net::AddressConfiguration::LeaseFlow::replace_routes`](flows.cppm.md#fn-03ec43c0ac5964da)
- [`xv::net::AddressConfiguration::LeaseFlow::reset`](flows.cppm.md#fn-dfdad8f3c0a1c6bb)
- [`xv::net::AddressConfiguration::LeaseFlow::unchanged`](flows.cppm.md#fn-be200b85c9a4cfe0)
- [`xv::net::AddressConfiguration::LeaseFlow::unique`](flows.cppm.md#fn-4c36433cceb9ad24)
- [`xv::net::AddressConfiguration::LeaseFlow::unreachable`](flows.cppm.md#fn-c548f905354be601)
- [`xv::net::AddressConfiguration::LeaseFlow::valid`](flows.cppm.md#fn-687da767fa876569)
- [`xv::net::AddressConfiguration::StaticFlow::add_routes`](flows.cppm.md#fn-513e1803f0e4766b)
- [`xv::net::AddressConfiguration::StaticFlow::busy`](flows.cppm.md#fn-9a59a8901c00cd32)
- [`xv::net::AddressConfiguration::StaticFlow::capacity`](flows.cppm.md#fn-91f51f526da10c51)
- [`xv::net::AddressConfiguration::StaticFlow::commit_add`](flows.cppm.md#fn-e0b56f314e9e9867)
- [`xv::net::AddressConfiguration::StaticFlow::commit_remove`](flows.cppm.md#fn-b36cfb1a74550c04)
- [`xv::net::AddressConfiguration::StaticFlow::enslaved`](flows.cppm.md#fn-ed83188161397eca)
- [`xv::net::AddressConfiguration::StaticFlow::exists`](flows.cppm.md#fn-17b161b8f62fa74e)
- [`xv::net::AddressConfiguration::StaticFlow::found`](flows.cppm.md#fn-3f6bf398c1e75ac6)
- [`xv::net::AddressConfiguration::StaticFlow::full`](flows.cppm.md#fn-1464e674cebd218f)
- [`xv::net::AddressConfiguration::StaticFlow::host_address`](flows.cppm.md#fn-a8120b7e011a6b20)
- [`xv::net::AddressConfiguration::StaticFlow::invalid`](flows.cppm.md#fn-16a564ed26b1df18)
- [`xv::net::AddressConfiguration::StaticFlow::locate`](flows.cppm.md#fn-228ac61d4b5155d7)
- [`xv::net::AddressConfiguration::StaticFlow::loopback`](flows.cppm.md#fn-d229c6b05266ac35)
- [`xv::net::AddressConfiguration::StaticFlow::missing`](flows.cppm.md#fn-214697d622348fed)
- [`xv::net::AddressConfiguration::StaticFlow::no_device`](flows.cppm.md#fn-efa3cc8c0fce4f45)
- [`xv::net::AddressConfiguration::StaticFlow::owned`](flows.cppm.md#fn-a76656aa27753dc0)
- [`xv::net::AddressConfiguration::StaticFlow::registered`](flows.cppm.md#fn-24d776cd66dbf486)
- [`xv::net::AddressConfiguration::StaticFlow::unique`](flows.cppm.md#fn-7bb29e28cf6d7e2a)
- [`xv::net::AddressConfiguration::StaticFlow::unsupported`](flows.cppm.md#fn-1db78af0b3495f20)
- [`xv::net::AddressConfiguration::StaticFlow::valid_address`](flows.cppm.md#fn-f2ed8ec34f4f9cf3)

<a id="fn-48344cb30cacc253"></a>

## `xv::net::AddressConfiguration::LeaseFlow::assign_owner`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::assign_owner(Context &c) noexcept
@brief 标记本次租约地址的接口归属；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8ceb87df6fb59295"></a>

## `xv::net::AddressConfiguration::LeaseFlow::capacity`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-111974d281a35373"></a>

## `xv::net::AddressConfiguration::LeaseFlow::commit`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::commit(Context &c) noexcept
@brief 提交已经通过预检的业务结果；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-83a9738259d1d3ea"></a>

## `xv::net::AddressConfiguration::LeaseFlow::conflict`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::conflict(Context &c) noexcept
@brief 记录地址冲突或检查封存时的地址重复；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d740abf87311e5d1"></a>

## `xv::net::AddressConfiguration::LeaseFlow::full`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c3748f1b5789c122"></a>

## `xv::net::AddressConfiguration::LeaseFlow::invalid`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ea6e907f39e1cf86"></a>

## `xv::net::AddressConfiguration::LeaseFlow::locate`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::locate(Context &c) noexcept
@brief 查找地址位置或选择重组槽；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fc2d9219a6a733e7"></a>

## `xv::net::AddressConfiguration::LeaseFlow::reachable`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::reachable(Context &c) noexcept
@brief 检查租约目标接口是否仍可配置；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-03ec43c0ac5964da"></a>

## `xv::net::AddressConfiguration::LeaseFlow::replace_routes`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::replace_routes(Context &c) noexcept
@brief 在容量预检后替换整组租约路由；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dfdad8f3c0a1c6bb"></a>

## `xv::net::AddressConfiguration::LeaseFlow::reset`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-be200b85c9a4cfe0"></a>

## `xv::net::AddressConfiguration::LeaseFlow::unchanged`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::unchanged(Context &c) noexcept
@brief 检查租约地址与整组路由是否完全未变；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4c36433cceb9ad24"></a>

## `xv::net::AddressConfiguration::LeaseFlow::unique`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::unique(Context &c) noexcept
@brief 检查当前对象、选项或地址没有冲突；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c548f905354be601"></a>

## `xv::net::AddressConfiguration::LeaseFlow::unreachable`

```doxygen
@fn static inline void xv::net::AddressConfiguration::LeaseFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-687da767fa876569"></a>

## `xv::net::AddressConfiguration::LeaseFlow::valid`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::LeaseFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::AddressConfiguration::LeaseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-513e1803f0e4766b"></a>

## `xv::net::AddressConfiguration::StaticFlow::add_routes`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::add_routes(Context &c) noexcept
@brief 为地址预先提交对应的路由组；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9a59a8901c00cd32"></a>

## `xv::net::AddressConfiguration::StaticFlow::busy`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::busy(Context &c) noexcept
@brief 记录当前资源忙的业务错误；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-91f51f526da10c51"></a>

## `xv::net::AddressConfiguration::StaticFlow::capacity`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::capacity(Context &c) noexcept
@brief 检查当前容量或索引是否允许继续操作；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e0b56f314e9e9867"></a>

## `xv::net::AddressConfiguration::StaticFlow::commit_add`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::commit_add(Context &c) noexcept
@brief 提交静态地址并推进接口代次；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b36cfb1a74550c04"></a>

## `xv::net::AddressConfiguration::StaticFlow::commit_remove`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::commit_remove(Context &c) noexcept
@brief 移除静态地址及对应路由并推进代次；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ed83188161397eca"></a>

## `xv::net::AddressConfiguration::StaticFlow::enslaved`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::enslaved(Context &c) noexcept
@brief 检查接口是否被网桥占用；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-17b161b8f62fa74e"></a>

## `xv::net::AddressConfiguration::StaticFlow::exists`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::exists(Context &c) noexcept
@brief 记录同名或同路径对象已存在的错误；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3f6bf398c1e75ac6"></a>

## `xv::net::AddressConfiguration::StaticFlow::found`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::found(Context &c) noexcept
@brief 检查本阶段查找是否命中对象；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1464e674cebd218f"></a>

## `xv::net::AddressConfiguration::StaticFlow::full`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::full(Context &c) noexcept
@brief 记录有界存储容量不足的业务错误；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a8120b7e011a6b20"></a>

## `xv::net::AddressConfiguration::StaticFlow::host_address`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::host_address(Context &c) noexcept
@brief 排除不允许的子网地址与定向广播地址；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-16a564ed26b1df18"></a>

## `xv::net::AddressConfiguration::StaticFlow::invalid`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-228ac61d4b5155d7"></a>

## `xv::net::AddressConfiguration::StaticFlow::locate`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::locate(Context &c) noexcept
@brief 查找地址位置或选择重组槽；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d229c6b05266ac35"></a>

## `xv::net::AddressConfiguration::StaticFlow::loopback`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::loopback(Context &c) noexcept
@brief 检查目标接口是否为保留的回环接口；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-214697d622348fed"></a>

## `xv::net::AddressConfiguration::StaticFlow::missing`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::missing(Context &c) noexcept
@brief 记录目标对象或记录不存在的业务结果；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-efa3cc8c0fce4f45"></a>

## `xv::net::AddressConfiguration::StaticFlow::no_device`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::no_device(Context &c) noexcept
@brief 记录管理目标接口不存在的负 ENODEV；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a76656aa27753dc0"></a>

## `xv::net::AddressConfiguration::StaticFlow::owned`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::owned(Context &c) noexcept
@brief 检查待删除地址是否由租约自动拥有；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-24d776cd66dbf486"></a>

## `xv::net::AddressConfiguration::StaticFlow::registered`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::registered(Context &c) noexcept
@brief 按对象身份检查接口是否仍在注册表；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7bb29e28cf6d7e2a"></a>

## `xv::net::AddressConfiguration::StaticFlow::unique`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::unique(Context &c) noexcept
@brief 检查当前对象、选项或地址没有冲突；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1db78af0b3495f20"></a>

## `xv::net::AddressConfiguration::StaticFlow::unsupported`

```doxygen
@fn static inline void xv::net::AddressConfiguration::StaticFlow::unsupported(Context &c) noexcept
@brief 记录操作不支持的业务错误；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f2ed8ec34f4f9cf3"></a>

## `xv::net::AddressConfiguration::StaticFlow::valid_address`

```doxygen
@fn static inline bool xv::net::AddressConfiguration::StaticFlow::valid_address(Context &c) noexcept
@brief 检查地址和前缀的基本约束；所属流程为 xv::net::AddressConfiguration::StaticFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
