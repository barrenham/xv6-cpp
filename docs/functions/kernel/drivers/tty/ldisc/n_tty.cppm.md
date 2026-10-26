# `kernel/drivers/tty/ldisc/n_tty.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/ldisc/n_tty.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:n_tty`。

直接模块依赖：`:ldisc`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::NttyDiscipline::NttyDiscipline`](n_tty.cppm.md#fn-766a5d849000f821)
- [`xv::drivers::NttyDiscipline::available`](n_tty.cpp.md#fn-7651ef1094a5143f)
- [`xv::drivers::NttyDiscipline::change_mode`](n_tty.cpp.md#fn-c066f78dfc35a199)
- [`xv::drivers::NttyDiscipline::consume`](n_tty.cpp.md#fn-bed37ea7e5b327f2)
- [`xv::drivers::NttyDiscipline::erase`](n_tty.cpp.md#fn-8229487820db48a0)
- [`xv::drivers::NttyDiscipline::flush`](n_tty.cpp.md#fn-f9f25d75c4f5be4f)
- [`xv::drivers::NttyDiscipline::front`](n_tty.cpp.md#fn-9cbd8077f9a49e96)
- [`xv::drivers::NttyDiscipline::input_room`](n_tty.cpp.md#fn-0f11390be0d4b82f)
- [`xv::drivers::NttyDiscipline::readable_bytes`](n_tty.cpp.md#fn-dc3eb47d5aa1609c)
- [`xv::drivers::NttyDiscipline::receive`](n_tty.cpp.md#fn-c7aed35643d5fe35)

<a id="fn-766a5d849000f821"></a>

## `xv::drivers::NttyDiscipline::NttyDiscipline`

```doxygen
@fn constexpr xv::drivers::NttyDiscipline::NttyDiscipline()
@brief 构造终端行规程，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
