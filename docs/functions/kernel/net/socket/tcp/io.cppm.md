# `kernel/net/socket/tcp/io.cppm`

类主体、流程关系、锁和资源约定见 [网络声明式流程](../../../../../network-flows.md).

## 函数导航

- [`xv::net::TcpSocket::AcceptFlow::accept`](io.cppm.md#fn-64d40b1ceb2ff4f8)
- [`xv::net::TcpSocket::AcceptFlow::accepted`](io.cppm.md#fn-5cbe87c9dfdbf37e)
- [`xv::net::TcpSocket::AcceptFlow::again`](io.cppm.md#fn-b8c5f33a26c3ac62)
- [`xv::net::TcpSocket::AcceptFlow::interrupted`](io.cppm.md#fn-160a6133920c6fec)
- [`xv::net::TcpSocket::AcceptFlow::invalid`](io.cppm.md#fn-36a28a3ad5cd8571)
- [`xv::net::TcpSocket::AcceptFlow::listening`](io.cppm.md#fn-22fb6d6c17436792)
- [`xv::net::TcpSocket::AcceptFlow::nonblocking`](io.cppm.md#fn-c7b73f4a9172d92f)
- [`xv::net::TcpSocket::AcceptFlow::success`](io.cppm.md#fn-a87fc4641d555ab0)
- [`xv::net::TcpSocket::AcceptFlow::unlock`](io.cppm.md#fn-cdde5d0a4659acac)
- [`xv::net::TcpSocket::AcceptFlow::wait`](io.cppm.md#fn-d8d8256d49dff7c2)
- [`xv::net::TcpSocket::ConnectFlow::bind`](io.cppm.md#fn-20aabb9cc0cddcb8)
- [`xv::net::TcpSocket::ConnectFlow::bound`](io.cppm.md#fn-3e733c2c30fc78c2)
- [`xv::net::TcpSocket::ConnectFlow::closed`](io.cppm.md#fn-d3b14bde800ad284)
- [`xv::net::TcpSocket::ConnectFlow::completed`](io.cppm.md#fn-84e545967a31969f)
- [`xv::net::TcpSocket::ConnectFlow::in_progress`](io.cppm.md#fn-0d9c22efb487541d)
- [`xv::net::TcpSocket::ConnectFlow::initialize`](io.cppm.md#fn-692e8e72d1bc5c62)
- [`xv::net::TcpSocket::ConnectFlow::interrupted`](io.cppm.md#fn-0e1e901648b7c95c)
- [`xv::net::TcpSocket::ConnectFlow::invalid`](io.cppm.md#fn-2a4bec32bb04b5d0)
- [`xv::net::TcpSocket::ConnectFlow::lock`](io.cppm.md#fn-1fb9f79e0ba994c7)
- [`xv::net::TcpSocket::ConnectFlow::nonblocking`](io.cppm.md#fn-7ba5fea2196dc72b)
- [`xv::net::TcpSocket::ConnectFlow::state_error`](io.cppm.md#fn-d3e1a5a38025d6f4)
- [`xv::net::TcpSocket::ConnectFlow::unlock`](io.cppm.md#fn-6a11b6e49dc88f5e)
- [`xv::net::TcpSocket::ConnectFlow::valid_peer`](io.cppm.md#fn-251d5e11ee6e5012)
- [`xv::net::TcpSocket::ConnectFlow::wait`](io.cppm.md#fn-717f828742f822fe)
- [`xv::net::TcpSocket::ConnectFlow::waiting`](io.cppm.md#fn-7498fe0ac9047250)
- [`xv::net::TcpSocket::ReadFlow::Context::Context`](io.cppm.md#fn-fdd788f2f8b54e5d)
- [`xv::net::TcpSocket::ReadFlow::again`](io.cppm.md#fn-be7ec0d389524938)
- [`xv::net::TcpSocket::ReadFlow::connected`](io.cppm.md#fn-f14a9dcebe4a189c)
- [`xv::net::TcpSocket::ReadFlow::copy_out`](io.cppm.md#fn-463fa3cc97adf9a1)
- [`xv::net::TcpSocket::ReadFlow::empty`](io.cppm.md#fn-e1d192ea648cb46e)
- [`xv::net::TcpSocket::ReadFlow::eof`](io.cppm.md#fn-8f93cd26c77c0500)
- [`xv::net::TcpSocket::ReadFlow::error`](io.cppm.md#fn-6d945eab9c6222da)
- [`xv::net::TcpSocket::ReadFlow::interrupted`](io.cppm.md#fn-00ccc5ec79a44090)
- [`xv::net::TcpSocket::ReadFlow::invalid`](io.cppm.md#fn-e7315d37c55afe7f)
- [`xv::net::TcpSocket::ReadFlow::lock`](io.cppm.md#fn-45c8f7afc4303435)
- [`xv::net::TcpSocket::ReadFlow::negative`](io.cppm.md#fn-1283cbf104fb4634)
- [`xv::net::TcpSocket::ReadFlow::nonblocking`](io.cppm.md#fn-67029573b2e7694c)
- [`xv::net::TcpSocket::ReadFlow::not_connected`](io.cppm.md#fn-bd9e814c554fb78e)
- [`xv::net::TcpSocket::ReadFlow::read`](io.cppm.md#fn-bc9fab7b35781b3d)
- [`xv::net::TcpSocket::ReadFlow::readable`](io.cppm.md#fn-a7bb859e067c45c1)
- [`xv::net::TcpSocket::ReadFlow::socket_error`](io.cppm.md#fn-df0ce613daf2cd1e)
- [`xv::net::TcpSocket::ReadFlow::unlock`](io.cppm.md#fn-42fe6b83a9224df2)
- [`xv::net::TcpSocket::ReadFlow::wait`](io.cppm.md#fn-a320824b704bf48d)
- [`xv::net::TcpSocket::WriteFlow::Context::Context`](io.cppm.md#fn-47345d9b4c618a1d)
- [`xv::net::TcpSocket::WriteFlow::again`](io.cppm.md#fn-99a06b5648328690)
- [`xv::net::TcpSocket::WriteFlow::append`](io.cppm.md#fn-27fc675041476f2a)
- [`xv::net::TcpSocket::WriteFlow::connected`](io.cppm.md#fn-9ba0ce0c90a44992)
- [`xv::net::TcpSocket::WriteFlow::copied`](io.cppm.md#fn-6758becbb4c8ab60)
- [`xv::net::TcpSocket::WriteFlow::empty`](io.cppm.md#fn-79410f7eb8dbeb27)
- [`xv::net::TcpSocket::WriteFlow::error`](io.cppm.md#fn-99eeadc38136d201)
- [`xv::net::TcpSocket::WriteFlow::interrupted`](io.cppm.md#fn-6b92a0059de6c421)
- [`xv::net::TcpSocket::WriteFlow::invalid`](io.cppm.md#fn-04978f67677552b9)
- [`xv::net::TcpSocket::WriteFlow::lock`](io.cppm.md#fn-3d22d4d81a70e457)
- [`xv::net::TcpSocket::WriteFlow::negative`](io.cppm.md#fn-075881a788010d31)
- [`xv::net::TcpSocket::WriteFlow::nonblocking`](io.cppm.md#fn-e8394eda42e36aaf)
- [`xv::net::TcpSocket::WriteFlow::not_connected`](io.cppm.md#fn-fdb64b6b02eb8fc4)
- [`xv::net::TcpSocket::WriteFlow::pipe`](io.cppm.md#fn-1e2ef1375c623b80)
- [`xv::net::TcpSocket::WriteFlow::routable`](io.cppm.md#fn-950f480d0c5695f9)
- [`xv::net::TcpSocket::WriteFlow::shut`](io.cppm.md#fn-361c85a6c19d3bb8)
- [`xv::net::TcpSocket::WriteFlow::socket_error`](io.cppm.md#fn-be0e2b2f596381f0)
- [`xv::net::TcpSocket::WriteFlow::space`](io.cppm.md#fn-dfb7553b84330a15)
- [`xv::net::TcpSocket::WriteFlow::stage`](io.cppm.md#fn-c8c28918f9b90615)
- [`xv::net::TcpSocket::WriteFlow::unlock`](io.cppm.md#fn-e0fc9d2f4b4b8865)
- [`xv::net::TcpSocket::WriteFlow::unreachable`](io.cppm.md#fn-f16a640ea413002f)
- [`xv::net::TcpSocket::WriteFlow::wait`](io.cppm.md#fn-b7224addfbb3e689)

<a id="fn-64d40b1ceb2ff4f8"></a>

## `xv::net::TcpSocket::AcceptFlow::accept`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::accept(Context &c) noexcept
@brief 尝试接管一个待接受子连接；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-5cbe87c9dfdbf37e"></a>

## `xv::net::TcpSocket::AcceptFlow::accepted`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcceptFlow::accepted(Context &c) noexcept
@brief 检查子连接或设备提交是否成功；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b8c5f33a26c3ac62"></a>

## `xv::net::TcpSocket::AcceptFlow::again`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::again(Context &c) noexcept
@brief 记录非阻塞暂不可用的负 EAGAIN；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-160a6133920c6fec"></a>

## `xv::net::TcpSocket::AcceptFlow::interrupted`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcceptFlow::interrupted(Context &) noexcept
@brief 检查当前等待是否已经被中断；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-36a28a3ad5cd8571"></a>

## `xv::net::TcpSocket::AcceptFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-22fb6d6c17436792"></a>

## `xv::net::TcpSocket::AcceptFlow::listening`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcceptFlow::listening(Context &c) noexcept
@brief 检查对象是否处于监听状态；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c7b73f4a9172d92f"></a>

## `xv::net::TcpSocket::AcceptFlow::nonblocking`

```doxygen
@fn static inline bool xv::net::TcpSocket::AcceptFlow::nonblocking(Context &c) noexcept
@brief 检查本次应用调用是否为非阻塞；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a87fc4641d555ab0"></a>

## `xv::net::TcpSocket::AcceptFlow::success`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::success(Context &c) noexcept
@brief 记录本阶段已经成功完成；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-cdde5d0a4659acac"></a>

## `xv::net::TcpSocket::AcceptFlow::unlock`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::unlock(Context &c) noexcept
@brief 释放本次 socket 调用仍持有的注册表锁；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d8d8256d49dff7c2"></a>

## `xv::net::TcpSocket::AcceptFlow::wait`

```doxygen
@fn static inline void xv::net::TcpSocket::AcceptFlow::wait(Context &c) noexcept
@brief 沿原等待协议登记、解锁睡眠并重新持锁；所属流程为 xv::net::TcpSocket::AcceptFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-20aabb9cc0cddcb8"></a>

## `xv::net::TcpSocket::ConnectFlow::bind`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::bind(Context &c) noexcept
@brief 完成绑定或进入租约已绑定状态；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3e733c2c30fc78c2"></a>

## `xv::net::TcpSocket::ConnectFlow::bound`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::bound(Context &c) noexcept
@brief 检查绑定步骤的业务结果是否成功；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d3b14bde800ad284"></a>

## `xv::net::TcpSocket::ConnectFlow::closed`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::closed(Context &c) noexcept
@brief 检查连接是否已经关闭；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-84e545967a31969f"></a>

## `xv::net::TcpSocket::ConnectFlow::completed`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::completed(Context &c) noexcept
@brief 根据握手最终状态生成 connect 返回码；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0d9c22efb487541d"></a>

## `xv::net::TcpSocket::ConnectFlow::in_progress`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::in_progress(Context &c) noexcept
@brief 记录非阻塞连接正在进行的负 EINPROGRESS；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-692e8e72d1bc5c62"></a>

## `xv::net::TcpSocket::ConnectFlow::initialize`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::initialize(Context &c) noexcept
@brief 初始化当前业务阶段状态；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-0e1e901648b7c95c"></a>

## `xv::net::TcpSocket::ConnectFlow::interrupted`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::interrupted(Context &) noexcept
@brief 检查当前等待是否已经被中断；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-2a4bec32bb04b5d0"></a>

## `xv::net::TcpSocket::ConnectFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1fb9f79e0ba994c7"></a>

## `xv::net::TcpSocket::ConnectFlow::lock`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::lock(Context &c) noexcept
@brief 获取本次 socket 操作原有的注册表锁；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7ba5fea2196dc72b"></a>

## `xv::net::TcpSocket::ConnectFlow::nonblocking`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::nonblocking(Context &c) noexcept
@brief 检查本次应用调用是否为非阻塞；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-d3e1a5a38025d6f4"></a>

## `xv::net::TcpSocket::ConnectFlow::state_error`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::state_error(Context &c) noexcept
@brief 根据当前连接状态返回 EALREADY 或 EISCONN；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6a11b6e49dc88f5e"></a>

## `xv::net::TcpSocket::ConnectFlow::unlock`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::unlock(Context &c) noexcept
@brief 释放本次 socket 调用仍持有的注册表锁；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-251d5e11ee6e5012"></a>

## `xv::net::TcpSocket::ConnectFlow::valid_peer`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::valid_peer(Context &c) noexcept
@brief 检查连接对端地址和端口是否均非零；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-717f828742f822fe"></a>

## `xv::net::TcpSocket::ConnectFlow::wait`

```doxygen
@fn static inline void xv::net::TcpSocket::ConnectFlow::wait(Context &c) noexcept
@brief 沿原等待协议登记、解锁睡眠并重新持锁；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-7498fe0ac9047250"></a>

## `xv::net::TcpSocket::ConnectFlow::waiting`

```doxygen
@fn static inline bool xv::net::TcpSocket::ConnectFlow::waiting(Context &c) noexcept
@brief 检查主动连接是否仍在握手等待；所属流程为 xv::net::TcpSocket::ConnectFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-be7ec0d389524938"></a>

## `xv::net::TcpSocket::ReadFlow::again`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::again(Context &c) noexcept
@brief 记录非阻塞暂不可用的负 EAGAIN；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f14a9dcebe4a189c"></a>

## `xv::net::TcpSocket::ReadFlow::connected`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::connected(Context &c) noexcept
@brief 检查当前 socket 是否允许本次连接操作；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-463fa3cc97adf9a1"></a>

## `xv::net::TcpSocket::ReadFlow::copy_out`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::copy_out(Context &c) noexcept
@brief 将已取出的业务数据复制给用户；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e1d192ea648cb46e"></a>

## `xv::net::TcpSocket::ReadFlow::empty`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::empty(Context &c) noexcept
@brief 检查零长度请求或空闲表槽；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-8f93cd26c77c0500"></a>

## `xv::net::TcpSocket::ReadFlow::eof`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::eof(Context &c) noexcept
@brief 检查接收是否到达 EOF 或读侧已经关闭；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6d945eab9c6222da"></a>

## `xv::net::TcpSocket::ReadFlow::error`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::error(Context &c) noexcept
@brief 检查 socket 是否有待返回的错误；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-00ccc5ec79a44090"></a>

## `xv::net::TcpSocket::ReadFlow::interrupted`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::interrupted(Context &) noexcept
@brief 检查当前等待是否已经被中断；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e7315d37c55afe7f"></a>

## `xv::net::TcpSocket::ReadFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-45c8f7afc4303435"></a>

## `xv::net::TcpSocket::ReadFlow::lock`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::lock(Context &c) noexcept
@brief 获取本次 socket 操作原有的注册表锁；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1283cbf104fb4634"></a>

## `xv::net::TcpSocket::ReadFlow::negative`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::negative(Context &c) noexcept
@brief 检查应用请求长度是否为负值；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-67029573b2e7694c"></a>

## `xv::net::TcpSocket::ReadFlow::nonblocking`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::nonblocking(Context &c) noexcept
@brief 检查本次应用调用是否为非阻塞；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bd9e814c554fb78e"></a>

## `xv::net::TcpSocket::ReadFlow::not_connected`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::not_connected(Context &c) noexcept
@brief 记录尚未连接的负 ENOTCONN；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-bc9fab7b35781b3d"></a>

## `xv::net::TcpSocket::ReadFlow::read`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::read(Context &c) noexcept
@brief 读取接收流或检查是否关闭读侧；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a7bb859e067c45c1"></a>

## `xv::net::TcpSocket::ReadFlow::readable`

```doxygen
@fn static inline bool xv::net::TcpSocket::ReadFlow::readable(Context &c) noexcept
@brief 检查当前队列或接收流是否存在可读内容；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-df0ce613daf2cd1e"></a>

## `xv::net::TcpSocket::ReadFlow::socket_error`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::socket_error(Context &c) noexcept
@brief 返回 socket 原有负错误码；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-42fe6b83a9224df2"></a>

## `xv::net::TcpSocket::ReadFlow::unlock`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::unlock(Context &c) noexcept
@brief 释放本次 socket 调用仍持有的注册表锁；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-a320824b704bf48d"></a>

## `xv::net::TcpSocket::ReadFlow::wait`

```doxygen
@fn static inline void xv::net::TcpSocket::ReadFlow::wait(Context &c) noexcept
@brief 沿原等待协议登记、解锁睡眠并重新持锁；所属流程为 xv::net::TcpSocket::ReadFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fdd788f2f8b54e5d"></a>

## `xv::net::TcpSocket::ReadFlow::Context::Context`

```doxygen
@fn xv::net::TcpSocket::ReadFlow::Context::Context(TcpSocket &socket, xv::io::Buffer buffer, int size)
@brief 借用业务对象并组装本次调用上下文；所属流程为 xv::net::TcpSocket::ReadFlow::Context。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-99a06b5648328690"></a>

## `xv::net::TcpSocket::WriteFlow::again`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::again(Context &c) noexcept
@brief 记录非阻塞暂不可用的负 EAGAIN；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-27fc675041476f2a"></a>

## `xv::net::TcpSocket::WriteFlow::append`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::append(Context &c) noexcept
@brief 追加已验证的数据、路由或对象记录；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-9ba0ce0c90a44992"></a>

## `xv::net::TcpSocket::WriteFlow::connected`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::connected(Context &c) noexcept
@brief 检查当前 socket 是否允许本次连接操作；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6758becbb4c8ab60"></a>

## `xv::net::TcpSocket::WriteFlow::copied`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::copied(Context &c) noexcept
@brief 检查用户输入是否复制成功；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-79410f7eb8dbeb27"></a>

## `xv::net::TcpSocket::WriteFlow::empty`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::empty(Context &c) noexcept
@brief 检查零长度请求或空闲表槽；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-99eeadc38136d201"></a>

## `xv::net::TcpSocket::WriteFlow::error`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::error(Context &c) noexcept
@brief 检查 socket 是否有待返回的错误；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-6b92a0059de6c421"></a>

## `xv::net::TcpSocket::WriteFlow::interrupted`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::interrupted(Context &) noexcept
@brief 检查当前等待是否已经被中断；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-04978f67677552b9"></a>

## `xv::net::TcpSocket::WriteFlow::invalid`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::invalid(Context &c) noexcept
@brief 记录当前业务输入不合法的错误；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-3d22d4d81a70e457"></a>

## `xv::net::TcpSocket::WriteFlow::lock`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::lock(Context &c) noexcept
@brief 获取本次 socket 操作原有的注册表锁；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-075881a788010d31"></a>

## `xv::net::TcpSocket::WriteFlow::negative`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::negative(Context &c) noexcept
@brief 检查应用请求长度是否为负值；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e8394eda42e36aaf"></a>

## `xv::net::TcpSocket::WriteFlow::nonblocking`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::nonblocking(Context &c) noexcept
@brief 检查本次应用调用是否为非阻塞；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-fdb64b6b02eb8fc4"></a>

## `xv::net::TcpSocket::WriteFlow::not_connected`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::not_connected(Context &c) noexcept
@brief 记录尚未连接的负 ENOTCONN；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-1e2ef1375c623b80"></a>

## `xv::net::TcpSocket::WriteFlow::pipe`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::pipe(Context &c) noexcept
@brief 记录写侧关闭后的负 EPIPE；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-950f480d0c5695f9"></a>

## `xv::net::TcpSocket::WriteFlow::routable`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::routable(Context &c) noexcept
@brief 检查当前路由是否能提供有效 MSS；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-361c85a6c19d3bb8"></a>

## `xv::net::TcpSocket::WriteFlow::shut`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::shut(Context &c) noexcept
@brief 检查 socket 写侧是否已经关闭；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-be0e2b2f596381f0"></a>

## `xv::net::TcpSocket::WriteFlow::socket_error`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::socket_error(Context &c) noexcept
@brief 返回 socket 原有负错误码；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-dfb7553b84330a15"></a>

## `xv::net::TcpSocket::WriteFlow::space`

```doxygen
@fn static inline bool xv::net::TcpSocket::WriteFlow::space(Context &c) noexcept
@brief 检查队列或发送缓冲是否仍有容量；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-c8c28918f9b90615"></a>

## `xv::net::TcpSocket::WriteFlow::stage`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::stage(Context &c) noexcept
@brief 限制本次应用写入的暂存长度；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-e0fc9d2f4b4b8865"></a>

## `xv::net::TcpSocket::WriteFlow::unlock`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::unlock(Context &c) noexcept
@brief 释放本次 socket 调用仍持有的注册表锁；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-f16a640ea413002f"></a>

## `xv::net::TcpSocket::WriteFlow::unreachable`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::unreachable(Context &c) noexcept
@brief 记录路由不可达或发送 ICMP 不可达响应；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-b7224addfbb3e689"></a>

## `xv::net::TcpSocket::WriteFlow::wait`

```doxygen
@fn static inline void xv::net::TcpSocket::WriteFlow::wait(Context &c) noexcept
@brief 沿原等待协议登记、解锁睡眠并重新持锁；所属流程为 xv::net::TcpSocket::WriteFlow。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```

<a id="fn-47345d9b4c618a1d"></a>

## `xv::net::TcpSocket::WriteFlow::Context::Context`

```doxygen
@fn xv::net::TcpSocket::WriteFlow::Context::Context(TcpSocket &socket, xv::io::Buffer buffer, int size)
@brief 借用业务对象并组装本次调用上下文；所属流程为 xv::net::TcpSocket::WriteFlow::Context。
@details 上下文借用或持有本次操作所需业务值；下一节点由 Graph 的类型策略决定，不由动作返回目标。
```
