# `kernel/net/stack.cpp`

## 函数导航

- [`xv::net::bind`](stack.cpp.md#fn-46a4f2ccb5a7fd32)
- [`xv::net::inherit`](stack.cpp.md#fn-735598ae0bc0796c)
- [`xv::net::initialize`](stack.cpp.md#fn-1de580fddaa77f1f)
- [`xv::net::receive`](stack.cpp.md#fn-f08e19a5999a9d51)
- [`xv::net::release_owner`](stack.cpp.md#fn-bf547a36e1223a9c)
- [`xv::net::send`](stack.cpp.md#fn-aa8207c3fb16e1a3)
- [`xv::net::unbind`](stack.cpp.md#fn-57f8814a97b1bb0c)
- [`xv::net::bytes`](stack.cpp.md#fn-df47e449c0e80659)
- [`xv::net::detach`](stack.cpp.md#fn-eb042b1c44d295d5)
- [`xv::net::equal`](stack.cpp.md#fn-7d5ad41e910f052a)
- [`xv::net::ethernet`](stack.cpp.md#fn-f7cb505105a495d5)
- [`xv::net::find`](stack.cpp.md#fn-f9950e87e799e97a)
- [`xv::net::read16`](stack.cpp.md#fn-abcfa58f2e216022)
- [`xv::net::read32`](stack.cpp.md#fn-be179f803c63b9cd)
- [`xv::net::sum`](stack.cpp.md#fn-3491284f3748179f)
- [`xv::net::write16`](stack.cpp.md#fn-a80c9df6b035c813)
- [`xv::net::write32`](stack.cpp.md#fn-92adcdfbcd8107ac)
- [`xv::net::Stack::arp`](stack.cpp.md#fn-94c5450242187bb8)
- [`xv::net::Stack::receive_frame`](stack.cpp.md#fn-b854a007987b9e1f)
- [`xv::net::Stack::udp`](stack.cpp.md#fn-ae7b2fc47b50bb2d)


独立 Doxygen 开发文档；运行代码位于 `kernel/`。


<a id="fn-1de580fddaa77f1f"></a>

## `xv::net::initialize`

```doxygen
@fn void xv::net::initialize()
@brief 初始化 UDP 注册锁并将以太网接收回调交给板级驱动。
@pre 在创建首个用户进程前调用一次；分配器和等待机制可用。
@details 使用固定 QEMU 用户网络地址 10.0.2.15、MAC 52:54:00:12:34:56。
```

<a id="fn-46a4f2ccb5a7fd32"></a>

## `xv::net::bind`

```doxygen
@fn int xv::net::bind(unsigned short number)
@brief 为当前进程独占绑定一个本机 UDP 端口。
@param number 主机字节序的非零 UDP 端口。
@return 成功为 0；重复端口、无设备或资源耗尽为 -1。
@details 全局最多 32 个端口、256 个进程持有引用。持锁选择空槽；每端口最多保留 16 个数据报。fork 通过 inherit 共享绑定及接收队列。
```

<a id="fn-57f8814a97b1bb0c"></a>

## `xv::net::unbind`

```doxygen
@fn int xv::net::unbind(unsigned short number)
@brief 释放当前进程对 UDP 端口的一个持有引用。
@param number 非零端口。
@return 成功 0；当前进程不持有该端口时 -1。
@details 其他 fork 继承者仍可接收；最后一个引用释放时清空队列并归还页面，端口重新可绑定。
```

<a id="fn-aa8207c3fb16e1a3"></a>

## `xv::net::send`

```doxygen
@fn int xv::net::send(unsigned short source, unsigned destination, unsigned short port, unsigned long data, unsigned length)
@brief 从用户缓冲区组装并发送一个不分片的 IPv4 UDP 数据报。
@param source 源 UDP 端口，不要求预先绑定。
@param destination 主机字节序 IPv4 地址。
@param port 目标非零 UDP 端口。
@param data 当前进程用户地址。
@param length 0 至 1472 字节，受 Ethernet MTU 1500 限制。
@return 提交成功为 0；非法参数、离线、分配或复制失败为 -1；发送环满为 -EAGAIN。
@details 目的 Ethernet MAC 固定为 QEMU slirp 网关。写入 IPv4 与 UDP 伪首部校验和；包括奇数字节尾部。失败由本函数释放页，成功由驱动在 DD 后释放。
```

<a id="fn-f08e19a5999a9d51"></a>

## `xv::net::receive`

```doxygen
@fn int xv::net::receive(unsigned short number, unsigned long source, unsigned long source_port, unsigned long data, unsigned capacity)
@brief 从当前进程持有的端口接收一个 UDP 数据报，必要时可中断睡眠。
@param number 绑定端口。
@param source 输出主机字节序源 IPv4 的用户地址。
@param source_port 输出 uint16 源端口的用户地址。
@param data 接收缓冲区用户地址。
@param capacity 缓冲区字节容量。
@return 实际复制的长度；空数据报可返回 0；端口无引用、信号中断或复制失败为 -1。系统调用层把捕获信号中断转换为 -EINTR，并支持 SA_RESTART。
@details 持队列锁检查条件并 prepare_interruptible_sleep，释放锁后 sleep，醒来重新检查。取走消息后释放锁再 copy_out；超过容量的尾部丢弃，任何复制失败也消费并释放该报。
```

<a id="fn-735598ae0bc0796c"></a>

## `xv::net::inherit`

```doxygen
@fn bool xv::net::inherit(int parent, int child)
@brief 在发布子进程前原子继承父进程的所有 UDP 绑定引用。
@param parent 已存在父 PID。
@param child 尚未可运行的新 PID。
@return true 表示全部继承；句柄容量不足返回 false 且完全不修改引用。
@details 与 bind/unbind 共用全局锁，先统计空槽再建立引用；队列共享，多个继承者竞争消费数据报。fork 失败时上层销毁未发布进程。
```

<a id="fn-bf547a36e1223a9c"></a>

## `xv::net::release_owner`

```doxygen
@fn void xv::net::release_owner(int owner)
@brief 释放退出进程拥有的全部 UDP 端口引用。
@param owner 退出进程 PID。
@details 在进程转入 Zombie 前调用，正常退出与信号终止共用此路径；只在最后一个引用消失时归还排队页。
```

<a id="fn-abcfa58f2e216022"></a>

## `xv::net::read16`

```doxygen
@fn unsigned xv::net::read16(const unsigned char *p)
@brief 按网络字节序从字节数组读取 16 位无符号值。
@param p 至少两个可读字节；不要求对齐。
@return 主机字节序数值。
```

<a id="fn-be179f803c63b9cd"></a>

## `xv::net::read32`

```doxygen
@fn unsigned xv::net::read32(const unsigned char *p)
@brief 按网络字节序组合一个 32 位 IPv4 数值。
@param p 至少四个可读字节，不要求对齐。
@return 主机字节序数值。
```

<a id="fn-a80c9df6b035c813"></a>

## `xv::net::write16`

```doxygen
@fn void xv::net::write16(unsigned char *p, unsigned n)
@brief 把数值低 16 位按网络字节序写入字节数组。
@param p 至少两个可写字节，不要求对齐。
@param n 要编码的数值。
```

<a id="fn-92adcdfbcd8107ac"></a>

## `xv::net::write32`

```doxygen
@fn void xv::net::write32(unsigned char *p, unsigned n)
@brief 把 32 位数值按网络字节序写入字节数组。
@param p 至少四个可写字节。
@param n 主机字节序数值。
```

<a id="fn-df47e449c0e80659"></a>

## `xv::net::bytes`

```doxygen
@fn void xv::net::bytes(void *to, const void *from, unsigned n)
@brief 逐字节复制协议字段，避免未对齐结构体访问。
@param to 目标缓冲区。
@param from 源缓冲区。
@param n 可访问字节数；本函数不提供重叠复制语义。
```

<a id="fn-7d5ad41e910f052a"></a>

## `xv::net::equal`

```doxygen
@fn bool xv::net::equal(const unsigned char *a, const unsigned char *b, unsigned n)
@brief 逐字节比较 MAC 等固定长度协议字段。
@param a 第一段可读字节。
@param b 第二段可读字节。
@param n 比较长度。
@return 全部相等为 true。
```

<a id="fn-3491284f3748179f"></a>

## `xv::net::sum`

```doxygen
@fn unsigned xv::net::sum(const unsigned char *data, unsigned length, unsigned value = 0)
@brief 计算 Internet 校验和的折叠反码和。
@param data 网络字节序字节序列。
@param length 可读长度。
@param value 可选初始和，用于 UDP 伪首部。
@return 折叠至 16 位的和，尚未按位取反；奇数尾字节位于高 8 位。合法含校验字段的报文和为 0xffff。
```

<a id="fn-f7cb505105a495d5"></a>

## `xv::net::ethernet`

```doxygen
@fn void xv::net::ethernet(unsigned char *p, const unsigned char *destination, unsigned type)
@brief 填写目的 MAC、本机源 MAC 和 Ethernet 类型字段。
@param p 至少 14 个可写字节。
@param destination 六字节目的地址。
@param type Ethernet II 协议类型。
@details 只写首部，不负责填充、FCS 或页面所有权转移。
```

<a id="fn-f9950e87e799e97a"></a>

## `xv::net::find`

```doxygen
@fn Handle *xv::net::find(unsigned short number, int owner)
@brief 查找指定进程持有的 UDP 端口句柄。
@param number 本机端口。
@param owner 进程 PID。
@pre 持有全局端口锁，PID 为正。
@return 借用的稳定句柄槽；未找到为 nullptr，不增加引用。
```

<a id="fn-eb042b1c44d295d5"></a>

## `xv::net::detach`

```doxygen
@fn void xv::net::detach(Handle &h)
@brief 撤销一个端口持有引用，必要时销毁排队数据。
@param h 有效持有者句柄。
@pre 持有全局端口锁。
@details 清空句柄并减少引用；仍有继承者时保留队列。最后一个引用消失后归还全部消息页、清空端口编号并通知等待者。
```

<a id="fn-b854a007987b9e1f"></a>

## `xv::net::Stack::receive_frame`

```doxygen
@fn void xv::net::Stack::receive_frame(void *page, unsigned length)
@brief 按目的 MAC 与 EtherType 分派接收帧，并收束所有页面释放路径。
@param page 驱动移交的完整独占页。
@param length 有效帧长度。
@details 接受本机或广播目的 MAC；ARP 生成独立回复页后释放输入页；合法 UDP 只有成功入队才保留原页。其他帧与错误报文立即归还。
```

<a id="fn-94c5450242187bb8"></a>

## `xv::net::Stack::arp`

```doxygen
@fn void xv::net::Stack::arp(const unsigned char *p, unsigned length)
@brief 对发往本机 IPv4 的合法 Ethernet/IPv4 ARP 请求生成应答。
@param p 输入帧，调用期间借用，不取得所有权。
@param length 输入有效长度。
@details 检查硬件/协议类型、地址长度、请求操作、目标 IP 及源 MAC 一致性；每次合法请求都应答。回复分配失败或 TX 满时丢弃并释放回复页。不建立通用 ARP 缓存。
```

<a id="fn-ae7b2fc47b50bb2d"></a>

## `xv::net::Stack::udp`

```doxygen
@fn bool xv::net::Stack::udp(void *page, unsigned length)
@brief 验证 IPv4/UDP 边界与校验和，并把数据报加入目标端口的有限队列。
@param page 输入帧独占页。
@param length 完整帧长度。
@return 成功入队为 true，此时队列取得 page；其他情况 false，调用者仍需释放。
@details 验证 IPv4 版本/IHL/总长/TTL/目标地址/校验和，拒绝保留标志与任何分片，允许 DF。检查 UDP 长度一致性及可选非零校验和；无绑定或队列已有 16 报时丢弃。队列持锁入队并唤醒接收者；不进行用户态复制。
```
