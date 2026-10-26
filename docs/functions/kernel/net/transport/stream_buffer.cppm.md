# `kernel/net/transport/stream_buffer.cppm`

## 函数导航

- [`xv::net::TcpPageBuffer::at`](stream_buffer.cppm.md#fn-f323d0ffa243da1c)
- [`xv::net::TcpPageBuffer::initialize`](stream_buffer.cppm.md#fn-d215ab99b2d641dd)
- [`xv::net::TcpPageBuffer::release`](stream_buffer.cppm.md#fn-705d59505a71c048)
- [`xv::net::TcpPageBuffer::span`](stream_buffer.cppm.md#fn-9abd8c5423fea2e6)
- [`xv::net::TcpReceiveBuffer::acceptable`](stream_buffer.cppm.md#fn-e38777d6fe48320b)
- [`xv::net::TcpReceiveBuffer::discard`](stream_buffer.cppm.md#fn-92e78acdac2921fd)
- [`xv::net::TcpReceiveBuffer::finished`](stream_buffer.cppm.md#fn-3b103917d41f437c)
- [`xv::net::TcpReceiveBuffer::initialize`](stream_buffer.cppm.md#fn-05735a9b95cfeab8)
- [`xv::net::TcpReceiveBuffer::insert`](stream_buffer.cppm.md#fn-e8c9cca903bc4860)
- [`xv::net::TcpReceiveBuffer::next`](stream_buffer.cppm.md#fn-7fef44d82c0b1fa6)
- [`xv::net::TcpReceiveBuffer::read`](stream_buffer.cppm.md#fn-2029608d75575f91)
- [`xv::net::TcpReceiveBuffer::readable`](stream_buffer.cppm.md#fn-547e389594618ad3)
- [`xv::net::TcpReceiveBuffer::release`](stream_buffer.cppm.md#fn-631ea25d09a4d215)
- [`xv::net::TcpReceiveBuffer::reset`](stream_buffer.cppm.md#fn-1d0e4c57c90c0e29)
- [`xv::net::TcpReceiveBuffer::window`](stream_buffer.cppm.md#fn-591b215cc33a6eca)
- [`xv::net::TcpSendBuffer::append`](stream_buffer.cppm.md#fn-0741bcdb3f1478e7)
- [`xv::net::TcpSendBuffer::begin`](stream_buffer.cppm.md#fn-521159d90d2bd47c)
- [`xv::net::TcpSendBuffer::discard`](stream_buffer.cppm.md#fn-55882a0de1a600a0)
- [`xv::net::TcpSendBuffer::end`](stream_buffer.cppm.md#fn-bc0a680fad6c6726)
- [`xv::net::TcpSendBuffer::initialize`](stream_buffer.cppm.md#fn-1e04edfa2096790e)
- [`xv::net::TcpSendBuffer::release`](stream_buffer.cppm.md#fn-c0f0073316a9971c)
- [`xv::net::TcpSendBuffer::reset`](stream_buffer.cppm.md#fn-0440b9a794375707)
- [`xv::net::TcpSendBuffer::size`](stream_buffer.cppm.md#fn-fb79fc32f05074b0)
- [`xv::net::TcpSendBuffer::space`](stream_buffer.cppm.md#fn-5e903465bc307dbc)
- [`xv::net::TcpSendBuffer::span`](stream_buffer.cppm.md#fn-0c9a6a7d69fc973d)

<a id="fn-d215ab99b2d641dd"></a>

## `xv::net::TcpPageBuffer::initialize`

```doxygen
@fn bool xv::net::TcpPageBuffer::initialize()
@brief 逐页申请组成 16 KiB 环的四个物理页。
@details 已有页保留，遇到分配失败返回 false；调用方必须在失败路径调用 release，工厂据此统一回滚。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-705d59505a71c048"></a>

## `xv::net::TcpPageBuffer::release`

```doxygen
@fn void xv::net::TcpPageBuffer::release()
@brief 归还所有已申请页并清空指针。
@details 允许部分初始化后调用，也允许重复清理；释放后访问内容需要重新初始化。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-f323d0ffa243da1c"></a>

## `xv::net::TcpPageBuffer::at`

```doxygen
@fn unsigned char &xv::net::TcpPageBuffer::at(unsigned sequence)
@brief 按绝对 TCP 序号定位环中的单个字节。
@details 容量为二的幂，取模映射同时适用于页边界和 32 位序号回绕；调用方保证页面已初始化及序号仍在有效窗口。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-9abd8c5423fea2e6"></a>

## `xv::net::TcpPageBuffer::span`

```doxygen
@fn const unsigned char *xv::net::TcpPageBuffer::span(unsigned sequence, unsigned &length) const
@brief 取得当前物理页内的连续字节视图。
@details length 按页剩余空间裁剪，返回指针只借用页面；调用方先按有效排队范围、MSS 和窗口限制请求长度。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-1e04edfa2096790e"></a>

## `xv::net::TcpSendBuffer::initialize`

```doxygen
@fn bool xv::net::TcpSendBuffer::initialize()
@brief 申请发送环的四个物理页。
@details 委托分页缓冲分配；失败由调用方 release 清理已取得资源。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-c0f0073316a9971c"></a>

## `xv::net::TcpSendBuffer::release`

```doxygen
@fn void xv::net::TcpSendBuffer::release()
@brief 释放发送环的所有物理页。
@details 不取得数据的其他拥有权，页面只由所属 TCP 对象清理。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-0440b9a794375707"></a>

## `xv::net::TcpSendBuffer::reset`

```doxygen
@fn void xv::net::TcpSendBuffer::reset(unsigned sequence)
@brief 把发送环清空并设置流起始序号。
@details begin_ 与 end_ 同时置为给定序号，不需要清零旧页内容；旧字节不再属于有效排队范围。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-521159d90d2bd47c"></a>

## `xv::net::TcpSendBuffer::begin`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::begin() const
@brief 取得最早尚未释放字节的序号。
@details 累计确认推进该前沿，环内保留范围为 begin 至 end。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-bc0a680fad6c6726"></a>

## `xv::net::TcpSendBuffer::end`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::end() const
@brief 取得已排队字节的末尾序号。
@details 它包含尚未发送字节，区别于连接的 SND.NXT。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-fb79fc32f05074b0"></a>

## `xv::net::TcpSendBuffer::size`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::size() const
@brief 取得已发送未确认和未发送字节的总排队量。
@details 使用无符号序号差，范围始终不超过 16384。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-5e903465bc307dbc"></a>

## `xv::net::TcpSendBuffer::space`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::space() const
@brief 取得发送环剩余的应用排队容量。
@details 容量减去排队量，供 write 和 poll 判定本地可写。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-0741bcdb3f1478e7"></a>

## `xv::net::TcpSendBuffer::append`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::append(const unsigned char *data, unsigned length)
@brief 把字节追加到发送环末尾并返回接纳长度。
@details 最多使用现有空闲空间，按序号逐字节映射，跨页与回绕保持连续流顺序。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-55882a0de1a600a0"></a>

## `xv::net::TcpSendBuffer::discard`

```doxygen
@fn unsigned xv::net::TcpSendBuffer::discard(unsigned acknowledgement)
@brief 按累计确认释放发送环前缀。
@details 过旧确认不推进，超过末尾的控制序号只释放已有数据；返回实际释放的字节数，供拥塞控制计算。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-0c9a6a7d69fc973d"></a>

## `xv::net::TcpSendBuffer::span`

```doxygen
@fn const unsigned char *xv::net::TcpSendBuffer::span(unsigned sequence, unsigned &length) const
@brief 借用发送环中当前页内的连续数据。
@details 不复制或转移拥有权；协议层据此把发送与重传数据复制到独立 Packet。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-05735a9b95cfeab8"></a>

## `xv::net::TcpReceiveBuffer::initialize`

```doxygen
@fn bool xv::net::TcpReceiveBuffer::initialize()
@brief 申请接收环的四个物理页。
@details 重排位图内嵌在对象中，页面分配失败由调用方统一回滚。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-631ea25d09a4d215"></a>

## `xv::net::TcpReceiveBuffer::release`

```doxygen
@fn void xv::net::TcpReceiveBuffer::release()
@brief 释放接收环的所有物理页。
@details 兼容部分初始化失败，不额外分配或释放重排位图。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-1d0e4c57c90c0e29"></a>

## `xv::net::TcpReceiveBuffer::reset`

```doxygen
@fn void xv::net::TcpReceiveBuffer::reset(unsigned sequence)
@brief 清空连续队列、重排位图和 FIN 状态。
@details 将读取位置和累计接收前沿设为初始序号，旧页内容只有重新标记后才可能成为有效数据。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-7fef44d82c0b1fa6"></a>

## `xv::net::TcpReceiveBuffer::next`

```doxygen
@fn unsigned xv::net::TcpReceiveBuffer::next() const
@brief 取得下一个期待的接收序号。
@details 只随连续数据推进，连续 FIN 额外消耗一个序号。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-547e389594618ad3"></a>

## `xv::net::TcpReceiveBuffer::readable`

```doxygen
@fn unsigned xv::net::TcpReceiveBuffer::readable() const
@brief 取得可立即交给应用的连续数据字节数。
@details 不包含乱序字节或 FIN。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-591b215cc33a6eca"></a>

## `xv::net::TcpReceiveBuffer::window`

```doxygen
@fn unsigned xv::net::TcpReceiveBuffer::window() const
@brief 取得可通告接收窗口的序号宽度。
@details 容量减去未读取的连续字节；乱序占位属于该窗口内部，累计前沿推进不缩小已给出的右边缘。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-3b103917d41f437c"></a>

## `xv::net::TcpReceiveBuffer::finished`

```doxygen
@fn bool xv::net::TcpReceiveBuffer::finished() const
@brief 查询 FIN 是否已经到达连续接收前沿。
@details 登记但仍被数据缺口阻挡的 FIN 不产生 EOF。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-e38777d6fe48320b"></a>

## `xv::net::TcpReceiveBuffer::acceptable`

```doxygen
@fn bool xv::net::TcpReceiveBuffer::acceptable(unsigned sequence, unsigned length) const
@brief 判断报文序号区间是否与当前接收窗口相交。
@details 零长度段按点检查；窗口为零时只接受位于接收前沿的零长度段。length 应包含 FIN 占用的序号。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-e8c9cca903bc4860"></a>

## `xv::net::TcpReceiveBuffer::insert`

```doxygen
@fn unsigned xv::net::TcpReceiveBuffer::insert(unsigned sequence, const unsigned char *data, unsigned length, bool fin)
@brief 保存窗口内字节并推进所有已连续数据和 FIN。
@details 忽略旧前缀与窗口外后缀，重叠字节保留首次到达内容；缺口填满后依次清除存在位并增加可读量。返回前沿推进量，包含已消费 FIN 序号。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-2029608d75575f91"></a>

## `xv::net::TcpReceiveBuffer::read`

```doxygen
@fn unsigned xv::net::TcpReceiveBuffer::read(unsigned char *out, unsigned length)
@brief 按流顺序复制并消费连续接收字节。
@details 最多返回 readable 字节，读取会释放窗口容量，FIN 不作为字节返回。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```

<a id="fn-92e78acdac2921fd"></a>

## `xv::net::TcpReceiveBuffer::discard`

```doxygen
@fn void xv::net::TcpReceiveBuffer::discard()
@brief 丢弃当前连续数据并释放接收窗口容量。
@details 用于读关闭；乱序区和尚未连续的 FIN 仍由后续接收处理，连接可以继续确认对端数据。
@pre 对象由所属连接串行访问；在线连接使用注册锁保护。
```
