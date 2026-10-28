# `kernel/net/core/packet.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Packet::Packet`](packet.cppm.md#fn-597161c987b8107f)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-5c3922f76c0ef31b)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-1a910797fa21702c)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-01632cbe68dab430)
- [`xv::net::Packet::operator=`](packet.cppm.md#fn-08a685fdd47de862)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-b88a374c79150150)
- [`xv::net::Packet::~Packet`](packet.cpp.md#fn-38f798f54b47cbb2)
- [`xv::net::Packet::allocate`](packet.cpp.md#fn-5633cc7891c8536f)
- [`xv::net::Packet::clone`](packet.cpp.md#fn-b249acc31eb1ae38)
- [`xv::net::Packet::data`](packet.cppm.md#fn-93666e5af03c5105)
- [`xv::net::Packet::data`](packet.cppm.md#fn-d33007dacc01e7f6)
- [`xv::net::Packet::context`](packet.cppm.md#fn-84e4c6593ec65664)
- [`xv::net::Packet::enter_tunnel`](packet.cppm.md#fn-adf125d19993490f)
- [`xv::net::Packet::size`](packet.cppm.md#fn-1d7d9da5732ac3be)
- [`xv::net::Packet::prepend`](packet.cppm.md#fn-46a5b790042dab38)
- [`xv::net::Packet::remove_prefix`](packet.cppm.md#fn-361caf43f836ba30)
- [`xv::net::Packet::resize`](packet.cppm.md#fn-e16cf3a638254443)
- [`xv::net::Packet::operator bool`](packet.cppm.md#fn-7c6cb6aacc141b80)
- [`xv::net::Packet::release`](packet.cppm.md#fn-b3b0ea47a1af76a5)

<a id="fn-597161c987b8107f"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet()
@brief 构造无页面拥有权、零长度和默认上下文的空报文。
```

<a id="fn-5c3922f76c0ef31b"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(void *page, unsigned length)
@brief 以默认处理上下文接管物理页面及有效长度，供设备新收包或分配页面使用。
@pre 页面具有唯一拥有者，长度不超过 Capacity；空指针表示无拥有权。
```

<a id="fn-1a910797fa21702c"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(void *page, unsigned length, Context context)
@brief 接管页面并恢复长度和处理上下文，供内核队列及重组完成页面使用。
@pre 页面具有唯一拥有者，长度不超过 Capacity；原持有方已 release，后续由本对象析构或再次移交。
```

<a id="fn-01632cbe68dab430"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(const Packet &)
@brief 禁止复制报文拥有者；显式 clone 才会分配独立页面。
```

<a id="fn-08a685fdd47de862"></a>

## `xv::net::Packet::operator=`

```doxygen
@fn Packet &xv::net::Packet::operator=(const Packet &)
@brief 禁止复制网络唯一资源。
@details 该成员在编译期删除，避免复制页面或流资源的唯一拥有权。
```

<a id="fn-b88a374c79150150"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(Packet &&other)
@brief 移交页面拥有权并复制长度与处理上下文；来源失去拥有权但保留元数据值，不再释放页面。
```

<a id="fn-93666e5af03c5105"></a>

## `xv::net::Packet::data`

```doxygen
@fn unsigned char *xv::net::Packet::data()
@brief 借用页面字节；可变报文返回可写视图，const 报文仅返回只读视图，不转移拥有权。
@note 空或已释放报文返回空指针，访问前须检查拥有权及大小边界。
```

<a id="fn-d33007dacc01e7f6"></a>

## `xv::net::Packet::data`

```doxygen
@fn const unsigned char *xv::net::Packet::data() const
@brief 借用页面字节；可变报文返回可写视图，const 报文仅返回只读视图，不转移拥有权。
@note 空或已释放报文返回空指针，访问前须检查拥有权及大小边界。
```

<a id="fn-84e4c6593ec65664"></a>

## `xv::net::Packet::context`

```doxygen
@fn Context xv::net::Packet::context() const
@brief 返回报文处理上下文的值副本，目前记录已解封的隧道深度。
@note 不改变拥有权，移动或 release 后保留的副本仍可用于队列接管。
```

<a id="fn-adf125d19993490f"></a>

## `xv::net::Packet::enter_tunnel`

```doxygen
@fn bool xv::net::Packet::enter_tunnel()
@brief 在隧道处理深度未达到 MaxTunnelDepth 时推进一次，达到或超限返回 false。
@details 深度属于报文上下文；克隆、移动及队列恢复必须保留，不能通过重组重置预算。
```

<a id="fn-1d7d9da5732ac3be"></a>

## `xv::net::Packet::size`

```doxygen
@fn unsigned xv::net::Packet::size() const
@brief Packet：取得本对象记录的有效帧字节数。
@details 取得本对象记录的有效帧字节数。
```

<a id="fn-46a5b790042dab38"></a>

## `xv::net::Packet::prepend`

```doxygen
@fn bool xv::net::Packet::prepend(unsigned count)
@brief 在同一四千零九十六字节页面内后移现有内容，清零新增首部；无页面或容量不足返回 false 且不修改。
@param count 要增加并清零的首部字节数。
@return 页面有效且容量足够时返回 true。
```

<a id="fn-361caf43f836ba30"></a>

## `xv::net::Packet::remove_prefix`

```doxygen
@fn bool xv::net::Packet::remove_prefix(unsigned count)
@brief 原地移除前缀并将余下数据搬回页面起点；非法长度返回 false，保持队列存储的页面布局一致。
@param count 要移除的前缀字节数，不得超过当前有效长度。
@return 成功搬移并缩短返回 true，失败不修改。
```

<a id="fn-e16cf3a638254443"></a>

## `xv::net::Packet::resize`

```doxygen
@fn bool xv::net::Packet::resize(unsigned length)
@brief 验证页面存在且新长度不超过 4096 后修改有效长度；不会清零新扩展的区域，失败保持原长度。
```

<a id="fn-7c6cb6aacc141b80"></a>

## `xv::net::Packet::operator bool`

```doxygen
@fn explicit xv::net::Packet::operator bool() const
@brief Packet：判断是否实际持有页面，不以有效长度判断空报文。
@details 判断是否实际持有页面，不以有效长度判断空报文。
```

<a id="fn-b3b0ea47a1af76a5"></a>

## `xv::net::Packet::release`

```doxygen
@fn void *xv::net::Packet::release()
@brief 仅移交物理页面并清空本对象拥有权，保留长度与上下文值。
@return 由调用方负责回收或接管的页面，原对象为空时返回空。
@note 释放后不能读写字节；队列需同时保存 size() 与 context()，恢复时使用带上下文的接管构造器。
```
