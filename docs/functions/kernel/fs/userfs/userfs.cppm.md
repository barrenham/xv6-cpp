# `kernel/fs/userfs/userfs.cppm`


<a id="fn-2206602d15d9b01c"></a>

## `xv::fs::UserFileSystem::Endpoint::Endpoint`

```doxygen
@fn constexpr explicit xv::fs::UserFileSystem::Endpoint::Endpoint(UserFileSystem &owner)
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

- [`xv::fs::UserFileSystem::UserFileSystem`](userfs.cppm.md#fn-83424965381a6939)

- [`xv::fs::UserFileSystem::allocate`](userfs.cppm.md#fn-382577c47fa9df33)

- [`xv::fs::UserFileSystem::apply`](userfs.cpp.md#fn-93489bf912e22a0d)

- [`xv::fs::UserFileSystem::attached`](userfs.cpp.md#fn-278a5df4c786740e)

- [`xv::fs::UserFileSystem::attributes`](userfs.cpp.md#fn-15c0006aecdc470c)

- [`xv::fs::UserFileSystem::begin`](userfs.cpp.md#fn-bfbe7e9ea8fde507)

- [`xv::fs::UserFileSystem::busy`](userfs.cpp.md#fn-ad3d62ed5a9767c5)

- [`xv::fs::UserFileSystem::create_at`](userfs.cpp.md#fn-28766acb0b15d964)

- [`xv::fs::UserFileSystem::disconnect`](userfs.cpp.md#fn-6e4001252f9ddf7e)

- [`xv::fs::UserFileSystem::endpoint`](userfs.cppm.md#fn-b9b39306aaf195d4)

- [`xv::fs::UserFileSystem::exchange`](userfs.cpp.md#fn-dd8ee631564b20f6)

- [`xv::fs::UserFileSystem::flush`](userfs.cpp.md#fn-d7d7c5d91ec3b811)

- [`xv::fs::UserFileSystem::initialize`](userfs.cppm.md#fn-36945e0ff30e1479)

- [`xv::fs::UserFileSystem::initialize_inodes`](userfs.cpp.md#fn-688baf0352027e87)

- [`xv::fs::UserFileSystem::link_at`](userfs.cpp.md#fn-9c36f8f999b78510)

- [`xv::fs::UserFileSystem::lock`](userfs.cppm.md#fn-6323796aaead36a8)

- [`xv::fs::UserFileSystem::lookup_entry`](userfs.cpp.md#fn-a92f57ea69fb7339)

- [`xv::fs::UserFileSystem::name`](userfs.cppm.md#fn-15f8979622e46f8c)

- [`xv::fs::UserFileSystem::name_max`](userfs.cppm.md#fn-d0abaa4428238cbb)

- [`xv::fs::UserFileSystem::next_directory`](userfs.cpp.md#fn-8d093708aef4cf92)

- [`xv::fs::UserFileSystem::node`](userfs.cpp.md#fn-224085493352d858)

- [`xv::fs::UserFileSystem::persistent`](userfs.cppm.md#fn-4db40388f721323d)

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

- [`xv::fs::UserFileSystem::truncate`](userfs.cppm.md#fn-599de649a4159d06)

- [`xv::fs::UserFileSystem::unlink_at`](userfs.cpp.md#fn-e71ece695cd8f1b4)

- [`xv::fs::UserFileSystem::unlock`](userfs.cppm.md#fn-7cec1386b3762632)

- [`xv::fs::UserFileSystem::unlock_release`](userfs.cppm.md#fn-8e648e63b7f3baac)

- [`xv::fs::UserFileSystem::update`](userfs.cpp.md#fn-c463cc29c2d7f849)

- [`xv::fs::UserFileSystem::write`](userfs.cpp.md#fn-b7b495fe053af575)

- [`xv::fs::UserFileSystem::Endpoint::Endpoint`](userfs.cppm.md#fn-2206602d15d9b01c)

- [`xv::fs::UserFileSystem::Endpoint::close`](userfs.cpp.md#fn-8d4a060efdc8b80a)

- [`xv::fs::UserFileSystem::Endpoint::poll`](userfs.cpp.md#fn-366d0a3011ed78c9)

- [`xv::fs::UserFileSystem::Endpoint::read`](userfs.cpp.md#fn-a1287b37c372eba1)

- [`xv::fs::UserFileSystem::Endpoint::write`](userfs.cpp.md#fn-ba6d04a492ccb1b6)

<a id="fn-83424965381a6939"></a>

## `xv::fs::UserFileSystem::UserFileSystem`

```doxygen
@fn constexpr xv::fs::UserFileSystem::UserFileSystem() = default
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-382577c47fa9df33"></a>

## `xv::fs::UserFileSystem::allocate`

```doxygen
@fn Inode *xv::fs::UserFileSystem::allocate(unsigned, short) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-b9b39306aaf195d4"></a>

## `xv::fs::UserFileSystem::endpoint`

```doxygen
@fn xv::io::IoObject &xv::fs::UserFileSystem::endpoint()
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-36945e0ff30e1479"></a>

## `xv::fs::UserFileSystem::initialize`

```doxygen
@fn void xv::fs::UserFileSystem::initialize(int) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-6323796aaead36a8"></a>

## `xv::fs::UserFileSystem::lock`

```doxygen
@fn void xv::fs::UserFileSystem::lock(Inode *) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-15f8979622e46f8c"></a>

## `xv::fs::UserFileSystem::name`

```doxygen
@fn const char *xv::fs::UserFileSystem::name() const override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用 VFS 代理从注册描述符获取类型名、根和只读策略，不包含存储算法。实例本体及 inode 页按需分配；每实例 64 个 inode 地址在引用期间固定。mounted/mounting/endpoint_open/completed 状态保护挂载与描述符分别固定的生命周期。旧 fsserve 使用匿名兼容实例；具名插件经注册表接入。
```

<a id="fn-d0abaa4428238cbb"></a>

## `xv::fs::UserFileSystem::name_max`

```doxygen
@fn unsigned xv::fs::UserFileSystem::name_max() const override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-4db40388f721323d"></a>

## `xv::fs::UserFileSystem::persistent`

```doxygen
@fn bool xv::fs::UserFileSystem::persistent() const override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-599de649a4159d06"></a>

## `xv::fs::UserFileSystem::truncate`

```doxygen
@fn void xv::fs::UserFileSystem::truncate(Inode *node) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-7cec1386b3762632"></a>

## `xv::fs::UserFileSystem::unlock`

```doxygen
@fn void xv::fs::UserFileSystem::unlock(Inode *) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-8e648e63b7f3baac"></a>

## `xv::fs::UserFileSystem::unlock_release`

```doxygen
@fn void xv::fs::UserFileSystem::unlock_release(Inode *node) override
@brief 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
@details 通用用户态文件系统代理。每实例有独立服务端点、串行请求锁、元数据睡眠锁、通道与引用自旋锁；64 个 inode 缓存槽固定地址，引用期间禁止重用。服务描述符和挂载分别固定实例寿命，二者均撤销后方可复用。
```

<a id="fn-204e2474744eae8c"></a>

## `xv::fs::UserFileSystem::anonymous`

```doxygen
@fn bool xv::fs::UserFileSystem::anonymous() const
@brief 通用 VFS 代理从注册描述符获取类型名、根和只读策略，不包含存储算法。实例本体及 inode 页按需分配；每实例 64 个 inode 地址在引用期间固定。mounted/mounting/endpoint_open/completed 状态保护挂载与描述符分别固定的生命周期。旧 fsserve 使用匿名兼容实例；具名插件经注册表接入。
@details 通用 VFS 代理从注册描述符获取类型名、根和只读策略，不包含存储算法。实例本体及 inode 页按需分配；每实例 64 个 inode 地址在引用期间固定。mounted/mounting/endpoint_open/completed 状态保护挂载与描述符分别固定的生命周期。旧 fsserve 使用匿名兼容实例；具名插件经注册表接入。
```

<a id="fn-ae37b877397ebebc"></a>

## `xv::fs::UserFileSystem::read_only`

```doxygen
@fn bool xv::fs::UserFileSystem::read_only() const override
@brief 通用 VFS 代理从注册描述符获取类型名、根和只读策略，不包含存储算法。实例本体及 inode 页按需分配；每实例 64 个 inode 地址在引用期间固定。mounted/mounting/endpoint_open/completed 状态保护挂载与描述符分别固定的生命周期。旧 fsserve 使用匿名兼容实例；具名插件经注册表接入。
@details 通用 VFS 代理从注册描述符获取类型名、根和只读策略，不包含存储算法。实例本体及 inode 页按需分配；每实例 64 个 inode 地址在引用期间固定。mounted/mounting/endpoint_open/completed 状态保护挂载与描述符分别固定的生命周期。旧 fsserve 使用匿名兼容实例；具名插件经注册表接入。
```

- [`xv::fs::UserFileSystem::anonymous`](userfs.cppm.md#fn-204e2474744eae8c)

- [`xv::fs::UserFileSystem::claim_mount`](userfs.cpp.md#fn-034c6e42cc937bd2)

- [`xv::fs::UserFileSystem::create`](userfs.cpp.md#fn-25daa08a236d724e)

- [`xv::fs::UserFileSystem::describe_provider`](userfs.cpp.md#fn-02e33bbbb44bae7d)

- [`xv::fs::UserFileSystem::destroy`](userfs.cpp.md#fn-3bc854bf55d0af33)

- [`xv::fs::UserFileSystem::read_only`](userfs.cppm.md#fn-ae37b877397ebebc)

- [`xv::fs::UserFileSystem::retired`](userfs.cpp.md#fn-355e01b2c468377a)

<a id="fn-42c0a47d71362909"></a>

## `xv::fs::UserFileSystem::open_block_device`

```doxygen
@fn xv::drivers::BlockDevice *xv::fs::UserFileSystem::open_block_device(Inode *node, bool writable) override
@brief 按服务返回的设备号连接内核块设备机制。
@details 通用 VFS 后端声明，属性含 major/minor 和逐次打开只读快照标志。设备节点由桥接查询到的设备号接入内核设备机制；服务端点、挂载和被借用 inode 各自持有实例寿命，实例关闭不假装重启或回退到原生 FS。
```

<a id="fn-db09ffe1a2c6f2c8"></a>

## `xv::fs::UserFileSystem::open_device`

```doxygen
@fn xv::drivers::CharacterDevice *xv::fs::UserFileSystem::open_device(Inode *node, bool nonblocking) override
@brief 按服务返回的设备号连接内核字符设备端点。
@details 通用 VFS 后端声明，属性含 major/minor 和逐次打开只读快照标志。设备节点由桥接查询到的设备号接入内核设备机制；服务端点、挂载和被借用 inode 各自持有实例寿命，实例关闭不假装重启或回退到原生 FS。
```

<a id="fn-c5a9023b677d40df"></a>

## `xv::fs::UserFileSystem::snapshot_reads`

```doxygen
@fn bool xv::fs::UserFileSystem::snapshot_reads() const override
@brief 检查 inode 是否请求按打开捕获只读快照。
@details 通用 VFS 后端声明，属性含 major/minor 和逐次打开只读快照标志。设备节点由桥接查询到的设备号接入内核设备机制；服务端点、挂载和被借用 inode 各自持有实例寿命，实例关闭不假装重启或回退到原生 FS。
```

- [`xv::fs::UserFileSystem::open_block_device`](userfs.cppm.md#fn-42c0a47d71362909)

- [`xv::fs::UserFileSystem::open_device`](userfs.cppm.md#fn-db09ffe1a2c6f2c8)

- [`xv::fs::UserFileSystem::snapshot`](userfs.cpp.md#fn-3370dfa7623315f5)

- [`xv::fs::UserFileSystem::snapshot_reads`](userfs.cppm.md#fn-c5a9023b677d40df)
