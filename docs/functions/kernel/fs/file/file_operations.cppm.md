# `kernel/fs/file/file_operations.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/file/file_operations.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:operations`。

直接模块依赖：`:file`, `:pipe`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::DeviceBackend::DeviceBackend`](file_operations.cppm.md#fn-13c9a137c3cbe09c)
- [`xv::fs::DeviceBackend::capability`](file_operations.cppm.md#fn-bb772dcd8a7f33f5)
- [`xv::fs::DeviceBackend::close`](file_operations.cpp.md#fn-92e70fc857d468f0)
- [`xv::fs::DeviceBackend::ioctl`](file_operations.cpp.md#fn-a21fe337437e1214)
- [`xv::fs::DeviceBackend::poll`](file_operations.cpp.md#fn-efd08a89bd78b547)
- [`xv::fs::DeviceBackend::read`](file_operations.cpp.md#fn-b9b2f2e24e02b04c)
- [`xv::fs::DeviceBackend::stat`](file_operations.cpp.md#fn-9655ed01c03eea58)
- [`xv::fs::DeviceBackend::write`](file_operations.cpp.md#fn-e5321d7ef73dc31e)
- [`xv::fs::EndpointBackend::EndpointBackend`](file_operations.cppm.md#fn-d87b32c670cb71cb)
- [`xv::fs::EndpointBackend::capability`](file_operations.cppm.md#fn-c70651db4d4aa98d)
- [`xv::fs::EndpointBackend::close`](file_operations.cpp.md#fn-34d2dde20b9c4f7c)
- [`xv::fs::EndpointBackend::ioctl`](file_operations.cpp.md#fn-c8010c3e18e186cd)
- [`xv::fs::EndpointBackend::poll`](file_operations.cpp.md#fn-efdd22fee0da96bf)
- [`xv::fs::EndpointBackend::read`](file_operations.cpp.md#fn-19336e2add16010e)
- [`xv::fs::EndpointBackend::stat`](file_operations.cpp.md#fn-73d806a8fe22282c)
- [`xv::fs::EndpointBackend::write`](file_operations.cpp.md#fn-c225090b7714abf6)
- [`xv::fs::InodeBackend::InodeBackend`](file_operations.cppm.md#fn-41bd7f233b77588b)
- [`xv::fs::InodeBackend::close`](file_operations.cpp.md#fn-ccfc01f791da65b4)
- [`xv::fs::InodeBackend::poll`](file_operations.cpp.md#fn-5fada80fee636ddb)
- [`xv::fs::InodeBackend::read`](file_operations.cpp.md#fn-483ee20032be6151)
- [`xv::fs::InodeBackend::stat`](file_operations.cpp.md#fn-f0d5394ca9774a31)
- [`xv::fs::InodeBackend::write`](file_operations.cpp.md#fn-1bb8e957e0af8807)
- [`xv::fs::PipeBackend::PipeBackend`](file_operations.cppm.md#fn-3872f22c437766f2)
- [`xv::fs::PipeBackend::close`](file_operations.cpp.md#fn-d129694fa64d854e)
- [`xv::fs::PipeBackend::poll`](file_operations.cpp.md#fn-862c9c14fb97258a)
- [`xv::fs::PipeBackend::read`](file_operations.cpp.md#fn-b4fd0e70d6fa5ac3)
- [`xv::fs::PipeBackend::write`](file_operations.cpp.md#fn-687505aae3abce7a)

<a id="fn-bb772dcd8a7f33f5"></a>

## `xv::fs::DeviceBackend::capability`

```doxygen
@fn xv::io::Capability *xv::fs::DeviceBackend::capability(xv::io::CapabilityId id)
@brief 仅对 DeviceControl::Id 返回实际能力基类指针；其他能力返回 nullptr，不把设备误认作 socket。
@details 仅对 DeviceControl::Id 返回实际能力基类指针；其他能力返回 nullptr，不把设备误认作 socket。
```

<a id="fn-3872f22c437766f2"></a>

## `xv::fs::PipeBackend::PipeBackend`

```doxygen
@fn xv::fs::PipeBackend::PipeBackend(Pipe &pipe, bool writable)
@brief 构造 PipeBackend。
@details 保存共享 Pipe 指针和本端方向，独占的是这一端的最终关闭责任。
```

<a id="fn-d87b32c670cb71cb"></a>

## `xv::fs::EndpointBackend::EndpointBackend`

```doxygen
@fn xv::fs::EndpointBackend::EndpointBackend(xv::drivers::CharacterDevice &endpoint)
@brief 构造 EndpointBackend。
@details 接收字符端点关闭责任，不额外分配物理页。
```

<a id="fn-c70651db4d4aa98d"></a>

## `xv::fs::EndpointBackend::capability`

```doxygen
@fn xv::io::Capability *xv::fs::EndpointBackend::capability(xv::io::CapabilityId id)
@brief 只暴露 DeviceControl 能力，其他查询返回 nullptr；公共 read/write 不需要能力判断。
@details 只暴露 DeviceControl 能力，其他查询返回 nullptr；公共 read/write 不需要能力判断。
```
<a id="fn-41bd7f233b77588b"></a>

## `xv::fs::InodeBackend::InodeBackend`

```doxygen
@fn explicit xv::fs::InodeBackend::InodeBackend(Inode &node, char *snapshot = nullptr, unsigned length = 0)
@brief 接管 inode 引用及可选的每次打开内容快照。
@details 快照缓冲和 length 已由文件表准备，构造不再分配；offset 从零开始。独立打开各有后端，dup/fork 共享同一后端。
```

<a id="fn-13c9a137c3cbe09c"></a>

## `xv::fs::DeviceBackend::DeviceBackend`

```doxygen
@fn xv::fs::DeviceBackend::DeviceBackend(Inode &node, xv::drivers::CharacterDevice &session)
@brief 接管设备节点引用和一次成功打开的会话。
@details 会话已由文件系统工厂取得，构造不再调用 open；最后 close 调用会话 close 一次，再释放节点引用。
```
