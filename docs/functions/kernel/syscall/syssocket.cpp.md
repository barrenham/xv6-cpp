# `kernel/syscall/syssocket.cpp`

## 函数导航

- [`xv::syscall::input_address`](syssocket.cpp.md#fn-b5d122765e74f3d7)
- [`xv::syscall::install`](syssocket.cpp.md#fn-8981757f3db9e0ef)
- [`xv::syscall::output_address`](syssocket.cpp.md#fn-8de84deafbadc94e)
- [`xv::syscall::socket_argument`](syssocket.cpp.md#fn-693314695aed9d5a)
- [`xv::syscall::sys_accept`](syssocket.cpp.md#fn-a551ee0c6cc31b66)
- [`xv::syscall::sys_connect`](syssocket.cpp.md#fn-6cc9bb91284d2288)
- [`xv::syscall::sys_getpeername`](syssocket.cpp.md#fn-ad79d7621aebac76)
- [`xv::syscall::sys_getsockname`](syssocket.cpp.md#fn-34ce2f3f218a1bcb)
- [`xv::syscall::sys_getsockopt`](syssocket.cpp.md#fn-dfca0367665d52c2)
- [`xv::syscall::sys_listen`](syssocket.cpp.md#fn-d8dff478749541a9)
- [`xv::syscall::sys_recvfrom`](syssocket.cpp.md#fn-f11541d42bf45977)
- [`xv::syscall::sys_sendto`](syssocket.cpp.md#fn-4280b55882ad0bde)
- [`xv::syscall::sys_shutdown`](syssocket.cpp.md#fn-936eb95148b68d1d)
- [`xv::syscall::sys_sockbind`](syssocket.cpp.md#fn-08947764259f8414)
- [`xv::syscall::sys_socket`](syssocket.cpp.md#fn-1248323fc95ac4c4)

<a id="fn-1248323fc95ac4c4"></a>

## `xv::syscall::sys_socket`

```doxygen
@fn unsigned long xv::syscall::sys_socket()
@brief Socket 系统调用：校验 AF_INET、STREAM/DGRAM、协议与 SOCK_NONBLOCK。
@details 校验 AF_INET、STREAM/DGRAM、协议与 SOCK_NONBLOCK，创建后端并安装 File，成功返回 fd。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-08947764259f8414"></a>

## `xv::syscall::sys_sockbind`

```doxygen
@fn unsigned long xv::syscall::sys_sockbind()
@brief 绑定 socket 本地地址和端口。
@details 复制并校验地址，1..1023 端口要求有效 UID 为 0；端口 0 的自动分配及普通端口对普通用户开放，其余状态校验由 socket 实现。
```

<a id="fn-6cc9bb91284d2288"></a>

## `xv::syscall::sys_connect`

```doxygen
@fn unsigned long xv::syscall::sys_connect()
@brief Socket 系统调用：读取 peer 后按 File 的 O_NONBLOCK 调用连接方法；非阻塞 TCP 返回 -EINPROGRESS。
@details 读取 peer 后按 File 的 O_NONBLOCK 调用连接方法；非阻塞 TCP 返回 -EINPROGRESS，后续 poll/SO_ERROR 确认结果。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-d8dff478749541a9"></a>

## `xv::syscall::sys_listen`

```doxygen
@fn unsigned long xv::syscall::sys_listen()
@brief Socket 系统调用：校验 Socket 描述符并传递 backlog 到监听实现。。
@details 校验 Socket 描述符并传递 backlog 到监听实现。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-a551ee0c6cc31b66"></a>

## `xv::syscall::sys_accept`

```doxygen
@fn unsigned long xv::syscall::sys_accept()
@brief Socket 系统调用：按监听 File 的非阻塞状态等待子连接。
@details 按监听 File 的非阻塞状态等待子连接，输出 peer 后安装新的可阻塞 File；复制或安装失败必须 close 新对象。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-4280b55882ad0bde"></a>

## `xv::syscall::sys_sendto`

```doxygen
@fn unsigned long xv::syscall::sys_sendto()
@brief Socket 系统调用：验证用户长度非负及 MSG_DONTWAIT。
@details 验证用户长度非负及 MSG_DONTWAIT，解析可选目的 sockaddr，将 flags 与 File 非阻塞标志合并后调用 Socket。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-f11541d42bf45977"></a>

## `xv::syscall::sys_recvfrom`

```doxygen
@fn unsigned long xv::syscall::sys_recvfrom()
@brief Socket 系统调用：验证容量与 MSG_DONTWAIT。
@details 验证容量与 MSG_DONTWAIT，接收后按可选 sockaddr/socklen_t 输出来源；数据报截断或字节流部分读取由后端决定。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-936eb95148b68d1d"></a>

## `xv::syscall::sys_shutdown`

```doxygen
@fn unsigned long xv::syscall::sys_shutdown()
@brief Socket 系统调用：将 SHUT_RD/WR/RDWR 转交网络能力接口。
@details 将 SHUT_RD/WR/RDWR 转交网络能力接口，保持 File 引用不变。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-34ce2f3f218a1bcb"></a>

## `xv::syscall::sys_getsockname`

```doxygen
@fn unsigned long xv::syscall::sys_getsockname()
@brief Socket 系统调用：复制本地 Endpoint 到用户 sockaddr_in。
@details 复制本地 Endpoint 到用户 sockaddr_in，并更新实际长度。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-ad79d7621aebac76"></a>

## `xv::syscall::sys_getpeername`

```doxygen
@fn unsigned long xv::syscall::sys_getpeername()
@brief Socket 系统调用：未连接时返回 -ENOTCONN。
@details 未连接时返回 -ENOTCONN，否则复制当前 peer 地址。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-dfca0367665d52c2"></a>

## `xv::syscall::sys_getsockopt`

```doxygen
@fn unsigned long xv::syscall::sys_getsockopt()
@brief Socket 系统调用：只接受 SOL_SOCKET/SO_ERROR。
@details 只接受 SOL_SOCKET/SO_ERROR，检查用户 int 缓冲容量，取出并清除后端挂起错误后复制到用户地址。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
```

<a id="fn-693314695aed9d5a"></a>

## `xv::syscall::socket_argument`

```doxygen
@fn xv::net::Socket *xv::syscall::socket_argument(xv::fs::File **file = nullptr)
@brief 查找第零参数对应的 File，通过 files().object 取得统一后端，并使用 io::query<Socket> 检查网络能力。
@details 查找第零参数对应的 File，通过 files().object 取得统一后端，并使用 io::query<Socket> 检查网络能力。普通文件、管道和设备返回 nullptr；不依赖 StreamKind、RTTI 或 File 类型标签。
```

<a id="fn-8981757f3db9e0ef"></a>

## `xv::syscall::install`

```doxygen
@fn int xv::syscall::install(xv::net::Socket *socket, bool nonblocking)
@brief 分配 File 后以 bind_object 接收新 socket 的 close 责任，再安装进当前描述符表。
@details 分配 File 后以 bind_object 接收新 socket 的 close 责任，再安装进当前描述符表。分配失败调用 socket close，安装失败释放 File，由统一生命周期回滚。
```
<a id="fn-b5d122765e74f3d7"></a>

## `xv::syscall::input_address`

```doxygen
@fn bool xv::syscall::input_address(unsigned long address, unsigned length, xv::net::Endpoint &endpoint)
@brief 从用户空间读取至少 sockaddr_in 长度。
@details 从用户空间读取至少 sockaddr_in 长度，校验 AF_INET，并把网络字节序 IPv4/端口转换为内部 Endpoint。
```

<a id="fn-8de84deafbadc94e"></a>

## `xv::syscall::output_address`

```doxygen
@fn int xv::syscall::output_address(unsigned long address, unsigned long length, xv::net::Endpoint endpoint)
@brief 按用户 socklen_t 容量截断复制 sockaddr_in。
@details 按用户 socklen_t 容量截断复制 sockaddr_in，并输出实际长度16；地址为空时允许调用者省略来源信息，非法用户地址返回 -1。
```
