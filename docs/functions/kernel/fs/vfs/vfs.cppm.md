# `kernel/fs/vfs/vfs.cppm`

## 函数导航

- [`xv::fs::VirtualFileSystem::VirtualFileSystem`](vfs.cppm.md#fn-882ab7feab1cf456)
- [`xv::fs::VirtualFileSystem::allocate`](vfs.cpp.md#fn-b14436f73059aab1)
- [`xv::fs::VirtualFileSystem::allowed`](vfs.cpp.md#fn-abf62fef3e7f21c5)
- [`xv::fs::VirtualFileSystem::ancestor`](vfs.cpp.md#fn-35b6020510aa9401)
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
- [`xv::fs::VirtualFileSystem::lookup_parent`](vfs.cpp.md#fn-8bfb0d7b5ac233f1)
- [`xv::fs::VirtualFileSystem::mount`](vfs.cpp.md#fn-fd77401a8aa09874)
- [`xv::fs::VirtualFileSystem::mount_ram`](vfs.cpp.md#fn-09eb092ae4d24474)
- [`xv::fs::VirtualFileSystem::mounts`](vfs.cpp.md#fn-0747d0fa1f7514e8)
- [`xv::fs::VirtualFileSystem::read`](vfs.cpp.md#fn-a35df3a283d2bfaf)
- [`xv::fs::VirtualFileSystem::release`](vfs.cpp.md#fn-e5559898f89824ab)
- [`xv::fs::VirtualFileSystem::rename`](vfs.cpp.md#fn-3ce7ce542ff58054)
- [`xv::fs::VirtualFileSystem::retain`](vfs.cpp.md#fn-8e0bbe0879ea9c9e)
- [`xv::fs::VirtualFileSystem::root`](vfs.cppm.md#fn-fd34710f4c94891e)
- [`xv::fs::VirtualFileSystem::same`](vfs.cpp.md#fn-7cbd893bc133ef55)
- [`xv::fs::VirtualFileSystem::stat`](vfs.cpp.md#fn-8e48167e5fe78464)
- [`xv::fs::VirtualFileSystem::truncate`](vfs.cpp.md#fn-c409ab8df43d2e55)
- [`xv::fs::VirtualFileSystem::unlink`](vfs.cpp.md#fn-5b042e585b21f4bf)
- [`xv::fs::VirtualFileSystem::unlink_at`](vfs.cpp.md#fn-b4095b4ec764cda2)
- [`xv::fs::VirtualFileSystem::unlock`](vfs.cpp.md#fn-a638e65a0686494e)
- [`xv::fs::VirtualFileSystem::unlock_release`](vfs.cpp.md#fn-ad637485c5f0cda9)
- [`xv::fs::VirtualFileSystem::unmount`](vfs.cpp.md#fn-5396bbf57ffd12ef)
- [`xv::fs::VirtualFileSystem::update`](vfs.cpp.md#fn-39fbbef1274fce08)
- [`xv::fs::VirtualFileSystem::walk`](vfs.cpp.md#fn-81e896ba99fe170a)
- [`xv::fs::VirtualFileSystem::write`](vfs.cpp.md#fn-80c4191a47122858)
- [`xv::fs::filesystem`](../service.cpp.md#fn-33ca000fe0c21f95)

<a id="fn-fd34710f4c94891e"></a>

## `xv::fs::VirtualFileSystem::root`

```doxygen
@fn Inode *xv::fs::VirtualFileSystem::root()
@brief 取得根磁盘节点的未锁定引用。
@details 不读取磁盘内容，支持首个进程创建时取得工作目录；VFS 是路径服务，不是一个节点所属文件系统。
```

<a id="fn-882ab7feab1cf456"></a>

## `xv::fs::VirtualFileSystem::VirtualFileSystem`

```doxygen
@fn constexpr xv::fs::VirtualFileSystem::VirtualFileSystem(FileSystem &disk, FileSystem &devices, FileSystem *processes = nullptr, FileSystem *temporary = nullptr, FileSystem **dynamic = nullptr)
@brief 借用固定后端及可选四槽动态后端池。
@details 构造只保存系统生命周期的后端引用；dynamic 非空时必须指向四个实例指针，启动初始化它们的同步，运行期分配其根。构造不执行磁盘或页面操作。
```
