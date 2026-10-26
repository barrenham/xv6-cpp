# `kernel/drivers/tty/ldisc/line_discipline.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/ldisc/line_discipline.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:ldisc`。

直接模块依赖：`:serial`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::LineDiscipline::LineDiscipline`](line_discipline.cppm.md#fn-664ba0f270462cf6)
- [`xv::drivers::LineDiscipline::LineDiscipline`](line_discipline.cppm.md#fn-ca2ff521136cf23b)
- [`xv::drivers::LineDiscipline::available`](line_discipline.cppm.md#fn-9f2b6756a27afe2b)
- [`xv::drivers::LineDiscipline::change_mode`](line_discipline.cppm.md#fn-0495e685717d173b)
- [`xv::drivers::LineDiscipline::consume`](line_discipline.cppm.md#fn-c69969801d4bd85b)
- [`xv::drivers::LineDiscipline::flush`](line_discipline.cppm.md#fn-01d55086bf24cd28)
- [`xv::drivers::LineDiscipline::front`](line_discipline.cppm.md#fn-63e1181117d8aa1d)
- [`xv::drivers::LineDiscipline::input_room`](line_discipline.cppm.md#fn-fd3ec4f0cd5066ba)
- [`xv::drivers::LineDiscipline::operator=`](line_discipline.cppm.md#fn-5e7daac5c8190ca5)
- [`xv::drivers::LineDiscipline::readable_bytes`](line_discipline.cppm.md#fn-fa4fded9bdaead89)
- [`xv::drivers::LineDiscipline::receive`](line_discipline.cppm.md#fn-0e54044c97d193a3)
- [`xv::drivers::LineDiscipline::~LineDiscipline`](line_discipline.cppm.md#fn-55cda5c7bc32bacd)

<a id="fn-ca2ff521136cf23b"></a>

## `xv::drivers::LineDiscipline::LineDiscipline`

```doxygen
@fn xv::drivers::LineDiscipline::LineDiscipline(const LineDiscipline &)
@brief 设备对象、MMIO、设备中断与终端数据流的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-5e7daac5c8190ca5"></a>

## `xv::drivers::LineDiscipline::operator=`

```doxygen
@fn LineDiscipline &xv::drivers::LineDiscipline::operator=(const LineDiscipline &)
@brief 设备对象、MMIO、设备中断与终端数据流的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-0e54044c97d193a3"></a>

## `xv::drivers::LineDiscipline::receive`

```doxygen
@fn virtual InputAction xv::drivers::LineDiscipline::receive(unsigned char byte, const termios &settings, TerminalOutput &output)
@brief 设备对象、MMIO、设备中断与终端数据流：按终端模式解释输入并更新记录缓冲。
@param byte 本次输入字节。
@param settings 当前终端模式与控制字符配置。
@param output 行规程使用的输出后端。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `InputAction (unsigned char, const termios &, TerminalOutput &)`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-9f2b6756a27afe2b"></a>

## `xv::drivers::LineDiscipline::available`

```doxygen
@fn virtual unsigned xv::drivers::LineDiscipline::available() const
@brief 设备对象、MMIO、设备中断与终端数据流：检查资源是否可用，并按参数登记等待依赖。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `unsigned int () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-fd3ec4f0cd5066ba"></a>

## `xv::drivers::LineDiscipline::input_room`

```doxygen
@fn virtual bool xv::drivers::LineDiscipline::input_room(bool canonical) const
@brief 设备对象、MMIO、设备中断与终端数据流：判断终端输入缓冲是否还能接受输入。
@param canonical 本次操作使用的 canonical 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `bool (bool) const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-fa4fded9bdaead89"></a>

## `xv::drivers::LineDiscipline::readable_bytes`

```doxygen
@fn virtual unsigned xv::drivers::LineDiscipline::readable_bytes() const
@brief 设备对象、MMIO、设备中断与终端数据流：可读数据字节区域。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `unsigned int () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-63e1181117d8aa1d"></a>

## `xv::drivers::LineDiscipline::front`

```doxygen
@fn virtual TerminalByte xv::drivers::LineDiscipline::front() const
@brief 设备对象、MMIO、设备中断与终端数据流：队首项。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `TerminalByte () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-c69969801d4bd85b"></a>

## `xv::drivers::LineDiscipline::consume`

```doxygen
@fn virtual void xv::drivers::LineDiscipline::consume()
@brief 设备对象、MMIO、设备中断与终端数据流：消费当前缓冲项或待处理信号。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-01d55086bf24cd28"></a>

## `xv::drivers::LineDiscipline::flush`

```doxygen
@fn virtual void xv::drivers::LineDiscipline::flush()
@brief 设备对象、MMIO、设备中断与终端数据流：清空当前输入记录或缓冲状态。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-0495e685717d173b"></a>

## `xv::drivers::LineDiscipline::change_mode`

```doxygen
@fn virtual void xv::drivers::LineDiscipline::change_mode(bool canonical)
@brief 设备对象、MMIO、设备中断与终端数据流：在规范与非规范模式之间调整缓冲解释。
@param canonical 本次操作使用的 canonical 值。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-664ba0f270462cf6"></a>

## `xv::drivers::LineDiscipline::LineDiscipline`

```doxygen
@fn constexpr xv::drivers::LineDiscipline::LineDiscipline()
@brief 构造设备对象、MMIO、设备中断与终端数据流，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-55cda5c7bc32bacd"></a>

## `xv::drivers::LineDiscipline::~LineDiscipline`

```doxygen
@fn xv::drivers::LineDiscipline::~LineDiscipline()
@brief 结束设备对象、MMIO、设备中断与终端数据流的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
