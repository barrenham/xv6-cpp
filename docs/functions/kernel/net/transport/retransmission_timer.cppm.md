# `kernel/net/transport/retransmission_timer.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::RetransmissionTimer::acknowledge`](retransmission_timer.cppm.md#fn-5f8f1333e86971f9)
- [`xv::net::RetransmissionTimer::expired`](retransmission_timer.cppm.md#fn-af87871c79fa8448)
- [`xv::net::RetransmissionTimer::reset`](retransmission_timer.cppm.md#fn-b720f042a7b82a6b)
- [`xv::net::RetransmissionTimer::retransmit`](retransmission_timer.cppm.md#fn-6f9f5aaceee1b531)
- [`xv::net::RetransmissionTimer::sent`](retransmission_timer.cppm.md#fn-10c88d723ee14fd7)
- [`xv::net::RetransmissionTimer::start`](retransmission_timer.cppm.md#fn-c48947286fe2884c)
- [`xv::net::RetransmissionTimer::uncertain`](retransmission_timer.cppm.md#fn-1753654941d4ceef)

<a id="fn-b720f042a7b82a6b"></a>

## `xv::net::RetransmissionTimer::reset`

```doxygen
@fn void xv::net::RetransmissionTimer::reset()
@brief 重置 RTT 估算、初始 RTO 与采样状态。
@details 初始间隔一秒，清空平滑 RTT、变化量和重传歧义，供新连接重新使用。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-c48947286fe2884c"></a>

## `xv::net::RetransmissionTimer::start`

```doxygen
@fn void xv::net::RetransmissionTimer::start(unsigned long now)
@brief 以当前 RTO 设置新的重传截止时间。
@details 只更新计时截止，不改变采样点或退避后的间隔；新在途数据和确认推进使用同一操作。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-af87871c79fa8448"></a>

## `xv::net::RetransmissionTimer::expired`

```doxygen
@fn bool xv::net::RetransmissionTimer::expired(unsigned long now) const
@brief RetransmissionTimer：比较绝对毫秒时刻与截止时间；由 TcpSocket 的 flight 标记控制计时器是否活动。
@details 比较绝对毫秒时刻与截止时间；由 TcpSocket 的 flight 标记控制计时器是否活动。
```

<a id="fn-10c88d723ee14fd7"></a>

## `xv::net::RetransmissionTimer::sent`

```doxygen
@fn void xv::net::RetransmissionTimer::sent(unsigned end, unsigned long now)
@brief 为尚无歧义的全新发送记录 RTT 采样边界。
@details 保存该发送的结束序号及时间，待累计 ACK 覆盖边界时采样；已有样本或尚处重传歧义期时不覆盖。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-6f9f5aaceee1b531"></a>

## `xv::net::RetransmissionTimer::retransmit`

```doxygen
@fn void xv::net::RetransmissionTimer::retransmit(unsigned long now)
@brief 执行 RTO 指数退避并重启计时。
@details 先取消相关 RTT 样本，随后将间隔翻倍至最多六十秒，避免重传段的确认污染估算。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-1753654941d4ceef"></a>

## `xv::net::RetransmissionTimer::uncertain`

```doxygen
@fn void xv::net::RetransmissionTimer::uncertain()
@brief 取消当前 RTT 样本并标记重传歧义。
@details 用于快速重传和 persist；间隔不在这里退避，待当前在途数据全部确认后解除歧义。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-5f8f1333e86971f9"></a>

## `xv::net::RetransmissionTimer::acknowledge`

```doxygen
@fn void xv::net::RetransmissionTimer::acknowledge(unsigned acknowledgement, unsigned long now, bool outstanding)
@brief 在累计 ACK 覆盖采样边界时更新 RTT 与 RTO。
@details 平滑 RTT 和变化量按既有权重计算，RTO 限制在一至六十秒。Karn 歧义期不采样；所有在途数据完成后允许下一次全新测量。 实现通过 AcknowledgeFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```
