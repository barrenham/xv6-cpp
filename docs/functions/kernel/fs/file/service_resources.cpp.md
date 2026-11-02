# `kernel/fs/file/service_resources.cpp`


<a id="fn-590f98370bf80022"></a>

## `xv::fs::create_fs_resources`

```doxygen
@fn File *xv::fs::create_fs_resources()
@brief 分配一个独立资源能力页面和 File，安装失败回收已取得资源。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-9ecaa952802bb088"></a>

## `xv::fs::ServiceResources::ServiceResources`

```doxygen
@fn xv::fs::ServiceResources::ServiceResources()
@brief 初始化类型化资源能力的串行查询锁。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-5bd8f6f7d81c7344"></a>

## `xv::fs::ServiceResources::cache_statistics`

```doxygen
@fn static void xv::fs::ServiceResources::cache_statistics(Query &query)
@brief 复制块缓存计数及其时间单位。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-a457dbf9a0c97011"></a>

## `xv::fs::ServiceResources::capability`

```doxygen
@fn xv::io::Capability *xv::fs::ServiceResources::capability(xv::io::CapabilityId id) override
@brief 提供类型化资源的设备控制接口。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-06bfda16986e5a14"></a>

## `xv::fs::ServiceResources::close`

```doxygen
@fn void xv::fs::ServiceResources::close() override
@brief 释放全部 PTY 固定引用及资源对象页面。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-9fa87783090824e1"></a>

## `xv::fs::ServiceResources::cpu`

```doxygen
@fn static void xv::fs::ServiceResources::cpu(Query &query)
@brief 组合指定 hart 的硬件标识与 CPU 记账记录。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-ef9aa748f15cd1d4"></a>

## `xv::fs::ServiceResources::descriptor`

```doxygen
@fn void xv::fs::ServiceResources::descriptor(Query &query)
@brief 从该能力已捕获的 fd 快照读取一项记录。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-d5e651b3091a16c8"></a>

## `xv::fs::ServiceResources::device`

```doxygen
@fn static void xv::fs::ServiceResources::device(Query &query)
@brief 按有效设备索引生成原始设备能力记录。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-e280b39d78ead0ab"></a>

## `xv::fs::ServiceResources::filesystem_type`

```doxygen
@fn static void xv::fs::ServiceResources::filesystem_type(Query &query)
@brief 复制当前注册的文件系统类型记录。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-0ab150001d17b98f"></a>

## `xv::fs::ServiceResources::ioctl`

```doxygen
@fn int xv::fs::ServiceResources::ioctl(unsigned request, xv::io::Buffer address) override
@brief 校验版本并串行执行资源查询、快照或受权能力创建。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-c4759dcd3f43521e"></a>

## `xv::fs::ServiceResources::mount`

```doxygen
@fn static void xv::fs::ServiceResources::mount(Query &query)
@brief 按索引复制一个已发布挂载记录，避免完整表上栈。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-dc2fb1393411c9a7"></a>

## `xv::fs::ServiceResources::owner`

```doxygen
@fn static void xv::fs::ServiceResources::owner(Query &query, unsigned number)
@brief 查询被固定 PTY 的编号及属主。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-9b7837db585601b4"></a>

## `xv::fs::ServiceResources::poll`

```doxygen
@fn unsigned xv::fs::ServiceResources::poll(xv::sync::PollTable *) override
@brief 报告该能力不支持流式 poll。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-471e9a388ea38a73"></a>

## `xv::fs::ServiceResources::process`

```doxygen
@fn static void xv::fs::ServiceResources::process(Query &query)
@brief 复制索引之后的下一个进程原始记录。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-c2b2d55aca55bd9d"></a>

## `xv::fs::ServiceResources::provider`

```doxygen
@fn static void xv::fs::ServiceResources::provider(Query &query)
@brief 复制指定用户态提供者的名称、PID 与生命周期状态。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-320543563991e70d"></a>

## `xv::fs::ServiceResources::pty`

```doxygen
@fn static void xv::fs::ServiceResources::pty(Query &query)
@brief 枚举当前已发布的命名 PTY 并查询属主。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-a7642220ea97e8d5"></a>

## `xv::fs::ServiceResources::read`

```doxygen
@fn int xv::fs::ServiceResources::read(xv::io::Buffer, int) override
@brief 拒绝普通流读取，资源记录通过 ioctl 查询。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-0786768ed7f906d3"></a>

## `xv::fs::ServiceResources::system`

```doxygen
@fn static void xv::fs::ServiceResources::system(Query &query)
@brief 复制内存、时间和进程调度总计原始数据。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-a06a50276e83a557"></a>

## `xv::fs::ServiceResources::write`

```doxygen
@fn int xv::fs::ServiceResources::write(xv::io::Buffer, int) override
@brief 拒绝普通流写入，资源记录通过 ioctl 查询。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

<a id="fn-4e19e7f6ae26b1eb"></a>

## `xv::fs::ServiceResources::reply`

```doxygen
@fn static void xv::fs::ServiceResources::reply(Query &query, const Record &record)
@brief 复制尺寸受限的类型化记录并标记成功结果。
@details 资源描述符通过 Version=2 的 QueryIoctl 提供设备、PTY、进程、CPU、挂载、插件与系统原始记录，不解释文件系统路径或生成 /proc 文本。查询由对象 SleepLock 串行，消息缓冲位于对象已有页面；挂载只复制一个已发布记录。进程私有信息按原请求 inspector UID 检查，fd/exec 捕获状态由能力对象持有；关闭释放所有 PTY 固定引用。Clone 创建独立捕获状态，Storage/Manager 仅允许 root 取得。
```

- [`xv::fs::create_fs_resources`](service_resources.cpp.md#fn-590f98370bf80022)

- [`xv::fs::ServiceResources::ServiceResources`](service_resources.cpp.md#fn-9ecaa952802bb088)

- [`xv::fs::ServiceResources::cache_statistics`](service_resources.cpp.md#fn-5bd8f6f7d81c7344)

- [`xv::fs::ServiceResources::capability`](service_resources.cpp.md#fn-a457dbf9a0c97011)

- [`xv::fs::ServiceResources::close`](service_resources.cpp.md#fn-06bfda16986e5a14)

- [`xv::fs::ServiceResources::cpu`](service_resources.cpp.md#fn-9fa87783090824e1)

- [`xv::fs::ServiceResources::descriptor`](service_resources.cpp.md#fn-ef9aa748f15cd1d4)

- [`xv::fs::ServiceResources::device`](service_resources.cpp.md#fn-d5e651b3091a16c8)

- [`xv::fs::ServiceResources::filesystem_type`](service_resources.cpp.md#fn-e280b39d78ead0ab)

- [`xv::fs::ServiceResources::ioctl`](service_resources.cpp.md#fn-0ab150001d17b98f)

- [`xv::fs::ServiceResources::mount`](service_resources.cpp.md#fn-c4759dcd3f43521e)

- [`xv::fs::ServiceResources::owner`](service_resources.cpp.md#fn-dc2fb1393411c9a7)

- [`xv::fs::ServiceResources::poll`](service_resources.cpp.md#fn-9b7837db585601b4)

- [`xv::fs::ServiceResources::process`](service_resources.cpp.md#fn-471e9a388ea38a73)

- [`xv::fs::ServiceResources::provider`](service_resources.cpp.md#fn-c2b2d55aca55bd9d)

- [`xv::fs::ServiceResources::pty`](service_resources.cpp.md#fn-320543563991e70d)

- [`xv::fs::ServiceResources::read`](service_resources.cpp.md#fn-a7642220ea97e8d5)

- [`xv::fs::ServiceResources::system`](service_resources.cpp.md#fn-0786768ed7f906d3)

- [`xv::fs::ServiceResources::write`](service_resources.cpp.md#fn-a06a50276e83a557)

- [`xv::fs::ServiceResources::reply`](service_resources.cpp.md#fn-4e19e7f6ae26b1eb)
