# `kernel/drivers/tty/ldisc/n_tty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/ldisc/n_tty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:n_tty`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::NttyDiscipline::available`](n_tty.cpp.md#fn-7651ef1094a5143f)
- [`xv::drivers::NttyDiscipline::change_mode`](n_tty.cpp.md#fn-c066f78dfc35a199)
- [`xv::drivers::NttyDiscipline::consume`](n_tty.cpp.md#fn-bed37ea7e5b327f2)
- [`xv::drivers::NttyDiscipline::erase`](n_tty.cpp.md#fn-8229487820db48a0)
- [`xv::drivers::NttyDiscipline::flush`](n_tty.cpp.md#fn-f9f25d75c4f5be4f)
- [`xv::drivers::NttyDiscipline::front`](n_tty.cpp.md#fn-9cbd8077f9a49e96)
- [`xv::drivers::NttyDiscipline::input_room`](n_tty.cpp.md#fn-0f11390be0d4b82f)
- [`xv::drivers::NttyDiscipline::readable_bytes`](n_tty.cpp.md#fn-dc3eb47d5aa1609c)
- [`xv::drivers::NttyDiscipline::receive`](n_tty.cpp.md#fn-c7aed35643d5fe35)
- [`xv::drivers::echo`](n_tty.cpp.md#fn-a268947349432388)
- [`xv::drivers::matches`](n_tty.cpp.md#fn-78f40d0f64625d84)
- [`xv::drivers::word_space`](n_tty.cpp.md#fn-e8d2a41047722c39)

<a id="fn-78f40d0f64625d84"></a>

## `xv::drivers::matches`

```doxygen
@fn bool xv::drivers::matches(unsigned char byte, const termios &settings, unsigned index)
@brief 设备对象、MMIO、设备中断与终端数据流：判断候选对象是否符合本次查找条件。
@param byte 本次输入字节。
@param settings 当前终端模式与控制字符配置。
@param index 数组、CPU 或页面的索引。
@return 各分支实际返回：`settings.c_cc[index] != 0 && byte == settings.c_cc[index]`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e8d2a41047722c39"></a>

## `xv::drivers::word_space`

```doxygen
@fn bool xv::drivers::word_space(unsigned char byte)
@brief 设备对象、MMIO、设备中断与终端数据流：读取或计算 `byte == ' ' || byte == '\t'`，返回当前查询结果。
@param byte 本次输入字节。
@return 各分支实际返回：`byte == ' ' || byte == '\t'`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-a268947349432388"></a>

## `xv::drivers::echo`

```doxygen
@fn void xv::drivers::echo(unsigned char byte, const termios &settings, TerminalOutput &serial)
@brief 设备对象、MMIO、设备中断与终端数据流：echo。
@param byte 本次输入字节。
@param settings 当前终端模式与控制字符配置。
@param serial 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`serial.put_sync`。
@details 需要特别跟踪的条件：`(settings.c_lflag & ECHOCTL) && (byte < 32 || byte == 127) && byte != '\n' && byte != '\t'`；`byte == '\n' && (settings.c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-0f11390be0d4b82f"></a>

## `xv::drivers::NttyDiscipline::input_room`

```doxygen
@fn bool xv::drivers::NttyDiscipline::input_room(bool canonical) const
@brief 终端行规程：判断终端输入缓冲是否还能接受输入。
@param canonical 本次操作使用的 canonical 值。
@return 各分支实际返回：`head_ - tail_ < Capacity && (!canonical || head_ - tail_ < Capacity - 1 || committed_ == tail_)`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-7651ef1094a5143f"></a>

## `xv::drivers::NttyDiscipline::available`

```doxygen
@fn unsigned xv::drivers::NttyDiscipline::available() const
@brief 终端行规程：返回已提交且尚未消费的缓冲项数量。
@return 各分支实际返回：`committed_ - tail_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-dc3eb47d5aa1609c"></a>

## `xv::drivers::NttyDiscipline::readable_bytes`

```doxygen
@fn unsigned xv::drivers::NttyDiscipline::readable_bytes() const
@brief 终端行规程：可读数据字节区域。
@details 需要特别跟踪的条件：`!ring_[i % Capacity].eof`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`count`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-9cbd8077f9a49e96"></a>

## `xv::drivers::NttyDiscipline::front`

```doxygen
@fn TerminalByte xv::drivers::NttyDiscipline::front() const
@brief 终端行规程：读取或计算 `ring_[tail_ % Capacity]`，返回当前查询结果。
@return 各分支实际返回：`ring_[tail_ % Capacity]`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-bed37ea7e5b327f2"></a>

## `xv::drivers::NttyDiscipline::consume`

```doxygen
@fn void xv::drivers::NttyDiscipline::consume()
@brief 终端行规程：消费当前缓冲项或待处理信号。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f9f25d75c4f5be4f"></a>

## `xv::drivers::NttyDiscipline::flush`

```doxygen
@fn void xv::drivers::NttyDiscipline::flush()
@brief 终端行规程：清空当前输入记录或缓冲状态。
@details 本函数可见的状态更新包括：`tail_ = committed_ = head_ = 0;`；`committed_ = head_ = 0;`；`head_ = 0;`；`literal_ = false;`。局部临时变量与对象字段应按作用域区分。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-c066f78dfc35a199"></a>

## `xv::drivers::NttyDiscipline::change_mode`

```doxygen
@fn void xv::drivers::NttyDiscipline::change_mode(bool canonical)
@brief 终端行规程：在规范与非规范模式之间调整缓冲解释。
@param canonical 本次操作使用的 canonical 值。
@details 本函数可见的状态更新包括：`byte.end = false;`；`ring_[destination++ % Capacity] = byte;`；`head_ = committed_ = destination;`；`committed_ = destination;`；`ring_[(destination - 1) % Capacity].end = true;`；`literal_ = false;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`!byte.eof`；`canonical && destination != tail_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-8229487820db48a0"></a>

## `xv::drivers::NttyDiscipline::erase`

```doxygen
@fn void xv::drivers::NttyDiscipline::erase(const termios &settings, TerminalOutput &output)
@brief 终端行规程：执行行规程的删除与相应回显。
@param settings 当前终端模式与控制字符配置。
@param output 行规程使用的输出后端。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`output.put_sync` → `echo`。
@details 需要特别跟踪的条件：`head_ == committed_`；`!(settings.c_lflag & ECHO)`；`settings.c_lflag & ECHOE`；`width--`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-c7aed35643d5fe35"></a>

## `xv::drivers::NttyDiscipline::receive`

```doxygen
@fn InputAction xv::drivers::NttyDiscipline::receive(unsigned char byte, const termios &settings, TerminalOutput &output)
@brief 终端行规程：按终端模式解释输入并更新记录缓冲。
@param byte 本次输入字节。
@param settings 当前终端模式与控制字符配置。
@param output 行规程使用的输出后端。
@details 先依据输入模式转换字节，再处理 literal-next、编辑、信号控制字符和 EOF/换行记录边界，必要时回显。规范行始终预留结束符槽，超长行继续采用截断策略。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`matches` → `flush` → `echo` → `erase` → `word_space`。
@details 本函数可见的状态更新包括：`literal_ = false;`；`literal_ = true;`；`head_ = committed_;`；`literal_ = quoted;`；`ring_[head_++ % Capacity] = {byte, end, eof};`；`committed_ = head_;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`settings.c_iflag & ISTRIP`；`byte == '\r'`；`settings.c_iflag & IGNCR`；`settings.c_iflag & ICRNL`；`byte == '\n' && (settings.c_iflag & INLCR)`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`InputAction::None`；`suspend ? InputAction::Suspend : InputAction::Interrupt`；`InputAction::Retry`。应结合上面的条件检查区分正常值、空值和错误值。
```
