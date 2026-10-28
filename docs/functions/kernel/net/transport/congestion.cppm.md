# `kernel/net/transport/congestion.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::TcpCongestion::initialize`](congestion.cppm.md#fn-897de3663634a9cc)
- [`xv::net::TcpCongestion::window`](congestion.cppm.md#fn-5aafbf9377c4fe12)
- [`xv::net::TcpCongestion::timeout`](congestion.cppm.md#fn-b7bc1e44d5976a33)
- [`xv::net::TcpCongestion::duplicate`](congestion.cppm.md#fn-eec120850e614e58)
- [`xv::net::TcpCongestion::acknowledged`](congestion.cppm.md#fn-e71caffb7e38751d)
- [`xv::net::TcpCongestion::reset_duplicates`](congestion.cppm.md#fn-68bbeb650c221e9d)

<a id="fn-897de3663634a9cc"></a>

## `xv::net::TcpCongestion::initialize`

```doxygen
@fn void xv::net::TcpCongestion::initialize(unsigned mss)
@brief 以协商 MSS 建立初始 NewReno 状态。
@details 初始拥塞窗口为两个 MSS，慢启动门限为 65535，清空重复确认、增长信用和恢复边界。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-5aafbf9377c4fe12"></a>

## `xv::net::TcpCongestion::window`

```doxygen
@fn unsigned xv::net::TcpCongestion::window() const
@brief 读取当前拥塞窗口的字节额度。
@details 发送端将它与对端接收窗口共同限制在途字节；窗口上限为 65535。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-b7bc1e44d5976a33"></a>

## `xv::net::TcpCongestion::timeout`

```doxygen
@fn void xv::net::TcpCongestion::timeout(unsigned flight, unsigned next)
@brief 按超时丢包收缩拥塞窗口。
@details 门限为 max(FlightSize/2,2*MSS)，窗口降至一个 MSS，记录恢复屏障，防止旧重复 ACK 立即再次进入快速恢复。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-eec120850e614e58"></a>

## `xv::net::TcpCongestion::duplicate`

```doxygen
@fn bool xv::net::TcpCongestion::duplicate(unsigned acknowledgement, unsigned flight, unsigned next)
@brief 累计重复 ACK 并执行快速重传入口与窗口膨胀。
@details 第三个合格重复确认记录恢复边界并请求重传，后续重复确认逐步增加恢复额度；超时恢复屏障之前的旧确认不重新触发恢复。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-e71caffb7e38751d"></a>

## `xv::net::TcpCongestion::acknowledged`

```doxygen
@fn bool xv::net::TcpCongestion::acknowledged(unsigned bytes, unsigned acknowledgement)
@brief 按新确认字节执行慢启动、拥塞避免或 NewReno 恢复。
@details 部分恢复 ACK 收缩膨胀额度并请求修复下一个缺口；覆盖恢复边界后退出。普通慢启动每 ACK 至多增加一个 MSS，拥塞避免按字节信用累计增长。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-68bbeb650c221e9d"></a>

## `xv::net::TcpCongestion::reset_duplicates`

```doxygen
@fn void xv::net::TcpCongestion::reset_duplicates()
@brief 清除普通传输阶段的重复 ACK 计数。
@details 窗口更新等非重复确认事件调用；快速恢复期间保留恢复状态与计数。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```
