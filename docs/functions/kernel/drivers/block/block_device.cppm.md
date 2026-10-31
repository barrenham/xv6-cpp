# `kernel/drivers/block/block_device.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/block/block_device.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:block`。

直接模块依赖：`:interrupt`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::BlockDevice::BlockDevice`](block_device.cppm.md#fn-372b368d601421b4)
- [`xv::drivers::BlockDevice::flush`](block_device.cppm.md#fn-5896623bca3f11d6)
- [`xv::drivers::BlockDevice::transfer`](block_device.cppm.md#fn-2601db1d20c13767)
- [`xv::drivers::BlockDevice::~BlockDevice`](block_device.cppm.md#fn-e35365167e433243)

<a id="fn-2601db1d20c13767"></a>

## `xv::drivers::BlockDevice::transfer`

```doxygen
@fn virtual void xv::drivers::BlockDevice::transfer(BlockOperation operation, unsigned long sector, void *data, unsigned count)
@brief 设备对象、MMIO、设备中断与终端数据流：提交块设备请求并等待请求完成。
@param operation 本次操作使用的 operation 值。
@param sector 本次操作使用的 sector 值。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 本次操作的数量上限。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-372b368d601421b4"></a>

## `xv::drivers::BlockDevice::BlockDevice`

```doxygen
@fn constexpr xv::drivers::BlockDevice::BlockDevice()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e35365167e433243"></a>

## `xv::drivers::BlockDevice::~BlockDevice`

```doxygen
@fn xv::drivers::BlockDevice::~BlockDevice()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5896623bca3f11d6"></a>

## `xv::drivers::BlockDevice::flush`

```doxygen
@fn virtual void xv::drivers::BlockDevice::flush()
@brief 提交设备的持久化屏障。
@details 默认不执行操作；支持持久写缓存的驱动须覆写，调用者不能把默认实现视为硬件落盘保证。
```
<a id="fn-bda7dac14ba4c3cb"></a>

## `xv::drivers::BlockDevice::capacity_sectors`

```doxygen
@fn virtual unsigned long xv::drivers::BlockDevice::capacity_sectors() const
@brief 返回设备真实的64位512字节扇区数。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
