# `kernel/fs/vfs/mount_flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::VirtualFileSystem::MountDiskFlow::publish`](mount_flow.cppm.md#fn-9af6e77e97004965)
- [`xv::fs::VirtualFileSystem::MountDiskFlow::ready`](mount_flow.cppm.md#fn-b9afb51c38dc23a8)
- [`xv::fs::VirtualFileSystem::MountDiskFlow::unique`](mount_flow.cppm.md#fn-da1f5012a12be1b6)
- [`xv::fs::VirtualFileSystem::MountFlow::create`](mount_flow.cppm.md#fn-29f279814feb5ffd)
- [`xv::fs::VirtualFileSystem::MountFlow::find_parent`](mount_flow.cppm.md#fn-c30b575b9c3c22c2)
- [`xv::fs::VirtualFileSystem::MountFlow::initialize`](mount_flow.cppm.md#fn-320a2c146fd111a2)
- [`xv::fs::VirtualFileSystem::MountFlow::lookup`](mount_flow.cppm.md#fn-66eeede823b28ace)
- [`xv::fs::VirtualFileSystem::MountFlow::publish`](mount_flow.cppm.md#fn-f2d4acf85fa2535b)
- [`xv::fs::VirtualFileSystem::MountProtocol::capacity`](mount_flow.cppm.md#fn-f8977203717dc065)
- [`xv::fs::VirtualFileSystem::MountProtocol::covered`](mount_flow.cppm.md#fn-1ab2eb52e62deac5)
- [`xv::fs::VirtualFileSystem::MountProtocol::inspect`](mount_flow.cppm.md#fn-68fdd31d27bcec83)
- [`xv::fs::VirtualFileSystem::MountProtocol::parent`](mount_flow.cppm.md#fn-098112b34c3b74b5)
- [`xv::fs::VirtualFileSystem::MountProtocol::release_covered`](mount_flow.cppm.md#fn-2d0eb9c0c649332c)
- [`xv::fs::VirtualFileSystem::MountProtocol::release_parent`](mount_flow.cppm.md#fn-e1db241756c9d262)
- [`xv::fs::VirtualFileSystem::MountProtocol::valid`](mount_flow.cppm.md#fn-7040f34cad53720e)
- [`xv::fs::VirtualFileSystem::MountProtocol::walk_covered`](mount_flow.cppm.md#fn-cbb2e097655825aa)
- [`xv::fs::VirtualFileSystem::MountProtocol::walk_parent`](mount_flow.cppm.md#fn-3ad35f70c593a99d)
- [`xv::fs::VirtualFileSystem::MountRamFlow::attributes`](mount_flow.cppm.md#fn-923e582d25f9bddc)
- [`xv::fs::VirtualFileSystem::MountRamFlow::available`](mount_flow.cppm.md#fn-9c4d93c970a6fa36)
- [`xv::fs::VirtualFileSystem::MountRamFlow::canonical`](mount_flow.cppm.md#fn-daa507360e16ea14)
- [`xv::fs::VirtualFileSystem::MountRamFlow::initialize`](mount_flow.cppm.md#fn-0e26156bbc543a0e)
- [`xv::fs::VirtualFileSystem::MountRamFlow::publish`](mount_flow.cppm.md#fn-fa89df8100a07d33)
- [`xv::fs::VirtualFileSystem::MountRamFlow::select_slot`](mount_flow.cppm.md#fn-537ba0aea8a35665)
- [`xv::fs::VirtualFileSystem::UnmountFlow::busy`](mount_flow.cppm.md#fn-5390718bd2aab328)
- [`xv::fs::VirtualFileSystem::UnmountFlow::disk`](mount_flow.cppm.md#fn-8bbde687f04887ca)
- [`xv::fs::VirtualFileSystem::UnmountFlow::flush`](mount_flow.cppm.md#fn-f3267ea30ef80bb6)
- [`xv::fs::VirtualFileSystem::UnmountFlow::found`](mount_flow.cppm.md#fn-f905cb3592b619b7)
- [`xv::fs::VirtualFileSystem::UnmountFlow::lookup`](mount_flow.cppm.md#fn-951c139144c1136a)
- [`xv::fs::VirtualFileSystem::UnmountFlow::release_pins`](mount_flow.cppm.md#fn-fff72cb805d06198)
- [`xv::fs::VirtualFileSystem::UnmountFlow::removable`](mount_flow.cppm.md#fn-c1b016e162b2deac)
- [`xv::fs::VirtualFileSystem::UnmountFlow::remove`](mount_flow.cppm.md#fn-a8caa11ebfcdc8e3)
- [`xv::fs::VirtualFileSystem::UnmountFlow::select`](mount_flow.cppm.md#fn-fed0dd6cf4f468ae)
- [`xv::fs::VirtualFileSystem::UnmountFlow::shutdown`](mount_flow.cppm.md#fn-f82c2b63b27b86b0)
- [`xv::fs::VirtualFileSystem::UnmountFlow::snapshot`](mount_flow.cppm.md#fn-e186658154c0ff91)
- [`xv::fs::VirtualFileSystem::UnmountFlow::success`](mount_flow.cppm.md#fn-64b6324b5ef10a54)

<a id="fn-9af6e77e97004965"></a>

## `xv::fs::VirtualFileSystem::MountDiskFlow::publish`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountDiskFlow::publish(Context &c) noexcept
@brief 取得存储根引用并发布动态标识为负二的挂载记录。
@details 存储就绪和基础路径检查发生在入口 Transaction 与 namespace 锁前，图中的所有失败都保留原挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b9afb51c38dc23a8"></a>

## `xv::fs::VirtualFileSystem::MountDiskFlow::ready`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountDiskFlow::ready(VirtualFileSystem &owner, const char *path) noexcept
@brief 检查存储后端已就绪且路径为非根绝对路径。
@details 存储就绪和基础路径检查发生在入口 Transaction 与 namespace 锁前，图中的所有失败都保留原挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-da1f5012a12be1b6"></a>

## `xv::fs::VirtualFileSystem::MountDiskFlow::unique`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountDiskFlow::unique(Context &c) noexcept
@brief 检查当前挂载表尚未包含该存储后端。
@details 存储就绪和基础路径检查发生在入口 Transaction 与 namespace 锁前，图中的所有失败都保留原挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-29f279814feb5ffd"></a>

## `xv::fs::VirtualFileSystem::MountFlow::create`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountFlow::create(Context &c) noexcept
@brief 尝试创建缺失的挂载目录，成功后释放其 inode 锁。
@details 用于启动挂载，按 lookup、必要时创建目录、检查父目录、初始化后端、发布记录的顺序执行；公共失败节点只归还已取得的被覆盖节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c30b575b9c3c22c2"></a>

## `xv::fs::VirtualFileSystem::MountFlow::find_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountFlow::find_parent(Context &c) noexcept
@brief 通过公开 lookup_parent 取得父目录引用。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-320a2c146fd111a2"></a>

## `xv::fs::VirtualFileSystem::MountFlow::initialize`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountFlow::initialize(Context &c) noexcept
@brief 初始化待挂载后端。
@details 用于启动挂载，按 lookup、必要时创建目录、检查父目录、初始化后端、发布记录的顺序执行；公共失败节点只归还已取得的被覆盖节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-66eeede823b28ace"></a>

## `xv::fs::VirtualFileSystem::MountFlow::lookup`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountFlow::lookup(Context &c) noexcept
@brief 通过公开 lookup 查找挂载点。
@details 用于启动挂载，按 lookup、必要时创建目录、检查父目录、初始化后端、发布记录的顺序执行；公共失败节点只归还已取得的被覆盖节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f2d4acf85fa2535b"></a>

## `xv::fs::VirtualFileSystem::MountFlow::publish`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountFlow::publish(Context &c) noexcept
@brief 取得后端根引用并发布挂载记录、路径及成功状态。
@details 用于启动挂载，按 lookup、必要时创建目录、检查父目录、初始化后端、发布记录的顺序执行；公共失败节点只归还已取得的被覆盖节点。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f8977203717dc065"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::capacity`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountProtocol::capacity(Context &c) noexcept
@brief 查询可安全暂存的日志块容量。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-1ab2eb52e62deac5"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::covered`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountProtocol::covered(Context &c) noexcept
@brief 检查是否取得被覆盖节点引用。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-68fdd31d27bcec83"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::inspect`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountProtocol::inspect(Context &c) noexcept
@brief 短暂锁住被覆盖节点检查目录类型，并拒绝已有挂载根。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-098112b34c3b74b5"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::parent`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountProtocol::parent(Context &c) noexcept
@brief 检查是否取得父目录引用。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2d0eb9c0c649332c"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::release_covered`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountProtocol::release_covered(Context &c) noexcept
@brief 归还被覆盖节点引用。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e1db241756c9d262"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::release_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountProtocol::release_parent(Context &c) noexcept
@brief 归还挂载点父目录引用。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-7040f34cad53720e"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::valid`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountProtocol::valid(Context &c) noexcept
@brief 检查本阶段目录或后端初始化校验成功。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cbb2e097655825aa"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::walk_covered`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountProtocol::walk_covered(Context &c) noexcept
@brief 在已有 namespace 锁内解析被覆盖节点。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3ad35f70c593a99d"></a>

## `xv::fs::VirtualFileSystem::MountProtocol::walk_parent`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountProtocol::walk_parent(Context &c) noexcept
@brief 在已有 namespace 锁内解析挂载点父目录。
@details 共用挂载检查和引用回收操作；启动挂载使用公开 lookup，运行期挂载在调用者 namespace 锁内使用 walk，失败不转移引用到挂载表。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-923e582d25f9bddc"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::attributes`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountRamFlow::attributes(Context &c) noexcept
@brief 取得新根引用并设置根所有者、组与模式。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9c4d93c970a6fa36"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::available`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountRamFlow::available(Context &c) noexcept
@brief 检查是否存在可用动态槽位。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-daa507360e16ea14"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::canonical`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::MountRamFlow::canonical(Context &c) noexcept
@brief 按原次序检查后端、绝对路径、容量、长度和无点分量的规范名称。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0e26156bbc543a0e"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::initialize`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountRamFlow::initialize(Context &c) noexcept
@brief 以递增设备号尝试初始化选定 RAM 后端并保存结果。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fa89df8100a07d33"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::publish`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountRamFlow::publish(Context &c) noexcept
@brief 发布挂载记录并标记动态槽已使用，记录成功码。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-537ba0aea8a35665"></a>

## `xv::fs::VirtualFileSystem::MountRamFlow::select_slot`

```doxygen
@fn static void xv::fs::VirtualFileSystem::MountRamFlow::select_slot(Context &c) noexcept
@brief 直接扫描四个动态后端槽选首个空闲项。
@details Transaction 与 namespace 锁由入口持有；后端初始化失败仍消耗原设备号，并按被覆盖节点、父目录次序归还引用，不占用动态槽位。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5390718bd2aab328"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::busy`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnmountFlow::busy(Context &c) noexcept
@brief 检查打开引用或子挂载引用是否阻止卸载。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8bbde687f04887ca"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::disk`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnmountFlow::disk(Context &c) noexcept
@brief 检查记录是否属于持久存储后端。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f3267ea30ef80bb6"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::flush`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::flush(Context &c) noexcept
@brief 刷新持久后端并归还其挂载根引用。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f905cb3592b619b7"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::found`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnmountFlow::found(Context &c) noexcept
@brief 检查路径是否存在。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-951c139144c1136a"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::lookup`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::lookup(Context &c) noexcept
@brief 解析待卸载路径并取得节点引用。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fff72cb805d06198"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::release_pins`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::release_pins(Context &c) noexcept
@brief 依次归还被覆盖节点和父目录引用。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c1b016e162b2deac"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::removable`

```doxygen
@fn static bool xv::fs::VirtualFileSystem::UnmountFlow::removable(Context &c) noexcept
@brief 检查匹配记录存在且不是固定启动挂载。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a8caa11ebfcdc8e3"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::remove`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::remove(Context &c) noexcept
@brief 以最后记录填补空洞并清空释放的表槽。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fed0dd6cf4f468ae"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::select`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::select(Context &c) noexcept
@brief 选择匹配的最后挂载记录并归还临时路径引用。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f82c2b63b27b86b0"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::shutdown`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::shutdown(Context &c) noexcept
@brief 关闭动态 RAM 后端并标记动态槽空闲。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e186658154c0ff91"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::snapshot`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::snapshot(Context &c) noexcept
@brief 只保存删除记录后仍需使用的引用、动态槽和后端。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-64b6324b5ef10a54"></a>

## `xv::fs::VirtualFileSystem::UnmountFlow::success`

```doxygen
@fn static void xv::fs::VirtualFileSystem::UnmountFlow::success(Context &c) noexcept
@brief 记录卸载成功码。
@details 入口持 Transaction 与 namespace 锁；检查 busy 前已归还路径解析产生的引用。先删除表记录再归还挂载点引用，磁盘 flush 后归还根，RAM shutdown 后释放动态槽。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
