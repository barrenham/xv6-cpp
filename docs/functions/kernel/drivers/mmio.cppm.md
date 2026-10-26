# `kernel/drivers/mmio.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../kernel/drivers/mmio.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:mmio`。

## 函数导航

- [`xv::drivers::detail::MmioRegion::MmioRegion`](mmio.cppm.md#fn-17850f7bd597acb9)
- [`xv::drivers::detail::MmioRegion::read32`](mmio.cppm.md#fn-6105f6658173eec1)
- [`xv::drivers::detail::MmioRegion::read8`](mmio.cppm.md#fn-e3d07c696b5c2182)
- [`xv::drivers::detail::MmioRegion::write32`](mmio.cppm.md#fn-1a37fe6ca0596ebf)
- [`xv::drivers::detail::MmioRegion::write8`](mmio.cppm.md#fn-5bdccfc37857fbbf)

<a id="fn-17850f7bd597acb9"></a>

## `xv::drivers::detail::MmioRegion::MmioRegion`

```doxygen
@fn constexpr explicit xv::drivers::detail::MmioRegion::MmioRegion(unsigned long base)
@brief 构造内存映射设备寄存器区间，建立本重载要求的成员初值和借用关系。
@param base 本次操作使用的 base 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`base_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e3d07c696b5c2182"></a>

## `xv::drivers::detail::MmioRegion::read8`

```doxygen
@fn unsigned char xv::drivers::detail::MmioRegion::read8(unsigned offset) const
@brief 内存映射设备寄存器区间：读取或计算 `*reinterpret_cast<volatile unsigned char *>(base_ + offset)`，返回当前查询结果。
@param offset 本次操作使用的 偏移 值。
@return 各分支实际返回：`*reinterpret_cast<volatile unsigned char *>(base_ + offset)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-5bdccfc37857fbbf"></a>

## `xv::drivers::detail::MmioRegion::write8`

```doxygen
@fn void xv::drivers::detail::MmioRegion::write8(unsigned offset, unsigned char value) const
@brief 内存映射设备寄存器区间：write8。
@param offset 本次操作使用的 偏移 值。
@param value 本次操作使用的 value 值。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6105f6658173eec1"></a>

## `xv::drivers::detail::MmioRegion::read32`

```doxygen
@fn unsigned xv::drivers::detail::MmioRegion::read32(unsigned offset) const
@brief 内存映射设备寄存器区间：读取或计算 `*reinterpret_cast<volatile unsigned *>(base_ + offset)`，返回当前查询结果。
@param offset 本次操作使用的 偏移 值。
@return 各分支实际返回：`*reinterpret_cast<volatile unsigned *>(base_ + offset)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1a37fe6ca0596ebf"></a>

## `xv::drivers::detail::MmioRegion::write32`

```doxygen
@fn void xv::drivers::detail::MmioRegion::write32(unsigned offset, unsigned value) const
@brief 内存映射设备寄存器区间：write32。
@param offset 本次操作使用的 偏移 值。
@param value 本次操作使用的 value 值。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
