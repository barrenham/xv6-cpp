# `kernel/fs/vfs/vfs.cppm`

## 函数导航

- [`xv::fs::VirtualFileSystem::VirtualFileSystem`](vfs.cppm.md#fn-882ab7feab1cf456)
- [`xv::fs::VirtualFileSystem::allocate`](vfs.cpp.md#fn-b14436f73059aab1)
- [`xv::fs::VirtualFileSystem::allowed`](vfs.cpp.md#fn-abf62fef3e7f21c5)
- [`xv::fs::VirtualFileSystem::ancestor`](vfs.cpp.md#fn-35b6020510aa9401)
- [`xv::fs::VirtualFileSystem::attach_storage`](vfs.cppm.md#fn-d4a420a6654ac25b)
- [`xv::fs::VirtualFileSystem::create`](vfs.cpp.md#fn-6591de3da7091bf9)
- [`xv::fs::VirtualFileSystem::create_at`](vfs.cpp.md#fn-6139d98544fd4958)
- [`xv::fs::VirtualFileSystem::enter_mount`](vfs.cpp.md#fn-4860e29021bbf700)
- [`xv::fs::VirtualFileSystem::initialize`](vfs.cpp.md#fn-e6a15b643dd8c735)
- [`xv::fs::VirtualFileSystem::initialize_inodes`](vfs.cpp.md#fn-3975fe035d893dce)
- [`xv::fs::VirtualFileSystem::link`](vfs.cpp.md#fn-365a4e7acb8561db)
- [`xv::fs::VirtualFileSystem::link_at`](vfs.cpp.md#fn-71de116d242b4ba0)
- [`xv::fs::VirtualFileSystem::lock`](vfs.cpp.md#fn-5967b2c0b5f42168)
- [`xv::fs::VirtualFileSystem::lookup`](vfs.cpp.md#fn-f4cf807217fcb7f0)
- [`xv::fs::VirtualFileSystem::lookup_entry`](vfs.cpp.md#fn-5d4339ff52e6c374)
- [`xv::fs::VirtualFileSystem::mount`](vfs.cpp.md#fn-fd77401a8aa09874)
- [`xv::fs::VirtualFileSystem::mount_ram`](vfs.cpp.md#fn-09eb092ae4d24474)
- [`xv::fs::VirtualFileSystem::read`](vfs.cpp.md#fn-a35df3a283d2bfaf)
- [`xv::fs::VirtualFileSystem::release`](vfs.cpp.md#fn-e5559898f89824ab)
- [`xv::fs::VirtualFileSystem::rename`](vfs.cpp.md#fn-3ce7ce542ff58054)
- [`xv::fs::VirtualFileSystem::retain`](vfs.cpp.md#fn-8e0bbe0879ea9c9e)
- [`xv::fs::VirtualFileSystem::root`](vfs.cppm.md#fn-fd34710f4c94891e)
- [`xv::fs::VirtualFileSystem::same`](vfs.cpp.md#fn-7cbd893bc133ef55)
- [`xv::fs::VirtualFileSystem::stat`](vfs.cpp.md#fn-8e48167e5fe78464)
- [`xv::fs::VirtualFileSystem::symlink`](vfs.cpp.md#fn-67eba603f2e485ac)
- [`xv::fs::VirtualFileSystem::truncate`](vfs.cpp.md#fn-c409ab8df43d2e55)
- [`xv::fs::VirtualFileSystem::unlink`](vfs.cpp.md#fn-5b042e585b21f4bf)
- [`xv::fs::VirtualFileSystem::unlink_at`](vfs.cpp.md#fn-b4095b4ec764cda2)
- [`xv::fs::VirtualFileSystem::unlock`](vfs.cpp.md#fn-a638e65a0686494e)
- [`xv::fs::VirtualFileSystem::unlock_release`](vfs.cpp.md#fn-ad637485c5f0cda9)
- [`xv::fs::VirtualFileSystem::unmount`](vfs.cpp.md#fn-5396bbf57ffd12ef)
- [`xv::fs::VirtualFileSystem::update`](vfs.cpp.md#fn-39fbbef1274fce08)
- [`xv::fs::VirtualFileSystem::write`](vfs.cpp.md#fn-80c4191a47122858)
- [`xv::fs::filesystem`](../service.cpp.md#fn-33ca000fe0c21f95)

<a id="fn-fd34710f4c94891e"></a>

## `xv::fs::VirtualFileSystem::root`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::root()
@brief 查询已发布根挂载的根 inode。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

<a id="fn-882ab7feab1cf456"></a>

## `xv::fs::VirtualFileSystem::VirtualFileSystem`

```doxygen
@fn constexpr xv::fs::VirtualFileSystem::VirtualFileSystem(FileSystem &disk, FileSystem &devices, FileSystem *processes = nullptr, FileSystem *temporary = nullptr, FileSystem **dynamic = nullptr, FileSystem *storage = nullptr)
@brief 构造指定原生夹具或服务模式的 VFS 初始状态。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

<a id="fn-d4a420a6654ac25b"></a>

## `xv::fs::VirtualFileSystem::attach_storage`

```doxygen
@fn void xv::fs::VirtualFileSystem::attach_storage(FileSystem &storage)
@brief 在启动时借用可选数据卷实例。
@details 在 initialize_inodes 之前由板级探测后的缓存装配调用，不转移所有权，不做运行期热插拔。
```

- [`xv::fs::VirtualFileSystem::lookup_parent`](vfs.cpp.md#fn-6d0bcfd9ae87ee67)

- [`xv::fs::VirtualFileSystem::mount_disk`](vfs.cpp.md#fn-d081694eae1d34f0)

- [`xv::fs::VirtualFileSystem::mounts`](vfs.cpp.md#fn-5c98bc2f3956ca9e)

- [`xv::fs::VirtualFileSystem::publish_mount_snapshot`](vfs.cpp.md#fn-4b81eaa3029bfdad)

- [`xv::fs::VirtualFileSystem::snapshot_mounts`](vfs.cpp.md#fn-49863f7ed2744744)

- [`xv::fs::VirtualFileSystem::walk`](vfs.cpp.md#fn-f77a31b05fc26086)

- [`xv::fs::VirtualFileSystem::mount_service`](vfs.cpp.md#fn-0ca0821ecfb5ad49)

<a id="fn-5337d9331f5daf83"></a>

## `xv::fs::VirtualFileSystem::VirtualFileSystem`

```doxygen
@fn constexpr xv::fs::VirtualFileSystem::VirtualFileSystem()
@brief 构造指定原生夹具或服务模式的 VFS 初始状态。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

<a id="fn-17bb94e69050343f"></a>

## `xv::fs::VirtualFileSystem::root_backend`

```doxygen
@fn FileSystem &xv::fs::VirtualFileSystem::root_backend() const
@brief 从已发布根挂载取得实际后端。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

<a id="fn-0b9153e89a385d2f"></a>

## `xv::fs::VirtualFileSystem::services`

```doxygen
@fn bool xv::fs::VirtualFileSystem::services() const
@brief 检查 VFS 是否处于用户态服务模式。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

<a id="fn-e57d4e023513aaf0"></a>

## `xv::fs::VirtualFileSystem::use_services`

```doxygen
@fn void xv::fs::VirtualFileSystem::use_services()
@brief 选择无需原生后端的用户态服务装配。
@details 显式 service_mode 构造用于生产装配，根后端从已发布根挂载获取；原生对象参数构造仅支持主机参考夹具。服务模式初始没有根，用户启动程序挂载根后才执行依赖路径的 init。
```

- [`xv::fs::VirtualFileSystem::VirtualFileSystem`](vfs.cppm.md#fn-5337d9331f5daf83)

- [`xv::fs::VirtualFileSystem::root_backend`](vfs.cppm.md#fn-17bb94e69050343f)

- [`xv::fs::VirtualFileSystem::services`](vfs.cppm.md#fn-0b9153e89a385d2f)

- [`xv::fs::VirtualFileSystem::snapshot_mount`](vfs.cpp.md#fn-ba24647a48f9b85c)

- [`xv::fs::VirtualFileSystem::synchronize_services`](vfs.cpp.md#fn-fee003f2c6b32003)

- [`xv::fs::VirtualFileSystem::use_services`](vfs.cppm.md#fn-e57d4e023513aaf0)
