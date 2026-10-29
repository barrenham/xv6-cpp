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
- [`xv::fs::InodeBackend::resize`](file_operations.cpp.md#fn-ae883e791f590ed4)
- [`xv::fs::InodeBackend::seek`](file_operations.cpp.md#fn-87a319e476eaf4ce)
- [`xv::fs::InodeBackend::stat`](file_operations.cpp.md#fn-f0d5394ca9774a31)
- [`xv::fs::InodeBackend::sync`](file_operations.cpp.md#fn-1e17b13e14602c79)
- [`xv::fs::InodeBackend::write`](file_operations.cpp.md#fn-1bb8e957e0af8807)
- [`xv::fs::PipeBackend::close`](file_operations.cpp.md#fn-d129694fa64d854e)
- [`xv::fs::PipeBackend::poll`](file_operations.cpp.md#fn-862c9c14fb97258a)
- [`xv::fs::PipeBackend::read`](file_operations.cpp.md#fn-b4fd0e70d6fa5ac3)
- [`xv::fs::PipeBackend::write`](file_operations.cpp.md#fn-687505aae3abce7a)

<a id="fn-ccfc01f791da65b4"></a>

## `xv::fs::InodeBackend::close`

```doxygen
@fn void xv::fs::InodeBackend::close()
@brief 释放节点引用并析构页链快照。
@details 持久节点使用条件磁盘事务，RAM/procfs 节点只更新内存归属。析构 PageBuffer 回收任意数量的快照页，不再假定单页内容。
```

<a id="fn-483ee20032be6151"></a>

## `xv::fs::InodeBackend::read`

```doxygen
@fn int xv::fs::InodeBackend::read(xv::io::Buffer destination, int count)
@brief 按共享偏移读取动态快照或实际节点。
@details 在所属文件系统锁内串行化偏移。快照调用 PageBuffer 跨页复制，普通文件委托节点 read；仅用实际成功字节数推进，失败复制不会跳过未完成部分。
@details 流程由 ReadFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
@return 保留具体后端读取结果；正返回值才推进共享文件偏移。
```

<a id="fn-1bb8e957e0af8807"></a>

## `xv::fs::InodeBackend::write`

```doxygen
@fn int xv::fs::InodeBackend::write(xv::io::Buffer source, int count)
@brief 串行更新共享偏移并执行分块写入。
@details 每块写入在事务和 inode 锁内清除特殊执行位；O_APPEND 在同一锁内从最新 EOF 定位，避免独立打开的追加覆盖。只推进实际成功写入的字节数。
@details 流程由 WriteFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
@return 返回已完成字节数；无进展的非空或中断失败返回 -1，空写入返回 0。
```

<a id="fn-f0d5394ca9774a31"></a>

## `xv::fs::InodeBackend::stat`

```doxygen
@fn int xv::fs::InodeBackend::stat(xv::io::Metadata &value)
@brief 读取节点元数据，并用动态快照长度覆盖打开文件大小。
@details 快照大小与本次打开的内容和 EOF 一致，不重新生成内容；普通 RAM/磁盘文件大小来自所属节点。
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
@brief 关闭会话并释放节点引用，最后析构适配器。
@details 先归还具体会话，PTY 可挂断或减少从端计数，再按节点持久性选择事务释放 inode。动态节点仍持 PTY 固定引用，确保会话关闭期间编号不会被提前复用；全局静态设备 close 为空操作。
```

<a id="fn-b9b2f2e24e02b04c"></a>

## `xv::fs::DeviceBackend::read`

```doxygen
@fn int xv::fs::DeviceBackend::read(xv::io::Buffer destination, int count)
@brief 向本次打开取得的设备会话转发 read。
@details 使用已取得的 CharacterDevice 会话指针，不在每次 I/O 重新按设备号查找，保证旧描述符仍指向原会话。传入用户缓冲、共享非阻塞标志或等待表；访问权限由 File 层过滤。
```

<a id="fn-e5321d7ef73dc31e"></a>

## `xv::fs::DeviceBackend::write`

```doxygen
@fn int xv::fs::DeviceBackend::write(xv::io::Buffer source, int count)
@brief 向本次打开取得的设备会话转发 write。
@details 使用已取得的 CharacterDevice 会话指针，不在每次 I/O 重新按设备号查找，保证旧描述符仍指向原会话。传入用户缓冲、共享非阻塞标志或等待表；访问权限由 File 层过滤。
```

<a id="fn-9655ed01c03eea58"></a>

## `xv::fs::DeviceBackend::stat`

```doxygen
@fn int xv::fs::DeviceBackend::stat(xv::io::Metadata &value)
@brief 在所属文件系统节点锁内读取设备元数据。
@details 统计来自被本 File 引用的节点，动态设备名撤下后已有 fd 仍保留原节点身份。
```

<a id="fn-efd08a89bd78b547"></a>

## `xv::fs::DeviceBackend::poll`

```doxygen
@fn unsigned xv::fs::DeviceBackend::poll(xv::sync::PollTable *table)
@brief 向本次打开取得的设备会话转发 poll。
@details 使用已取得的 CharacterDevice 会话指针，不在每次 I/O 重新按设备号查找，保证旧描述符仍指向原会话。传入用户缓冲、共享非阻塞标志或等待表；访问权限由 File 层过滤。
```

<a id="fn-a21fe337437e1214"></a>

## `xv::fs::DeviceBackend::ioctl`

```doxygen
@fn int xv::fs::DeviceBackend::ioctl(unsigned request, xv::io::Buffer argument)
@brief 向本次打开取得的设备会话转发 ioctl。
@details 使用已取得的 CharacterDevice 会话指针，不在每次 I/O 重新按设备号查找，保证旧描述符仍指向原会话。传入用户缓冲、共享非阻塞标志或等待表；访问权限由 File 层过滤。
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

<a id="fn-87a319e476eaf4ce"></a>

## `xv::fs::InodeBackend::seek`

```doxygen
@fn long xv::fs::InodeBackend::seek(long offset, int origin)
@brief 在普通文件上调整共享偏移。
@details inode 锁内按 SET/CUR/END 计算，拒绝未知 origin、负结果和超出当前整型范围的偏移；快照的 END 使用固定快照长度，失败不改变偏移。
@details 流程由 SeekFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
@return 成功返回提交的新偏移；类型、origin 或范围不合法返回 -1。
```

<a id="fn-ae883e791f590ed4"></a>

## `xv::fs::InodeBackend::resize`

```doxygen
@fn int xv::fs::InodeBackend::resize(unsigned size)
@brief 在事务内调整可写普通文件的长度。
@details 调用者已检查打开权限；锁住 inode，拒绝快照和只读后端，清除 setid 位并交给后端 resize，不改变本次打开的偏移。
@details 流程由 ResizeFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
@return 保留具体文件系统 resize 的返回值；快照或只读后端返回 -1。
```

<a id="fn-1e17b13e14602c79"></a>

## `xv::fs::InodeBackend::sync`

```doxygen
@fn int xv::fs::InodeBackend::sync()
@brief 等待相关日志批次并刷新所属后端。
@details 只接受普通文件和目录；持久节点等待根盘和可选数据日志，再 flush 所属设备，RAM 节点成功返回而不写磁盘。
@details 流程由 SyncFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
@return 普通文件或目录同步成功返回 0，其他类型返回 -1。
```
