# `kernel/net/filter/hook.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::PacketFilter::accept`](hook.cppm.md#fn-17fddce2f525b508)
- [`xv::net::PacketFilter::~PacketFilter`](hook.cppm.md#fn-5f22964c25a92d2e)

<a id="fn-17fddce2f525b508"></a>

## `xv::net::PacketFilter::accept`

```doxygen
@fn virtual bool xv::net::PacketFilter::accept(FilterHook hook, const unsigned char *data, unsigned size, unsigned interface)
@brief 按顺序遍历链、保存返回位置并累计命中计数，返回放行判决。
@details 只借用有效报文字节和接口身份；true 放行、false 拒绝。不得保留字节指针或接管 Packet 所有权。
```

<a id="fn-5f22964c25a92d2e"></a>

## `xv::net::PacketFilter::~PacketFilter`

```doxygen
@fn xv::net::PacketFilter::~PacketFilter()
@brief 结束借用过滤接口的基类生命周期，不允许通过基类指针删除。
```
