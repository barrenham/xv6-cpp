# `kernel/drivers/device.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../kernel/drivers/device.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:device`。

## 函数导航

- [`xv::drivers::Device::Device`](device.cppm.md#fn-5f058c0a31241662)
- [`xv::drivers::Device::Device`](device.cppm.md#fn-bb4c1f66fd9372c5)
- [`xv::drivers::Device::initialize`](device.cppm.md#fn-215e0b0a4c7703e5)
- [`xv::drivers::Device::name`](device.cppm.md#fn-1241c6c263252505)
- [`xv::drivers::Device::operator=`](device.cppm.md#fn-cd63a080ede12176)
- [`xv::drivers::Device::~Device`](device.cppm.md#fn-ef3b98c6cd7a61d1)

<a id="fn-bb4c1f66fd9372c5"></a>

## `xv::drivers::Device::Device`

```doxygen
@fn xv::drivers::Device::Device(const Device &)
@brief 设备对象、MMIO、设备中断与终端数据流的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-cd63a080ede12176"></a>

## `xv::drivers::Device::operator=`

```doxygen
@fn Device &xv::drivers::Device::operator=(const Device &)
@brief 设备对象、MMIO、设备中断与终端数据流的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-1241c6c263252505"></a>

## `xv::drivers::Device::name`

```doxygen
@fn virtual const char *xv::drivers::Device::name() const
@brief 设备对象、MMIO、设备中断与终端数据流：名称。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `const char *() const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-215e0b0a4c7703e5"></a>

## `xv::drivers::Device::initialize`

```doxygen
@fn virtual void xv::drivers::Device::initialize()
@brief 设备对象、MMIO、设备中断与终端数据流：建立该对象开始工作所需的初始状态。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-5f058c0a31241662"></a>

## `xv::drivers::Device::Device`

```doxygen
@fn constexpr xv::drivers::Device::Device()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ef3b98c6cd7a61d1"></a>

## `xv::drivers::Device::~Device`

```doxygen
@fn xv::drivers::Device::~Device()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
