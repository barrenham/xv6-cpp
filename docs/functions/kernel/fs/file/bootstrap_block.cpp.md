# `kernel/fs/file/bootstrap_block.cpp`


<a id="fn-7fcaabef60aa884d"></a>

## `xv::fs::create_boot_disk`

```doxygen
@fn File *xv::fs::create_boot_disk(unsigned index)
@brief 取得指定原始盘的独占可写租约并创建 File 能力，失败回收中间资源。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-03fe5851a0aa36a6"></a>

## `xv::fs::BootDisk::BootDisk`

```doxygen
@fn xv::fs::BootDisk::BootDisk(BlockCache &cache, xv::drivers::BlockDevice &disk, unsigned index)
@brief 绑定原始盘、块缓存、设备索引并初始化共享描述符锁。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-74f95e97d335d9b1"></a>

## `xv::fs::BootDisk::capability`

```doxygen
@fn xv::io::Capability *xv::fs::BootDisk::capability(xv::io::CapabilityId id) override
@brief 只暴露设备控制能力接口。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-977950b01c2ee6f7"></a>

## `xv::fs::BootDisk::close`

```doxygen
@fn void xv::fs::BootDisk::close() override
@brief 最后引用关闭时归还该盘可写租约并释放能力页面。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-64252aa10de0dbf5"></a>

## `xv::fs::BootDisk::ioctl`

```doxygen
@fn int xv::fs::BootDisk::ioctl(unsigned request, xv::io::Buffer argument) override
@brief 提供容量、扇区、可写属性及实时时间标量查询。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-c1626fe1f1a5e316"></a>

## `xv::fs::BootDisk::poll`

```doxygen
@fn unsigned xv::fs::BootDisk::poll(xv::sync::PollTable *) override
@brief 报告原始块能力可读写。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-a4587513dd156bb8"></a>

## `xv::fs::BootDisk::position`

```doxygen
@fn unsigned long xv::fs::BootDisk::position() const override
@brief 读取描述符当前字节偏移。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-c7221d4105d5fa64"></a>

## `xv::fs::BootDisk::read`

```doxygen
@fn int xv::fs::BootDisk::read(xv::io::Buffer buffer, int count) override
@brief 从共享盘偏移读取原始字节并推进已完成部分。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-8fff40da3256c9fb"></a>

## `xv::fs::BootDisk::seek`

```doxygen
@fn long xv::fs::BootDisk::seek(long delta, int origin) override
@brief 在设备容量内调整共享偏移，拒绝溢出或无效 origin。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-c88738e9db8ec91b"></a>

## `xv::fs::BootDisk::stat`

```doxygen
@fn int xv::fs::BootDisk::stat(xv::io::Metadata &value) override
@brief 返回块设备容量及 root 专用访问属性。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-e31db91bc06d6aea"></a>

## `xv::fs::BootDisk::sync`

```doxygen
@fn int xv::fs::BootDisk::sync() override
@brief 持描述符锁刷新块缓存至设备。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-1b1ac9ba3fd56e3c"></a>

## `xv::fs::BootDisk::transfer`

```doxygen
@fn int xv::fs::BootDisk::transfer(xv::io::Buffer buffer, int count, bool writing)
@brief 按缓存块分段传输，写前完成复制后才发布本段字节。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

<a id="fn-a807e1db905a68a1"></a>

## `xv::fs::BootDisk::write`

```doxygen
@fn int xv::fs::BootDisk::write(xv::io::Buffer buffer, int count) override
@brief 从用户缓冲写入原始块，复制失败保留已完成段的进度。
@details 原始块设备能力，不解释目录或磁盘文件系统。每块盘仅允许一个可写租约，dup/fork 共享偏移和锁，最后一个 File 引用关闭后释放租约。读写经内核块缓存，复制成功后才发布本段修改，sync 刷新设备；容量、扇区与时间查询使用标量 ABI。普通 /dev 块节点仍只读。
```

- [`xv::fs::create_boot_disk`](bootstrap_block.cpp.md#fn-7fcaabef60aa884d)

- [`xv::fs::BootDisk::BootDisk`](bootstrap_block.cpp.md#fn-03fe5851a0aa36a6)

- [`xv::fs::BootDisk::capability`](bootstrap_block.cpp.md#fn-74f95e97d335d9b1)

- [`xv::fs::BootDisk::close`](bootstrap_block.cpp.md#fn-977950b01c2ee6f7)

- [`xv::fs::BootDisk::ioctl`](bootstrap_block.cpp.md#fn-64252aa10de0dbf5)

- [`xv::fs::BootDisk::poll`](bootstrap_block.cpp.md#fn-c1626fe1f1a5e316)

- [`xv::fs::BootDisk::position`](bootstrap_block.cpp.md#fn-a4587513dd156bb8)

- [`xv::fs::BootDisk::read`](bootstrap_block.cpp.md#fn-c7221d4105d5fa64)

- [`xv::fs::BootDisk::seek`](bootstrap_block.cpp.md#fn-8fff40da3256c9fb)

- [`xv::fs::BootDisk::stat`](bootstrap_block.cpp.md#fn-c88738e9db8ec91b)

- [`xv::fs::BootDisk::sync`](bootstrap_block.cpp.md#fn-e31db91bc06d6aea)

- [`xv::fs::BootDisk::transfer`](bootstrap_block.cpp.md#fn-1b1ac9ba3fd56e3c)

- [`xv::fs::BootDisk::write`](bootstrap_block.cpp.md#fn-a807e1db905a68a1)
