# `kernel/fs/userfs/userfs.cpp`


<a id="fn-ba6d04a492ccb1b6"></a>

## `xv::fs::UserFileSystem::Endpoint::write`

```doxygen
@fn int xv::fs::UserFileSystem::Endpoint::write(xv::io::Buffer in, int count)
@brief 验证回复大小、版本、操作和关联编号后唤醒调用者。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

- [`xv::fs::UserFileSystem::apply`](userfs.cpp.md#fn-93489bf912e22a0d)

- [`xv::fs::UserFileSystem::attached`](userfs.cpp.md#fn-278a5df4c786740e)

- [`xv::fs::UserFileSystem::attributes`](userfs.cpp.md#fn-15c0006aecdc470c)

- [`xv::fs::UserFileSystem::begin`](userfs.cpp.md#fn-bfbe7e9ea8fde507)

- [`xv::fs::UserFileSystem::busy`](userfs.cpp.md#fn-ad3d62ed5a9767c5)

- [`xv::fs::UserFileSystem::create_at`](userfs.cpp.md#fn-28766acb0b15d964)

- [`xv::fs::UserFileSystem::disconnect`](userfs.cpp.md#fn-6e4001252f9ddf7e)

- [`xv::fs::UserFileSystem::exchange`](userfs.cpp.md#fn-dd8ee631564b20f6)

- [`xv::fs::UserFileSystem::flush`](userfs.cpp.md#fn-d7d7c5d91ec3b811)

- [`xv::fs::UserFileSystem::initialize_inodes`](userfs.cpp.md#fn-688baf0352027e87)

- [`xv::fs::UserFileSystem::link_at`](userfs.cpp.md#fn-9c36f8f999b78510)

- [`xv::fs::UserFileSystem::lookup_entry`](userfs.cpp.md#fn-a92f57ea69fb7339)

- [`xv::fs::UserFileSystem::next_directory`](userfs.cpp.md#fn-8d093708aef4cf92)

- [`xv::fs::UserFileSystem::node`](userfs.cpp.md#fn-224085493352d858)

- [`xv::fs::UserFileSystem::read`](userfs.cpp.md#fn-f99fcdc7246df9ce)

- [`xv::fs::UserFileSystem::release`](userfs.cpp.md#fn-1d31b6319014fba9)

- [`xv::fs::UserFileSystem::rename_at`](userfs.cpp.md#fn-033fa449ab57d9ca)

- [`xv::fs::UserFileSystem::reserve`](userfs.cpp.md#fn-dffdb18d206eca77)

- [`xv::fs::UserFileSystem::resize`](userfs.cpp.md#fn-a5ee913e0eea1df4)

- [`xv::fs::UserFileSystem::retain`](userfs.cpp.md#fn-0c6a3f0fdaf8d048)

- [`xv::fs::UserFileSystem::root`](userfs.cpp.md#fn-160a7f332a01d185)

- [`xv::fs::UserFileSystem::shutdown`](userfs.cpp.md#fn-2434bc0efe603055)

- [`xv::fs::UserFileSystem::stat`](userfs.cpp.md#fn-eaef7f0b70f02e48)

- [`xv::fs::UserFileSystem::status`](userfs.cpp.md#fn-c3f98681291fb21b)

- [`xv::fs::UserFileSystem::synchronize`](userfs.cpp.md#fn-8b3b49707d614cb9)

- [`xv::fs::UserFileSystem::unlink_at`](userfs.cpp.md#fn-e71ece695cd8f1b4)

- [`xv::fs::UserFileSystem::update`](userfs.cpp.md#fn-c463cc29c2d7f849)

- [`xv::fs::UserFileSystem::write`](userfs.cpp.md#fn-b7b495fe053af575)

- [`xv::fs::UserFileSystem::Endpoint::close`](userfs.cpp.md#fn-8d4a060efdc8b80a)

- [`xv::fs::UserFileSystem::Endpoint::poll`](userfs.cpp.md#fn-366d0a3011ed78c9)

- [`xv::fs::UserFileSystem::Endpoint::read`](userfs.cpp.md#fn-a1287b37c372eba1)

- [`xv::fs::UserFileSystem::Endpoint::write`](userfs.cpp.md#fn-ba6d04a492ccb1b6)

<a id="fn-93489bf912e22a0d"></a>

## `xv::fs::UserFileSystem::apply`

```doxygen
@fn void xv::fs::UserFileSystem::apply(Inode *n, const xv::userfs::Attributes &a)
@brief 把服务返回属性写入代理 inode，包括设备号和快照标志。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-278a5df4c786740e"></a>

## `xv::fs::UserFileSystem::attached`

```doxygen
@fn void xv::fs::UserFileSystem::attached(bool value)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-15c0006aecdc470c"></a>

## `xv::fs::UserFileSystem::attributes`

```doxygen
@fn bool xv::fs::UserFileSystem::attributes(Inode *n, unsigned uid, unsigned gid, unsigned mode)
@brief 服务确认 Setattr 后提交本地 uid、gid 和 mode。
@details 服务确认 Setattr 后提交本地 uid、gid 和 mode。
```

<a id="fn-bfbe7e9ea8fde507"></a>

## `xv::fs::UserFileSystem::begin`

```doxygen
@fn void xv::fs::UserFileSystem::begin(Operation operation, unsigned node)
@brief 构造含原调用者身份的通用请求。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-ad3d62ed5a9767c5"></a>

## `xv::fs::UserFileSystem::busy`

```doxygen
@fn bool xv::fs::UserFileSystem::busy()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。busy 先取得元数据锁，等待已减引用但仍在发送 Forget 的最后归还结束，防止卸载后提前回收实例。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-28766acb0b15d964"></a>

## `xv::fs::UserFileSystem::create_at`

```doxygen
@fn Inode *xv::fs::UserFileSystem::create_at(Inode *directory, const char *name, short type, short major, short minor)
@brief 请求服务创建子项并发布对应代理 inode。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-6e4001252f9ddf7e"></a>

## `xv::fs::UserFileSystem::disconnect`

```doxygen
@fn void xv::fs::UserFileSystem::disconnect()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-dd8ee631564b20f6"></a>

## `xv::fs::UserFileSystem::exchange`

```doxygen
@fn bool xv::fs::UserFileSystem::exchange()
@brief 发布一条用户态 FS 请求，在 5 秒内等待对应回复，失败时停用连接。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-d7d7c5d91ec3b811"></a>

## `xv::fs::UserFileSystem::flush`

```doxygen
@fn void xv::fs::UserFileSystem::flush()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-688baf0352027e87"></a>

## `xv::fs::UserFileSystem::initialize_inodes`

```doxygen
@fn void xv::fs::UserFileSystem::initialize_inodes()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-9c36f8f999b78510"></a>

## `xv::fs::UserFileSystem::link_at`

```doxygen
@fn int xv::fs::UserFileSystem::link_at(Inode *directory, const char *name, Inode *target)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-a92f57ea69fb7339"></a>

## `xv::fs::UserFileSystem::lookup_entry`

```doxygen
@fn Inode *xv::fs::UserFileSystem::lookup_entry(Inode *directory, const char *name, unsigned *offset)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-8d093708aef4cf92"></a>

## `xv::fs::UserFileSystem::next_directory`

```doxygen
@fn int xv::fs::UserFileSystem::next_directory(Inode *n, unsigned long &cookie, DirectoryRecord &entry)
@brief 请求服务枚举目录项并校验名称范围。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-224085493352d858"></a>

## `xv::fs::UserFileSystem::node`

```doxygen
@fn Inode *xv::fs::UserFileSystem::node(const xv::userfs::Attributes &a)
@brief 查找或借用返回属性对应的稳定地址代理 inode。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-f99fcdc7246df9ce"></a>

## `xv::fs::UserFileSystem::read`

```doxygen
@fn int xv::fs::UserFileSystem::read(Inode *n, int user, unsigned long out, unsigned offset, unsigned count)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-1d31b6319014fba9"></a>

## `xv::fs::UserFileSystem::release`

```doxygen
@fn void xv::fs::UserFileSystem::release(Inode *n)
@brief 最后一次归还通知服务 Forget，在零引用后回收对象与本地缓存身份。
@details 最后一次归还通知服务 Forget，在零引用后回收对象与本地缓存身份。
```

<a id="fn-033fa449ab57d9ca"></a>

## `xv::fs::UserFileSystem::rename_at`

```doxygen
@fn int xv::fs::UserFileSystem::rename_at(Inode *from, const char *old, Inode *to, const char *next)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-dffdb18d206eca77"></a>

## `xv::fs::UserFileSystem::reserve`

```doxygen
@fn bool xv::fs::UserFileSystem::reserve(unsigned device)
@brief 保留同时未挂载且服务描述符已关闭的实例槽位，建立根 inode。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-a5ee913e0eea1df4"></a>

## `xv::fs::UserFileSystem::resize`

```doxygen
@fn int xv::fs::UserFileSystem::resize(Inode *n, unsigned size)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-0c6a3f0fdaf8d048"></a>

## `xv::fs::UserFileSystem::retain`

```doxygen
@fn Inode *xv::fs::UserFileSystem::retain(Inode *n)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-160a7f332a01d185"></a>

## `xv::fs::UserFileSystem::root`

```doxygen
@fn Inode *xv::fs::UserFileSystem::root()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-2434bc0efe603055"></a>

## `xv::fs::UserFileSystem::shutdown`

```doxygen
@fn void xv::fs::UserFileSystem::shutdown()
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-eaef7f0b70f02e48"></a>

## `xv::fs::UserFileSystem::stat`

```doxygen
@fn void xv::fs::UserFileSystem::stat(Inode *n, Stat *out)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-c3f98681291fb21b"></a>

## `xv::fs::UserFileSystem::status`

```doxygen
@fn int xv::fs::UserFileSystem::status(Inode *n, Stat *out)
@brief 从服务刷新属性并返回可失败的 stat 结果。
@details 从服务刷新属性并返回可失败的 stat 结果。
```

<a id="fn-8b3b49707d614cb9"></a>

## `xv::fs::UserFileSystem::synchronize`

```doxygen
@fn int xv::fs::UserFileSystem::synchronize(Inode *n)
@brief 等待服务确认 Sync，传播断开或持久化失败。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-e71ece695cd8f1b4"></a>

## `xv::fs::UserFileSystem::unlink_at`

```doxygen
@fn int xv::fs::UserFileSystem::unlink_at(Inode *directory, const char *name)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-c463cc29c2d7f849"></a>

## `xv::fs::UserFileSystem::update`

```doxygen
@fn void xv::fs::UserFileSystem::update(Inode *n)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
```

<a id="fn-b7b495fe053af575"></a>

## `xv::fs::UserFileSystem::write`

```doxygen
@fn int xv::fs::UserFileSystem::write(Inode *n, int user, unsigned long in, unsigned offset, unsigned count)
@brief 按既有错误段提交语义拆分写 RPC 并更新代理属性。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

<a id="fn-8d4a060efdc8b80a"></a>

## `xv::fs::UserFileSystem::Endpoint::close`

```doxygen
@fn void xv::fs::UserFileSystem::Endpoint::close()
@brief 最后一个服务描述符关闭时断开连接；未挂载实例才能释放槽位。
@details 最后一个服务描述符关闭时断开连接；未挂载实例才能释放槽位。
```

<a id="fn-366d0a3011ed78c9"></a>

## `xv::fs::UserFileSystem::Endpoint::poll`

```doxygen
@fn unsigned xv::fs::UserFileSystem::Endpoint::poll(xv::sync::PollTable *table)
@brief 元数据由 tree_ 睡眠锁保护；serial_ 保证每实例一条在途消息，state_ 保护发布、交付和回复状态。等待回复前释放通道自旋锁，先订阅再睡眠且设 5 秒截止时间。关闭、被杀、中断或超时返回错误；中断/超时停用连接，防止迟到回复串入后续操作。根和活跃 inode 引用阻止槽位复用，忙卸载失败。详细协议、原有 namespace 锁限制和示例见 docs/user-filesystems.md。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-a1287b37c372eba1"></a>

## `xv::fs::UserFileSystem::Endpoint::read`

```doxygen
@fn int xv::fs::UserFileSystem::Endpoint::read(xv::io::Buffer out, int count)
@brief 向服务交付一整条待处理请求，支持非阻塞及错误指针重试。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-034c6e42cc937bd2"></a>

## `xv::fs::UserFileSystem::claim_mount`

```doxygen
@fn bool xv::fs::UserFileSystem::claim_mount()
@brief 原子选择一个在线、待挂载且未被其他挂载占用的实例。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-25daa08a236d724e"></a>

## `xv::fs::UserFileSystem::create`

```doxygen
@fn UserFileSystem *xv::fs::UserFileSystem::create(unsigned device, const xv::userfs::Registration &registration, bool anonymous)
@brief 按注册配置分配独立服务实例及固定地址 inode 存储。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
@note 驱动命令契约：只读实例仍剔除标准修改操作，但保留驱动明确声明的 Control 位；每次命令通过内核设置的 ReadOnly 上下文交由驱动按命令语义限制。旧注册默认 StandardOperations 不包含 Control。
```

<a id="fn-02e33bbbb44bae7d"></a>

## `xv::fs::UserFileSystem::describe_provider`

```doxygen
@fn bool xv::fs::UserFileSystem::describe_provider(xv::userfs::ProviderInfo &info)
@brief 在实例状态锁下复制名称、PID、协议、只读标志和状态。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-3bc854bf55d0af33"></a>

## `xv::fs::UserFileSystem::destroy`

```doxygen
@fn void xv::fs::UserFileSystem::destroy()
@brief 在注册表已摘除且无外部使用时释放实例与 inode 页。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

<a id="fn-355e01b2c468377a"></a>

## `xv::fs::UserFileSystem::retired`

```doxygen
@fn bool xv::fs::UserFileSystem::retired()
@brief 检查服务描述符、挂载和挂载选择是否都已撤销。
@details 消息、inode 及引用协议保留；初始根和名称来自注册配置，读写分段按协商载荷限制，不支持能力在发布前失败。注册端点在挂载前可以阻塞接收；发布窗口的 mounting 状态也允许请求，避免挂载可见后服务误收 EOF。实例和固定地址 inode 页由页分配器创建/销毁，只有端点关闭且无挂载或选择占用时可回收；只读注册限制可发送修改操作。详细锁序、5 秒失败及聚合 Forget 见 docs/user-filesystems.md。
```

- [`xv::fs::UserFileSystem::claim_mount`](userfs.cpp.md#fn-034c6e42cc937bd2)

- [`xv::fs::UserFileSystem::create`](userfs.cpp.md#fn-25daa08a236d724e)

- [`xv::fs::UserFileSystem::describe_provider`](userfs.cpp.md#fn-02e33bbbb44bae7d)

- [`xv::fs::UserFileSystem::destroy`](userfs.cpp.md#fn-3bc854bf55d0af33)

- [`xv::fs::UserFileSystem::retired`](userfs.cpp.md#fn-355e01b2c468377a)

<a id="fn-3370dfa7623315f5"></a>

## `xv::fs::UserFileSystem::snapshot`

```doxygen
@fn bool xv::fs::UserFileSystem::snapshot(Inode *node, PageBuffer &buffer)
@brief 向服务请求该次打开的不可变只读字节快照。
@details 通用 Version=3 RPC 代理保留 5 秒超时、按实例串行与聚合 Forget。请求携带原调用者 PID/有效 UID/GID；读取快照、目录条目和设备号由服务返回。普通写入保留既有 1 KiB 分段的错误前缀语义，消息分段不改变出错段的长度/偏移提交边界。内核只复制、校验与固定 inode，不实现后端格式算法。
```

- [`xv::fs::UserFileSystem::snapshot`](userfs.cpp.md#fn-3370dfa7623315f5)

<a id="fn-2aad75e7a347d7f4"></a>

```doxygen
@fn int UserFileSystem::xv::fs::UserFileSystem::control(Inode *node, unsigned long address, unsigned access)
@brief 显式 Control 能力才受理。
@details 显式 Control 能力才受理。分配临时页复制并验证版本、NUL 命名空间、输入及输出长度；在已有 inode 锁下串行发 RPC，附上真实调用者和描述符权限/只读状态，并将输出容量限制到协商载荷。验证回复长度、对象身份和属性范围，更新共享 inode 后清零尾部并复制输出，所有路径归还临时页。驱动自身授权原生命令；命令失败不承诺回滚已发生的副作用。
```
