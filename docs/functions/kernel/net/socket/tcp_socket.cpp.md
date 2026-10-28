# `kernel/net/socket/tcp_socket.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpSocket::TcpSocket`](tcp_socket.cpp.md#fn-feacfcf142e49326)
- [`xv::net::TcpSocket::initialize`](tcp_socket.cpp.md#fn-c62835414fcd2c60)
- [`xv::net::TcpSocket::fail`](tcp_socket.cpp.md#fn-42ede9b5fed95194)
- [`xv::net::TcpSocket::enter_time_wait`](tcp_socket.cpp.md#fn-d8a9c5a73ce2babb)
- [`xv::net::TcpSocket::matches`](tcp_socket.cpp.md#fn-8d2f58bae366b18d)
- [`xv::net::TcpSocket::listening`](tcp_socket.cpp.md#fn-ecfbb01f47787bd0)
- [`xv::net::TcpSocket::connect`](tcp_socket.cpp.md#fn-9e07fc39cb3c17a2)
- [`xv::net::TcpSocket::listen`](tcp_socket.cpp.md#fn-7e3a04ae59672f21)
- [`xv::net::TcpSocket::passive`](tcp_socket.cpp.md#fn-c8523a7152fb629f)
- [`xv::net::TcpSocket::accept`](tcp_socket.cpp.md#fn-eb2f93e4dada729f)
- [`xv::net::TcpSocket::read`](tcp_socket.cpp.md#fn-8c178dd34591a5e9)
- [`xv::net::TcpSocket::write`](tcp_socket.cpp.md#fn-633245e696cf6ebf)
- [`xv::net::TcpSocket::send_to`](tcp_socket.cpp.md#fn-231bbbb4f11fc234)
- [`xv::net::TcpSocket::receive_from`](tcp_socket.cpp.md#fn-12f33b275e0e4dec)
- [`xv::net::TcpSocket::poll`](tcp_socket.cpp.md#fn-31670288cdab90bc)
- [`xv::net::TcpSocket::shutdown`](tcp_socket.cpp.md#fn-1eb72bf0aca4a8a8)
- [`xv::net::TcpSocket::close`](tcp_socket.cpp.md#fn-c7164ce40d0f4ab4)
- [`xv::net::TcpSocket::destroy`](tcp_socket.cpp.md#fn-9f612ac9dc8ef596)
- [`xv::net::TcpSocket::tick`](tcp_socket.cpp.md#fn-b862f8511a782329)

<a id="fn-feacfcf142e49326"></a>

## `xv::net::TcpSocket::TcpSocket`

```doxygen
@fn xv::net::TcpSocket::TcpSocket(SocketRegistry &registry)
@brief 创建由统一 SocketRegistry 管理的 TCP 对象。
@details 收发环在 initialize 中分阶段申请，构造函数本身不分配页面。
```

<a id="fn-c62835414fcd2c60"></a>

## `xv::net::TcpSocket::initialize`

```doxygen
@fn bool xv::net::TcpSocket::initialize()
@brief 为连接初始化 16 KiB 发送环和 16 KiB 接收环。
@details 四个发送页和四个接收页按顺序分配；任一步失败返回 false，由工厂调用 destroy 回收对象页及所有已分配页。
```

<a id="fn-42ede9b5fed95194"></a>

## `xv::net::TcpSocket::fail`

```doxygen
@fn void xv::net::TcpSocket::fail(int error)
@brief 终止连接并取消其待发送责任。
@details 记录错误、进入 CLOSED、对齐发送确认前沿并清空排队状态，避免定时器再次发送旧数据。应用对象由最后关闭路径释放，未接入子连接由注册表回收。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-d8a9c5a73ce2babb"></a>

## `xv::net::TcpSocket::enter_time_wait`

```doxygen
@fn void xv::net::TcpSocket::enter_time_wait()
@brief TcpSocket：持注册锁转入 TIME-WAIT，设置本项目 30 秒保留期并通知等待者；匹配的重发 FIN 会重新计时。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁转入 TIME-WAIT，设置本项目 30 秒保留期并通知等待者；匹配的重发 FIN 会重新计时。
```

<a id="fn-8d2f58bae366b18d"></a>

## `xv::net::TcpSocket::matches`

```doxygen
@fn bool xv::net::TcpSocket::matches(Endpoint local, Endpoint peer) const
@brief TcpSocket：持注册锁匹配活动连接四元组，排除 CLOSED 与 LISTEN；用于避免把同端口子连接误当 listener。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁匹配活动连接四元组，排除 CLOSED 与 LISTEN；用于避免把同端口子连接误当 listener。
```

<a id="fn-ecfbb01f47787bd0"></a>

## `xv::net::TcpSocket::listening`

```doxygen
@fn bool xv::net::TcpSocket::listening(Endpoint local) const
@brief TcpSocket：持注册锁匹配 LISTEN 端口与本地地址，ANY 可匹配到本机目标。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁匹配 LISTEN 端口与本地地址，ANY 可匹配到本机目标。
```

<a id="fn-9e07fc39cb3c17a2"></a>

## `xv::net::TcpSocket::connect`

```doxygen
@fn int xv::net::TcpSocket::connect(Endpoint peer, bool nonblocking)
@brief 执行主动打开，支持阻塞和非阻塞连接。
@details 检查状态并自动绑定本地地址，初始化收发序号与 SYN 重试；非阻塞返回 -EINPROGRESS。阻塞等待可被信号打断，连接握手仍保留供 poll/SO_ERROR 查询。
```

<a id="fn-7e3a04ae59672f21"></a>

## `xv::net::TcpSocket::listen`

```doxygen
@fn int xv::net::TcpSocket::listen(int backlog)
@brief 把已绑定或自动分配端口的对象切换为监听状态。
@details 允许 CLOSED 或已有 LISTEN 状态，backlog 限制为一至八；后续 SYN 由协议分派器分配独立子连接。
```

<a id="fn-c8523a7152fb629f"></a>

## `xv::net::TcpSocket::passive`

```doxygen
@fn void xv::net::TcpSocket::passive(TcpSocket &listener, Endpoint local, Endpoint peer, const unsigned char *syn, unsigned length)
@brief 为监听 socket 建立一个尚未 accept 的被动连接。
@details 校验 MSS，保存四元组、对端窗口和接收初始序号，再发送 SYN-ACK。owned_ 为 false，握手失败或监听关闭时由注册表负责回收。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-eb2f93e4dada729f"></a>

## `xv::net::TcpSocket::accept`

```doxygen
@fn Socket *xv::net::TcpSocket::accept(bool nonblocking, int &error)
@brief 在统一注册锁内请求注册表接管一个可接受子连接，没有就绪项则按非阻塞或可中断等待规则处理。
@details 注册表负责查找与解除监听关联；socket 保持原有的等待登记、解锁睡眠和唤醒检查协议。成功返回同一连接对象及收发队列。
```

<a id="fn-8c178dd34591a5e9"></a>

## `xv::net::TcpSocket::read`

```doxygen
@fn int xv::net::TcpSocket::read(xv::io::Buffer destination, int count)
@brief 把已连续的接收字节复制给应用并更新窗口。
@details 每次最多暂存 1024 字节，释放注册锁后复制到用户空间；从零窗口恢复、完全腾空或累计释放达到 MSS 时发送窗口更新。无数据时区分 EOF、错误、非阻塞和信号中断。
```

<a id="fn-633245e696cf6ebf"></a>

## `xv::net::TcpSocket::write`

```doxygen
@fn int xv::net::TcpSocket::write(xv::io::Buffer source, int count)
@brief 把应用数据排入发送环并尝试推进网络发送。
@details 每次最多复制 1024 字节，返回已排队字节数。发送环有空间时不等待对端 ACK 或非零窗口；满时阻塞或返回 -EAGAIN。首次活动设置超时基准。
```

<a id="fn-231bbbb4f11fc234"></a>

## `xv::net::TcpSocket::send_to`

```doxygen
@fn int xv::net::TcpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
@brief 按连接字节流语义处理 sendto。
@details 已连接 TCP 不允许逐次指定远端，传入 peer 返回 -EISCONN，否则委托 write。
```

<a id="fn-12f33b275e0e4dec"></a>

## `xv::net::TcpSocket::receive_from`

```doxygen
@fn int xv::net::TcpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
@brief 按字节流语义读取数据并可返回固定远端。
@details 委托 read，成功或 EOF 后可复制当前 peer；没有 UDP 数据报边界。
```

<a id="fn-31670288cdab90bc"></a>

## `xv::net::TcpSocket::poll`

```doxygen
@fn unsigned xv::net::TcpSocket::poll(xv::sync::PollTable *table)
@brief 汇总连接、收发缓冲与关闭状态的就绪事件，监听状态委托注册表检查待接受子连接。
@details 在统一注册锁内登记 poll 观察并读取状态，监听 POLLIN 与 accept 共用同一子连接就绪条件。
```

<a id="fn-1eb72bf0aca4a8a8"></a>

## `xv::net::TcpSocket::shutdown`

```doxygen
@fn int xv::net::TcpSocket::shutdown(int how)
@brief 关闭连接的读方向、写方向或两个方向。
@details 读关闭丢弃当前连续队列并通知窗口；写关闭通过 pump 将 FIN 排在待发数据后，保留未确认字节和另一方向。
```

<a id="fn-c7164ce40d0f4ab4"></a>

## `xv::net::TcpSocket::close`

```doxygen
@fn void xv::net::TcpSocket::close()
@brief 移除应用接管责任，监听关闭时委托注册表复位并回收未接受子连接。
@details 已接受连接继续独立存活；握手中连接发送复位，已建立连接继续发送排队数据和 FIN，CLOSED 对象立即由注册表回收。整个状态转换仍在统一注册锁内。
```

<a id="fn-9f612ac9dc8ef596"></a>

## `xv::net::TcpSocket::destroy`

```doxygen
@fn void xv::net::TcpSocket::destroy()
@brief 归还 TCP 对象持有的所有页面。
@details 先释放发送环与接收环已分配的各页，再析构并归还对象页；同一清理路径覆盖任意阶段分配失败。
```

<a id="fn-b862f8511a782329"></a>

## `xv::net::TcpSocket::tick`

```doxygen
@fn void xv::net::TcpSocket::tick(unsigned long now)
@brief 维护重传、零窗口、发送推进与关闭回收。
@details 普通超时按 NewReno 收缩拥塞窗口并指数退避，零窗口使用独立 persist。挂起数据长期无活动时失败，过旧跨核时钟快照不会造成减法下溢；最后回收无应用拥有权的 CLOSED 对象。
@pre 调用者持有 SocketRegistry 注册锁。
```
