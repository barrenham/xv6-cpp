# `kernel/net/socket/tcp_socket.cpp`

## 函数导航

- [`xv::net::TcpSocket::TcpSocket`](tcp_socket.cpp.md#fn-feacfcf142e49326)
- [`xv::net::TcpSocket::accept`](tcp_socket.cpp.md#fn-eb2f93e4dada729f)
- [`xv::net::TcpSocket::acknowledge`](tcp_socket.cpp.md#fn-908ef432e52e4bd8)
- [`xv::net::TcpSocket::close`](tcp_socket.cpp.md#fn-c7164ce40d0f4ab4)
- [`xv::net::TcpSocket::connect`](tcp_socket.cpp.md#fn-9e07fc39cb3c17a2)
- [`xv::net::TcpSocket::destroy`](tcp_socket.cpp.md#fn-9f612ac9dc8ef596)
- [`xv::net::TcpSocket::emit`](tcp_socket.cpp.md#fn-1e8c0fad1b1d52f4)
- [`xv::net::TcpSocket::enter_time_wait`](tcp_socket.cpp.md#fn-d8a9c5a73ce2babb)
- [`xv::net::TcpSocket::fail`](tcp_socket.cpp.md#fn-42ede9b5fed95194)
- [`xv::net::TcpSocket::finish`](tcp_socket.cpp.md#fn-cdfa0effd85a294d)
- [`xv::net::TcpSocket::incoming`](tcp_socket.cpp.md#fn-e3735def055c42b7)
- [`xv::net::TcpSocket::initialize`](tcp_socket.cpp.md#fn-c62835414fcd2c60)
- [`xv::net::TcpSocket::listen`](tcp_socket.cpp.md#fn-7e3a04ae59672f21)
- [`xv::net::TcpSocket::listening`](tcp_socket.cpp.md#fn-ecfbb01f47787bd0)
- [`xv::net::TcpSocket::matches`](tcp_socket.cpp.md#fn-8d2f58bae366b18d)
- [`xv::net::TcpSocket::passive`](tcp_socket.cpp.md#fn-c8523a7152fb629f)
- [`xv::net::TcpSocket::poll`](tcp_socket.cpp.md#fn-31670288cdab90bc)
- [`xv::net::TcpSocket::queue`](tcp_socket.cpp.md#fn-2d9f1c832b17f59b)
- [`xv::net::TcpSocket::read`](tcp_socket.cpp.md#fn-8c178dd34591a5e9)
- [`xv::net::TcpSocket::receive_from`](tcp_socket.cpp.md#fn-12f33b275e0e4dec)
- [`xv::net::TcpSocket::send_to`](tcp_socket.cpp.md#fn-231bbbb4f11fc234)
- [`xv::net::TcpSocket::shutdown`](tcp_socket.cpp.md#fn-1eb72bf0aca4a8a8)
- [`xv::net::TcpSocket::tick`](tcp_socket.cpp.md#fn-b862f8511a782329)
- [`xv::net::TcpSocket::window`](tcp_socket.cpp.md#fn-92a05e6d9952e0c5)
- [`xv::net::TcpSocket::write`](tcp_socket.cpp.md#fn-633245e696cf6ebf)
- [`xv::net::initial_sequence`](tcp_socket.cpp.md#fn-d8dab64ecfa6d2b6)
- [`xv::net::peer_mss`](tcp_socket.cpp.md#fn-84f125de527bd05b)

<a id="fn-d8dab64ecfa6d2b6"></a>

## `xv::net::initial_sequence`

```doxygen
@fn unsigned xv::net::initial_sequence()
@brief 把单调时间与原子递增计数混合生成教学 TCP 初始序号；不是密码学随机源。
@details 把单调时间与原子递增计数混合生成教学 TCP 初始序号；不是密码学随机源，不宣称生产级 ISN 防护。
```

<a id="fn-84f125de527bd05b"></a>

## `xv::net::peer_mss`

```doxygen
@fn unsigned xv::net::peer_mss(const unsigned char *p, unsigned length)
@brief 在 SYN data offset 范围内有界解析 TCP 选项。
@details 在 SYN data offset 范围内有界解析 TCP 选项，跳过 NOP/未知项、识别 MSS；结构错误返回零，协商上限 1460。
```

<a id="fn-feacfcf142e49326"></a>

## `xv::net::TcpSocket::TcpSocket`

```doxygen
@fn xv::net::TcpSocket::TcpSocket(SocketRegistry &registry)
@brief 构造 TcpSocket 的状态与依赖。
@details 构造 TCP 协议标识与初始 CLOSED 状态，独立 RX 页由 initialize 申请。
```

<a id="fn-c62835414fcd2c60"></a>

## `xv::net::TcpSocket::initialize`

```doxygen
@fn bool xv::net::TcpSocket::initialize()
@brief TcpSocket：为 TCP 字节流申请独立 4096 字节 RX 环；失败返回 false，外层工厂负责回滚对象页。
@details 为 TCP 字节流申请独立 4096 字节 RX 环；失败返回 false，外层工厂负责回滚对象页。
```

<a id="fn-92a05e6d9952e0c5"></a>

## `xv::net::TcpSocket::window`

```doxygen
@fn unsigned xv::net::TcpSocket::window() const
@brief TcpSocket：持注册锁计算 4096 字节 RX 环的剩余容量；读关闭后按丢弃接收模式通告完整窗口。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁计算 4096 字节 RX 环的剩余容量；读关闭后按丢弃接收模式通告完整窗口。
```

<a id="fn-1e8c0fad1b1d52f4"></a>

## `xv::net::TcpSocket::emit`

```doxygen
@fn void xv::net::TcpSocket::emit(unsigned flags, const void *data, unsigned length)
@brief TcpSocket：持注册锁按当前 send_next/receive_next 和可用接收窗口发送控制段，不登记新的重传数据。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁按当前 send_next/receive_next 和可用接收窗口发送控制段，不登记新的重传数据。
```

<a id="fn-2d9f1c832b17f59b"></a>

## `xv::net::TcpSocket::queue`

```doxygen
@fn void xv::net::TcpSocket::queue(unsigned flags, const void *data, unsigned length)
@brief TcpSocket：持注册锁复制发送负载到独占重传缓冲，记录序号/控制位/长度并推进 send_next，启动重传计时再发送。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁复制发送负载到独占重传缓冲，记录序号/控制位/长度并推进 send_next，启动重传计时再发送。一次最多一个未确认段，SYN/FIN 各消耗一个序号。
```

<a id="fn-42ede9b5fed95194"></a>

## `xv::net::TcpSocket::fail`

```doxygen
@fn void xv::net::TcpSocket::fail(int error)
@brief TcpSocket：持注册锁记录正 errno、转 CLOSED、停止在途重传并通知应用及 listener；不会直接释放仍由应用 File 持有的对象。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁记录正 errno、转 CLOSED、停止在途重传并通知应用及 listener；不会直接释放仍由应用 File 持有的对象。
```

<a id="fn-d8a9c5a73ce2babb"></a>

## `xv::net::TcpSocket::enter_time_wait`

```doxygen
@fn void xv::net::TcpSocket::enter_time_wait()
@brief TcpSocket：持注册锁转入 TIME-WAIT，设置本项目 30 秒保留期并通知等待者；匹配的重发 FIN 会重新计时。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁转入 TIME-WAIT，设置本项目 30 秒保留期并通知等待者；匹配的重发 FIN 会重新计时。
```

<a id="fn-cdfa0effd85a294d"></a>

## `xv::net::TcpSocket::finish`

```doxygen
@fn void xv::net::TcpSocket::finish()
@brief TcpSocket：持注册锁检查写半关闭且没有在途数据，分别从 ESTABLISHED 或 CLOSE-WAIT 排队 FIN，进入 FIN-WAIT-1 或 LAST-ACK。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁检查写半关闭且没有在途数据，分别从 ESTABLISHED 或 CLOSE-WAIT 排队 FIN，进入 FIN-WAIT-1 或 LAST-ACK。
```

<a id="fn-908ef432e52e4bd8"></a>

## `xv::net::TcpSocket::acknowledge`

```doxygen
@fn void xv::net::TcpSocket::acknowledge(unsigned acknowledgement)
@brief TcpSocket：验证 ACK 位于已发送序号范围；完整 ACK 清除在途段并采样 RTT，部分 ACK 裁剪已确认前缀并重启计时。
@pre 调用者持有 SocketRegistry 条件锁。
@details 验证 ACK 位于已发送序号范围；完整 ACK 清除在途段并采样 RTT，部分 ACK 裁剪已确认前缀并重启计时。FIN 确认推进 FIN-WAIT-2/CLOSED/TIME-WAIT，必要时触发等待中的 FIN。
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

<a id="fn-7e3a04ae59672f21"></a>

## `xv::net::TcpSocket::listen`

```doxygen
@fn int xv::net::TcpSocket::listen(int backlog)
@brief TcpSocket：把 CLOSED socket 转为 LISTEN，必要时自动分配端口；backlog 限制 1..8。
@details 把 CLOSED socket 转为 LISTEN，必要时自动分配端口；backlog 限制 1..8。重复 listen 可更新上限，不允许把已连接对象改为监听。
```

<a id="fn-c8523a7152fb629f"></a>

## `xv::net::TcpSocket::passive`

```doxygen
@fn void xv::net::TcpSocket::passive(TcpSocket &listener, Endpoint local, Endpoint peer, const unsigned char *syn, unsigned length)
@brief TcpSocket：持注册锁按 SYN 填充四元组、peer MSS/窗口与初始序号，登记 listener，标记尚无应用 File，进入 SYN-RECEIVED 并发 SYN+ACK。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁按 SYN 填充四元组、peer MSS/窗口与初始序号，登记 listener，标记尚无应用 File，进入 SYN-RECEIVED 并发 SYN+ACK。
```

<a id="fn-eb2f93e4dada729f"></a>

## `xv::net::TcpSocket::accept`

```doxygen
@fn Socket *xv::net::TcpSocket::accept(bool nonblocking, int &error)
@brief TcpSocket：在注册锁下寻找已建立或已收到 FIN 的未接受子连接，移交应用拥有权并清除 listener 指针。
@details 在注册锁下寻找已建立或已收到 FIN 的未接受子连接，移交应用拥有权并清除 listener 指针。无连接时非阻塞返回 -EAGAIN，阻塞可被信号中断；返回对象须安装到 File，失败须 close。
```

<a id="fn-e3735def055c42b7"></a>

## `xv::net::TcpSocket::incoming`

```doxygen
@fn void xv::net::TcpSocket::incoming(Endpoint, Endpoint, const unsigned char *p, unsigned length)
@brief TcpSocket：持注册锁推进连接状态机，检查 ACK 范围与接收序号、挑战非精确 RST。
@pre 调用者持有 SocketRegistry 条件锁。
@details 持注册锁推进连接状态机，检查 ACK 范围与接收序号、挑战非精确 RST。处理 SYN/ACK、重复数据裁剪、按序字节接收、窗口更新、FIN 与半关闭；乱序段不入缓冲，返回累计 ACK 等待重传。协议回调只访问内核数据。
```

<a id="fn-8c178dd34591a5e9"></a>

## `xv::net::TcpSocket::read`

```doxygen
@fn int xv::net::TcpSocket::read(xv::io::Buffer destination, int count)
@brief TcpSocket：从 RX 环取至多 min(count,512) 字节，解锁后复制到用户缓冲并通告新窗口。
@details 从 RX 环取至多 min(count,512) 字节，解锁后复制到用户缓冲并通告新窗口。优先返回已接收数据；FIN/读关闭返回 0，RST 返回负错误。空队列支持 -EAGAIN 或可中断等待。
```

<a id="fn-231bbbb4f11fc234"></a>

## `xv::net::TcpSocket::send_to`

```doxygen
@fn int xv::net::TcpSocket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
@brief TcpSocket：TCP 已连接字节流不接受单次指定目的地址；peer 非空返回 -EISCONN，否则委托 write 并保留非阻塞标志。
@details TCP 已连接字节流不接受单次指定目的地址；peer 非空返回 -EISCONN，否则委托 write 并保留非阻塞标志。
```

<a id="fn-12f33b275e0e4dec"></a>

## `xv::net::TcpSocket::receive_from`

```doxygen
@fn int xv::net::TcpSocket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
@brief TcpSocket：委托 TCP read；成功时可输出已连接 peer，长度和 EOF 语义与字节流 read 一致。
@details 委托 TCP read；成功时可输出已连接 peer，长度和 EOF 语义与字节流 read 一致。
```

<a id="fn-31670288cdab90bc"></a>

## `xv::net::TcpSocket::poll`

```doxygen
@fn unsigned xv::net::TcpSocket::poll(xv::sync::PollTable *table)
@brief TcpSocket：注册本对象等待通道后读取状态。
@details 注册本对象等待通道后读取状态。LISTEN 有已建立子连接时可读；连接有数据/EOF 时可读，发送槽可用且窗口非零时可写；错误/关闭报告 POLLERR/POLLHUP，便于非阻塞 connect 完成检查。
```

<a id="fn-1eb72bf0aca4a8a8"></a>

## `xv::net::TcpSocket::shutdown`

```doxygen
@fn int xv::net::TcpSocket::shutdown(int how)
@brief TcpSocket：处理 SHUT_RD/WR/RDWR。
@details 处理 SHUT_RD/WR/RDWR。读关闭丢弃缓存并开放接收窗口；写关闭等待当前在途段确认后排队 FIN，不释放 File，另一方向仍可传输。
```

<a id="fn-c7164ce40d0f4ab4"></a>

## `xv::net::TcpSocket::close`

```doxygen
@fn void xv::net::TcpSocket::close()
@brief TcpSocket：应用最后 File 释放时撤销应用拥有权。
@details 应用最后 File 释放时撤销应用拥有权。监听对象中止未接受子连接；建立中对象中止；已建立对象等待在途数据确认再 FIN。后续由注册表持有，直到关闭/TIME_WAIT/超时完成才析构。
```

<a id="fn-9f612ac9dc8ef596"></a>

## `xv::net::TcpSocket::destroy`

```doxygen
@fn void xv::net::TcpSocket::destroy()
@brief TcpSocket：归还可选 RX 页，显式析构 TcpSocket 再归还对象页；支持 RX 分配失败后的部分构造回滚。
@details 归还可选 RX 页，显式析构 TcpSocket 再归还对象页；支持 RX 分配失败后的部分构造回滚。调用者已撤销注册，不得再使用 this。
```

<a id="fn-b862f8511a782329"></a>

## `xv::net::TcpSocket::tick`

```doxygen
@fn void xv::net::TcpSocket::tick(unsigned long now)
@brief TcpSocket：在途段到期按 RTO 重传并指数退避，最多六次重传后报告 ETIMEDOUT；关闭等待与 TIME_WAIT 到期清理，零窗口发送探测 ACK。
@pre 调用者持有 SocketRegistry 条件锁。
@details 在途段到期按 RTO 重传并指数退避，最多六次重传后报告 ETIMEDOUT；关闭等待与 TIME_WAIT 到期清理，零窗口发送探测 ACK。未被应用持有且已 CLOSED 时从注册表销毁。
```
<a id="fn-9e07fc39cb3c17a2"></a>

## `xv::net::TcpSocket::connect`

```doxygen
@fn int xv::net::TcpSocket::connect(Endpoint peer, bool nonblocking)
@brief 选择可路由源地址并自动绑定，初始化主动连接状态后发 SYN；保留非阻塞、可中断等待和连接错误语义。
```

<a id="fn-633245e696cf6ebf"></a>

## `xv::net::TcpSocket::write`

```doxygen
@fn int xv::net::TcpSocket::write(xv::io::Buffer source, int count)
@brief 先暂存用户数据再在注册锁内发送；单段长度受出口 MTU 减四十、对端 MSS 和窗口共同限制，保留部分写、信号及非阻塞语义。
```
