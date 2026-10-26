# `kernel/net/core/packet.cppm`

## 函数导航

- [`xv::net::Packet::Packet`](packet.cppm.md#fn-597161c987b8107f)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-b88a374c79150150)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-01632cbe68dab430)
- [`xv::net::Packet::Packet`](packet.cppm.md#fn-5c3922f76c0ef31b)
- [`xv::net::Packet::allocate`](packet.cppm.md#fn-5633cc7891c8536f)
- [`xv::net::Packet::clone`](packet.cpp.md#fn-b249acc31eb1ae38)
- [`xv::net::Packet::data`](packet.cppm.md#fn-d33007dacc01e7f6)
- [`xv::net::Packet::enter_tunnel`](packet.cppm.md#fn-adf125d19993490f)
- [`xv::net::Packet::operator bool`](packet.cppm.md#fn-7c6cb6aacc141b80)
- [`xv::net::Packet::operator=`](packet.cppm.md#fn-08a685fdd47de862)
- [`xv::net::Packet::prepend`](packet.cppm.md#fn-46a5b790042dab38)
- [`xv::net::Packet::release`](packet.cppm.md#fn-b3b0ea47a1af76a5)
- [`xv::net::Packet::remove_prefix`](packet.cppm.md#fn-361caf43f836ba30)
- [`xv::net::Packet::resize`](packet.cppm.md#fn-e16cf3a638254443)
- [`xv::net::Packet::size`](packet.cppm.md#fn-1d7d9da5732ac3be)
- [`xv::net::Packet::~Packet`](packet.cppm.md#fn-38f798f54b47cbb2)

<a id="fn-597161c987b8107f"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet()
@brief 构造 Packet 的状态与依赖。
@details 接管页面与有效长度；移动构造移交拥有权并清空源指针，默认构造为空对象。
```

<a id="fn-5c3922f76c0ef31b"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(void *page, unsigned length)
@brief 构造 Packet 的状态与依赖。
@details 接管页面与有效长度；移动构造移交拥有权并清空源指针，默认构造为空对象。
```

<a id="fn-01632cbe68dab430"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(const Packet &)
@brief 禁止复制网络唯一资源。
@details 该成员在编译期删除，避免复制页面或流资源的唯一拥有权。
```

<a id="fn-08a685fdd47de862"></a>

## `xv::net::Packet::operator=`

```doxygen
@fn Packet &xv::net::Packet::operator=(const Packet &)
@brief 禁止复制网络唯一资源。
@details 该成员在编译期删除，避免复制页面或流资源的唯一拥有权。
```

<a id="fn-d33007dacc01e7f6"></a>

## `xv::net::Packet::data`

```doxygen
@fn unsigned char *xv::net::Packet::data() const
@brief Packet：借用页首字节指针；不增加引用，空对象返回 nullptr。
@details 借用页首字节指针；不增加引用，空对象返回 nullptr。
```

<a id="fn-1d7d9da5732ac3be"></a>

## `xv::net::Packet::size`

```doxygen
@fn unsigned xv::net::Packet::size() const
@brief Packet：取得本对象记录的有效帧字节数。
@details 取得本对象记录的有效帧字节数。
```

<a id="fn-7c6cb6aacc141b80"></a>

## `xv::net::Packet::operator bool`

```doxygen
@fn xv::net::Packet::operator bool() const
@brief Packet：判断是否实际持有页面，不以有效长度判断空报文。
@details 判断是否实际持有页面，不以有效长度判断空报文。
```

<a id="fn-b3b0ea47a1af76a5"></a>

## `xv::net::Packet::release`

```doxygen
@fn void *xv::net::Packet::release()
@brief Packet：移交整页所有权并把自身指针清空；记录的长度保持不变，便于构造队列条目。
@details 移交整页所有权并把自身指针清空；记录的长度保持不变，便于构造队列条目。
```
<a id="fn-38f798f54b47cbb2"></a>

## `xv::net::Packet::~Packet`

```doxygen
@fn xv::net::Packet::~Packet()
@brief 释放仍由对象持有的页面；已显式移交页面的空对象不释放。
```

<a id="fn-5633cc7891c8536f"></a>

## `xv::net::Packet::allocate`

```doxygen
@fn Packet xv::net::Packet::allocate(unsigned length)
@brief 申请独占页面并设置有效长度，超过 4096 或分配失败得到空对象；页面由析构或显式 release 管理。
```

<a id="fn-b88a374c79150150"></a>

## `xv::net::Packet::Packet`

```doxygen
@fn xv::net::Packet::Packet(Packet &&other)
@brief 移动构造转移唯一页面并保留有效长度和解封深度，原对象页面置空；不复制页面内容。
```

<a id="fn-adf125d19993490f"></a>

## `xv::net::Packet::enter_tunnel`

```doxygen
@fn bool xv::net::Packet::enter_tunnel()
@brief 记录本 Packet 的解封层数，最多四层；达到上限返回 false，不继续递归投递。
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
