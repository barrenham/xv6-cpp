# `kernel/drivers/tty/core/tty.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../../kernel/drivers/tty/core/tty.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:tty`。

直接模块依赖：`:character`, `:ldisc`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::drivers::Tty::Tty`](tty.cppm.md#fn-9a9492dd25010578)
- [`xv::drivers::Tty::hangup`](tty.cpp.md#fn-3acc0f0e4a2096c9)
- [`xv::drivers::Tty::initialize`](tty.cpp.md#fn-2a48d8f477ebb1b7)
- [`xv::drivers::Tty::inject`](tty.cpp.md#fn-258d0222cee0993a)
- [`xv::drivers::Tty::input_writable`](tty.cpp.md#fn-ec3f40ec3ea02161)
- [`xv::drivers::Tty::ioctl`](tty.cpp.md#fn-3625e17fbe0b06ab)
- [`xv::drivers::Tty::name`](tty.cppm.md#fn-bd71d07141df44bc)
- [`xv::drivers::Tty::poll`](tty.cpp.md#fn-f52d9721bfdab4d5)
- [`xv::drivers::Tty::put_sync`](tty.cpp.md#fn-d96051693f210b8e)
- [`xv::drivers::Tty::read`](tty.cpp.md#fn-2b943588ba88bb06)
- [`xv::drivers::Tty::receive`](tty.cpp.md#fn-a871a5e9e5abafda)
- [`xv::drivers::Tty::timer_tick`](tty.cpp.md#fn-eaea2170608ca21d)
- [`xv::drivers::Tty::write`](tty.cpp.md#fn-7a0276d465b23c53)

<a id="fn-9a9492dd25010578"></a>

## `xv::drivers::Tty::Tty`

```doxygen
@fn constexpr xv::drivers::Tty::Tty(TerminalOutput &output, LineDiscipline &discipline)
@brief 构造终端核心，建立本重载要求的成员初值和借用关系。
@param output 行规程使用的输出后端。
@param discipline 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`output_` → `discipline_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-bd71d07141df44bc"></a>

## `xv::drivers::Tty::name`

```doxygen
@fn const char *xv::drivers::Tty::name() const override
@brief 终端核心：读取或计算 `"`，返回当前查询结果。
@return 各分支实际返回：无值返回。应结合上面的条件检查区分正常值、空值和错误值。
```
