# `kernel/net/link/arp.cpp`

## 函数导航

- [`xv::net::ArpCache::initialize`](arp.cpp.md#fn-9716d9e48bbba0d6)
- [`xv::net::ArpCache::receive`](arp.cpp.md#fn-704e5894f62445e0)
- [`xv::net::ArpCache::request`](arp.cpp.md#fn-d772ee0820984b06)
- [`xv::net::ArpCache::send`](arp.cpp.md#fn-ef8526d27ee87536)
- [`xv::net::ArpCache::tick`](arp.cpp.md#fn-ab7954406875ed6d)

<a id="fn-9716d9e48bbba0d6"></a>

## `xv::net::ArpCache::initialize`

```doxygen
@fn void xv::net::ArpCache::initialize()
@brief ArpCache：启动时初始化邻居表与待解析队列锁。
@details 启动时初始化邻居表与待解析队列锁。
```

<a id="fn-ab7954406875ed6d"></a>

## `xv::net::ArpCache::tick`

```doxygen
@fn void xv::net::ArpCache::tick(unsigned long now)
@brief ArpCache：按毫秒到期重试 ARP，每个待解析报文最多三次请求；最终超时丢弃并释放持有页。
@details 按毫秒到期重试 ARP，每个待解析报文最多三次请求；最终超时丢弃并释放持有页。对链路调用前释放 ARP 锁。
```
<a id="fn-d772ee0820984b06"></a>

## `xv::net::ArpCache::request`

```doxygen
@fn void xv::net::ArpCache::request(unsigned address)
@brief 从本接口地址列表选择 ARP 源 IPv4，以本接口 MAC 广播请求下一跳；无源地址或页面时不发送。
```

<a id="fn-ef8526d27ee87536"></a>

## `xv::net::ArpCache::send`

```doxygen
@fn int xv::net::ArpCache::send(Packet &packet, unsigned next_hop)
@brief 只查本接口的邻居缓存；命中后添加以太首部发送，否则把纯 IP 页面移交有界待解析队列并发起请求，队满返回 -ENOBUFS。
```

<a id="fn-704e5894f62445e0"></a>

## `xv::net::ArpCache::receive`

```doxygen
@fn void xv::net::ArpCache::receive(Packet &packet)
@brief 验证 ARP 格式、发送者 MAC 及属于本接口的目标地址；更新本接口邻居、释放匹配待发包，并以被查询地址回复请求。
```
