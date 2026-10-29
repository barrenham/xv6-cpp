# `kernel/syscall/syssocket.cpp`

## 函数导航

- [`xv::syscall::SocketAcceptFlow::accept`](syssocket.cpp.md#fn-d8a95de30b8eef95)
- [`xv::syscall::SocketAcceptFlow::close`](syssocket.cpp.md#fn-eb2e7584017327ac)
- [`xv::syscall::SocketAcceptFlow::copy`](syssocket.cpp.md#fn-bd5659f74868e76d)
- [`xv::syscall::SocketAcceptFlow::install`](syssocket.cpp.md#fn-ca9dc50c0ab54c6a)
- [`xv::syscall::SocketAcceptFlow::rejected`](syssocket.cpp.md#fn-dfcad093cec1e8e0)
- [`xv::syscall::SocketAddressFlow::capacity`](syssocket.cpp.md#fn-74ffb829d21572be)
- [`xv::syscall::SocketAddressFlow::copy_address`](syssocket.cpp.md#fn-659804b32def9d74)
- [`xv::syscall::SocketAddressFlow::copy_length`](syssocket.cpp.md#fn-8186c961bdc48d1e)
- [`xv::syscall::SocketAddressFlow::prepare`](syssocket.cpp.md#fn-310cd005ba3433d6)
- [`xv::syscall::SocketAddressFlow::success`](syssocket.cpp.md#fn-56e2fec78b555dd8)
- [`xv::syscall::SocketBindFlow::bind`](syssocket.cpp.md#fn-22a929ccd04acb10)
- [`xv::syscall::SocketBindFlow::copy`](syssocket.cpp.md#fn-ffded64b6e914ed6)
- [`xv::syscall::SocketBindFlow::permitted`](syssocket.cpp.md#fn-80c40b3ea07e8208)
- [`xv::syscall::SocketConnectFlow::connect`](syssocket.cpp.md#fn-b5bc7941be11a19f)
- [`xv::syscall::SocketConnectFlow::copy`](syssocket.cpp.md#fn-1f21561b90d6323e)
- [`xv::syscall::SocketInstallFlow::allocate`](syssocket.cpp.md#fn-58f4471b24bebaaf)
- [`xv::syscall::SocketInstallFlow::bind`](syssocket.cpp.md#fn-5b24bc8c89ddfb68)
- [`xv::syscall::SocketInstallFlow::close`](syssocket.cpp.md#fn-f44099d96d2a5301)
- [`xv::syscall::SocketInstallFlow::install`](syssocket.cpp.md#fn-e10fae42da5345c0)
- [`xv::syscall::SocketInstallFlow::release`](syssocket.cpp.md#fn-ff7c02f422c42830)
- [`xv::syscall::SocketOptionFlow::consume`](syssocket.cpp.md#fn-f2b4ce3bf689109b)
- [`xv::syscall::SocketOptionFlow::copy_length`](syssocket.cpp.md#fn-8f9ed4caa0de3dbf)
- [`xv::syscall::SocketOptionFlow::copy_value`](syssocket.cpp.md#fn-95903de2babad901)
- [`xv::syscall::SocketOptionFlow::success`](syssocket.cpp.md#fn-c930c9b73f93bba0)
- [`xv::syscall::SocketOptionFlow::valid`](syssocket.cpp.md#fn-041bb9fc97b88014)
- [`xv::syscall::SocketReceiveFlow::bad_copy`](syssocket.cpp.md#fn-5f7adbc3602ed852)
- [`xv::syscall::SocketReceiveFlow::copy`](syssocket.cpp.md#fn-13b164a31f481bc7)
- [`xv::syscall::SocketReceiveFlow::receive`](syssocket.cpp.md#fn-6e47cc9263f4799b)
- [`xv::syscall::SocketReceiveFlow::received`](syssocket.cpp.md#fn-4e8422acb6819a70)
- [`xv::syscall::SocketSendFlow::addressed`](syssocket.cpp.md#fn-0ecf9b251bec7dec)
- [`xv::syscall::SocketSendFlow::copy`](syssocket.cpp.md#fn-d9a6d9751e391bf8)
- [`xv::syscall::SocketSendFlow::send`](syssocket.cpp.md#fn-3fff7debc4bbd0fc)
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
@details 流程由 SocketBindFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-6cc9bb91284d2288"></a>

## `xv::syscall::sys_connect`

```doxygen
@fn unsigned long xv::syscall::sys_connect()
@brief Socket 系统调用：读取 peer 后按 File 的 O_NONBLOCK 调用连接方法；非阻塞 TCP 返回 -EINPROGRESS。
@details 读取 peer 后按 File 的 O_NONBLOCK 调用连接方法；非阻塞 TCP 返回 -EINPROGRESS，后续 poll/SO_ERROR 确认结果。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
@details 流程由 SocketConnectFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 SocketAcceptFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-4280b55882ad0bde"></a>

## `xv::syscall::sys_sendto`

```doxygen
@fn unsigned long xv::syscall::sys_sendto()
@brief Socket 系统调用：验证用户长度非负及 MSG_DONTWAIT。
@details 验证用户长度非负及 MSG_DONTWAIT，解析可选目的 sockaddr，将 flags 与 File 非阻塞标志合并后调用 Socket。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
@details 流程由 SocketSendFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-f11541d42bf45977"></a>

## `xv::syscall::sys_recvfrom`

```doxygen
@fn unsigned long xv::syscall::sys_recvfrom()
@brief Socket 系统调用：验证容量与 MSG_DONTWAIT。
@details 验证容量与 MSG_DONTWAIT，接收后按可选 sockaddr/socklen_t 输出来源；数据报截断或字节流部分读取由后端决定。
@return 实际 fd/字节数或约定成功值；失败返回负错误。
@details 流程由 SocketReceiveFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 SocketOptionFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 SocketInstallFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
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
@details 流程由 SocketAddressFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-d8a95de30b8eef95"></a>

## `xv::syscall::SocketAcceptFlow::accept`

```doxygen
@fn static bool xv::syscall::SocketAcceptFlow::accept(Context &c) noexcept
@brief 按监听文件非阻塞状态取得候选子 socket 或原业务错误。
@details 原监听 socket/文件查询位置不变；接受失败保留业务错误，地址输出失败关闭 child，交给共享安装入口后不再重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-eb2e7584017327ac"></a>

## `xv::syscall::SocketAcceptFlow::close`

```doxygen
@fn static void xv::syscall::SocketAcceptFlow::close(Context &c) noexcept { c.child->close(); }
@brief 地址输出失败时关闭未安装子 socket。
@details 原监听 socket/文件查询位置不变；接受失败保留业务错误，地址输出失败关闭 child，交给共享安装入口后不再重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bd5659f74868e76d"></a>

## `xv::syscall::SocketAcceptFlow::copy`

```doxygen
@fn static bool xv::syscall::SocketAcceptFlow::copy(Context &c) noexcept
@brief 输出候选子 socket 的对端地址。
@details 原监听 socket/文件查询位置不变；接受失败保留业务错误，地址输出失败关闭 child，交给共享安装入口后不再重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ca9dc50c0ab54c6a"></a>

## `xv::syscall::SocketAcceptFlow::install`

```doxygen
@fn static void xv::syscall::SocketAcceptFlow::install(Context &c) noexcept
@brief 调用公共 socket 安装入口交接候选并保存描述符。
@details 原监听 socket/文件查询位置不变；接受失败保留业务错误，地址输出失败关闭 child，交给共享安装入口后不再重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dfcad093cec1e8e0"></a>

## `xv::syscall::SocketAcceptFlow::rejected`

```doxygen
@fn static void xv::syscall::SocketAcceptFlow::rejected(Context &c) noexcept { c.result = c.error; }
@brief 保存未取得子 socket 的业务错误。
@details 原监听 socket/文件查询位置不变；接受失败保留业务错误，地址输出失败关闭 child，交给共享安装入口后不再重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-74ffb829d21572be"></a>

## `xv::syscall::SocketAddressFlow::capacity`

```doxygen
@fn static bool xv::syscall::SocketAddressFlow::capacity(Context &c) noexcept
@brief 从用户长度指针读取地址缓冲容量。
@details 空地址在原入口直接成功；其余先读容量，再按容量复制地址，最后写真实长度，后一失败保留前一输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-659804b32def9d74"></a>

## `xv::syscall::SocketAddressFlow::copy_address`

```doxygen
@fn static bool xv::syscall::SocketAddressFlow::copy_address(Context &c) noexcept
@brief 按用户容量先复制地址记录。
@details 空地址在原入口直接成功；其余先读容量，再按容量复制地址，最后写真实长度，后一失败保留前一输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8186c961bdc48d1e"></a>

## `xv::syscall::SocketAddressFlow::copy_length`

```doxygen
@fn static bool xv::syscall::SocketAddressFlow::copy_length(Context &c) noexcept
@brief 地址复制成功后写回真实地址长度。
@details 空地址在原入口直接成功；其余先读容量，再按容量复制地址，最后写真实长度，后一失败保留前一输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-310cd005ba3433d6"></a>

## `xv::syscall::SocketAddressFlow::prepare`

```doxygen
@fn static void xv::syscall::SocketAddressFlow::prepare(Context &c) noexcept
@brief 准备清零的 IPv4 地址记录并按真实长度夹住容量。
@details 空地址在原入口直接成功；其余先读容量，再按容量复制地址，最后写真实长度，后一失败保留前一输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-56e2fec78b555dd8"></a>

## `xv::syscall::SocketAddressFlow::success`

```doxygen
@fn static void xv::syscall::SocketAddressFlow::success(Context &c) noexcept { c.result = 0; }
@brief 记录两次地址输出均成功。
@details 空地址在原入口直接成功；其余先读容量，再按容量复制地址，最后写真实长度，后一失败保留前一输出。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-22a929ccd04acb10"></a>

## `xv::syscall::SocketBindFlow::bind`

```doxygen
@fn static void xv::syscall::SocketBindFlow::bind(Context &c) noexcept { c.result = c.socket->bind(c.local); }
@brief 提交本地地址绑定并保存业务返回值。
@details 先复制并验证地址，再按原低端口规则检查有效身份，最后才提交绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ffded64b6e914ed6"></a>

## `xv::syscall::SocketBindFlow::copy`

```doxygen
@fn static bool xv::syscall::SocketBindFlow::copy(Context &c) noexcept
@brief 复制并验证待绑定的 IPv4 地址。
@details 先复制并验证地址，再按原低端口规则检查有效身份，最后才提交绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-80c40b3ea07e8208"></a>

## `xv::syscall::SocketBindFlow::permitted`

```doxygen
@fn static bool xv::syscall::SocketBindFlow::permitted(Context &c) noexcept
@brief 按原非零低端口和有效 UID 规则检查绑定权限。
@details 先复制并验证地址，再按原低端口规则检查有效身份，最后才提交绑定。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b5bc7941be11a19f"></a>

## `xv::syscall::SocketConnectFlow::connect`

```doxygen
@fn static void xv::syscall::SocketConnectFlow::connect(Context &c) noexcept
@brief 用原文件非阻塞状态提交连接并保存结果。
@details 保留参数门槛、地址复制以及文件共享非阻塞状态读取的位置；连接生命周期仍由原网络业务负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1f21561b90d6323e"></a>

## `xv::syscall::SocketConnectFlow::copy`

```doxygen
@fn static bool xv::syscall::SocketConnectFlow::copy(Context &c) noexcept
@brief 复制并验证连接对端地址。
@details 保留参数门槛、地址复制以及文件共享非阻塞状态读取的位置；连接生命周期仍由原网络业务负责。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-58f4471b24bebaaf"></a>

## `xv::syscall::SocketInstallFlow::allocate`

```doxygen
@fn static bool xv::syscall::SocketInstallFlow::allocate(Context &c) noexcept
@brief 取得用于承载 socket 的候选文件引用。
@details socket 创建和 accept 共用本安装入口；文件分配失败直接关闭 socket，绑定后安装失败由文件释放回收 socket，不能重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5b24bc8c89ddfb68"></a>

## `xv::syscall::SocketInstallFlow::bind`

```doxygen
@fn static void xv::syscall::SocketInstallFlow::bind(Context &c) noexcept
@brief 按原非阻塞标志把 socket 交给候选文件。
@details socket 创建和 accept 共用本安装入口；文件分配失败直接关闭 socket，绑定后安装失败由文件释放回收 socket，不能重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f44099d96d2a5301"></a>

## `xv::syscall::SocketInstallFlow::close`

```doxygen
@fn static void xv::syscall::SocketInstallFlow::close(Context &c) noexcept { c.socket->close(); }
@brief 文件分配失败时关闭尚未交接的 socket。
@details socket 创建和 accept 共用本安装入口；文件分配失败直接关闭 socket，绑定后安装失败由文件释放回收 socket，不能重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-e10fae42da5345c0"></a>

## `xv::syscall::SocketInstallFlow::install`

```doxygen
@fn static bool xv::syscall::SocketInstallFlow::install(Context &c) noexcept
@brief 把已绑定文件安装到当前进程描述符表。
@details socket 创建和 accept 共用本安装入口；文件分配失败直接关闭 socket，绑定后安装失败由文件释放回收 socket，不能重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ff7c02f422c42830"></a>

## `xv::syscall::SocketInstallFlow::release`

```doxygen
@fn static void xv::syscall::SocketInstallFlow::release(Context &c) noexcept { xv::fs::files().release(c.file); }
@brief 安装失败时释放文件并由原协议关闭 socket。
@details socket 创建和 accept 共用本安装入口；文件分配失败直接关闭 socket，绑定后安装失败由文件释放回收 socket，不能重复关闭。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-f2b4ce3bf689109b"></a>

## `xv::syscall::SocketOptionFlow::consume`

```doxygen
@fn static void xv::syscall::SocketOptionFlow::consume(Context &c) noexcept
@brief 读取并消费原 socket 错误，再设置实际输出长度。
@details 先完整验证 socket、选项和用户容量，再消费 SO_ERROR；结果值先复制，真实长度后复制，失败不恢复已消费的错误。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8f9ed4caa0de3dbf"></a>

## `xv::syscall::SocketOptionFlow::copy_length`

```doxygen
@fn static bool xv::syscall::SocketOptionFlow::copy_length(Context &c) noexcept
@brief 错误值复制成功后写回实际长度。
@details 先完整验证 socket、选项和用户容量，再消费 SO_ERROR；结果值先复制，真实长度后复制，失败不恢复已消费的错误。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-95903de2babad901"></a>

## `xv::syscall::SocketOptionFlow::copy_value`

```doxygen
@fn static bool xv::syscall::SocketOptionFlow::copy_value(Context &c) noexcept
@brief 先把已取出的 socket 错误复制给用户。
@details 先完整验证 socket、选项和用户容量，再消费 SO_ERROR；结果值先复制，真实长度后复制，失败不恢复已消费的错误。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c930c9b73f93bba0"></a>

## `xv::syscall::SocketOptionFlow::success`

```doxygen
@fn static void xv::syscall::SocketOptionFlow::success(Context &c) noexcept { c.result = 0; }
@brief 记录两次 socket 选项输出均成功。
@details 先完整验证 socket、选项和用户容量，再消费 SO_ERROR；结果值先复制，真实长度后复制，失败不恢复已消费的错误。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-041bb9fc97b88014"></a>

## `xv::syscall::SocketOptionFlow::valid`

```doxygen
@fn static bool xv::syscall::SocketOptionFlow::valid(Context &c) noexcept
@brief 按原短路顺序验证 socket、SO_ERROR 及用户容量。
@details 先完整验证 socket、选项和用户容量，再消费 SO_ERROR；结果值先复制，真实长度后复制，失败不恢复已消费的错误。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-5f7adbc3602ed852"></a>

## `xv::syscall::SocketReceiveFlow::bad_copy`

```doxygen
@fn static void xv::syscall::SocketReceiveFlow::bad_copy(Context &c) noexcept { c.result = -1; }
@brief 记录接收后地址输出失败而保留既有接收效果。
@details 接收非负结果才尝试输出对端；后续地址输出失败不撤销已经发生的数据接收，只把返回值改为负一。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-13b164a31f481bc7"></a>

## `xv::syscall::SocketReceiveFlow::copy`

```doxygen
@fn static bool xv::syscall::SocketReceiveFlow::copy(Context &c) noexcept
@brief 输出本次接收报告的对端地址。
@details 接收非负结果才尝试输出对端；后续地址输出失败不撤销已经发生的数据接收，只把返回值改为负一。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-6e47cc9263f4799b"></a>

## `xv::syscall::SocketReceiveFlow::receive`

```doxygen
@fn static void xv::syscall::SocketReceiveFlow::receive(Context &c) noexcept
@brief 按原文件状态及 MSG_DONTWAIT 执行一次接收。
@details 接收非负结果才尝试输出对端；后续地址输出失败不撤销已经发生的数据接收，只把返回值改为负一。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4e8422acb6819a70"></a>

## `xv::syscall::SocketReceiveFlow::received`

```doxygen
@fn static bool xv::syscall::SocketReceiveFlow::received(Context &c) noexcept { return c.result >= 0; }
@brief 检查接收是否得到非负业务结果。
@details 接收非负结果才尝试输出对端；后续地址输出失败不撤销已经发生的数据接收，只把返回值改为负一。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-0ecf9b251bec7dec"></a>

## `xv::syscall::SocketSendFlow::addressed`

```doxygen
@fn static bool xv::syscall::SocketSendFlow::addressed(Context &c) noexcept { return c.address != 0; }
@brief 检查本次是否提供显式目的地址。
@details 显式地址存在时才复制对端；发送仍读取文件当前非阻塞状态并组合 MSG_DONTWAIT，不缓存或重复发送。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-d9a6d9751e391bf8"></a>

## `xv::syscall::SocketSendFlow::copy`

```doxygen
@fn static bool xv::syscall::SocketSendFlow::copy(Context &c) noexcept
@brief 复制并验证显式目的地址。
@details 显式地址存在时才复制对端；发送仍读取文件当前非阻塞状态并组合 MSG_DONTWAIT，不缓存或重复发送。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3fff7debc4bbd0fc"></a>

## `xv::syscall::SocketSendFlow::send`

```doxygen
@fn static void xv::syscall::SocketSendFlow::send(Context &c) noexcept
@brief 按原非阻塞策略执行一次发送并保存结果。
@details 显式地址存在时才复制对端；发送仍读取文件当前非阻塞状态并组合 MSG_DONTWAIT，不缓存或重复发送。 下一节点由图策略声明，业务结果与控制信号分离。
```
