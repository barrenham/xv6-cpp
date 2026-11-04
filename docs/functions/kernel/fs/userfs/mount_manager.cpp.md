# `kernel/fs/userfs/mount_manager.cpp`


<a id="fn-595c0678c1279512"></a>

## `xv::fs::create_mount_manager`

```doxygen
@fn File *xv::fs::create_mount_manager()
@brief 创建独占管理端点描述符，失败撤销占用。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-ab350520a2c38313"></a>

## `xv::fs::initialize_mount_manager`

```doxygen
@fn void xv::fs::initialize_mount_manager()
@brief 初始化通用挂载管理服务的内核通道状态。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-35d7f25e98afdc8e"></a>

## `xv::fs::managed_mounts_available`

```doxygen
@fn bool xv::fs::managed_mounts_available()
@brief 检查当前调用者能否委托挂载管理器。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-3ea101f5d38e440d"></a>

## `xv::fs::request_managed_mount`

```doxygen
@fn int xv::fs::request_managed_mount(const char *type, const char *path, const char *source, const char *options)
@brief 通过在线管理器请求用户态后端挂载。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
@note ELF 插件改造复核：把类型、绝对路径、可选源和选项转交同一管理通道；不识别文件系统格式。
```

<a id="fn-f0e91b1b29e2c204"></a>

## `xv::fs::MountManager::MountManager`

```doxygen
@fn constexpr xv::fs::MountManager::MountManager()
@brief 绑定静态挂载管理端点。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-f960b25f0540a325"></a>

## `xv::fs::MountManager::available`

```doxygen
@fn bool xv::fs::MountManager::available()
@brief 检查通道在线且调用者不是管理器自身。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-9fdf5c8696cf537e"></a>

## `xv::fs::MountManager::claim`

```doxygen
@fn bool xv::fs::MountManager::claim()
@brief 在前一通道请求完成后独占登记管理器身份。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-15f9e1f86c315842"></a>

## `xv::fs::MountManager::disconnect`

```doxygen
@fn void xv::fs::MountManager::disconnect()
@brief 撤销连接状态并唤醒请求方与服务方。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-1b3d8ed0be376baa"></a>

## `xv::fs::MountManager::endpoint`

```doxygen
@fn xv::io::IoObject &xv::fs::MountManager::endpoint()
@brief 返回由管理器状态固定寿命的 I/O 端点。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-a350863a0cf2174b"></a>

## `xv::fs::MountManager::initialize`

```doxygen
@fn void xv::fs::MountManager::initialize()
@brief 初始化挂载管理状态锁与串行请求锁。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-46e0a25e72d850fb"></a>

## `xv::fs::MountManager::request`

```doxygen
@fn int xv::fs::MountManager::request(const char *type, const char *path, const char *source, const char *options)
@brief 序列化发送挂载请求并有界等待对应响应。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
@note ELF 插件改造复核：验证路径/源/选项长度，串行提交管理请求并记录原始 PID；等待不持有命名空间锁。进度在锁内更新无响应期限，等待重新观察期限；取消/无响应会断开通道拒绝迟到回复。
```

<a id="fn-96d1e052f51471fe"></a>

## `xv::fs::MountManager::Endpoint::Endpoint`

```doxygen
@fn constexpr explicit xv::fs::MountManager::Endpoint::Endpoint(MountManager &owner)
@brief 固定所属管理器引用。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-6da534a7df37f30e"></a>

## `xv::fs::MountManager::Endpoint::close`

```doxygen
@fn void xv::fs::MountManager::Endpoint::close() override
@brief 关闭管理连接并唤醒仍在等待的调用者。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-a03180ac7e3e4e01"></a>

## `xv::fs::MountManager::Endpoint::poll`

```doxygen
@fn unsigned xv::fs::MountManager::Endpoint::poll(xv::sync::PollTable *table) override
@brief 订阅管理器事件并报告请求或响应方向就绪。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-618444b3131076ad"></a>

## `xv::fs::MountManager::Endpoint::read`

```doxygen
@fn int xv::fs::MountManager::Endpoint::read(xv::io::Buffer out, int count) override
@brief 仅向登记管理器交付一个尚未送达的挂载请求。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
```

<a id="fn-33b834fa070554d3"></a>

## `xv::fs::MountManager::Endpoint::write`

```doxygen
@fn int xv::fs::MountManager::Endpoint::write(xv::io::Buffer in, int count)
@brief 核对管理器身份、请求号和版本后提交对应响应。
@details 单个受控服务描述符接收版本化挂载请求，只允许登记的进程读写。请求在不持有 namespace/inode/注册锁时串行发送，采用 30 秒无响应期限，管理器 Progress 回复可续期；非阻塞 read 返回 EAGAIN，poll 暴露消息方向。响应核对版本和请求号；取消或关闭使连接退役并唤醒双方，旧通道完成后才可重新 claim。用户态管理器负责选择、启动及挂载后端，内核不按格式创建服务。
@note ELF 插件改造复核：验证管理进程、版本、请求 id 与原始 pid；Progress 只续期当前已交付请求的 30 秒无响应期限并唤醒等待者，最终回复仅接受成功或失败。
```

- [`xv::fs::create_mount_manager`](mount_manager.cpp.md#fn-595c0678c1279512)

- [`xv::fs::initialize_mount_manager`](mount_manager.cpp.md#fn-ab350520a2c38313)

- [`xv::fs::managed_mounts_available`](mount_manager.cpp.md#fn-35d7f25e98afdc8e)

- [`xv::fs::request_managed_mount`](mount_manager.cpp.md#fn-3ea101f5d38e440d)

- [`xv::fs::MountManager::MountManager`](mount_manager.cpp.md#fn-f0e91b1b29e2c204)

- [`xv::fs::MountManager::available`](mount_manager.cpp.md#fn-f960b25f0540a325)

- [`xv::fs::MountManager::claim`](mount_manager.cpp.md#fn-9fdf5c8696cf537e)

- [`xv::fs::MountManager::disconnect`](mount_manager.cpp.md#fn-15f9e1f86c315842)

- [`xv::fs::MountManager::endpoint`](mount_manager.cpp.md#fn-1b3d8ed0be376baa)

- [`xv::fs::MountManager::initialize`](mount_manager.cpp.md#fn-a350863a0cf2174b)

- [`xv::fs::MountManager::request`](mount_manager.cpp.md#fn-46e0a25e72d850fb)

- [`xv::fs::MountManager::Endpoint::Endpoint`](mount_manager.cpp.md#fn-96d1e052f51471fe)

- [`xv::fs::MountManager::Endpoint::close`](mount_manager.cpp.md#fn-6da534a7df37f30e)

- [`xv::fs::MountManager::Endpoint::poll`](mount_manager.cpp.md#fn-a03180ac7e3e4e01)

- [`xv::fs::MountManager::Endpoint::read`](mount_manager.cpp.md#fn-618444b3131076ad)

- [`xv::fs::MountManager::Endpoint::write`](mount_manager.cpp.md#fn-33b834fa070554d3)
