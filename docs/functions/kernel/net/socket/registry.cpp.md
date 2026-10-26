# `kernel/net/socket/registry.cpp`

## 函数导航

- [`xv::net::create_socket`](registry.cpp.md#fn-75b80a83f7b1ba17)
- [`xv::net::SocketRegistry::add`](registry.cpp.md#fn-8ebcbaf798cf3b49)
- [`xv::net::SocketRegistry::create`](registry.cpp.md#fn-b6e4a131f2727ec9)
- [`xv::net::SocketRegistry::create_child`](registry.cpp.md#fn-312d78452a442d45)
- [`xv::net::SocketRegistry::deliver`](registry.cpp.md#fn-9bba05169d63bfe0)
- [`xv::net::SocketRegistry::destroy`](registry.cpp.md#fn-5728efd1d2787600)
- [`xv::net::SocketRegistry::ephemeral`](registry.cpp.md#fn-5f18bd69e55bbc95)
- [`xv::net::SocketRegistry::initialize`](registry.cpp.md#fn-659351b0c1327217)
- [`xv::net::SocketRegistry::port_used`](registry.cpp.md#fn-843c6c84c36d20b9)
- [`xv::net::SocketRegistry::tick`](registry.cpp.md#fn-c06a8341d8d22f53)

<a id="fn-659351b0c1327217"></a>

## `xv::net::SocketRegistry::initialize`

```doxygen
@fn void xv::net::SocketRegistry::initialize()
@brief SocketRegistry：启动时初始化统一网络条件锁；socket 状态、UDP 绑定与旧 API 端口表遵循同一把锁。
@details 启动时初始化统一网络条件锁；socket 状态、UDP 绑定与旧 API 端口表遵循同一把锁。
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
@details 验证 STREAM/DGRAM 与协议匹配，在物理页内 placement 构造具体 TcpSocket 或 UdpSocket；分配或最多 64 项注册失败时完整回滚。返回对象唯一的应用拥有权，交给 File 管理。
```

<a id="fn-312d78452a442d45"></a>

## `xv::net::SocketRegistry::create_child`

```doxygen
@fn TcpSocket *xv::net::SocketRegistry::create_child()
@brief SocketRegistry：持注册锁构造一个 TcpSocket 并申请其独立 4096 字节 RX 页。
@pre 调用者持有统一注册锁。
@details 持注册锁构造一个 TcpSocket 并申请其独立 4096 字节 RX 页。任一阶段失败都析构并归还已经分配的页；被动连接随后由 listener 标记为未接受。
```

<a id="fn-5f18bd69e55bbc95"></a>

## `xv::net::SocketRegistry::ephemeral`

```doxygen
@fn unsigned short xv::net::SocketRegistry::ephemeral(unsigned protocol)
@brief SocketRegistry：持注册锁从 49152..65535 循环选择未占用端口，耗尽返回零，避免与已注册连接/旧 UDP 端口冲突。
@pre 调用者持有统一注册锁。
@details 持注册锁从 49152..65535 循环选择未占用端口，耗尽返回零，避免与已注册连接/旧 UDP 端口冲突。
```

<a id="fn-9bba05169d63bfe0"></a>

## `xv::net::SocketRegistry::deliver`

```doxygen
@fn bool xv::net::SocketRegistry::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
@brief SocketRegistry：持注册锁遍历 UDP socket，执行端口、地址及已连接 peer 筛选；命中队列满也视为命中并丢包，防止误发 ICMP 不可达。
@details 持注册锁遍历 UDP socket，执行端口、地址及已连接 peer 筛选；命中队列满也视为命中并丢包，防止误发 ICMP 不可达。
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
<a id="fn-843c6c84c36d20b9"></a>

## `xv::net::SocketRegistry::port_used`

```doxygen
@fn bool xv::net::SocketRegistry::port_used(unsigned protocol, unsigned short port, const InetSocket *except, unsigned local_address) const
@brief 在注册锁保护下按协议、端口和本地地址判断占用；不同具体地址可复用端口，ANY 或旧 PID 绑定与所有具体地址冲突。
```
