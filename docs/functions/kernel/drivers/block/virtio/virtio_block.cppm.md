# `kernel/drivers/block/virtio/virtio_block.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/block/virtio/virtio_block.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:virtio`。

直接模块依赖：`:protocol`, `:block`, `:mmio`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::VirtioBlock::VirtioBlock`](virtio_block.cppm.md#fn-b23636ada5f36433)
- [`xv::drivers::VirtioBlock::allocate_chain`](virtio_block.cpp.md#fn-f8ec5a9a894c4d73)
- [`xv::drivers::VirtioBlock::allocate_descriptor`](virtio_block.cpp.md#fn-ec9a5a1df2982759)
- [`xv::drivers::VirtioBlock::free_chain`](virtio_block.cpp.md#fn-e4fe48e63edf2388)
- [`xv::drivers::VirtioBlock::free_descriptor`](virtio_block.cpp.md#fn-bfd4e838d44d17f4)
- [`xv::drivers::VirtioBlock::handle_interrupt`](virtio_block.cpp.md#fn-0b67b739b5b86885)
- [`xv::drivers::VirtioBlock::initialize`](virtio_block.cpp.md#fn-4c7be18d8f3eca1c)
- [`xv::drivers::VirtioBlock::irq`](virtio_block.cppm.md#fn-a613accbdfb69602)
- [`xv::drivers::VirtioBlock::name`](virtio_block.cppm.md#fn-cd18c146369ac28e)
- [`xv::drivers::VirtioBlock::transfer`](virtio_block.cpp.md#fn-266fcc4568332ef4)

<a id="fn-b23636ada5f36433"></a>

## `xv::drivers::VirtioBlock::VirtioBlock`

```doxygen
@fn constexpr xv::drivers::VirtioBlock::VirtioBlock(unsigned long base, unsigned irq)
@brief 构造VirtIO 块设备，建立本重载要求的成员初值和借用关系。
@param base 本次操作使用的 base 值。
@param irq 本次操作使用的 中断号 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`registers_` → `irq_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-cd18c146369ac28e"></a>

## `xv::drivers::VirtioBlock::name`

```doxygen
@fn const char *xv::drivers::VirtioBlock::name() const override
@brief VirtIO 块设备：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a613accbdfb69602"></a>

## `xv::drivers::VirtioBlock::irq`

```doxygen
@fn unsigned xv::drivers::VirtioBlock::irq() const override
@brief VirtIO 块设备：读取或计算 `irq_`，返回当前查询结果。
@return 各分支实际返回：`irq_`。应结合上面的条件检查区分正常值、空值和错误值。
```
