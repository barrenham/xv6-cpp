# `kernel/net/socket/tcp_sender.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpSocket::acknowledge`](tcp_sender.cpp.md#fn-908ef432e52e4bd8)
- [`xv::net::TcpSocket::emit`](tcp_sender.cpp.md#fn-1e8c0fad1b1d52f4)
- [`xv::net::TcpSocket::finish`](tcp_sender.cpp.md#fn-cdfa0effd85a294d)
- [`xv::net::TcpSocket::mss`](tcp_sender.cpp.md#fn-21edc71336b6198c)
- [`xv::net::TcpSocket::persist`](tcp_sender.cpp.md#fn-ddde9d68f5d02226)
- [`xv::net::TcpSocket::pump`](tcp_sender.cpp.md#fn-0a573b0a54d401fc)
- [`xv::net::TcpSocket::retransmit`](tcp_sender.cpp.md#fn-a3fb96da6efa099c)
- [`xv::net::TcpSocket::segment`](tcp_sender.cpp.md#fn-3533387c7fba430c)
- [`xv::net::TcpSocket::synchronize`](tcp_sender.cpp.md#fn-8f8d8c5361cbc218)
- [`xv::net::TcpSocket::update_window`](tcp_sender.cpp.md#fn-12a68173f8514fe3)

<a id="fn-21edc71336b6198c"></a>

## `xv::net::TcpSocket::mss`

```doxygen
@fn unsigned xv::net::TcpSocket::mss() const
@brief 根据出口 MTU、对端 MSS 和本地上限取得当前发送段上限。
@details MTU 小于 68 或无路由时返回零；其他情况取 MTU 减四十、对端 MSS 与 1460 的最小值。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-3533387c7fba430c"></a>

## `xv::net::TcpSocket::segment`

```doxygen
@fn int xv::net::TcpSocket::segment(unsigned sequence, unsigned flags, const void *data, unsigned length)
@brief 通过本注册表注入的 TCP 对象发送指定序号的段，成功发送 ACK 后记录通告窗口右边缘。
@details 使用本地/远端端点、当前接收累计序号及窗口，不访问全局 TCP 实例。
@pre 调用者持本注册表锁，载荷为可同步借用的内核字节。
```

<a id="fn-1e8c0fad1b1d52f4"></a>

## `xv::net::TcpSocket::emit`

```doxygen
@fn void xv::net::TcpSocket::emit(unsigned flags)
@brief 从 SND.NXT 发出无载荷的控制或累计 ACK 段。
@details 委托 segment 统一生成确认号、接收窗口及通告记录，不消耗发送序号。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-8f8d8c5361cbc218"></a>

## `xv::net::TcpSocket::synchronize`

```doxygen
@fn void xv::net::TcpSocket::synchronize(unsigned sequence)
@brief 初始化一次主动或被动握手并发送 SYN 或 SYN-ACK。
@details 建立初始发送序号、空发送队列、重传计时与 RTT 采样点，清空旧关闭状态。SYN 消耗一个序号；发送失败仍保留重试责任。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-0a573b0a54d401fc"></a>

## `xv::net::TcpSocket::pump`

```doxygen
@fn void xv::net::TcpSocket::pump()
@brief 按对端窗口和拥塞窗口推进多段发送。
@details 额度为 min(rwnd,cwnd) 减去在途字节，按 MSS 和物理页连续空间切段。成功交给 IP 层才推进 SND.NXT，失败保留未发送队列；最后尝试顺序发送 FIN。 实现通过 PumpFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-cdfa0effd85a294d"></a>

## `xv::net::TcpSocket::finish`

```doxygen
@fn void xv::net::TcpSocket::finish()
@brief 把 FIN 排在全部待发送字节之后。
@details 必须已请求写关闭且数据已全部发出，同时还有发送额度；无需等待数据全部确认。FIN 消耗一个序号，分别进入 FIN_WAIT_1 或 LAST_ACK。 实现通过 FinishFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-a3fb96da6efa099c"></a>

## `xv::net::TcpSocket::retransmit`

```doxygen
@fn void xv::net::TcpSocket::retransmit()
@brief 重发最早未确认的握手、数据或 FIN。
@details 按当前 MSS 和页边界重发数据，不改变 SND.NXT；同时使相关 RTT 样本失效，避免把重传确认误作原始 RTT。 实现通过 RetransmitFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-12a68173f8514fe3"></a>

## `xv::net::TcpSocket::update_window`

```doxygen
@fn bool xv::net::TcpSocket::update_window(unsigned sequence, unsigned acknowledgement, unsigned value)
@brief 校验 ACK 范围和窗口更新顺序后接受对端窗口。
@details 通过窗口序号与确认号拒绝过旧更新；窗口从零恢复时重置 persist 退避和普通重传截止时间。返回窗口数值是否变化，用于区分重复 ACK 与窗口更新。 实现通过 WindowFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-908ef432e52e4bd8"></a>

## `xv::net::TcpSocket::acknowledge`

```doxygen
@fn void xv::net::TcpSocket::acknowledge(unsigned acknowledgement, bool duplicate)
@brief 处理累计 ACK、发送队列释放和快速恢复。
@details 超前或过旧确认不释放数据；新确认推进 SND.UNA、回收对应字节、更新 RTT/RTO 与 NewReno。重复 ACK 可触发快速重传，FIN 确认可推进关闭状态，并唤醒阻塞写入。 实现通过 AcknowledgeFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```

<a id="fn-ddde9d68f5d02226"></a>

## `xv::net::TcpSocket::persist`

```doxygen
@fn void xv::net::TcpSocket::persist(unsigned long now)
@brief 在对端零窗口期间发送有退避的数据或 ACK 探测。
@details 最早未确认或尚未发送的数据用一个字节探测；首次发送的新探测字节占用序号。间隔从一秒退避至六十秒，接收窗口恢复后重置，不按拥塞超时处理。 实现通过 PersistFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有 SocketRegistry 注册锁。
```
