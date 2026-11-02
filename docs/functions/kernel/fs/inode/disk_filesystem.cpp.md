# `kernel/fs/inode/disk_filesystem.cpp`

> 主机参考实现：此文件仅用于主机合同测试，不链接生产 `kernel.elf`。运行时后端见[文件系统服务迁移](../../../../fs-service-migration.md)。

<a id="fn-c1721da8654614a2"></a>

## `xv::fs::DiskFileSystem::initialize`

```doxygen
@fn void DiskFileSystem::xv::fs::DiskFileSystem::initialize(int device)
@brief 实现 xv::fs::DiskFileSystem::initialize 的模块操作，按声明参数读写其封装状态。
@details 内部磁盘格式选择器：读取超级块magic后委托xv6或ext3。返回inode仍归属具体后端，公共VFS无需判断格式。
```

- [`xv::fs::DiskFileSystem::initialize`](disk_filesystem.cpp.md#fn-c1721da8654614a2)
