# `kernel/trap/handlers/handlers.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/handlers/handlers.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap:handlers`。

直接模块依赖：`:dispatch`, `:clock`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::ExternalInterruptHandler::ExternalInterruptHandler`](handlers.cppm.md#fn-182283d57fd978bf)
- [`xv::trap::ExternalInterruptHandler::accepts`](external_interrupt.cpp.md#fn-3dbf6fac56b35930)
- [`xv::trap::ExternalInterruptHandler::handle`](external_interrupt.cpp.md#fn-1178044b9798eeb1)
- [`xv::trap::ExternalInterruptHandler::name`](handlers.cppm.md#fn-21ef6a1deef20de4)
- [`xv::trap::PageFaultHandler::PageFaultHandler`](handlers.cppm.md#fn-1e8a3601d8a9081f)
- [`xv::trap::PageFaultHandler::accepts`](page_fault.cpp.md#fn-fd588082eee9ffc3)
- [`xv::trap::PageFaultHandler::handle`](page_fault.cpp.md#fn-b2145e70bef0bcc2)
- [`xv::trap::PageFaultHandler::name`](handlers.cppm.md#fn-4f2bd2390aa9077d)
- [`xv::trap::SystemCallHandler::SystemCallHandler`](handlers.cppm.md#fn-32a689f0da7e3ec9)
- [`xv::trap::SystemCallHandler::accepts`](system_call.cpp.md#fn-9f95e4acf0aa1611)
- [`xv::trap::SystemCallHandler::handle`](system_call.cpp.md#fn-92d0af62cc7bf29f)
- [`xv::trap::SystemCallHandler::name`](handlers.cppm.md#fn-16a5361fab05cdcf)
- [`xv::trap::TimerInterruptHandler::TimerInterruptHandler`](handlers.cppm.md#fn-f894a8eb7966d548)
- [`xv::trap::TimerInterruptHandler::accepts`](timer_interrupt.cpp.md#fn-8c99301b45685008)
- [`xv::trap::TimerInterruptHandler::handle`](timer_interrupt.cpp.md#fn-aead5b95281b47a7)
- [`xv::trap::TimerInterruptHandler::name`](handlers.cppm.md#fn-97ef27dc99551431)

<a id="fn-32a689f0da7e3ec9"></a>

## `xv::trap::SystemCallHandler::SystemCallHandler`

```doxygen
@fn constexpr xv::trap::SystemCallHandler::SystemCallHandler()
@brief 构造用户系统调用处理器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-16a5361fab05cdcf"></a>

## `xv::trap::SystemCallHandler::name`

```doxygen
@fn const char *xv::trap::SystemCallHandler::name() const override
@brief 用户系统调用处理器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-182283d57fd978bf"></a>

## `xv::trap::ExternalInterruptHandler::ExternalInterruptHandler`

```doxygen
@fn constexpr xv::trap::ExternalInterruptHandler::ExternalInterruptHandler()
@brief 构造外部中断处理器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-21ef6a1deef20de4"></a>

## `xv::trap::ExternalInterruptHandler::name`

```doxygen
@fn const char *xv::trap::ExternalInterruptHandler::name() const override
@brief 外部中断处理器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-f894a8eb7966d548"></a>

## `xv::trap::TimerInterruptHandler::TimerInterruptHandler`

```doxygen
@fn constexpr explicit xv::trap::TimerInterruptHandler::TimerInterruptHandler(TickClock &clock)
@brief 构造定时中断处理器，建立本重载要求的成员初值和借用关系。
@param clock 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`clock_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-97ef27dc99551431"></a>

## `xv::trap::TimerInterruptHandler::name`

```doxygen
@fn const char *xv::trap::TimerInterruptHandler::name() const override
@brief 定时中断处理器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-1e8a3601d8a9081f"></a>

## `xv::trap::PageFaultHandler::PageFaultHandler`

```doxygen
@fn constexpr xv::trap::PageFaultHandler::PageFaultHandler()
@brief 构造用户缺页处理器，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-4f2bd2390aa9077d"></a>

## `xv::trap::PageFaultHandler::name`

```doxygen
@fn const char *xv::trap::PageFaultHandler::name() const override
@brief 用户缺页处理器：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
