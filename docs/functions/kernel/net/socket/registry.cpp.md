# `kernel/net/socket/registry.cpp`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::SocketRegistry::accept_child`](registry.cpp.md#fn-87dc7407fb12bac2)
- [`xv::net::SocketRegistry::add`](registry.cpp.md#fn-8ebcbaf798cf3b49)
- [`xv::net::SocketRegistry::child_count`](registry.cpp.md#fn-75a9c43c96312ec3)
- [`xv::net::SocketRegistry::close_children`](registry.cpp.md#fn-f87eb5a1600039d3)
- [`xv::net::SocketRegistry::create`](registry.cpp.md#fn-b6e4a131f2727ec9)
- [`xv::net::SocketRegistry::create_tcp`](registry.cpp.md#fn-2719e55329019906)
- [`xv::net::SocketRegistry::deliver`](registry.cpp.md#fn-9bba05169d63bfe0)
- [`xv::net::SocketRegistry::deliver_segment`](registry.cpp.md#fn-137b9bf56a27cf7b)
- [`xv::net::SocketRegistry::destroy`](registry.cpp.md#fn-5728efd1d2787600)
- [`xv::net::SocketRegistry::ephemeral`](registry.cpp.md#fn-5f18bd69e55bbc95)
- [`xv::net::SocketRegistry::has_ready_child`](registry.cpp.md#fn-ddfb832d8ebb61c6)
- [`xv::net::SocketRegistry::initial_sequence`](registry.cpp.md#fn-d1d3a7a6b8106a56)
- [`xv::net::SocketRegistry::initialize`](registry.cpp.md#fn-659351b0c1327217)
- [`xv::net::SocketRegistry::port_used`](registry.cpp.md#fn-843c6c84c36d20b9)
- [`xv::net::SocketRegistry::ready_child`](registry.cpp.md#fn-9e2f303bbf2cbbd5)
- [`xv::net::SocketRegistry::tick`](registry.cpp.md#fn-c06a8341d8d22f53)
- [`xv::net::create_socket`](registry.cpp.md#fn-75b80a83f7b1ba17)

- [`xv::net::SocketRegistry::deliver_echo`](registry.cpp.md#fn-203f77c89adc34f4)

<a id="fn-659351b0c1327217"></a>

## `xv::net::SocketRegistry::initialize`

```doxygen
@fn void xv::net::SocketRegistry::initialize()
@brief SocketRegistry：启动时初始化统一网络条件锁；socket 状态与 UDP/TCP 绑定遵循同一把锁。
@details 启动时初始化统一网络条件锁；socket 状态与 UDP/TCP 绑定遵循同一把锁。
```

<a id="fn-8ebcbaf798cf3b49"></a>

## `xv::net::SocketRegistry::add`

```doxygen
@fn bool xv::net::SocketRegistry::add(InetSocket &socket)
@brief SocketRegistry：持注册锁把借用对象登记到第一个空槽；没有空槽返回 false，不取得额外页面。
@pre 调用者持有统一注册锁。
@details 持注册锁把借用对象登记到第一个空槽；没有空槽返回 false，不取得额外页面。
```

<a id="fn-5728efd1d2787600"></a>

## `xv::net::SocketRegistry::destroy`

```doxygen
@fn void xv::net::SocketRegistry::destroy(InetSocket &socket)
@brief SocketRegistry：持注册锁先清空对应槽，再多态调用具体 destroy；调用后 socket 引用失效。
@pre 调用者持有统一注册锁。
@details 持注册锁先清空对应槽，再多态调用具体 destroy；调用后 socket 引用失效。禁止仍被应用文件持有时销毁。
```

<a id="fn-b6e4a131f2727ec9"></a>

## `xv::net::SocketRegistry::create`

```doxygen
@fn Socket *xv::net::SocketRegistry::create(int type, int protocol)
@brief SocketRegistry：验证 STREAM/DGRAM 与协议匹配，在物理页内 placement 构造具体 TcpSocket 或 UdpSocket；分配或最多 64 项注册失败时完整回滚。
@details 验证 STREAM/DGRAM 与协议匹配，在物理页内 placement 构造具体 TcpSocket 或 UdpSocket；分配或最多 64 项注册失败时完整回滚。返回对象唯一的应用拥有权，交给 File 管理。 实现通过 CreationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
```

<a id="fn-2719e55329019906"></a>

## `xv::net::SocketRegistry::create_tcp`

```doxygen
@fn TcpSocket *xv::net::SocketRegistry::create_tcp()
@brief 持注册锁构造主动或被动 TCP 对象，申请发送与接收环并登记空槽。
@details 页面或注册失败完整回滚；被动调用方随后设置监听关联和未接受标记。 实现通过 CreationFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre 调用者持有本注册表锁。
```

<a id="fn-843c6c84c36d20b9"></a>

## `xv::net::SocketRegistry::port_used`

```doxygen
@fn bool xv::net::SocketRegistry::port_used(unsigned protocol, unsigned short port, const InetSocket *except, unsigned local_address) const
@brief 在统一 socket 登记表中检查协议、本地地址与端口是否冲突。
@param protocol TCP 为 6，UDP 为 17，两个协议的端口空间独立。
@param except 排除正在检查或绑定的对象，可为空。
@param local_address 零表示通配地址；通配绑定与任意同号绑定冲突，具体地址可区分复用。
@return 端口不可用为 true，否则 false。
@pre 调用者持有注册锁。
@note UDP 68 仍为内核 DHCP 客户端保留；没有 PID 端口兼容表。
```

<a id="fn-5f18bd69e55bbc95"></a>

## `xv::net::SocketRegistry::ephemeral`

```doxygen
@fn unsigned short xv::net::SocketRegistry::ephemeral(unsigned protocol)
@brief SocketRegistry：持注册锁从 49152..65535 循环选择未占用端口，耗尽返回零，避免与已注册 socket 端口冲突。
@pre 调用者持有统一注册锁。
@details 持注册锁从 49152..65535 循环选择未占用端口，耗尽返回零，避免与已注册 socket 端口冲突。
```

<a id="fn-d1d3a7a6b8106a56"></a>

## `xv::net::SocketRegistry::initial_sequence`

```doxygen
@fn unsigned xv::net::SocketRegistry::initial_sequence()
@brief 使用本注册表的序号计数器及单调时钟生成下一连接的初始序号，不共享其他网络实例的计数状态。
@pre 调用者持有本注册表锁。
@note 保持原有的教学栈计数/时钟方案，不表示密码学随机序号。
```

<a id="fn-9bba05169d63bfe0"></a>

## `xv::net::SocketRegistry::deliver`

```doxygen
@fn bool xv::net::SocketRegistry::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
@brief SocketRegistry：持注册锁遍历 UDP socket，执行端口、地址及已连接 peer 筛选；命中队列满也视为命中并丢包，防止误发 ICMP 不可达。
@details 持注册锁遍历 UDP socket，执行端口、地址及已连接 peer 筛选；命中队列满也视为命中并丢包，防止误发 ICMP 不可达。
```

<a id="fn-137b9bf56a27cf7b"></a>

## `xv::net::SocketRegistry::deliver_segment`

```doxygen
@fn bool xv::net::SocketRegistry::deliver_segment(const TcpSegment &segment)
@brief 在统一注册锁内匹配现有 TCP 四元组，或把监听 SYN 交给有容量的被动子连接。
@details 原有连接优先；监听命中但 backlog 满或分配失败仍返回 true，保持静默丢弃。没有连接或有效监听 SYN 返回 false，由协议层处理未匹配响应。 实现通过 DeliveryFlow 的编译期图连接具名业务阶段；返回码、锁范围和资源所有权保持原类契约。
@pre segment 是协议层校验完成的完整 TCP 段，只借用其字节；调用方未持注册锁。
```

<a id="fn-75a9c43c96312ec3"></a>

## `xv::net::SocketRegistry::child_count`

```doxygen
@fn unsigned xv::net::SocketRegistry::child_count(const TcpSocket &listener) const
@brief 统计仍关联指定监听器的所有 TCP 子连接，包括尚在握手及已完成但未接受的连接。
@pre 调用者持有本注册表锁。
```

<a id="fn-9e2f303bbf2cbbd5"></a>

## `xv::net::SocketRegistry::ready_child`

```doxygen
@fn TcpSocket *xv::net::SocketRegistry::ready_child(const TcpSocket &listener) const
@brief 按注册槽位次序找到属于指定监听器的首个 ESTABLISHED 或 CLOSE_WAIT 子连接。
@pre 调用者持有本注册表锁，返回的借用指针只在该锁和对象生命周期约束下使用。
```

<a id="fn-ddfb832d8ebb61c6"></a>

## `xv::net::SocketRegistry::has_ready_child`

```doxygen
@fn bool xv::net::SocketRegistry::has_ready_child(const TcpSocket &listener) const
@brief 查询监听器是否有可接受子连接，供监听 poll 复用统一查找规则。
@pre 调用者已持本注册表锁，本方法不重复加锁。
```

<a id="fn-87dc7407fb12bac2"></a>

## `xv::net::SocketRegistry::accept_child`

```doxygen
@fn TcpSocket *xv::net::SocketRegistry::accept_child(TcpSocket &listener)
@brief 查找可接受子连接，成功时设置应用接管标记并解除监听关联，未找到返回空。
@pre 调用者已持本注册表锁，且监听器仍处于 LISTEN；本方法不重复加锁。
@note 解除关联后监听关闭不会回收该连接，应用后续负责 close。
```

<a id="fn-f87eb5a1600039d3"></a>

## `xv::net::SocketRegistry::close_children`

```doxygen
@fn void xv::net::SocketRegistry::close_children(TcpSocket &listener)
@brief 对仍关联监听器的未接受 TCP 子连接发送复位，并移除注册槽、销毁连接及其缓冲页。
@pre 调用者已持本注册表锁，且即将关闭监听器；本方法不重复加锁。
@note 已接受子连接已解除监听关联，不受此操作影响。
```

<a id="fn-c06a8341d8d22f53"></a>

## `xv::net::SocketRegistry::tick`

```doxygen
@fn void xv::net::SocketRegistry::tick(unsigned long now)
@brief SocketRegistry：持注册锁调用各对象的计时处理，允许对象在最终状态撤销自身槽位；定时器不进行用户复制或睡眠。
@details 持注册锁调用各对象的计时处理，允许对象在最终状态撤销自身槽位；定时器不进行用户复制或睡眠。
```

<a id="fn-75b80a83f7b1ba17"></a>

## `xv::net::create_socket`

```doxygen
@fn Socket *xv::net::create_socket(int type, int protocol)
@brief 通过唯一 SocketRegistry 工厂创建支持的 STREAM/DGRAM 实例。
@details 通过唯一 SocketRegistry 工厂创建支持的 STREAM/DGRAM 实例，调用者取得应用拥有权，须安装 File 或 close 回滚。
```

<a id="fn-203f77c89adc34f4"></a>

## `xv::net::SocketRegistry::deliver_echo`

```doxygen
@fn bool xv::net::SocketRegistry::deliver_echo(Packet &packet, const IpDatagram &d)
@brief 从 Echo 标识符选择 protocol 1 socket，在注册表锁内交付接收队列。
@details 同时匹配本地地址及已连接对端地址。队列满时按原 DatagramSocket 有界队列规则丢弃并释放；未匹配的报文保持调用者所有权。
```
