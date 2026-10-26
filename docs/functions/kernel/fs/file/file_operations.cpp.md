# `kernel/fs/file/file_operations.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/file_operations.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:platform`, `:operations`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::DeviceBackend::close`](file_operations.cpp.md#fn-92e70fc857d468f0)
- [`xv::fs::DeviceBackend::ioctl`](file_operations.cpp.md#fn-a21fe337437e1214)
- [`xv::fs::DeviceBackend::poll`](file_operations.cpp.md#fn-efd08a89bd78b547)
- [`xv::fs::DeviceBackend::read`](file_operations.cpp.md#fn-b9b2f2e24e02b04c)
- [`xv::fs::DeviceBackend::stat`](file_operations.cpp.md#fn-9655ed01c03eea58)
- [`xv::fs::DeviceBackend::write`](file_operations.cpp.md#fn-e5321d7ef73dc31e)
- [`xv::fs::EndpointBackend::close`](file_operations.cpp.md#fn-34d2dde20b9c4f7c)
- [`xv::fs::EndpointBackend::ioctl`](file_operations.cpp.md#fn-c8010c3e18e186cd)
- [`xv::fs::EndpointBackend::poll`](file_operations.cpp.md#fn-efdd22fee0da96bf)
- [`xv::fs::EndpointBackend::read`](file_operations.cpp.md#fn-19336e2add16010e)
- [`xv::fs::EndpointBackend::stat`](file_operations.cpp.md#fn-73d806a8fe22282c)
- [`xv::fs::EndpointBackend::write`](file_operations.cpp.md#fn-c225090b7714abf6)
- [`xv::fs::InodeBackend::close`](file_operations.cpp.md#fn-ccfc01f791da65b4)
- [`xv::fs::InodeBackend::poll`](file_operations.cpp.md#fn-5fada80fee636ddb)
- [`xv::fs::InodeBackend::read`](file_operations.cpp.md#fn-483ee20032be6151)
- [`xv::fs::InodeBackend::stat`](file_operations.cpp.md#fn-f0d5394ca9774a31)
- [`xv::fs::InodeBackend::write`](file_operations.cpp.md#fn-1bb8e957e0af8807)
- [`xv::fs::PipeBackend::close`](file_operations.cpp.md#fn-d129694fa64d854e)
- [`xv::fs::PipeBackend::poll`](file_operations.cpp.md#fn-862c9c14fb97258a)
- [`xv::fs::PipeBackend::read`](file_operations.cpp.md#fn-b4fd0e70d6fa5ac3)
- [`xv::fs::PipeBackend::write`](file_operations.cpp.md#fn-687505aae3abce7a)

<a id="fn-ccfc01f791da65b4"></a>

## `xv::fs::InodeBackend::close`

```doxygen
@fn void xv::fs::InodeBackend::close()
@brief 在 Transaction 内归还拥有的 inode 引用，必要时完成 unlink 后的截断与日志操作，随后显式析构 File 内置对象；不释放 File 存储。
@details 在 Transaction 内归还拥有的 inode 引用，必要时完成 unlink 后的截断与日志操作，随后显式析构 File 内置对象；不释放 File 存储。
```

<a id="fn-483ee20032be6151"></a>

## `xv::fs::InodeBackend::read`

```doxygen
@fn int xv::fs::InodeBackend::read(xv::io::Buffer destination, int count)
@brief 持 inode 睡眠锁按本打开对象的 offset_ 读取用户数据，正进度才增加偏移。
@details 持 inode 睡眠锁按本打开对象的 offset_ 读取用户数据，正进度才增加偏移。dup/fork 共享同一 Backend，独立 open 分别构造对象，偏移互不影响。
```

<a id="fn-1bb8e957e0af8807"></a>

## `xv::fs::InodeBackend::write`

```doxygen
@fn int xv::fs::InodeBackend::write(xv::io::Buffer source, int count)
@brief 保存原有 MAXOPBLOCKS 事务分段写策略；在事务和 inode 锁内按本打开对象偏移写入并更新正进度，保留原有中断及短写返回约定。
@details 保存原有 MAXOPBLOCKS 事务分段写策略；在事务和 inode 锁内按本打开对象偏移写入并更新正进度，保留原有中断及短写返回约定。
```

<a id="fn-f0d5394ca9774a31"></a>

## `xv::fs::InodeBackend::stat`

```doxygen
@fn int xv::fs::InodeBackend::stat(xv::io::Metadata &value)
@brief 持 inode 锁把文件系统元数据填到内核 Metadata，返回0；不在后端锁内执行用户 copy_out。
@details 持 inode 锁把文件系统元数据填到内核 Metadata，返回0；不在后端锁内执行用户 copy_out。
```

<a id="fn-5fada80fee636ddb"></a>

## `xv::fs::InodeBackend::poll`

```doxygen
@fn unsigned xv::fs::InodeBackend::poll(xv::sync::PollTable *)
@brief 普通 inode 后端立即报告 POLLIN|POLLOUT，最终访问权限由 File 统一过滤，不登记等待通道。
@details 普通 inode 后端立即报告 POLLIN|POLLOUT，最终访问权限由 File 统一过滤，不登记等待通道。
```

<a id="fn-92e70fc857d468f0"></a>

## `xv::fs::DeviceBackend::close`

```doxygen
@fn void xv::fs::DeviceBackend::close()
@brief 只在事务中归还设备节点 inode 引用并析构内置适配器，不关闭全局 CharacterDevice 实例，保持控制台生命周期语义。
@details 只在事务中归还设备节点 inode 引用并析构内置适配器，不关闭全局 CharacterDevice 实例，保持控制台生命周期语义。
```

<a id="fn-b9b2f2e24e02b04c"></a>

## `xv::fs::DeviceBackend::read`

```doxygen
@fn int xv::fs::DeviceBackend::read(xv::io::Buffer destination, int count)
@brief 按保存的 major 查询长期存活字符设备，传递用户地址和 File 非阻塞标志；设备缺失返回 -1，不使用 inode 文件偏移。
@details 按保存的 major 查询长期存活字符设备，传递用户地址和 File 非阻塞标志；设备缺失返回 -1，不使用 inode 文件偏移。
```

<a id="fn-e5321d7ef73dc31e"></a>

## `xv::fs::DeviceBackend::write`

```doxygen
@fn int xv::fs::DeviceBackend::write(xv::io::Buffer source, int count)
@brief 按 major 借用字符设备并转发用户 Buffer 与非阻塞状态；不存在设备时返回 -1。
@details 按 major 借用字符设备并转发用户 Buffer 与非阻塞状态；不存在设备时返回 -1。
```

<a id="fn-9655ed01c03eea58"></a>

## `xv::fs::DeviceBackend::stat`

```doxygen
@fn int xv::fs::DeviceBackend::stat(xv::io::Metadata &value)
@brief 持 inode 锁读取设备节点真实元数据到 Metadata，保留设备节点 inode 身份。
@details 持 inode 锁读取设备节点真实元数据到 Metadata，保留设备节点 inode 身份。
```

<a id="fn-efd08a89bd78b547"></a>

## `xv::fs::DeviceBackend::poll`

```doxygen
@fn unsigned xv::fs::DeviceBackend::poll(xv::sync::PollTable *table)
@brief 把 PollTable 交给对应字符设备登记通道并读取状态；无设备报告 POLLERR。
@details 把 PollTable 交给对应字符设备登记通道并读取状态；无设备报告 POLLERR。
```

<a id="fn-a21fe337437e1214"></a>

## `xv::fs::DeviceBackend::ioctl`

```doxygen
@fn int xv::fs::DeviceBackend::ioctl(unsigned request, xv::io::Buffer argument)
@brief 独立 DeviceControl 能力实现，查询 major 对应字符设备并以用户地址转发控制请求；无设备返回 -1。
@details 独立 DeviceControl 能力实现，查询 major 对应字符设备并以用户地址转发控制请求；无设备返回 -1。
```

<a id="fn-d129694fa64d854e"></a>

## `xv::fs::PipeBackend::close`

```doxygen
@fn void xv::fs::PipeBackend::close()
@brief 关闭共享 Pipe 的对应方向并唤醒对端，最后方向关闭时 Pipe 自行归还缓冲页；随后析构 File 内置适配器。
@details 关闭共享 Pipe 的对应方向并唤醒对端，最后方向关闭时 Pipe 自行归还缓冲页；随后析构 File 内置适配器。
```

<a id="fn-b4fd0e70d6fa5ac3"></a>

## `xv::fs::PipeBackend::read`

```doxygen
@fn int xv::fs::PipeBackend::read(xv::io::Buffer destination, int count)
@brief 把用户目标地址、长度和非阻塞状态转给共享 Pipe；File 已校验此端具有读权限。
@details 把用户目标地址、长度和非阻塞状态转给共享 Pipe；File 已校验此端具有读权限。
```

<a id="fn-687505aae3abce7a"></a>

## `xv::fs::PipeBackend::write`

```doxygen
@fn int xv::fs::PipeBackend::write(xv::io::Buffer source, int count)
@brief 把用户源地址、长度和非阻塞状态转给共享 Pipe；保留满缓冲等待、原子小写和对端关闭语义。
@details 把用户源地址、长度和非阻塞状态转给共享 Pipe；保留满缓冲等待、原子小写和对端关闭语义。
```

<a id="fn-862c9c14fb97258a"></a>

## `xv::fs::PipeBackend::poll`

```doxygen
@fn unsigned xv::fs::PipeBackend::poll(xv::sync::PollTable *table)
@brief 按本端 writable_ 方向让 Pipe 登记对应读/写条件通道并返回状态。
@details 按本端 writable_ 方向让 Pipe 登记对应读/写条件通道并返回状态。
```

<a id="fn-34d2dde20b9c4f7c"></a>

## `xv::fs::EndpointBackend::close`

```doxygen
@fn void xv::fs::EndpointBackend::close()
@brief 最后 File 引用触发端点 close，再显式析构内置适配器。
@details 最后 File 引用触发端点 close，再显式析构内置适配器。PTY 端点可能在 close 中释放共享页面，本函数随后不再访问该端点。
```

<a id="fn-19336e2add16010e"></a>

## `xv::fs::EndpointBackend::read`

```doxygen
@fn int xv::fs::EndpointBackend::read(xv::io::Buffer destination, int count)
@brief 把用户地址及非阻塞状态转交独立 CharacterDevice 端点，适配 TTY/PTY 的原有读协议。
@details 把用户地址及非阻塞状态转交独立 CharacterDevice 端点，适配 TTY/PTY 的原有读协议。
```

<a id="fn-c225090b7714abf6"></a>

## `xv::fs::EndpointBackend::write`

```doxygen
@fn int xv::fs::EndpointBackend::write(xv::io::Buffer source, int count)
@brief 把用户地址和非阻塞状态转交端点，保持部分写与终端行规程语义。
@details 把用户地址和非阻塞状态转交端点，保持部分写与终端行规程语义。
```

<a id="fn-73d806a8fe22282c"></a>

## `xv::fs::EndpointBackend::stat`

```doxygen
@fn int xv::fs::EndpointBackend::stat(xv::io::Metadata &value)
@brief 为不经过 inode 的匿名字符端点提供 T_DEVICE、nlink=1 的内核元数据。
@details 为不经过 inode 的匿名字符端点提供 T_DEVICE、nlink=1 的内核元数据。
```

<a id="fn-efdd22fee0da96bf"></a>

## `xv::fs::EndpointBackend::poll`

```doxygen
@fn unsigned xv::fs::EndpointBackend::poll(xv::sync::PollTable *table)
@brief 将 PollTable 原样转发端点，由端点锁内登记条件与计算就绪状态。
@details 将 PollTable 原样转发端点，由端点锁内登记条件与计算就绪状态。
```

<a id="fn-c8010c3e18e186cd"></a>

## `xv::fs::EndpointBackend::ioctl`

```doxygen
@fn int xv::fs::EndpointBackend::ioctl(unsigned request, xv::io::Buffer argument)
@brief 通过独立设备控制接口转发用户控制参数，保留 termios、前台进程组和终端请求处理。
@details 通过独立设备控制接口转发用户控制参数，保留 termios、前台进程组和终端请求处理。
```
