# `kernel/drivers/tty/core/terminal_output.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/core/terminal_output.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:output`。

直接模块依赖：`xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::TerminalOutput::TerminalOutput`](terminal_output.cppm.md#fn-8396f282c12f02ed)
- [`xv::drivers::TerminalOutput::drain`](terminal_output.cppm.md#fn-fd8664acf73eca1d)
- [`xv::drivers::TerminalOutput::put_sync`](terminal_output.cppm.md#fn-a0a3ad603d839a89)
- [`xv::drivers::TerminalOutput::transmit`](terminal_output.cppm.md#fn-bdab7d048b1f161d)
- [`xv::drivers::TerminalOutput::writable`](terminal_output.cppm.md#fn-2a0d1ec5e6e26921)
- [`xv::drivers::TerminalOutput::~TerminalOutput`](terminal_output.cppm.md#fn-196648d6ce44e6ae)

<a id="fn-bdab7d048b1f161d"></a>

## `xv::drivers::TerminalOutput::transmit`

```doxygen
@fn virtual int xv::drivers::TerminalOutput::transmit(const char *data, int count, bool nonblocking = false)
@brief 设备对象、MMIO、设备中断与终端数据流：向输出后端完整提交一个转换单元或小块。
@param data 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param count 请求传输的字节数上限；成功与错误约定见本函数的返回分支。
@param nonblocking 为真时，资源暂不可用不得进入睡眠等待。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `int (const char *, int, bool)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-a0a3ad603d839a89"></a>

## `xv::drivers::TerminalOutput::put_sync`

```doxygen
@fn virtual void xv::drivers::TerminalOutput::put_sync(int character)
@brief 设备对象、MMIO、设备中断与终端数据流：通过不能睡眠的输出路径提交一个字符。
@param character 要输出的单个字符值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-fd8664acf73eca1d"></a>

## `xv::drivers::TerminalOutput::drain`

```doxygen
@fn virtual void xv::drivers::TerminalOutput::drain()
@brief 设备对象、MMIO、设备中断与终端数据流：等待已提交输出排空。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-2a0d1ec5e6e26921"></a>

## `xv::drivers::TerminalOutput::writable`

```doxygen
@fn virtual bool xv::drivers::TerminalOutput::writable(xv::sync::PollTable * = nullptr)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `true`，返回当前查询结果。
@return 各分支实际返回：`true`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-8396f282c12f02ed"></a>

## `xv::drivers::TerminalOutput::TerminalOutput`

```doxygen
@fn constexpr xv::drivers::TerminalOutput::TerminalOutput()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-196648d6ce44e6ae"></a>

## `xv::drivers::TerminalOutput::~TerminalOutput`

```doxygen
@fn xv::drivers::TerminalOutput::~TerminalOutput()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
