# `kernel/drivers/block/virtio/virtio_block.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/block/virtio/virtio_block.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:protocol`, `:virtio`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::VirtioBlock::allocate_chain`](virtio_block.cpp.md#fn-f8ec5a9a894c4d73)
- [`xv::drivers::VirtioBlock::allocate_descriptor`](virtio_block.cpp.md#fn-ec9a5a1df2982759)
- [`xv::drivers::VirtioBlock::free_chain`](virtio_block.cpp.md#fn-e4fe48e63edf2388)
- [`xv::drivers::VirtioBlock::free_descriptor`](virtio_block.cpp.md#fn-bfd4e838d44d17f4)
- [`xv::drivers::VirtioBlock::handle_interrupt`](virtio_block.cpp.md#fn-0b67b739b5b86885)
- [`xv::drivers::VirtioBlock::initialize`](virtio_block.cpp.md#fn-4c7be18d8f3eca1c)
- [`xv::drivers::VirtioBlock::transfer`](virtio_block.cpp.md#fn-266fcc4568332ef4)

<a id="fn-4c7be18d8f3eca1c"></a>

## `xv::drivers::VirtioBlock::initialize`

```doxygen
@fn void xv::drivers::VirtioBlock::initialize()
@brief VirtIO 块设备：建立该对象开始工作所需的初始状态。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.init` → `registers_.read32` → `detail::fail` → `registers_.write32` → `detail::allocate_dma_page` → `detail::zero_page`。
@details 本函数可见的状态更新包括：`descriptors_ = static_cast<struct virtq_desc *>(detail::allocate_dma_page());`；`available_ = static_cast<struct virtq_avail *>(detail::allocate_dma_page());`；`used_ = static_cast<struct virtq_used *>(detail::allocate_dma_page());`；`free_[i] = 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!(status & VIRTIO_CONFIG_S_FEATURES_OK)`；`registers_.read32(VIRTIO_MMIO_QUEUE_READY)`；`max == 0`；`max < QueueSize`；`!descriptors_ || !available_ || !used_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
@brief VirtIO 块设备：提交块设备请求并等待请求完成。
@param operation 本次操作使用的 operation 值。
@param sector 本次操作使用的 sector 值。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`detail::fail` → `lock_.acquire` → `allocate_chain` → `detail::prepare_sleep` → `lock_.release` → `detail::sleep_process` → `detail::io_barrier` → `registers_.write32` → `free_chain`。
@details 本函数可见的状态更新包括：`buf0->type = VIRTIO_BLK_T_OUT;`；`buf0->type = VIRTIO_BLK_T_IN;`；`buf0->reserved = 0;`；`buf0->sector = sector;`；`available_->ring[available_->idx % QueueSize] = idx[0];`；`available_->idx += 1;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!data || count == 0 || count % SectorSize != 0`；`1`；`allocate_chain(idx) == 0`；`write`；`requests_[idx[0]].pending`。这些条件决定检查、重试或提前返回路径。
@note 源码中出现的锁操作：`lock_.acquire` → `lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@pre 等待路径必须遵守“条件锁下登记、释放条件锁后真正睡眠”的协议；不能携带无关自旋锁进入调度。是否可中断由具体登记入口决定。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
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
