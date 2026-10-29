# `kernel/drivers/tty/ldisc/n_tty.cpp`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/ldisc/n_tty.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers`。

直接模块依赖：`:n_tty`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::NttyDiscipline::ReceiveFlow::delimiter`](n_tty.cpp.md#fn-223080924420d798)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::editing`](n_tty.cpp.md#fn-4b32bf519586be91)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::erase`](n_tty.cpp.md#fn-9af9b34c152168b9)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::erasing`](n_tty.cpp.md#fn-de86024e93408531)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::full`](n_tty.cpp.md#fn-8af3ed7a476f90bf)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::ignored`](n_tty.cpp.md#fn-4fd6aa3e7bb599cc)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::kill`](n_tty.cpp.md#fn-857489b665b2efb7)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::killing`](n_tty.cpp.md#fn-378a2a87bfd6b289)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::post_signal`](n_tty.cpp.md#fn-c51714e3eac83381)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::publish`](n_tty.cpp.md#fn-507345efcb0e81e5)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::quote`](n_tty.cpp.md#fn-73cb6d98b60214be)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::quoted_next`](n_tty.cpp.md#fn-03485f2f7a0c1012)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::room`](n_tty.cpp.md#fn-dc32473452da33dd)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::signal`](n_tty.cpp.md#fn-3aa4e0d844640dc9)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::state`](n_tty.cpp.md#fn-c813838350d1a8d0)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::translate`](n_tty.cpp.md#fn-b68c218517d870f4)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::word_erase`](n_tty.cpp.md#fn-2e1e79dff4562efc)
- [`xv::drivers::NttyDiscipline::ReceiveFlow::word_erasing`](n_tty.cpp.md#fn-46701aeb9a21309b)
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
@return 各分支实际返回：`InputAction::None`；`suspend ? InputAction::Suspend : InputAction::Interrupt`；`InputAction::Retry`。应结合上面的条件检查区分正常值、空值和错误值。
@details 流程由 ReceiveFlow::Graph 的编译期策略连接；保留原资源、锁、返回值及事件消费协议。
```

<a id="fn-223080924420d798"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::delimiter`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::delimiter(Context &c) noexcept
@brief 计算本字节的 EOF 与记录终止标志。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4b32bf519586be91"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::editing`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::editing(Context &c) noexcept
@brief 检查本字节是否进入非引用的规范编辑。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-9af9b34c152168b9"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::erase`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::erase(Context &c) noexcept
@brief 按原行规程删除一个待提交字节及回显。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-de86024e93408531"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::erasing`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::erasing(Context &c) noexcept
@brief 检查是否匹配擦除字符。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-8af3ed7a476f90bf"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::full`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::full(Context &c) noexcept
@brief 有待消费记录时恢复引用标志并返回 Retry，否则保持原丢弃规则。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-4fd6aa3e7bb599cc"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::ignored`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::ignored(Context &c) noexcept
@brief 检查输入是否被 IGNCR 丢弃。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-857489b665b2efb7"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::kill`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::kill(Context &c) noexcept
@brief 按原回显策略直接清除未提交行。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-378a2a87bfd6b289"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::killing`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::killing(Context &c) noexcept
@brief 检查是否匹配整行删除字符。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c51714e3eac83381"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::post_signal`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::post_signal(Context &c) noexcept
@brief 清输入并按设置回显，返回原中断或停止动作。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-507345efcb0e81e5"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::publish`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::publish(Context &c) noexcept
@brief 写入环、按规则回显并在记录可见时发布 committed。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-73cb6d98b60214be"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::quote`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::quote(Context &c) noexcept
@brief 为下一个输入字节设置引用标志。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-03485f2f7a0c1012"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::quoted_next`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::quoted_next(Context &c) noexcept
@brief 检查是否启用并匹配 literal-next。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-dc32473452da33dd"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::room`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::room(Context &c) noexcept
@brief 按原预留分隔槽规则检查环容量。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-3aa4e0d844640dc9"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::signal`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::signal(Context &c) noexcept
@brief 按原控制字符优先级检查前台中断或停止动作。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-c813838350d1a8d0"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::state`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::state(Context &c) noexcept
@brief 保存规范与引用状态并按原时机清 literal-next。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-b68c218517d870f4"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::translate`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::translate(Context &c) noexcept
@brief 应用 ISTRIP 和 CR/NL 规则，忽略 CR 时不清 literal-next。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-2e1e79dff4562efc"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::word_erase`

```doxygen
@fn static void xv::drivers::NttyDiscipline::ReceiveFlow::word_erase(Context &c) noexcept
@brief 直接先擦空白再擦词，保持原回显次序。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```

<a id="fn-46701aeb9a21309b"></a>

## `xv::drivers::NttyDiscipline::ReceiveFlow::word_erasing`

```doxygen
@fn static bool xv::drivers::NttyDiscipline::ReceiveFlow::word_erasing(Context &c) noexcept
@brief 检查扩展字删除控制字符。
@details 调用者持 TTY 状态锁；按原输入转换、literal-next、信号、编辑、分隔和容量顺序处理一个字节。规范编辑和环数据搬移保留紧凑有界循环，Retry 只恢复原引用标志，不复用先前动作结果。 下一节点由图策略声明，业务结果与控制信号分离。
```
