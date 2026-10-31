# `kernel/drivers/block/virtio/virtio_block.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/block/virtio/virtio_block.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:protocol`, `:virtio`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::VirtioBlock::ChainFlow::advance`](virtio_block.cpp.md#fn-36eba3f76f9c7a25)
- [`xv::drivers::VirtioBlock::ChainFlow::allocate`](virtio_block.cpp.md#fn-b323ca80c039a54e)
- [`xv::drivers::VirtioBlock::ChainFlow::allocated`](virtio_block.cpp.md#fn-22b18fad796442d8)
- [`xv::drivers::VirtioBlock::ChainFlow::remaining`](virtio_block.cpp.md#fn-9bc0672d6ffa44dc)
- [`xv::drivers::VirtioBlock::ChainFlow::rollback`](virtio_block.cpp.md#fn-274377630e85444c)
- [`xv::drivers::VirtioBlock::ChainFlow::success`](virtio_block.cpp.md#fn-ec75815ee705177d)
- [`xv::drivers::VirtioBlock::InitializeFlow::allocate`](virtio_block.cpp.md#fn-c644ebc2424c1ca6)
- [`xv::drivers::VirtioBlock::InitializeFlow::features`](virtio_block.cpp.md#fn-63635adc0a247e89)
- [`xv::drivers::VirtioBlock::InitializeFlow::handshake`](virtio_block.cpp.md#fn-1258e53f0810f28d)
- [`xv::drivers::VirtioBlock::InitializeFlow::publish`](virtio_block.cpp.md#fn-793be39fc8561bae)
- [`xv::drivers::VirtioBlock::InitializeFlow::queue`](virtio_block.cpp.md#fn-75e1a3547e20d666)
- [`xv::drivers::VirtioBlock::InitializeFlow::ready`](virtio_block.cpp.md#fn-fcc558d37d6bfebe)
- [`xv::drivers::VirtioBlock::InitializeFlow::validate`](virtio_block.cpp.md#fn-bbc15f1f6764bafb)
- [`xv::drivers::VirtioBlock::InterruptFlow::acknowledge`](virtio_block.cpp.md#fn-dd5545dc07686384)
- [`xv::drivers::VirtioBlock::InterruptFlow::complete`](virtio_block.cpp.md#fn-a38d716f418fc489)
- [`xv::drivers::VirtioBlock::InterruptFlow::pending`](virtio_block.cpp.md#fn-ea6b08122bf7a7ba)
- [`xv::drivers::VirtioBlock::InterruptFlow::read`](virtio_block.cpp.md#fn-8937f57d3588d613)
- [`xv::drivers::VirtioBlock::TransferFlow::allocate`](virtio_block.cpp.md#fn-1c047e5bd8398c8c)
- [`xv::drivers::VirtioBlock::TransferFlow::ok`](virtio_block.cpp.md#fn-194cc67b88c87068)
- [`xv::drivers::VirtioBlock::TransferFlow::pending`](virtio_block.cpp.md#fn-fa2d7f3f9156bdf2)
- [`xv::drivers::VirtioBlock::TransferFlow::prepare`](virtio_block.cpp.md#fn-3c572b76a9aec1d0)
- [`xv::drivers::VirtioBlock::TransferFlow::reclaim`](virtio_block.cpp.md#fn-3bfd703ed3a1acf9)
- [`xv::drivers::VirtioBlock::TransferFlow::submit`](virtio_block.cpp.md#fn-4ed006cd661e3621)
- [`xv::drivers::VirtioBlock::TransferFlow::wait_completion`](virtio_block.cpp.md#fn-b48f6df713a588c1)
- [`xv::drivers::VirtioBlock::TransferFlow::wait_descriptors`](virtio_block.cpp.md#fn-ec5daaa5040a9f2c)
- [`xv::drivers::VirtioBlock::allocate_chain`](virtio_block.cpp.md#fn-f8ec5a9a894c4d73)
- [`xv::drivers::VirtioBlock::allocate_descriptor`](virtio_block.cpp.md#fn-ec9a5a1df2982759)
- [`xv::drivers::VirtioBlock::flush`](virtio_block.cpp.md#fn-6a305340ef7d40e8)
- [`xv::drivers::VirtioBlock::free_chain`](virtio_block.cpp.md#fn-e4fe48e63edf2388)
- [`xv::drivers::VirtioBlock::free_descriptor`](virtio_block.cpp.md#fn-bfd4e838d44d17f4)
- [`xv::drivers::VirtioBlock::handle_interrupt`](virtio_block.cpp.md#fn-0b67b739b5b86885)
- [`xv::drivers::VirtioBlock::initialize`](virtio_block.cpp.md#fn-4c7be18d8f3eca1c)
- [`xv::drivers::VirtioBlock::present`](virtio_block.cpp.md#fn-3b90475ad6a2ea72)
- [`xv::drivers::VirtioBlock::transfer`](virtio_block.cpp.md#fn-266fcc4568332ef4)

<a id="fn-4c7be18d8f3eca1c"></a>

## `xv::drivers::VirtioBlock::initialize`

```doxygen
@fn void xv::drivers::VirtioBlock::initialize()
@brief 协商并初始化独立 VirtIO 队列。
@details 记录设备是否提供 FLUSH，保留该特性参与协商；其余描述符、可用环与完成环建立流程保持原契约。
@details 流程由 InitializeFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-ec9a5a1df2982759"></a>

## `xv::drivers::VirtioBlock::allocate_descriptor`

```doxygen
@fn int xv::drivers::VirtioBlock::allocate_descriptor()
@brief VirtIO 块设备：在 VirtIO 描述符池中取得一个空闲项。
@details 本函数可见的状态更新包括：`free_[i] = 0;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`free_[i]`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`i`；`-1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bfd4e838d44d17f4"></a>

## `xv::drivers::VirtioBlock::free_descriptor`

```doxygen
@fn void xv::drivers::VirtioBlock::free_descriptor(int i)
@brief VirtIO 块设备：归还指定 VirtIO 描述符。
@param i 本次操作使用的 i 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`descriptors_[i].addr = 0;`；`descriptors_[i].len = 0;`；`descriptors_[i].flags = 0;`；`descriptors_[i].next = 0;`；`free_[i] = 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`i < 0 || i >= QueueSize`；`free_[i]`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e4fe48e63edf2388"></a>

## `xv::drivers::VirtioBlock::free_chain`

```doxygen
@fn void xv::drivers::VirtioBlock::free_chain(int i)
@brief VirtIO 块设备：逐项归还 VirtIO 描述符链。
@param i 本次操作使用的 i 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`free_descriptor`。
@details 需要特别跟踪的条件：`1`；`flag & VRING_DESC_F_NEXT`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f8ec5a9a894c4d73"></a>

## `xv::drivers::VirtioBlock::allocate_chain`

```doxygen
@fn int xv::drivers::VirtioBlock::allocate_chain(int *idx)
@brief VirtIO 块设备：为一次块请求取得完整描述符链。
@param idx 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ChainFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-266fcc4568332ef4"></a>

## `xv::drivers::VirtioBlock::transfer`

```doxygen
@fn void xv::drivers::VirtioBlock::transfer(BlockOperation operation, unsigned long sector, void *data, unsigned count)
@brief 提交数据传输或内部 FLUSH 请求并等待完成。
@details 内部空数据零长度请求编码为 FLUSH，sector 为零，设备链仅含头和状态。三个预留描述符全部归还；正常读写继续检查扇区倍数并保持 DMA 数据到返回时有效。
@details 流程由 TransferFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-0b67b739b5b86885"></a>

## `xv::drivers::VirtioBlock::handle_interrupt`

```doxygen
@fn void xv::drivers::VirtioBlock::handle_interrupt()
@brief VirtIO 块设备：响应设备中断并完成或推进挂起的设备操作。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
@details 流程由 InterruptFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-3b90475ad6a2ea72"></a>

## `xv::drivers::VirtioBlock::present`

```doxygen
@fn bool xv::drivers::VirtioBlock::present() const
@brief 探测此 MMIO 槽是否为块设备。
@details 只检查 magic 和 device ID，不分配队列或发起 I/O；供可选第二块盘的装配使用。
```

<a id="fn-6a305340ef7d40e8"></a>

## `xv::drivers::VirtioBlock::flush`

```doxygen
@fn void xv::drivers::VirtioBlock::flush()
@brief 等待一个协商过的 FLUSH 请求完成。
@details 设备提供 FLUSH 时经统一队列提交并等待中断确认；未提供时不发送不支持的请求。
```

<a id="fn-36eba3f76f9c7a25"></a>

## `xv::drivers::VirtioBlock::ChainFlow::advance`

```doxygen
@fn static void xv::drivers::VirtioBlock::ChainFlow::advance(Context &c) noexcept
@brief 推进本次候选槽索引。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b323ca80c039a54e"></a>

## `xv::drivers::VirtioBlock::ChainFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioBlock::ChainFlow::allocate(Context &c) noexcept
@brief 取得当前索引对应的描述符槽。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-22b18fad796442d8"></a>

## `xv::drivers::VirtioBlock::ChainFlow::allocated`

```doxygen
@fn static bool xv::drivers::VirtioBlock::ChainFlow::allocated(Context &c) noexcept
@brief 检查本槽分配是否成功。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9bc0672d6ffa44dc"></a>

## `xv::drivers::VirtioBlock::ChainFlow::remaining`

```doxygen
@fn static bool xv::drivers::VirtioBlock::ChainFlow::remaining(Context &c) noexcept
@brief 检查是否仍需取得三个槽中的后续槽。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-274377630e85444c"></a>

## `xv::drivers::VirtioBlock::ChainFlow::rollback`

```doxygen
@fn static void xv::drivers::VirtioBlock::ChainFlow::rollback(Context &c) noexcept
@brief 分配失败时按原顺序释放已经取得的候选槽。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ec75815ee705177d"></a>

## `xv::drivers::VirtioBlock::ChainFlow::success`

```doxygen
@fn static void xv::drivers::VirtioBlock::ChainFlow::success(Context &c) noexcept
@brief 记录完整三个槽已取得。
@details 调用者持设备锁；逐个取得原三个槽，任一步失败按原正向次序归还此前候选并保留唤醒；成功才由调用者构造链。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c644ebc2424c1ca6"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::allocate(Context &c) noexcept
@brief 取得原三个 DMA 页，检查后按原次序清零。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-63635adc0a247e89"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::features`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::features(Context &c) noexcept
@brief 读取并筛选特性、保存 flush 能力，发布并复查 FEATURES_OK。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1258e53f0810f28d"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::handshake`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::handshake(Context &c) noexcept
@brief 按原次序复位并发布 ACKNOWLEDGE 与 DRIVER 状态。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-793be39fc8561bae"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::publish`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::publish(Context &c) noexcept
@brief 设置队列长度、六个地址寄存器并发布 ready。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-75e1a3547e20d666"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::queue`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::queue(Context &c) noexcept
@brief 选择队列零并检查尚未 ready 与支持的描述符容量。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fcc558d37d6bfebe"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::ready`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::ready(Context &c) noexcept
@brief 初始化全部空闲描述符并发布原 DRIVER_OK 状态。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-bbc15f1f6764bafb"></a>

## `xv::drivers::VirtioBlock::InitializeFlow::validate`

```doxygen
@fn static void xv::drivers::VirtioBlock::InitializeFlow::validate(Context &c) noexcept
@brief 初始化设备锁并按原次序验证 VirtIO 标识、版本、类型和厂商。
@details 保持原 MMIO 检查、状态握手、特性协商、队列检查、三页分配与发布顺序；失败仍为原 fatal 诊断，不擅自添加恢复或回滚。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dd5545dc07686384"></a>

## `xv::drivers::VirtioBlock::InterruptFlow::acknowledge`

```doxygen
@fn static void xv::drivers::VirtioBlock::InterruptFlow::acknowledge(Context &c) noexcept
@brief 按原状态掩码确认中断并执行首个 DMA 屏障。
@details 入口 ScopedLock 持设备锁；先 ACK 与屏障，再按 volatile used 索引迭代处理，验证 ID、pending 与状态后才清 pending、唤醒并推进消费索引。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-a38d716f418fc489"></a>

## `xv::drivers::VirtioBlock::InterruptFlow::complete`

```doxygen
@fn static void xv::drivers::VirtioBlock::InterruptFlow::complete(Context &c) noexcept
@brief 清除请求 pending、唤醒等待者并推进 used 消费索引。
@details 入口 ScopedLock 持设备锁；先 ACK 与屏障，再按 volatile used 索引迭代处理，验证 ID、pending 与状态后才清 pending、唤醒并推进消费索引。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ea6b08122bf7a7ba"></a>

## `xv::drivers::VirtioBlock::InterruptFlow::pending`

```doxygen
@fn static bool xv::drivers::VirtioBlock::InterruptFlow::pending(Context &c) noexcept
@brief 检查 used 环是否仍有未消费完成。
@details 入口 ScopedLock 持设备锁；先 ACK 与屏障，再按 volatile used 索引迭代处理，验证 ID、pending 与状态后才清 pending、唤醒并推进消费索引。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8937f57d3588d613"></a>

## `xv::drivers::VirtioBlock::InterruptFlow::read`

```doxygen
@fn static void xv::drivers::VirtioBlock::InterruptFlow::read(Context &c) noexcept
@brief 屏障后读取完成 ID 并验证边界、pending 与设备状态。
@details 入口 ScopedLock 持设备锁；先 ACK 与屏障，再按 volatile used 索引迭代处理，验证 ID、pending 与状态后才清 pending、唤醒并推进消费索引。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-1c047e5bd8398c8c"></a>

## `xv::drivers::VirtioBlock::TransferFlow::allocate`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::allocate(Context &c) noexcept
@brief 尝试取得完整描述符候选链。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-194cc67b88c87068"></a>

## `xv::drivers::VirtioBlock::TransferFlow::ok`

```doxygen
@fn static bool xv::drivers::VirtioBlock::TransferFlow::ok(Context &c) noexcept
@brief 检查候选链分配是否成功。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-fa2d7f3f9156bdf2"></a>

## `xv::drivers::VirtioBlock::TransferFlow::pending`

```doxygen
@fn static bool xv::drivers::VirtioBlock::TransferFlow::pending(Context &c) noexcept
@brief 读取本请求是否仍等待设备完成。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3c572b76a9aec1d0"></a>

## `xv::drivers::VirtioBlock::TransferFlow::prepare`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::prepare(Context &c) noexcept
@brief 按原方向和 flush 规则填写三个描述符、状态与 pending。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3bfd703ed3a1acf9"></a>

## `xv::drivers::VirtioBlock::TransferFlow::reclaim`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::reclaim(Context &c) noexcept
@brief 完成后按普通链或 flush 规则归还全部槽位。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4ed006cd661e3621"></a>

## `xv::drivers::VirtioBlock::TransferFlow::submit`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::submit(Context &c) noexcept
@brief 写 avail 环，按原两次屏障推进索引并通知设备。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b48f6df713a588c1"></a>

## `xv::drivers::VirtioBlock::TransferFlow::wait_completion`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::wait_completion(Context &c) noexcept
@brief 登记原非可中断完成等待，解锁睡眠并复取设备锁。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-ec5daaa5040a9f2c"></a>

## `xv::drivers::VirtioBlock::TransferFlow::wait_descriptors`

```doxygen
@fn static void xv::drivers::VirtioBlock::TransferFlow::wait_descriptors(Context &c) noexcept
@brief 在设备锁内登记槽位等待后解锁睡眠并复取。
@details 入口持设备锁并保留原未初始化索引数组的调用帧；等待在条件锁内登记后解锁睡眠。提交中的屏障保持相邻，调用者 DMA 缓冲活到完成，flush 仍归还全部三个槽。 下一节点由图策略声明，业务结果与控制信号分离。
```
<a id="fn-bf8018e059e14ed7"></a>

## `xv::drivers::VirtioBlock::capacity_sectors`

```doxygen
@fn unsigned long VirtioBlock::xv::drivers::VirtioBlock::capacity_sectors() const
@brief 返回设备真实的64位512字节扇区数。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
