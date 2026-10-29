# `kernel/net/socket/socket.cppm`

分层职责、所有权与维护约定见 [网络说明](../../../../network.md)。

## 函数导航

- [`xv::net::Socket::Socket`](socket.cppm.md#fn-d80018a2029ae04d)
- [`xv::net::Socket::accept`](socket.cppm.md#fn-c4cda7ae200f4be8)
- [`xv::net::Socket::bind`](socket.cppm.md#fn-d1a904f28f9e59a5)
- [`xv::net::Socket::capability`](socket.cppm.md#fn-e7ffb7211c283e3e)
- [`xv::net::Socket::connect`](socket.cppm.md#fn-38fb7c9245bff3a5)
- [`xv::net::Socket::error`](socket.cppm.md#fn-8344cc63a2be1d42)
- [`xv::net::Socket::listen`](socket.cppm.md#fn-be62ec4113c62554)
- [`xv::net::Socket::local`](socket.cppm.md#fn-75b6aefb0ac151b9)
- [`xv::net::Socket::peer`](socket.cppm.md#fn-a17c97503695a001)
- [`xv::net::Socket::receive_from`](socket.cppm.md#fn-4fa8c343625ba09f)
- [`xv::net::Socket::send_to`](socket.cppm.md#fn-9ba32e2969f70f90)
- [`xv::net::Socket::shutdown`](socket.cppm.md#fn-d7f9eecc8327fa97)
- [`xv::net::Socket::stat`](socket.cppm.md#fn-f3b6acd3d9378a95)
- [`xv::net::Socket::~Socket`](socket.cppm.md#fn-fe19627c0299fb5d)
- [`xv::net::create_socket`](registry.cpp.md#fn-75b80a83f7b1ba17)

<a id="fn-e7ffb7211c283e3e"></a>

## `xv::net::Socket::capability`

```doxygen
@fn xv::io::Capability *xv::net::Socket::capability(xv::io::CapabilityId id) final
@brief 只对 Socket::Id 暴露网络能力子对象，供网络系统调用在统一 IoObject 后端中安全取得 Socket；不再使用 StreamKind 分类。
@details 只对 Socket::Id 暴露网络能力子对象，供网络系统调用在统一 IoObject 后端中安全取得 Socket；不再使用 StreamKind 分类。
```

<a id="fn-f3b6acd3d9378a95"></a>

## `xv::net::Socket::stat`

```doxygen
@fn int xv::net::Socket::stat(xv::io::Metadata &value)
@brief Socket 自己填写 T_SOCKET、nlink=1 的内核 Metadata，File 不再包含 socket 特判。
@details Socket 自己填写 T_SOCKET、nlink=1 的内核 Metadata，File 不再包含 socket 特判。
```

<a id="fn-d1a904f28f9e59a5"></a>

## `xv::net::Socket::bind`

```doxygen
@fn int xv::net::Socket::bind(Endpoint local)
@brief 网络能力接口：绑定本地 Endpoint，端口零可请求自动分配。
@details 绑定本地 Endpoint，端口零可请求自动分配；失败不应发布半绑定状态。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-38fb7c9245bff3a5"></a>

## `xv::net::Socket::connect`

```doxygen
@fn int xv::net::Socket::connect(Endpoint peer, bool nonblocking)
@brief 网络能力接口：设置默认 peer 或建立传输连接。
@details 设置默认 peer 或建立传输连接；nonblocking 要求立即返回，TCP 可返回 -EINPROGRESS。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-be62ec4113c62554"></a>

## `xv::net::Socket::listen`

```doxygen
@fn int xv::net::Socket::listen(int backlog)
@brief 网络能力接口：把支持被动打开的 socket 设为监听并限制 backlog。
@details 把支持被动打开的 socket 设为监听并限制 backlog；不支持的协议返回负错误。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-c4cda7ae200f4be8"></a>

## `xv::net::Socket::accept`

```doxygen
@fn Socket *xv::net::Socket::accept(bool nonblocking, int &error)
@brief 网络能力接口：移交一个已经建立的子 socket 的应用拥有权。
@details 移交一个已经建立的子 socket 的应用拥有权；失败通过 error 返回负值，调用者须把成功对象安装到 File。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-9ba32e2969f70f90"></a>

## `xv::net::Socket::send_to`

```doxygen
@fn int xv::net::Socket::send_to(xv::io::Buffer data, unsigned length, const Endpoint *peer)
@brief 网络能力接口：从用户 Buffer 提交数据，允许协议返回部分长度或负错误。
@details 从用户 Buffer 提交数据，允许协议返回部分长度或负错误；显式 peer 仅适用于数据报。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-4fa8c343625ba09f"></a>

## `xv::net::Socket::receive_from`

```doxygen
@fn int xv::net::Socket::receive_from(xv::io::Buffer data, unsigned length, Endpoint *peer)
@brief 网络能力接口：把一个数据报或流字节复制到用户 Buffer，并可输出来源。
@details 把一个数据报或流字节复制到用户 Buffer，并可输出来源；支持非阻塞和可中断等待。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-d7f9eecc8327fa97"></a>

## `xv::net::Socket::shutdown`

```doxygen
@fn int xv::net::Socket::shutdown(int how)
@brief 网络能力接口：关闭接收或发送方向，保留文件引用及另一方向。
@details 关闭接收或发送方向，保留文件引用及另一方向；how 为 SHUT_RD/WR/RDWR。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-75b6aefb0ac151b9"></a>

## `xv::net::Socket::local`

```doxygen
@fn Endpoint xv::net::Socket::local()
@brief 网络能力接口：复制本地地址和端口。
@details 复制本地地址和端口；未自动绑定时端口可为零。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-a17c97503695a001"></a>

## `xv::net::Socket::peer`

```doxygen
@fn Endpoint xv::net::Socket::peer()
@brief 网络能力接口：复制已连接 peer。
@details 复制已连接 peer；端口零表示尚未连接。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-8344cc63a2be1d42"></a>

## `xv::net::Socket::error`

```doxygen
@fn int xv::net::Socket::error()
@brief 网络能力接口：取得并清除挂起的正 errno，零表示没有挂起错误。。
@details 取得并清除挂起的正 errno，零表示没有挂起错误。
@note 接口通过 Stream/File 拥有关系访问，禁止通过基类删除。
```

<a id="fn-d80018a2029ae04d"></a>

## `xv::net::Socket::Socket`

```doxygen
@fn xv::net::Socket::Socket()
@brief 构造 Socket 的状态与依赖。
@details 只保存借用的层间依赖或初始化空状态；不执行网络 I/O。被引用的服务必须比本对象存活更久。
```

<a id="fn-fe19627c0299fb5d"></a>

## `xv::net::Socket::~Socket`

```doxygen
@fn xv::net::Socket::~Socket()
@brief 结束 Socket 的基类或资源生命周期。
@details protected 非虚析构只销毁基类部分；不能通过接口 delete。对象拥有权按工厂/注册表/File 契约转交。
```
