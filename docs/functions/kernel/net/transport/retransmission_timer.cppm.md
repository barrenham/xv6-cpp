# `kernel/net/transport/retransmission_timer.cppm`

## 函数导航

- [`xv::net::RetransmissionTimer::acknowledge`](retransmission_timer.cppm.md#fn-5f8f1333e86971f9)
- [`xv::net::RetransmissionTimer::expired`](retransmission_timer.cppm.md#fn-af87871c79fa8448)
- [`xv::net::RetransmissionTimer::restart`](retransmission_timer.cppm.md#fn-ef972b334557019e)
- [`xv::net::RetransmissionTimer::retransmit`](retransmission_timer.cppm.md#fn-6f9f5aaceee1b531)
- [`xv::net::RetransmissionTimer::start`](retransmission_timer.cppm.md#fn-c48947286fe2884c)

<a id="fn-c48947286fe2884c"></a>

## `xv::net::RetransmissionTimer::start`

```doxygen
@fn void xv::net::RetransmissionTimer::start(unsigned long now)
@brief RetransmissionTimer：记录新在途段的发送时刻并按当前 RTO 设置截止时间，允许本次未重传的 ACK 成为 RTT 样本。
@details 记录新在途段的发送时刻并按当前 RTO 设置截止时间，允许本次未重传的 ACK 成为 RTT 样本。初始 RTO 为 1000 毫秒。
```

<a id="fn-af87871c79fa8448"></a>

## `xv::net::RetransmissionTimer::expired`

```doxygen
@fn bool xv::net::RetransmissionTimer::expired(unsigned long now) const
@brief RetransmissionTimer：比较绝对毫秒时刻与截止时间；由 TcpSocket 的 flight 标记控制计时器是否活动。
@details 比较绝对毫秒时刻与截止时间；由 TcpSocket 的 flight 标记控制计时器是否活动。
```

<a id="fn-6f9f5aaceee1b531"></a>

## `xv::net::RetransmissionTimer::retransmit`

```doxygen
@fn void xv::net::RetransmissionTimer::retransmit(unsigned long now)
@brief RetransmissionTimer：发生超时重传后按指数加倍 RTO，上限 60 秒，更新截止时间并标记禁止从本次 ACK 采样 RTT（Karn）。
@details 发生超时重传后按指数加倍 RTO，上限 60 秒，更新截止时间并标记禁止从本次 ACK 采样 RTT（Karn）。
```

<a id="fn-ef972b334557019e"></a>

## `xv::net::RetransmissionTimer::restart`

```doxygen
@fn void xv::net::RetransmissionTimer::restart(unsigned long now)
@brief RetransmissionTimer：部分 ACK 后重启同一在途段截止时间，并保守地禁止该段采样，避免歧义 RTT。
@details 部分 ACK 后重启同一在途段截止时间，并保守地禁止该段采样，避免歧义 RTT。
```

<a id="fn-5f8f1333e86971f9"></a>

## `xv::net::RetransmissionTimer::acknowledge`

```doxygen
@fn void xv::net::RetransmissionTimer::acknowledge(unsigned long now)
@brief RetransmissionTimer：未发生重传时按 alpha=1/8、beta=1/4 更新平滑 RTT 与方差，先计算方差再更新 RTT。
@details 未发生重传时按 alpha=1/8、beta=1/4 更新平滑 RTT 与方差，先计算方差再更新 RTT。按 100ms 计时粒度计算 RTO 并限制在 1..60 秒；重传过的报文不参与采样。
```
