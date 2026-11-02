# `kernel/fs/file/device_registry.cpp`


<a id="fn-52bfca3b8cd3eb39"></a>

## `xv::fs::initialize_device_registry`

```doxygen
@fn void xv::fs::initialize_device_registry()
@brief 注册内核设备端点及设备号，不创建文件系统目录。
@details 仅注册内核设备机制与设备号，不生成 devfs 目录。内存设备和普通设备 I/O 留在内核；用户态 DeviceBackend 根据版本化资源记录生成名称和属性，再由通用 VFS 代理绑定实际端点。
```

- [`xv::fs::initialize_device_registry`](device_registry.cpp.md#fn-52bfca3b8cd3eb39)

- [`xv::fs::MemoryDevice::MemoryDevice`](../devfs/devfs.cpp.md#fn-d216c823111a921f)

- [`xv::fs::MemoryDevice::initialize`](../devfs/devfs.cpp.md#fn-7ea96491bdf7a542)

- [`xv::fs::MemoryDevice::name`](../devfs/devfs.cpp.md#fn-807dbcfa2dee03aa)

- [`xv::fs::MemoryDevice::poll`](../devfs/devfs.cpp.md#fn-1ee610cc543a4533)

- [`xv::fs::MemoryDevice::put_sync`](../devfs/devfs.cpp.md#fn-2b92efdd15790e32)

- [`xv::fs::MemoryDevice::read`](../devfs/devfs.cpp.md#fn-78dcb885ab7432e2)

- [`xv::fs::MemoryDevice::write`](../devfs/devfs.cpp.md#fn-eecfc6d4b67d727b)
