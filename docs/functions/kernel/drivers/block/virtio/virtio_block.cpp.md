# `kernel/drivers/block/virtio/virtio_block.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/block/virtio/virtio_block.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:protocol`, `:virtio`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

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
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`allocate_descriptor` → `free_descriptor`。
@details 本函数可见的状态更新包括：`idx[i] = allocate_descriptor();`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`idx[i] < 0`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-1`；`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-266fcc4568332ef4"></a>

## `xv::drivers::VirtioBlock::transfer`

```doxygen
@fn void xv::drivers::VirtioBlock::transfer(BlockOperation operation, unsigned long sector, void *data, unsigned count)
@brief 提交数据传输或内部 FLUSH 请求并等待完成。
@details 内部空数据零长度请求编码为 FLUSH，sector 为零，设备链仅含头和状态。三个预留描述符全部归还；正常读写继续检查扇区倍数并保持 DMA 数据到返回时有效。
```

<a id="fn-0b67b739b5b86885"></a>

## `xv::drivers::VirtioBlock::handle_interrupt`

```doxygen
@fn void xv::drivers::VirtioBlock::handle_interrupt()
@brief VirtIO 块设备：响应设备中断并完成或推进挂起的设备操作。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`guard` → `registers_.write32` → `registers_.read32` → `detail::io_barrier` → `detail::fail` → `detail::wake_processes`。
@details 本函数可见的状态更新包括：`requests_[id].pending = false;`；`used_index_ += 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`used_index_ != used_->idx`；`id < 0 || id >= QueueSize || !requests_[id].pending`；`requests_[id].status != 0`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
