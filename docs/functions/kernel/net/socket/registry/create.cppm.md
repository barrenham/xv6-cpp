# `kernel/net/socket/registry/create.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::SocketRegistry::CreationFlow::allocate`](create.cppm.md#fn-060072046d1c0704)
- [`xv::net::SocketRegistry::CreationFlow::allocated`](create.cppm.md#fn-cff21809ac05c622)
- [`xv::net::SocketRegistry::CreationFlow::commit_tcp`](create.cppm.md#fn-c355e7bd2337de7e)
- [`xv::net::SocketRegistry::CreationFlow::commit_udp`](create.cppm.md#fn-e7f7389591976c63)
- [`xv::net::SocketRegistry::CreationFlow::construct_tcp`](create.cppm.md#fn-1e58d1eddeb27091)
- [`xv::net::SocketRegistry::CreationFlow::construct_udp`](create.cppm.md#fn-3b2b53f14d116849)
- [`xv::net::SocketRegistry::CreationFlow::destroy_tcp`](create.cppm.md#fn-2cd07b1b0e322b2a)
- [`xv::net::SocketRegistry::CreationFlow::destroy_udp`](create.cppm.md#fn-3de26a490591d450)
- [`xv::net::SocketRegistry::CreationFlow::initialize_tcp`](create.cppm.md#fn-18fa57783d4f3469)
- [`xv::net::SocketRegistry::CreationFlow::locked`](create.cppm.md#fn-7d4cc960d6157f8d)
- [`xv::net::SocketRegistry::CreationFlow::register_tcp`](create.cppm.md#fn-7b67f112d31d11a6)
- [`xv::net::SocketRegistry::CreationFlow::register_udp`](create.cppm.md#fn-3ae610c415c24f73)
- [`xv::net::SocketRegistry::CreationFlow::tcp`](create.cppm.md#fn-8e99843f7b79fad4)
- [`xv::net::SocketRegistry::CreationFlow::valid`](create.cppm.md#fn-cf21ea8badeadb0f)

<a id="fn-060072046d1c0704"></a>

## `xv::net::SocketRegistry::CreationFlow::allocate`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::allocate(Context &c) noexcept
@brief 通过原有分配接口创建本阶段所需资源；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cff21809ac05c622"></a>

## `xv::net::SocketRegistry::CreationFlow::allocated`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::allocated(Context &c) noexcept
@brief 检查本阶段分配是否成功；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c355e7bd2337de7e"></a>

## `xv::net::SocketRegistry::CreationFlow::commit_tcp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::commit_tcp(Context &c) noexcept
@brief 将已初始化且已登记的 TCP 对象作为创建结果；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e7f7389591976c63"></a>

## `xv::net::SocketRegistry::CreationFlow::commit_udp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::commit_udp(Context &c) noexcept
@brief 将已登记的 UDP 对象作为创建结果；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e58d1eddeb27091"></a>

## `xv::net::SocketRegistry::CreationFlow::construct_tcp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::construct_tcp(Context &c) noexcept
@brief 在已分配页面原位构造 TCP 对象；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3b2b53f14d116849"></a>

## `xv::net::SocketRegistry::CreationFlow::construct_udp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::construct_udp(Context &c) noexcept
@brief 在已分配页面原位构造 UDP 对象；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2cd07b1b0e322b2a"></a>

## `xv::net::SocketRegistry::CreationFlow::destroy_tcp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::destroy_tcp(Context &c) noexcept
@brief 回收未完成创建的 TCP 对象及其部分缓冲；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 销毁阶段为相应路径终点，不再使用已销毁对象。
```

<a id="fn-3de26a490591d450"></a>

## `xv::net::SocketRegistry::CreationFlow::destroy_udp`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::destroy_udp(Context &c) noexcept
@brief 回收未能登记的 UDP 对象；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 销毁阶段为相应路径终点，不再使用已销毁对象。
```

<a id="fn-18fa57783d4f3469"></a>

## `xv::net::SocketRegistry::CreationFlow::initialize_tcp`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::initialize_tcp(Context &c) noexcept
@brief 初始化新 TCP 对象的发送和接收缓冲；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7d4cc960d6157f8d"></a>

## `xv::net::SocketRegistry::CreationFlow::locked`

```doxygen
@fn static inline void xv::net::SocketRegistry::CreationFlow::locked(Context &c) noexcept
@brief 在原锁作用域内执行对应业务图；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-7b67f112d31d11a6"></a>

## `xv::net::SocketRegistry::CreationFlow::register_tcp`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::register_tcp(Context &c) noexcept
@brief 将已初始化 TCP 对象加入所属注册表；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3ae610c415c24f73"></a>

## `xv::net::SocketRegistry::CreationFlow::register_udp`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::register_udp(Context &c) noexcept
@brief 将新 UDP 对象加入所属注册表；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8e99843f7b79fad4"></a>

## `xv::net::SocketRegistry::CreationFlow::tcp`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::tcp(Context &c) noexcept
@brief 检查当前协议或对象是否为 TCP；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf21ea8badeadb0f"></a>

## `xv::net::SocketRegistry::CreationFlow::valid`

```doxygen
@fn static inline bool xv::net::SocketRegistry::CreationFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::SocketRegistry::CreationFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
