# `kernel/drivers/serial/serial_port.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/serial/serial_port.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:serial`。

直接模块依赖：`:interrupt`, `:output`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::InputReceiver::InputReceiver`](serial_port.cppm.md#fn-60a20caa88007cd2)
- [`xv::drivers::InputReceiver::receive`](serial_port.cppm.md#fn-81112387cd1b1921)
- [`xv::drivers::InputReceiver::~InputReceiver`](serial_port.cppm.md#fn-17d08be313bf6107)
- [`xv::drivers::SerialPort::SerialPort`](serial_port.cppm.md#fn-0d0e384a5bfb3d49)
- [`xv::drivers::SerialPort::attach`](serial_port.cppm.md#fn-c17996e4d322e56b)
- [`xv::drivers::SerialPort::drain`](serial_port.cppm.md#fn-753ae0dfcfd279a8)
- [`xv::drivers::SerialPort::put_sync`](serial_port.cppm.md#fn-d91dce9b4e1bfd3f)
- [`xv::drivers::SerialPort::transmit`](serial_port.cppm.md#fn-90855f8aaa562d1e)
- [`xv::drivers::SerialPort::write`](serial_port.cppm.md#fn-28c0c4ae89206e39)
- [`xv::drivers::SerialPort::~SerialPort`](serial_port.cppm.md#fn-d3bd2607d4dbefc6)

<a id="fn-81112387cd1b1921"></a>

## `xv::drivers::InputReceiver::receive`

```doxygen
@fn virtual void xv::drivers::InputReceiver::receive(int character)
@brief 设备对象、MMIO、设备中断与终端数据流：按终端模式解释输入并更新记录缓冲。
@param character 要输出的单个字符值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-60a20caa88007cd2"></a>

## `xv::drivers::InputReceiver::InputReceiver`

```doxygen
@fn constexpr xv::drivers::InputReceiver::InputReceiver()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-17d08be313bf6107"></a>

## `xv::drivers::InputReceiver::~InputReceiver`

```doxygen
@fn xv::drivers::InputReceiver::~InputReceiver()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c17996e4d322e56b"></a>

## `xv::drivers::SerialPort::attach`

```doxygen
@fn virtual void xv::drivers::SerialPort::attach(InputReceiver &receiver)
@brief 设备对象、MMIO、设备中断与终端数据流：关联后端对象、接收器或组成员。
@param receiver 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-28c0c4ae89206e39"></a>

## `xv::drivers::SerialPort::write`

```doxygen
@fn virtual void xv::drivers::SerialPort::write(const char *data, int count)
@brief 设备对象、MMIO、设备中断与终端数据流：提交输入数据并报告传输结果。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-90855f8aaa562d1e"></a>

## `xv::drivers::SerialPort::transmit`

```doxygen
@fn int xv::drivers::SerialPort::transmit(const char *data, int count, bool nonblocking = false) override
@brief 设备对象、MMIO、设备中断与终端数据流：向输出后端完整提交一个转换单元或小块。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`write`。
@details 需要特别跟踪的条件：`nonblocking`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`-EAGAIN`；`count`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-d91dce9b4e1bfd3f"></a>

## `xv::drivers::SerialPort::put_sync`

```doxygen
@fn void xv::drivers::SerialPort::put_sync(int character) override
@brief 设备对象、MMIO、设备中断与终端数据流：通过不能睡眠的输出路径提交一个字符。
@param character 要输出的单个字符值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-753ae0dfcfd279a8"></a>

## `xv::drivers::SerialPort::drain`

```doxygen
@fn void xv::drivers::SerialPort::drain() override
@brief 设备对象、MMIO、设备中断与终端数据流：等待已提交输出排空。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0d0e384a5bfb3d49"></a>

## `xv::drivers::SerialPort::SerialPort`

```doxygen
@fn constexpr xv::drivers::SerialPort::SerialPort()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d3bd2607d4dbefc6"></a>

## `xv::drivers::SerialPort::~SerialPort`

```doxygen
@fn xv::drivers::SerialPort::~SerialPort()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
