# `kernel/net/link/arp.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::ArpCache::clear`](arp.cpp.md#fn-f92a3d89216fa13d)
- [`xv::net::ArpCache::initialize`](arp.cpp.md#fn-9716d9e48bbba0d6)
- [`xv::net::ArpCache::neighbor`](arp.cpp.md#fn-ee8e9de0c540ad2b)
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

<a id="fn-d772ee0820984b06"></a>

## `xv::net::ArpCache::request`

```doxygen
@fn void xv::net::ArpCache::request(unsigned address)
@brief 短暂锁定配置以取得一致且启用的本接口源地址，再在锁外编码并广播 ARP；无源或分配失败不发送。
@details 实现通过 RequestFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-ef8526d27ee87536"></a>

## `xv::net::ArpCache::send`

```doxygen
@fn int xv::net::ArpCache::send(Packet &packet, unsigned next_hop)
@brief 先验证接口和源地址快照，邻居命中则直接发送，否则有界保留页面、长度、上下文与配置代次等待解析。
@details ARP 请求在邻居锁外发送，队列失败不接管报文；恢复待发报文时保留处理上下文。 实现通过 SendFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-704e5894f62445e0"></a>

## `xv::net::ArpCache::receive`

```doxygen
@fn void xv::net::ArpCache::receive(Packet &packet)
@brief 验证并学习面向本机的 ARP 请求/回复，按配置代次恢复待解析报文，再在锁外发送。
@details 队列恢复时携带原 Packet 上下文；过期配置的页面丢弃。有效请求继续生成本机 ARP 回复，不在持邻居锁时回调驱动。 实现通过 ReceiveFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-ab7954406875ed6d"></a>

## `xv::net::ArpCache::tick`

```doxygen
@fn void xv::net::ArpCache::tick(unsigned long now)
@brief 在邻居锁内先释放代次失效或接口停用的待解析页，再按毫秒截止时间重试或超时释放；请求编码在解锁后执行。
@details 实现通过 TickFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-ee8e9de0c540ad2b"></a>

## `xv::net::ArpCache::neighbor`

```doxygen
@fn bool xv::net::ArpCache::neighbor(unsigned index, NeighborInfo &result)
@brief 在 ARP 锁内跳过过期项，按有效邻居序号复制 IPv4/MAC 与剩余有效毫秒；没有该项返回 false，不返回缓存引用。
@param index 仅在当前有效邻居中的序号。
@param[out] result 地址、MAC 和相对剩余有效毫秒，不含内核引用。
@return 找到返回 true；没有该项返回 false。
@pre 不持配置锁。
```

<a id="fn-f92a3d89216fa13d"></a>

## `xv::net::ArpCache::clear`

```doxygen
@fn void xv::net::ArpCache::clear()
@brief 在邻居锁内清零缓存并由 Packet RAII 释放所有待解析页；调用者不得持配置锁，以保持单向锁顺序。
```
