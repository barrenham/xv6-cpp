# `kernel/net/stack.cpp`

## 函数导航

- [`xv::net::bind`](stack.cpp.md#fn-46a4f2ccb5a7fd32)
- [`xv::net::inherit`](stack.cpp.md#fn-735598ae0bc0796c)
- [`xv::net::initialize_legacy_udp`](stack.cpp.md#fn-2c6316eefc279413)
- [`xv::net::legacy_port_used`](stack.cpp.md#fn-70bad3dd1519b0d6)
- [`xv::net::receive`](stack.cpp.md#fn-f08e19a5999a9d51)
- [`xv::net::release_owner`](stack.cpp.md#fn-bf547a36e1223a9c)
- [`xv::net::send`](stack.cpp.md#fn-aa8207c3fb16e1a3)
- [`xv::net::unbind`](stack.cpp.md#fn-57f8814a97b1bb0c)
- [`xv::net::detach`](stack.cpp.md#fn-eb042b1c44d295d5)
- [`xv::net::find`](stack.cpp.md#fn-f9950e87e799e97a)
- [`xv::net::legacy_lock`](stack.cpp.md#fn-f9bbe368471e7b58)
- [`xv::net::LegacyUdp::deliver`](stack.cpp.md#fn-ba813c95f31783b0)

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

<a id="fn-46a4f2ccb5a7fd32"></a>

## `xv::net::bind`

```doxygen
@fn int xv::net::bind(unsigned short number)
@brief 为当前进程独占绑定一个本机 UDP 端口。
@details 旧 UDP API 与 SocketRegistry 共用条件锁和 UDP 端口命名空间；任一 socket 已占用同号 UDP 端口时绑定失败。仍保留原有 PID 引用表与 fork 继承。
```

<a id="fn-57f8814a97b1bb0c"></a>

## `xv::net::unbind`

```doxygen
@fn int xv::net::unbind(unsigned short number)
@brief 释放当前进程对 UDP 端口的一个持有引用。
@details 使用 SocketRegistry 的统一锁撤销旧 PID 端口引用；与新文件描述符 socket 生命周期独立，最后引用归还数据报页。
```

<a id="fn-f08e19a5999a9d51"></a>

## `xv::net::receive`

```doxygen
@fn int xv::net::receive(unsigned short number, unsigned long source, unsigned long source_port, unsigned long data, unsigned capacity)
@brief 从当前进程持有的端口接收一个 UDP 数据报，必要时可中断睡眠。
@details 旧 PID 接收队列使用统一网络注册锁，保留条件检查和可中断等待协议；实际 IPv4/UDP 校验已迁移到独立类。
```

<a id="fn-735598ae0bc0796c"></a>

## `xv::net::inherit`

```doxygen
@fn bool xv::net::inherit(int parent, int child)
@brief 在发布子进程前原子继承父进程的所有 UDP 绑定引用。
@details 共享注册锁下原子复制旧 UDP 绑定引用。新 socket 不经过 PID 引用表，直接通过 DescriptorTable/File 的 fork 继承共享。
```

<a id="fn-bf547a36e1223a9c"></a>

## `xv::net::release_owner`

```doxygen
@fn void xv::net::release_owner(int owner)
@brief 释放退出进程拥有的全部 UDP 端口引用。
@details 统一锁下清理旧端口 API 引用；文件描述符 socket 由进程 close_all 经 File 最后引用关闭，不在这里重复清理。
```

<a id="fn-2c6316eefc279413"></a>

## `xv::net::initialize_legacy_udp`

```doxygen
@fn void xv::net::initialize_legacy_udp()
@brief 启动时将旧 PID 端口适配器挂接到 UdpProtocol；不单独初始化网络锁或驱动。。
@details 启动时将旧 PID 端口适配器挂接到 UdpProtocol；不单独初始化网络锁或驱动。
```

<a id="fn-70bad3dd1519b0d6"></a>

## `xv::net::legacy_port_used`

```doxygen
@fn bool xv::net::legacy_port_used(unsigned short port)
@brief 调用者持统一注册锁。
@details 调用者持统一注册锁，检查旧 PID 端口表是否已占用同号 UDP 端口。
```

<a id="fn-f9bbe368471e7b58"></a>

## `xv::net::legacy_lock`

```doxygen
@fn xv::sync::SpinLock &xv::net::legacy_lock()
@brief 借用 SocketRegistry 统一锁。
@details 借用 SocketRegistry 统一锁，避免兼容端口与 socket 端口查重时形成双锁反向顺序。
```

<a id="fn-ba813c95f31783b0"></a>

## `xv::net::LegacyUdp::deliver`

```doxygen
@fn bool xv::net::LegacyUdp::deliver(Packet &packet, Endpoint source, Endpoint destination, unsigned offset, unsigned length)
@brief LegacyUdp：在统一注册锁下命中旧 PID 端口队列并移交 Packet 页面，最多保留十六报；命中但满时丢弃并返回 true。
@details 在统一注册锁下命中旧 PID 端口队列并移交 Packet 页面，最多保留十六报；命中但满时丢弃并返回 true。
```
<a id="fn-aa8207c3fb16e1a3"></a>

## `xv::net::send`

```doxygen
@fn int xv::net::send(unsigned short source, unsigned destination, unsigned short port, unsigned long data, unsigned length)
@brief 兼容旧 PID UDP 接口，暂存用户内容后把 IPv4 源留为零交路由选择；保留旧接口 1472 字节上限和首设备就绪检查。
```
