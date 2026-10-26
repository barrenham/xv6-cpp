# `kernel/drivers/console/character_device.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/console/character_device.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:character`。

直接模块依赖：`:device`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::CharacterDevice::CharacterDevice`](character_device.cppm.md#fn-149998f34d50f2a8)
- [`xv::drivers::CharacterDevice::close`](character_device.cppm.md#fn-16e23ef5c5540a67)
- [`xv::drivers::CharacterDevice::ioctl`](character_device.cppm.md#fn-c85b367e0cb4f3d1)
- [`xv::drivers::CharacterDevice::poll`](character_device.cppm.md#fn-9885d48feba8adf5)
- [`xv::drivers::CharacterDevice::put_sync`](character_device.cppm.md#fn-621660e7b36c91d2)
- [`xv::drivers::CharacterDevice::read`](character_device.cppm.md#fn-0644cee71cd0ad7a)
- [`xv::drivers::CharacterDevice::write`](character_device.cppm.md#fn-c8346f8625b61be7)
- [`xv::drivers::CharacterDevice::~CharacterDevice`](character_device.cppm.md#fn-0906a5cdc1e9b50d)

<a id="fn-0644cee71cd0ad7a"></a>

## `xv::drivers::CharacterDevice::read`

```doxygen
@fn virtual int xv::drivers::CharacterDevice::read(IoBuffer destination, int count)
@brief 设备对象、MMIO、设备中断与终端数据流：读取已有数据并按接口返回读取结果。
@param destination 接收结果的缓冲或目标对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (IoBuffer, int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-c8346f8625b61be7"></a>

## `xv::drivers::CharacterDevice::write`

```doxygen
@fn virtual int xv::drivers::CharacterDevice::write(IoBuffer source, int count)
@brief 设备对象、MMIO、设备中断与终端数据流：提交输入数据并报告传输结果。
@param source 输入数据位置或源对象。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (IoBuffer, int)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-621660e7b36c91d2"></a>

## `xv::drivers::CharacterDevice::put_sync`

```doxygen
@fn virtual void xv::drivers::CharacterDevice::put_sync(int character)
@brief 设备对象、MMIO、设备中断与终端数据流：通过不能睡眠的输出路径提交一个字符。
@param character 要输出的单个字符值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-9885d48feba8adf5"></a>

## `xv::drivers::CharacterDevice::poll`

```doxygen
@fn virtual unsigned xv::drivers::CharacterDevice::poll(xv::sync::PollTable * = nullptr)
@brief 设备对象、MMIO、设备中断与终端数据流：查询当前就绪状态，并在提供表时登记通知依赖。
@return 各分支实际返回：`0`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-16e23ef5c5540a67"></a>

## `xv::drivers::CharacterDevice::close`

```doxygen
@fn virtual void xv::drivers::CharacterDevice::close()
@brief 设备对象、MMIO、设备中断与终端数据流：关闭当前端点或释放文件关联。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c85b367e0cb4f3d1"></a>

## `xv::drivers::CharacterDevice::ioctl`

```doxygen
@fn virtual int xv::drivers::CharacterDevice::ioctl(unsigned, IoBuffer)
@brief 设备对象、MMIO、设备中断与终端数据流：分派设备控制请求并处理参数复制。
@return 各分支实际返回：`-1`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-149998f34d50f2a8"></a>

## `xv::drivers::CharacterDevice::CharacterDevice`

```doxygen
@fn constexpr xv::drivers::CharacterDevice::CharacterDevice()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0906a5cdc1e9b50d"></a>

## `xv::drivers::CharacterDevice::~CharacterDevice`

```doxygen
@fn xv::drivers::CharacterDevice::~CharacterDevice()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
