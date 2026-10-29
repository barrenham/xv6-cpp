# `kernel/net/socket/tcp/lifecycle.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpSocket::CloseFlow::close_listener`](lifecycle.cppm.md#fn-d3e31dc270c6e2c6)
- [`xv::net::TcpSocket::CloseFlow::closed`](lifecycle.cppm.md#fn-ef1ae80df34f98be)
- [`xv::net::TcpSocket::CloseFlow::connecting`](lifecycle.cppm.md#fn-819687f3bb7fe5e4)
- [`xv::net::TcpSocket::CloseFlow::destroy`](lifecycle.cppm.md#fn-c1498d2aec0fb181)
- [`xv::net::TcpSocket::CloseFlow::listening`](lifecycle.cppm.md#fn-a7bc3d698c2fee7c)
- [`xv::net::TcpSocket::CloseFlow::pump`](lifecycle.cppm.md#fn-45bc5f6daa633987)
- [`xv::net::TcpSocket::CloseFlow::reset`](lifecycle.cppm.md#fn-7330a1bb417fff32)
- [`xv::net::TcpSocket::CloseFlow::shutdown`](lifecycle.cppm.md#fn-cf74fbaf30f23ec7)
- [`xv::net::TcpSocket::CloseFlow::unown`](lifecycle.cppm.md#fn-65318cd25d4a4d83)
- [`xv::net::TcpSocket::ListenFlow::choose_port`](lifecycle.cppm.md#fn-bfa3441c7df35f97)
- [`xv::net::TcpSocket::ListenFlow::eligible`](lifecycle.cppm.md#fn-3678e56bd3afa005)
- [`xv::net::TcpSocket::ListenFlow::has_port`](lifecycle.cppm.md#fn-77015f9fdf9226b6)
- [`xv::net::TcpSocket::ListenFlow::invalid`](lifecycle.cppm.md#fn-31dfa6a626522253)
- [`xv::net::TcpSocket::ListenFlow::listen`](lifecycle.cppm.md#fn-966bb7bafa33cffc)
- [`xv::net::TcpSocket::ListenFlow::used`](lifecycle.cppm.md#fn-080aff4fdde55641)
- [`xv::net::TcpSocket::PassiveFlow::claim`](lifecycle.cppm.md#fn-f34d7c31db34dcc3)
- [`xv::net::TcpSocket::PassiveFlow::initialize`](lifecycle.cppm.md#fn-1ad5b496031cdb34)
- [`xv::net::TcpSocket::PassiveFlow::reset`](lifecycle.cppm.md#fn-4a68a03388ccd34e)
- [`xv::net::TcpSocket::PassiveFlow::synchronize`](lifecycle.cppm.md#fn-7294e8b895e70082)
- [`xv::net::TcpSocket::PassiveFlow::valid_mss`](lifecycle.cppm.md#fn-80ba6f439d0ab7cd)
- [`xv::net::TcpSocket::PollFlow::hangup`](lifecycle.cppm.md#fn-9d7b1138a5f20c13)
- [`xv::net::TcpSocket::PollFlow::hung_up`](lifecycle.cppm.md#fn-3545ad413cda0d01)
- [`xv::net::TcpSocket::PollFlow::incoming`](lifecycle.cppm.md#fn-a455edde8a3215ab)
- [`xv::net::TcpSocket::PollFlow::listening`](lifecycle.cppm.md#fn-8b63a81324277f5a)
- [`xv::net::TcpSocket::PollFlow::outgoing`](lifecycle.cppm.md#fn-ceb27c1ec753736a)
- [`xv::net::TcpSocket::PollFlow::pending_child`](lifecycle.cppm.md#fn-05e50ea051dda65c)
- [`xv::net::TcpSocket::PollFlow::readable`](lifecycle.cppm.md#fn-7613808a1111eae6)
- [`xv::net::TcpSocket::PollFlow::watch`](lifecycle.cppm.md#fn-a74db3846f5949d7)
- [`xv::net::TcpSocket::PollFlow::writable`](lifecycle.cppm.md#fn-e67f062e71103ad6)
- [`xv::net::TcpSocket::ShutdownFlow::close_read`](lifecycle.cppm.md#fn-0fb3be6a8cdbd4d5)
- [`xv::net::TcpSocket::ShutdownFlow::close_write`](lifecycle.cppm.md#fn-09fe2f6091ebf37e)
- [`xv::net::TcpSocket::ShutdownFlow::connected`](lifecycle.cppm.md#fn-fc59b3c3a8f47752)
- [`xv::net::TcpSocket::ShutdownFlow::disconnected`](lifecycle.cppm.md#fn-29ab82951dc051d5)
- [`xv::net::TcpSocket::ShutdownFlow::invalid`](lifecycle.cppm.md#fn-d6f066bb979499d8)
- [`xv::net::TcpSocket::ShutdownFlow::locked`](lifecycle.cppm.md#fn-a7a11a4eff0c1842)
- [`xv::net::TcpSocket::ShutdownFlow::notify`](lifecycle.cppm.md#fn-7095fbc597cea250)
- [`xv::net::TcpSocket::ShutdownFlow::read`](lifecycle.cppm.md#fn-f3a48e58bcf9ea7c)
- [`xv::net::TcpSocket::ShutdownFlow::valid`](lifecycle.cppm.md#fn-2fe4cc0740a4cb21)
- [`xv::net::TcpSocket::ShutdownFlow::write`](lifecycle.cppm.md#fn-6fe1740e4e2d9e2e)
- [`xv::net::TcpSocket::WindowFlow::assign`](lifecycle.cppm.md#fn-f86681d553c1d3f3)
- [`xv::net::TcpSocket::WindowFlow::compare`](lifecycle.cppm.md#fn-a84ad346e969b16e)
- [`xv::net::TcpSocket::WindowFlow::current`](lifecycle.cppm.md#fn-fa975c46768d0151)
- [`xv::net::TcpSocket::WindowFlow::reopened`](lifecycle.cppm.md#fn-80789982f17ede58)
- [`xv::net::TcpSocket::WindowFlow::reset_persist`](lifecycle.cppm.md#fn-3d5efc8635aba957)

<a id="fn-d3e31dc270c6e2c6"></a>

## `xv::net::TcpSocket::CloseFlow::close_listener`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::close_listener(Context &c) noexcept
@brief 关闭监听的未接受子连接并关闭监听状态；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ef1ae80df34f98be"></a>

## `xv::net::TcpSocket::CloseFlow::closed`

```doxygen
@fn static inline bool xv::net::TcpSocket::CloseFlow::closed(Context &c) noexcept
@brief 检查连接是否已经关闭；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-819687f3bb7fe5e4"></a>

## `xv::net::TcpSocket::CloseFlow::connecting`

```doxygen
@fn static inline bool xv::net::TcpSocket::CloseFlow::connecting(Context &c) noexcept
@brief 检查连接是否仍处于握手阶段；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c1498d2aec0fb181"></a>

## `xv::net::TcpSocket::CloseFlow::destroy`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::destroy(Context &c) noexcept
@brief 销毁已经不再被业务持有的对象；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 销毁阶段为相应路径终点，不再使用已销毁对象。
```

<a id="fn-a7bc3d698c2fee7c"></a>

## `xv::net::TcpSocket::CloseFlow::listening`

```doxygen
@fn static inline bool xv::net::TcpSocket::CloseFlow::listening(Context &c) noexcept
@brief 检查对象是否处于监听状态；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-45bc5f6daa633987"></a>

## `xv::net::TcpSocket::CloseFlow::pump`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::pump(Context &c) noexcept
@brief 按当前状态与窗口推进连接发送；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7330a1bb417fff32"></a>

## `xv::net::TcpSocket::CloseFlow::reset`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cf74fbaf30f23ec7"></a>

## `xv::net::TcpSocket::CloseFlow::shutdown`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::shutdown(Context &c) noexcept
@brief 标记连接两侧关闭并丢弃待读数据；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-65318cd25d4a4d83"></a>

## `xv::net::TcpSocket::CloseFlow::unown`

```doxygen
@fn static inline void xv::net::TcpSocket::CloseFlow::unown(Context &c) noexcept
@brief 移交应用关闭后的连接回收责任；所属流程为 xv::net::TcpSocket::CloseFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bfa3441c7df35f97"></a>

## `xv::net::TcpSocket::ListenFlow::choose_port`

```doxygen
@fn static inline void xv::net::TcpSocket::ListenFlow::choose_port(Context &c) noexcept
@brief 按需选择临时本地端口；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3678e56bd3afa005"></a>

## `xv::net::TcpSocket::ListenFlow::eligible`

```doxygen
@fn static inline bool xv::net::TcpSocket::ListenFlow::eligible(Context &c) noexcept
@brief 检查本业务操作的入口状态和对象条件；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-77015f9fdf9226b6"></a>

## `xv::net::TcpSocket::ListenFlow::has_port`

```doxygen
@fn static inline bool xv::net::TcpSocket::ListenFlow::has_port(Context &c) noexcept
@brief 检查本地端口是否已分配成功；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-31dfa6a626522253"></a>

## `xv::net::TcpSocket::ListenFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::ListenFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-966bb7bafa33cffc"></a>

## `xv::net::TcpSocket::ListenFlow::listen`

```doxygen
@fn static inline void xv::net::TcpSocket::ListenFlow::listen(Context &c) noexcept
@brief 提交监听状态和有界 backlog；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-080aff4fdde55641"></a>

## `xv::net::TcpSocket::ListenFlow::used`

```doxygen
@fn static inline void xv::net::TcpSocket::ListenFlow::used(Context &c) noexcept
@brief 记录本地端口已被占用的负 EADDRINUSE；所属流程为 xv::net::TcpSocket::ListenFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f34d7c31db34dcc3"></a>

## `xv::net::TcpSocket::PassiveFlow::claim`

```doxygen
@fn static inline void xv::net::TcpSocket::PassiveFlow::claim(Context &c) noexcept
@brief 记录本次报文或对象已由当前服务接管处理；所属流程为 xv::net::TcpSocket::PassiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1ad5b496031cdb34"></a>

## `xv::net::TcpSocket::PassiveFlow::initialize`

```doxygen
@fn static inline void xv::net::TcpSocket::PassiveFlow::initialize(Context &c) noexcept
@brief 初始化当前业务阶段状态；所属流程为 xv::net::TcpSocket::PassiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-4a68a03388ccd34e"></a>

## `xv::net::TcpSocket::PassiveFlow::reset`

```doxygen
@fn static inline void xv::net::TcpSocket::PassiveFlow::reset(Context &c) noexcept
@brief 重置本阶段输出、探测状态或连接状态；所属流程为 xv::net::TcpSocket::PassiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7294e8b895e70082"></a>

## `xv::net::TcpSocket::PassiveFlow::synchronize`

```doxygen
@fn static inline void xv::net::TcpSocket::PassiveFlow::synchronize(Context &c) noexcept
@brief 初始化本端握手序号并发送握手报文；所属流程为 xv::net::TcpSocket::PassiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-80ba6f439d0ab7cd"></a>

## `xv::net::TcpSocket::PassiveFlow::valid_mss`

```doxygen
@fn static inline bool xv::net::TcpSocket::PassiveFlow::valid_mss(Context &c) noexcept
@brief 检查解析得到的对端 MSS 是否非零；所属流程为 xv::net::TcpSocket::PassiveFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9d7b1138a5f20c13"></a>

## `xv::net::TcpSocket::PollFlow::hangup`

```doxygen
@fn static inline void xv::net::TcpSocket::PollFlow::hangup(Context &c) noexcept
@brief 将关闭连接的 HUP 和可写事件加入结果；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3545ad413cda0d01"></a>

## `xv::net::TcpSocket::PollFlow::hung_up`

```doxygen
@fn static inline bool xv::net::TcpSocket::PollFlow::hung_up(Context &c) noexcept
@brief 检查关闭连接是否应发布 HUP 事件；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a455edde8a3215ab"></a>

## `xv::net::TcpSocket::PollFlow::incoming`

```doxygen
@fn static inline void xv::net::TcpSocket::PollFlow::incoming(Context &c) noexcept
@brief 为 poll 结果加入可读标记；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8b63a81324277f5a"></a>

## `xv::net::TcpSocket::PollFlow::listening`

```doxygen
@fn static inline bool xv::net::TcpSocket::PollFlow::listening(Context &c) noexcept
@brief 检查对象是否处于监听状态；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-ceb27c1ec753736a"></a>

## `xv::net::TcpSocket::PollFlow::outgoing`

```doxygen
@fn static inline void xv::net::TcpSocket::PollFlow::outgoing(Context &c) noexcept
@brief 为 poll 结果加入可写标记；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-05e50ea051dda65c"></a>

## `xv::net::TcpSocket::PollFlow::pending_child`

```doxygen
@fn static inline bool xv::net::TcpSocket::PollFlow::pending_child(Context &c) noexcept
@brief 检查监听是否已有可接管子连接；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7613808a1111eae6"></a>

## `xv::net::TcpSocket::PollFlow::readable`

```doxygen
@fn static inline bool xv::net::TcpSocket::PollFlow::readable(Context &c) noexcept
@brief 检查当前队列或接收流是否存在可读内容；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a74db3846f5949d7"></a>

## `xv::net::TcpSocket::PollFlow::watch`

```doxygen
@fn static inline void xv::net::TcpSocket::PollFlow::watch(Context &c) noexcept
@brief 登记 poll 观察者并初始化错误事件；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e67f062e71103ad6"></a>

## `xv::net::TcpSocket::PollFlow::writable`

```doxygen
@fn static inline bool xv::net::TcpSocket::PollFlow::writable(Context &c) noexcept
@brief 检查连接状态、写侧关闭与发送缓冲空间；所属流程为 xv::net::TcpSocket::PollFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0fb3be6a8cdbd4d5"></a>

## `xv::net::TcpSocket::ShutdownFlow::close_read`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::close_read(Context &c) noexcept
@brief 丢弃接收缓冲并执行读侧关闭；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-09fe2f6091ebf37e"></a>

## `xv::net::TcpSocket::ShutdownFlow::close_write`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::close_write(Context &c) noexcept
@brief 执行写侧关闭并推进待发送数据；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fc59b3c3a8f47752"></a>

## `xv::net::TcpSocket::ShutdownFlow::connected`

```doxygen
@fn static inline bool xv::net::TcpSocket::ShutdownFlow::connected(Context &c) noexcept
@brief 检查当前 socket 是否允许本次连接操作；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-29ab82951dc051d5"></a>

## `xv::net::TcpSocket::ShutdownFlow::disconnected`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::disconnected(Context &c) noexcept
@brief 记录 socket 尚未连接的业务错误；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d6f066bb979499d8"></a>

## `xv::net::TcpSocket::ShutdownFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a7a11a4eff0c1842"></a>

## `xv::net::TcpSocket::ShutdownFlow::locked`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::locked(Context &c) noexcept
@brief 在原锁作用域内执行对应业务图；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。 本阶段保留局部 Guard 的锁作用域，离开该作用域后再进入外部回调阶段。 内部同步运行 LockedGraph，其 stop 结束该次内部 run。
```

<a id="fn-7095fbc597cea250"></a>

## `xv::net::TcpSocket::ShutdownFlow::notify`

```doxygen
@fn static inline void xv::net::TcpSocket::ShutdownFlow::notify(Context &c) noexcept
@brief 发布本阶段状态变化的等待唤醒；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f3a48e58bcf9ea7c"></a>

## `xv::net::TcpSocket::ShutdownFlow::read`

```doxygen
@fn static inline bool xv::net::TcpSocket::ShutdownFlow::read(Context &c) noexcept
@brief 读取接收流或检查是否关闭读侧；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2fe4cc0740a4cb21"></a>

## `xv::net::TcpSocket::ShutdownFlow::valid`

```doxygen
@fn static inline bool xv::net::TcpSocket::ShutdownFlow::valid(Context &c) noexcept
@brief 检查本阶段输入与业务状态是否合法；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6fe1740e4e2d9e2e"></a>

## `xv::net::TcpSocket::ShutdownFlow::write`

```doxygen
@fn static inline bool xv::net::TcpSocket::ShutdownFlow::write(Context &c) noexcept
@brief 检查本次 shutdown 是否需要关闭写侧；所属流程为 xv::net::TcpSocket::ShutdownFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f86681d553c1d3f3"></a>

## `xv::net::TcpSocket::WindowFlow::assign`

```doxygen
@fn static inline void xv::net::TcpSocket::WindowFlow::assign(Context &c) noexcept
@brief 提交已验证的对端窗口及其序号记录；所属流程为 xv::net::TcpSocket::WindowFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a84ad346e969b16e"></a>

## `xv::net::TcpSocket::WindowFlow::compare`

```doxygen
@fn static inline void xv::net::TcpSocket::WindowFlow::compare(Context &c) noexcept
@brief 记录新旧窗口值是否变化；所属流程为 xv::net::TcpSocket::WindowFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fa975c46768d0151"></a>

## `xv::net::TcpSocket::WindowFlow::current`

```doxygen
@fn static inline bool xv::net::TcpSocket::WindowFlow::current(Context &c) noexcept
@brief 检查请求或快照是否仍是当前有效状态；所属流程为 xv::net::TcpSocket::WindowFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-80789982f17ede58"></a>

## `xv::net::TcpSocket::WindowFlow::reopened`

```doxygen
@fn static inline bool xv::net::TcpSocket::WindowFlow::reopened(Context &c) noexcept
@brief 检查对端窗口是否从零重新打开；所属流程为 xv::net::TcpSocket::WindowFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3d5efc8635aba957"></a>

## `xv::net::TcpSocket::WindowFlow::reset_persist`

```doxygen
@fn static inline void xv::net::TcpSocket::WindowFlow::reset_persist(Context &c) noexcept
@brief 在窗口重开时重置探测与重传计时；所属流程为 xv::net::TcpSocket::WindowFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
