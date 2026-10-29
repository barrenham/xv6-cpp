# `kernel/fs/inode/flow.cppm`

流程与资源约定见 [内核流程迁移](../../../../kernel-flows.md).

## 函数导航

- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::advance`](flow.cppm.md#fn-d9ee76957dc74fb0)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::bitmap`](flow.cppm.md#fn-9b5c02f74ab70bf2)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::exhausted`](flow.cppm.md#fn-9b74690a1147a2dc)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::find`](flow.cppm.md#fn-a5116f5fd61b0f93)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::found`](flow.cppm.md#fn-dbf5512a7fb40fb6)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::release`](flow.cppm.md#fn-b7660da86ed9c213)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::remaining`](flow.cppm.md#fn-4b1882743a62611b)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::reserve`](flow.cppm.md#fn-1f54244c693aebf8)
- [`xv::fs::Xv6FileSystem::AllocateBlockFlow::zero`](flow.cppm.md#fn-85bf15838e939ee9)
- [`xv::fs::Xv6FileSystem::AllocateFlow::advance`](flow.cppm.md#fn-c0b987054eef57d0)
- [`xv::fs::Xv6FileSystem::AllocateFlow::exhausted`](flow.cppm.md#fn-2cea51f298dd0df7)
- [`xv::fs::Xv6FileSystem::AllocateFlow::free`](flow.cppm.md#fn-d51479f854904d64)
- [`xv::fs::Xv6FileSystem::AllocateFlow::get`](flow.cppm.md#fn-c14abd74f27194ec)
- [`xv::fs::Xv6FileSystem::AllocateFlow::read`](flow.cppm.md#fn-aeb40a12b4295d20)
- [`xv::fs::Xv6FileSystem::AllocateFlow::release`](flow.cppm.md#fn-20d2f9d5d13dda20)
- [`xv::fs::Xv6FileSystem::AllocateFlow::remaining`](flow.cppm.md#fn-dacfadef90664c97)
- [`xv::fs::Xv6FileSystem::AllocateFlow::reserve`](flow.cppm.md#fn-b7937d16c2028a68)
- [`xv::fs::Xv6FileSystem::BlockIoProtocol::minimum`](flow.cppm.md#fn-c1bdd37b748a1fe1)
- [`xv::fs::Xv6FileSystem::GetFlow::available`](flow.cppm.md#fn-f61eb58c324e4af3)
- [`xv::fs::Xv6FileSystem::GetFlow::exhausted`](flow.cppm.md#fn-0cf9c6b36e768f3e)
- [`xv::fs::Xv6FileSystem::GetFlow::find`](flow.cppm.md#fn-f49eb4d3ae3dcdb1)
- [`xv::fs::Xv6FileSystem::GetFlow::found`](flow.cppm.md#fn-bea70f09a1403f53)
- [`xv::fs::Xv6FileSystem::GetFlow::initialize`](flow.cppm.md#fn-088731f968a7de51)
- [`xv::fs::Xv6FileSystem::GetFlow::retain`](flow.cppm.md#fn-d8b1921ba0dc0980)
- [`xv::fs::Xv6FileSystem::GetFlow::unlock`](flow.cppm.md#fn-242fcf8f00253ea3)
- [`xv::fs::Xv6FileSystem::InitializeFlow::invalid`](flow.cppm.md#fn-40adfd7d8c8119d0)
- [`xv::fs::Xv6FileSystem::InitializeFlow::journal`](flow.cppm.md#fn-403fa33d73ddc06f)
- [`xv::fs::Xv6FileSystem::InitializeFlow::reclaim`](flow.cppm.md#fn-d04787a584b89709)
- [`xv::fs::Xv6FileSystem::InitializeFlow::superblock`](flow.cppm.md#fn-e13ae393b612a679)
- [`xv::fs::Xv6FileSystem::InitializeFlow::valid`](flow.cppm.md#fn-930cdc72cbaf6f24)
- [`xv::fs::Xv6FileSystem::LockFlow::cached`](flow.cppm.md#fn-60459a9b05de4e3d)
- [`xv::fs::Xv6FileSystem::LockFlow::invalid`](flow.cppm.md#fn-25dbfb53ccd48b84)
- [`xv::fs::Xv6FileSystem::LockFlow::live`](flow.cppm.md#fn-f3aec62d698f60f6)
- [`xv::fs::Xv6FileSystem::LockFlow::load`](flow.cppm.md#fn-db3b0dc18427341b)
- [`xv::fs::Xv6FileSystem::LockFlow::lock`](flow.cppm.md#fn-d512289c6e1279fa)
- [`xv::fs::Xv6FileSystem::LockFlow::missing_type`](flow.cppm.md#fn-b15bbbdf4d27c73f)
- [`xv::fs::Xv6FileSystem::LockFlow::typed`](flow.cppm.md#fn-b596a9e85013e687)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::allocate`](flow.cppm.md#fn-6c0a38a84655daad)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::create`](flow.cppm.md#fn-a788f9dd207edd5e)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::direct`](flow.cppm.md#fn-92eeb8670d175efa)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::indirect`](flow.cppm.md#fn-c97fc8bb9cb91f93)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::indirect_index`](flow.cppm.md#fn-6702fd4c15c42e98)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::invalid`](flow.cppm.md#fn-cb88e5578620209a)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::load`](flow.cppm.md#fn-8096c7816360edc2)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::present`](flow.cppm.md#fn-def30a3e1f23e66e)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::record`](flow.cppm.md#fn-0b5a702d4d78e006)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::release`](flow.cppm.md#fn-6568ef274b77c832)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::select_direct`](flow.cppm.md#fn-f23e51caecfc4557)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::select_root`](flow.cppm.md#fn-49598c3b95a42d3f)
- [`xv::fs::Xv6FileSystem::MapBlockFlow::store`](flow.cppm.md#fn-5a2d3339fdcda75c)
- [`xv::fs::Xv6FileSystem::ReadFlow::allocated`](flow.cppm.md#fn-5305105b5a619a63)
- [`xv::fs::Xv6FileSystem::ReadFlow::clamp`](flow.cppm.md#fn-f2ccfb251af4002f)
- [`xv::fs::Xv6FileSystem::ReadFlow::complete`](flow.cppm.md#fn-69b69d2416702848)
- [`xv::fs::Xv6FileSystem::ReadFlow::copy_data`](flow.cppm.md#fn-09be5a97e6be235a)
- [`xv::fs::Xv6FileSystem::ReadFlow::copy_zero`](flow.cppm.md#fn-c423320b1c72757a)
- [`xv::fs::Xv6FileSystem::ReadFlow::data_failure`](flow.cppm.md#fn-fe10fd5559d65c18)
- [`xv::fs::Xv6FileSystem::ReadFlow::more`](flow.cppm.md#fn-3b07e64b98b34542)
- [`xv::fs::Xv6FileSystem::ReadFlow::prepare`](flow.cppm.md#fn-bb2e8e1dfc8d1e20)
- [`xv::fs::Xv6FileSystem::ReadFlow::progress`](flow.cppm.md#fn-f9d08e8a2b70ff05)
- [`xv::fs::Xv6FileSystem::ReadFlow::read`](flow.cppm.md#fn-62cd5e8df2500cdc)
- [`xv::fs::Xv6FileSystem::ReadFlow::release`](flow.cppm.md#fn-832928e9aee55a40)
- [`xv::fs::Xv6FileSystem::ReadFlow::sparse_failure`](flow.cppm.md#fn-ff5fc7be58dace84)
- [`xv::fs::Xv6FileSystem::ReadFlow::valid`](flow.cppm.md#fn-a9f0cfeb1c84c6e7)
- [`xv::fs::Xv6FileSystem::ReclaimFlow::advance`](flow.cppm.md#fn-d6c598288ec590b2)
- [`xv::fs::Xv6FileSystem::ReclaimFlow::find`](flow.cppm.md#fn-09c4190e4e5cf078)
- [`xv::fs::Xv6FileSystem::ReclaimFlow::found`](flow.cppm.md#fn-08b1a8dcb8737971)
- [`xv::fs::Xv6FileSystem::ReclaimFlow::recover`](flow.cppm.md#fn-8d7b9ca7bdfea57c)
- [`xv::fs::Xv6FileSystem::ReclaimFlow::remaining`](flow.cppm.md#fn-32755ce65e76aaf1)
- [`xv::fs::Xv6FileSystem::ReleaseFlow::drop`](flow.cppm.md#fn-905787f34f734109)
- [`xv::fs::Xv6FileSystem::ReleaseFlow::free_inode`](flow.cppm.md#fn-1e833e7017d26cb8)
- [`xv::fs::Xv6FileSystem::ReleaseFlow::lock_node`](flow.cppm.md#fn-52b69540fbee4f12)
- [`xv::fs::Xv6FileSystem::ReleaseFlow::truncate`](flow.cppm.md#fn-8e78cffcb60e5a02)
- [`xv::fs::Xv6FileSystem::ReleaseFlow::unlock_node`](flow.cppm.md#fn-1333e0d89dca9690)
- [`xv::fs::Xv6FileSystem::ResizeFlow::direct`](flow.cppm.md#fn-df33cc97c39ccc0d)
- [`xv::fs::Xv6FileSystem::ResizeFlow::indirect`](flow.cppm.md#fn-465c38bb4e08b358)
- [`xv::fs::Xv6FileSystem::ResizeFlow::partial_tail`](flow.cppm.md#fn-87345712f37936b5)
- [`xv::fs::Xv6FileSystem::ResizeFlow::publish`](flow.cppm.md#fn-a59efee285af8015)
- [`xv::fs::Xv6FileSystem::ResizeFlow::trim_indirect`](flow.cppm.md#fn-4f0041974efdb812)
- [`xv::fs::Xv6FileSystem::ResizeFlow::valid`](flow.cppm.md#fn-57f0478caa42ed8f)
- [`xv::fs::Xv6FileSystem::ResizeFlow::zero_tail`](flow.cppm.md#fn-acd33ae2e02abb80)
- [`xv::fs::Xv6FileSystem::TruncateFlow::direct`](flow.cppm.md#fn-c78c16205330269d)
- [`xv::fs::Xv6FileSystem::TruncateFlow::free_indirect`](flow.cppm.md#fn-5ea7f13ba55e1438)
- [`xv::fs::Xv6FileSystem::TruncateFlow::indirect`](flow.cppm.md#fn-0652cbfe832970ee)
- [`xv::fs::Xv6FileSystem::TruncateFlow::publish`](flow.cppm.md#fn-9cfdd655289f8eb7)
- [`xv::fs::Xv6FileSystem::WriteFlow::copy`](flow.cppm.md#fn-8e2870f2d576e0ee)
- [`xv::fs::Xv6FileSystem::WriteFlow::map`](flow.cppm.md#fn-798ea646d2eab7ae)
- [`xv::fs::Xv6FileSystem::WriteFlow::mapped`](flow.cppm.md#fn-d41a1993b57940cf)
- [`xv::fs::Xv6FileSystem::WriteFlow::more`](flow.cppm.md#fn-21ea818785b84119)
- [`xv::fs::Xv6FileSystem::WriteFlow::progress`](flow.cppm.md#fn-9f687a048de80561)
- [`xv::fs::Xv6FileSystem::WriteFlow::publish`](flow.cppm.md#fn-82db69e3755c8b70)
- [`xv::fs::Xv6FileSystem::WriteFlow::read`](flow.cppm.md#fn-20b01bc5116f30bf)
- [`xv::fs::Xv6FileSystem::WriteFlow::record_release`](flow.cppm.md#fn-d2422d3268432f30)
- [`xv::fs::Xv6FileSystem::WriteFlow::valid`](flow.cppm.md#fn-c80ec2be5ab10255)

<a id="fn-d9ee76957dc74fb0"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::advance`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::advance(Context &c) noexcept
@brief 将位图覆盖起点推进 BPB 个块。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9b5c02f74ab70bf2"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::bitmap`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::bitmap(Context &c) noexcept
@brief 读取当前范围对应的位图块。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9b74690a1147a2dc"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::exhausted`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::exhausted(Context &) noexcept
@brief 报告磁盘块已耗尽，保留零结果。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a5116f5fd61b0f93"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::find`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::find(Context &c) noexcept
@brief 在当前位图直接查找首个空闲有效块位。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-dbf5512a7fb40fb6"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::found`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::AllocateBlockFlow::found(Context &c) noexcept
@brief 检查当前位图是否找到空闲块。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b7660da86ed9c213"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::release`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::release(Context &c) noexcept
@brief 归还当前位图缓冲。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4b1882743a62611b"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::remaining`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::AllocateBlockFlow::remaining(Context &c) noexcept
@brief 检查是否还有位图覆盖范围待扫描。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1f54244c693aebf8"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::reserve`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::reserve(Context &c) noexcept
@brief 设置空闲位并记录位图日志。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-85bf15838e939ee9"></a>

## `xv::fs::Xv6FileSystem::AllocateBlockFlow::zero`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateBlockFlow::zero(Context &c) noexcept
@brief 清零选定块并保存磁盘块号。
@details 调用者处于事务；在日志记录位图占用后先释放位图缓冲，再清零选定数据块。位图页内逐位扫描保持直接循环。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c0b987054eef57d0"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::advance`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::advance(Context &c) noexcept
@brief 推进到下一个 inode 号。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-2cea51f298dd0df7"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::exhausted`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::exhausted(Context &) noexcept
@brief 报告磁盘 inode 耗尽，保留空结果。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d51479f854904d64"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::free`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::AllocateFlow::free(Context &c) noexcept
@brief 检查磁盘 inode 类型为零。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c14abd74f27194ec"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::get`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::get(Context &c) noexcept
@brief 取得已分配 inode 的缓存引用。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-aeb40a12b4295d20"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::read`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::read(Context &c) noexcept
@brief 读取当前 inode 所在块并定位磁盘记录。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-20d2f9d5d13dda20"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::release`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::release(Context &c) noexcept
@brief 归还当前 inode 块缓冲。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-dacfadef90664c97"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::remaining`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::AllocateFlow::remaining(Context &c) noexcept
@brief 检查是否还有非零 inode 号待查找。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b7937d16c2028a68"></a>

## `xv::fs::Xv6FileSystem::AllocateFlow::reserve`

```doxygen
@fn static void xv::fs::Xv6FileSystem::AllocateFlow::reserve(Context &c) noexcept
@brief 清零空闲磁盘 inode、设置类型并记录日志。
@details 调用者处于事务；逐个读取并检查磁盘 inode，在磁盘类型与日志发布后释放缓冲，再取得缓存 inode 引用。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c1bdd37b748a1fe1"></a>

## `xv::fs::Xv6FileSystem::BlockIoProtocol::minimum`

```doxygen
@fn static unsigned xv::fs::Xv6FileSystem::BlockIoProtocol::minimum(unsigned first, unsigned second) noexcept
@brief 选择两个无符号长度中的较小值。
@details 读取和写入共用的无状态范围计算业务原语，不返回流程控制信号。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f61eb58c324e4af3"></a>

## `xv::fs::Xv6FileSystem::GetFlow::available`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::GetFlow::available(Context &c) noexcept
@brief 检查缓存表是否还有可回收槽。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0cf9c6b36e768f3e"></a>

## `xv::fs::Xv6FileSystem::GetFlow::exhausted`

```doxygen
@fn static void xv::fs::Xv6FileSystem::GetFlow::exhausted(Context &) noexcept
@brief 诊断 inode 缓存表耗尽。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f49eb4d3ae3dcdb1"></a>

## `xv::fs::Xv6FileSystem::GetFlow::find`

```doxygen
@fn static void xv::fs::Xv6FileSystem::GetFlow::find(Context &c) noexcept
@brief 直接扫描活动 inode 身份并记住首个空槽。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-bea70f09a1403f53"></a>

## `xv::fs::Xv6FileSystem::GetFlow::found`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::GetFlow::found(Context &c) noexcept
@brief 检查是否命中了既有缓存 inode。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-088731f968a7de51"></a>

## `xv::fs::Xv6FileSystem::GetFlow::initialize`

```doxygen
@fn static void xv::fs::Xv6FileSystem::GetFlow::initialize(Context &c) noexcept
@brief 在空槽设置设备、号码、一个引用和无效缓存标记。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d8b1921ba0dc0980"></a>

## `xv::fs::Xv6FileSystem::GetFlow::retain`

```doxygen
@fn static void xv::fs::Xv6FileSystem::GetFlow::retain(Context &c) noexcept
@brief 增加命中 inode 的引用。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-242fcf8f00253ea3"></a>

## `xv::fs::Xv6FileSystem::GetFlow::unlock`

```doxygen
@fn static void xv::fs::Xv6FileSystem::GetFlow::unlock(Context &c) noexcept
@brief 释放 inode 表自旋锁。
@details 入口已持 inode 表自旋锁；扫描期间只选择首个空槽，命中则保留原缓存身份，所有成功路径在返回前统一解锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-40adfd7d8c8119d0"></a>

## `xv::fs::Xv6FileSystem::InitializeFlow::invalid`

```doxygen
@fn static void xv::fs::Xv6FileSystem::InitializeFlow::invalid(Context &) noexcept
@brief 诊断文件系统魔数无效。
@details 按原顺序读取超级块、验证魔数、恢复日志再回收孤儿 inode；初始化设备号也在读取之前。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-403fa33d73ddc06f"></a>

## `xv::fs::Xv6FileSystem::InitializeFlow::journal`

```doxygen
@fn static void xv::fs::Xv6FileSystem::InitializeFlow::journal(Context &c) noexcept
@brief 初始化并恢复该后端的日志。
@details 按原顺序读取超级块、验证魔数、恢复日志再回收孤儿 inode；初始化设备号也在读取之前。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d04787a584b89709"></a>

## `xv::fs::Xv6FileSystem::InitializeFlow::reclaim`

```doxygen
@fn static void xv::fs::Xv6FileSystem::InitializeFlow::reclaim(Context &c) noexcept
@brief 扫描并回收磁盘孤儿 inode。
@details 按原顺序读取超级块、验证魔数、恢复日志再回收孤儿 inode；初始化设备号也在读取之前。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-e13ae393b612a679"></a>

## `xv::fs::Xv6FileSystem::InitializeFlow::superblock`

```doxygen
@fn static void xv::fs::Xv6FileSystem::InitializeFlow::superblock(Context &c) noexcept
@brief 发布设备号并读取超级块。
@details 按原顺序读取超级块、验证魔数、恢复日志再回收孤儿 inode；初始化设备号也在读取之前。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-930cdc72cbaf6f24"></a>

## `xv::fs::Xv6FileSystem::InitializeFlow::valid`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::InitializeFlow::valid(Context &c) noexcept
@brief 检查超级块魔数匹配磁盘格式。
@details 按原顺序读取超级块、验证魔数、恢复日志再回收孤儿 inode；初始化设备号也在读取之前。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-60459a9b05de4e3d"></a>

## `xv::fs::Xv6FileSystem::LockFlow::cached`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LockFlow::cached(Context &c) noexcept
@brief 检查 inode 是否已缓存有效元数据。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-25dbfb53ccd48b84"></a>

## `xv::fs::Xv6FileSystem::LockFlow::invalid`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LockFlow::invalid(Context &) noexcept
@brief 诊断无效 inode 加锁请求。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f3aec62d698f60f6"></a>

## `xv::fs::Xv6FileSystem::LockFlow::live`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LockFlow::live(Context &c) noexcept
@brief 检查输入节点非空且仍有引用。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-db3b0dc18427341b"></a>

## `xv::fs::Xv6FileSystem::LockFlow::load`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LockFlow::load(Context &c) noexcept
@brief 读取磁盘元数据，复制地址和权限，释放块缓冲后标记缓存有效。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d512289c6e1279fa"></a>

## `xv::fs::Xv6FileSystem::LockFlow::lock`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LockFlow::lock(Context &c) noexcept
@brief 取得 inode 睡眠锁。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b15bbbdf4d27c73f"></a>

## `xv::fs::Xv6FileSystem::LockFlow::missing_type`

```doxygen
@fn static void xv::fs::Xv6FileSystem::LockFlow::missing_type(Context &) noexcept
@brief 诊断加载到没有类型的 inode。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-b596a9e85013e687"></a>

## `xv::fs::Xv6FileSystem::LockFlow::typed`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::LockFlow::typed(Context &c) noexcept
@brief 检查本次新加载的磁盘类型非零。
@details 入口的 BlockHandle 保留词法作用域；先验证引用、取得 inode 睡眠锁，只有无效缓存才从磁盘加载，成功返回时继续持 inode 锁。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6c0a38a84655daad"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::allocate`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::allocate(Context &c) noexcept
@brief 分配一个已清零磁盘块并保存返回块号。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a788f9dd207edd5e"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::create`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::MapBlockFlow::create(Context &c) noexcept
@brief 检查调用者是否允许分配缺失块。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-92eeb8670d175efa"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::direct`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::MapBlockFlow::direct(Context &c) noexcept
@brief 检查逻辑块号是否属于直接地址范围。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c97fc8bb9cb91f93"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::indirect`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::MapBlockFlow::indirect(Context &c) noexcept
@brief 检查间接表下标位于有效范围。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6702fd4c15c42e98"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::indirect_index`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::indirect_index(Context &c) noexcept
@brief 减去直接块数量以形成间接表下标。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-cb88e5578620209a"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::invalid`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::invalid(Context &) noexcept
@brief 诊断逻辑块号超出文件地址范围。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8096c7816360edc2"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::load`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::load(Context &c) noexcept
@brief 取得间接表缓冲并读取目标数据块号。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-def30a3e1f23e66e"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::present`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::MapBlockFlow::present(Context &c) noexcept
@brief 检查当前阶段是否取得非零磁盘块号。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0b5a702d4d78e006"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::record`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::record(Context &c) noexcept
@brief 将成功分配的数据块号写入间接表并记录日志。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-6568ef274b77c832"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::release`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::release(Context &c) noexcept
@brief 归还间接表缓冲。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f23e51caecfc4557"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::select_direct`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::select_direct(Context &c) noexcept
@brief 选择直接地址槽并读取已有块号。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-49598c3b95a42d3f"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::select_root`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::select_root(Context &c) noexcept
@brief 选择 inode 的间接根地址槽并读取块号。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5a2d3339fdcda75c"></a>

## `xv::fs::Xv6FileSystem::MapBlockFlow::store`

```doxygen
@fn static void xv::fs::Xv6FileSystem::MapBlockFlow::store(Context &c) noexcept
@brief 将分配成功的块号写入已选择的 inode 地址槽。
@details 调用者持 inode 锁，创建时处于事务；直接块和间接根的分配、保存复用相同动作但使用不同节点身份。间接表缓冲在返回前释放，分配失败保留已拥有的间接根。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5305105b5a619a63"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::allocated`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReadFlow::allocated(Context &c) noexcept
@brief 检查本次逻辑块是否已有物理块。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f2ccfb251af4002f"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::clamp`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::clamp(Context &c) noexcept
@brief 将读取长度裁剪到 inode 逻辑末尾。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-69b69d2416702848"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::complete`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::complete(Context &c) noexcept
@brief 记录完整读取的字节数。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-09be5a97e6be235a"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::copy_data`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReadFlow::copy_data(Context &c) noexcept
@brief 复制普通块内容并保留原精确负一失败判定。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c423320b1c72757a"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::copy_zero`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReadFlow::copy_zero(Context &c) noexcept
@brief 从静态零块复制稀疏内容并按负值失败规则报告结果。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-fe10fd5559d65c18"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::data_failure`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::data_failure(Context &c) noexcept
@brief 归还普通块缓冲并按原规则记录负一。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-3b07e64b98b34542"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::more`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReadFlow::more(Context &c) noexcept
@brief 检查是否还有读取字节。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-bb2e8e1dfc8d1e20"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::prepare`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::prepare(Context &c) noexcept
@brief 只查找现有映射并计算本次块内复制长度。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-f9d08e8a2b70ff05"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::progress`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::progress(Context &c) noexcept
@brief 同步推进已完成字节数、文件偏移与目标地址。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-62cd5e8df2500cdc"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::read`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::read(Context &c) noexcept
@brief 取得已分配数据块的缓存缓冲。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-832928e9aee55a40"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::release`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::release(Context &c) noexcept
@brief 归还普通数据块缓冲。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-ff5fc7be58dace84"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::sparse_failure`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReadFlow::sparse_failure(Context &c) noexcept
@brief 记录已有字节进展，首段稀疏复制失败为负一。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a9f0cfeb1c84c6e7"></a>

## `xv::fs::Xv6FileSystem::ReadFlow::valid`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReadFlow::valid(Context &c) noexcept
@brief 检查起点不越过逻辑末尾且偏移加长度没有回绕。
@details 调用者持 inode 锁；稀疏读取不分配块。普通块复制失败保留原返回负一语义，稀疏零块复制失败按原策略保留此前进展，两个失败出口不能合并。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d6c598288ec590b2"></a>

## `xv::fs::Xv6FileSystem::ReclaimFlow::advance`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReclaimFlow::advance(Context &c) noexcept
@brief 推进扫描 inode 号。
@details 启动扫描按 inode 号迭代，释放扫描块缓冲后才进入局部事务；每个孤儿单独加载并归还，最终释放复用普通 release 协议。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-09c4190e4e5cf078"></a>

## `xv::fs::Xv6FileSystem::ReclaimFlow::find`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReclaimFlow::find(Context &c) noexcept
@brief 读取当前磁盘记录，报告并取得零链接孤儿的缓存引用，再释放扫描缓冲。
@details 启动扫描按 inode 号迭代，释放扫描块缓冲后才进入局部事务；每个孤儿单独加载并归还，最终释放复用普通 release 协议。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-08b1a8dcb8737971"></a>

## `xv::fs::Xv6FileSystem::ReclaimFlow::found`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReclaimFlow::found(Context &c) noexcept
@brief 检查本轮是否取得孤儿 inode。
@details 启动扫描按 inode 号迭代，释放扫描块缓冲后才进入局部事务；每个孤儿单独加载并归还，最终释放复用普通 release 协议。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8d7b9ca7bdfea57c"></a>

## `xv::fs::Xv6FileSystem::ReclaimFlow::recover`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReclaimFlow::recover(Context &c) noexcept
@brief 在局部日志事务中加锁加载、解锁并归还孤儿以触发回收。
@details 启动扫描按 inode 号迭代，释放扫描块缓冲后才进入局部事务；每个孤儿单独加载并归还，最终释放复用普通 release 协议。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-32755ce65e76aaf1"></a>

## `xv::fs::Xv6FileSystem::ReclaimFlow::remaining`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ReclaimFlow::remaining(Context &c) noexcept
@brief 检查是否还有磁盘 inode 待扫描。
@details 启动扫描按 inode 号迭代，释放扫描块缓冲后才进入局部事务；每个孤儿单独加载并归还，最终释放复用普通 release 协议。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-905787f34f734109"></a>

## `xv::fs::Xv6FileSystem::ReleaseFlow::drop`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReleaseFlow::drop(Context &c) noexcept
@brief 减少缓存引用并释放表锁。
@details 入口已确认最后无链接引用，持 inode 表锁并保存设备、号码；内容回收时换成 inode 锁，减引用后不再读取可能被重用的缓存槽身份。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1e833e7017d26cb8"></a>

## `xv::fs::Xv6FileSystem::ReleaseFlow::free_inode`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReleaseFlow::free_inode(Context &c) noexcept
@brief 用入口保存的设备和号码清除磁盘 inode 类型。
@details 入口已确认最后无链接引用，持 inode 表锁并保存设备、号码；内容回收时换成 inode 锁，减引用后不再读取可能被重用的缓存槽身份。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-52b69540fbee4f12"></a>

## `xv::fs::Xv6FileSystem::ReleaseFlow::lock_node`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReleaseFlow::lock_node(Context &c) noexcept
@brief 取得独占 inode 锁后释放 inode 表锁。
@details 入口已确认最后无链接引用，持 inode 表锁并保存设备、号码；内容回收时换成 inode 锁，减引用后不再读取可能被重用的缓存槽身份。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8e78cffcb60e5a02"></a>

## `xv::fs::Xv6FileSystem::ReleaseFlow::truncate`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReleaseFlow::truncate(Context &c) noexcept
@brief 释放 inode 内容并清除缓存有效标记。
@details 入口已确认最后无链接引用，持 inode 表锁并保存设备、号码；内容回收时换成 inode 锁，减引用后不再读取可能被重用的缓存槽身份。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-1333e0d89dca9690"></a>

## `xv::fs::Xv6FileSystem::ReleaseFlow::unlock_node`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ReleaseFlow::unlock_node(Context &c) noexcept
@brief 释放 inode 锁并重新取得 inode 表锁。
@details 入口已确认最后无链接引用，持 inode 表锁并保存设备、号码；内容回收时换成 inode 锁，减引用后不再读取可能被重用的缓存槽身份。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-df33cc97c39ccc0d"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::direct`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ResizeFlow::direct(Context &c) noexcept
@brief 计算保留块数并直接释放不再保留的直接块。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-465c38bb4e08b358"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::indirect`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ResizeFlow::indirect(Context &c) noexcept
@brief 检查是否有间接表需要处理。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-87345712f37936b5"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::partial_tail`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ResizeFlow::partial_tail(Context &c) noexcept
@brief 检查本次收缩是否保留一个部分使用的尾块。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-a59efee285af8015"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::publish`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ResizeFlow::publish(Context &c) noexcept
@brief 提交逻辑长度、更新 inode 并记录成功码。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-4f0041974efdb812"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::trim_indirect`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ResizeFlow::trim_indirect(Context &c) noexcept
@brief 直接释放间接尾块，保留非空表并记日志，空表在释放缓冲后归还。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-57f0478caa42ed8f"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::valid`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::ResizeFlow::valid(Context &c) noexcept
@brief 检查节点为普通文件且目标长度不超过最大文件范围。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-acd33ae2e02abb80"></a>

## `xv::fs::Xv6FileSystem::ResizeFlow::zero_tail`

```doxygen
@fn static void xv::fs::Xv6FileSystem::ResizeFlow::zero_tail(Context &c) noexcept
@brief 只查找已有尾块，存在时清零截断后缀并记录日志。
@details 调用者持 inode 锁且处于事务；长度增长保持稀疏，收缩先释放尾块再清零保留尾块后缀，最后更新逻辑长度。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c78c16205330269d"></a>

## `xv::fs::Xv6FileSystem::TruncateFlow::direct`

```doxygen
@fn static void xv::fs::Xv6FileSystem::TruncateFlow::direct(Context &c) noexcept
@brief 直接遍历并归还非零直接块，清空对应地址槽。
@details 调用者持 inode 锁且处于事务；先归还直接数据块，再归还间接数据块和间接表，最后清零大小并更新磁盘 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-5ea7f13ba55e1438"></a>

## `xv::fs::Xv6FileSystem::TruncateFlow::free_indirect`

```doxygen
@fn static void xv::fs::Xv6FileSystem::TruncateFlow::free_indirect(Context &c) noexcept
@brief 直接归还间接数据块，释放缓冲后归还间接表并清空根地址。
@details 调用者持 inode 锁且处于事务；先归还直接数据块，再归还间接数据块和间接表，最后清零大小并更新磁盘 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-0652cbfe832970ee"></a>

## `xv::fs::Xv6FileSystem::TruncateFlow::indirect`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::TruncateFlow::indirect(Context &c) noexcept
@brief 检查是否存在间接表。
@details 调用者持 inode 锁且处于事务；先归还直接数据块，再归还间接数据块和间接表，最后清零大小并更新磁盘 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9cfdd655289f8eb7"></a>

## `xv::fs::Xv6FileSystem::TruncateFlow::publish`

```doxygen
@fn static void xv::fs::Xv6FileSystem::TruncateFlow::publish(Context &c) noexcept
@brief 将 inode 大小置零并持久记录 inode 元数据。
@details 调用者持 inode 锁且处于事务；先归还直接数据块，再归还间接数据块和间接表，最后清零大小并更新磁盘 inode。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-8e2870f2d576e0ee"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::copy`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::WriteFlow::copy(Context &c) noexcept
@brief 复制输入字节并保留原精确负一失败判定。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-798ea646d2eab7ae"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::map`

```doxygen
@fn static void xv::fs::Xv6FileSystem::WriteFlow::map(Context &c) noexcept
@brief 取得或创建当前逻辑块映射。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d41a1993b57940cf"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::mapped`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::WriteFlow::mapped(Context &c) noexcept
@brief 检查是否取得可写磁盘块。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-21ea818785b84119"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::more`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::WriteFlow::more(Context &c) noexcept
@brief 检查是否还有写入字节。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-9f687a048de80561"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::progress`

```doxygen
@fn static void xv::fs::Xv6FileSystem::WriteFlow::progress(Context &c) noexcept
@brief 同步推进完整写入字节数、文件偏移与输入地址。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-82db69e3755c8b70"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::publish`

```doxygen
@fn static void xv::fs::Xv6FileSystem::WriteFlow::publish(Context &c) noexcept
@brief 按最终偏移增大逻辑长度，始终更新 inode 并记录完成字节数。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-20b01bc5116f30bf"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::read`

```doxygen
@fn static void xv::fs::Xv6FileSystem::WriteFlow::read(Context &c) noexcept
@brief 取得数据块缓存并计算本段块内长度。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-d2422d3268432f30"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::record_release`

```doxygen
@fn static void xv::fs::Xv6FileSystem::WriteFlow::record_release(Context &c) noexcept
@brief 记录可能已被部分修改的数据块并释放缓冲。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```

<a id="fn-c80ec2be5ab10255"></a>

## `xv::fs::Xv6FileSystem::WriteFlow::valid`

```doxygen
@fn static bool xv::fs::Xv6FileSystem::WriteFlow::valid(Context &c) noexcept
@brief 检查偏移相加不回绕且不超过最大文件范围。
@details 调用者持 inode 锁且处于事务；成功和失败复制分别经过不同日志节点，复用同一记录动作，只有完整段才推进偏移。最终即使长度未变也更新 inode 地址元数据。 下一节点由图类型策略声明，业务状态与控制信号分离。
```
