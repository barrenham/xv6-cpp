# `kernel/net/link/arp.cpp`

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
```

<a id="fn-ef8526d27ee87536"></a>

## `xv::net::ArpCache::send`

```doxygen
@fn int xv::net::ArpCache::send(Packet &packet, unsigned next_hop)
@brief 先在配置锁下核对接口与 IP 源并保存代次，再查询邻居或接管纯 IP 页面入待解析队列。
@details 保存选源时的代次，避免清空后迟到入队的旧包冒充新配置。
```

<a id="fn-704e5894f62445e0"></a>

## `xv::net::ArpCache::receive`

```doxygen
@fn void xv::net::ArpCache::receive(Packet &packet)
@brief 验证接口状态、ARP 格式与本接口目的，更新邻居后逐包核对配置代次及状态，再移交待发页；旧代次报文直接释放，回复和发送在邻居锁外。
```

<a id="fn-ab7954406875ed6d"></a>

## `xv::net::ArpCache::tick`

```doxygen
@fn void xv::net::ArpCache::tick(unsigned long now)
@brief 在邻居锁内先释放代次失效或接口停用的待解析页，再按毫秒截止时间重试或超时释放；请求编码在解锁后执行。
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
