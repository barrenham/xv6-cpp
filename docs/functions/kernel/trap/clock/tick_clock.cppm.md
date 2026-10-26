# `kernel/trap/clock/tick_clock.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/clock/tick_clock.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap:clock`。

直接模块依赖：`xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::clock`](../service.cpp.md#fn-7c77589a1cbc708a)
- [`xv::trap::TickClock::TickClock`](tick_clock.cppm.md#fn-1534a7aa73b688e2)
- [`xv::trap::TickClock::TickClock`](tick_clock.cppm.md#fn-5207fdf0bf983785)
- [`xv::trap::TickClock::initialize`](../tick-clock.md#clock-tickclock-initialize)
- [`xv::trap::TickClock::interrupt`](../tick-clock.md#clock-tickclock-interrupt)
- [`xv::trap::TickClock::operator=`](tick_clock.cppm.md#fn-f2bbe0fa1e621477)
- [`xv::trap::TickClock::pause`](../tick-clock.md#clock-tickclock-pause)
- [`xv::trap::TickClock::uptime`](../tick-clock.md#clock-tickclock-uptime)

<a id="fn-5207fdf0bf983785"></a>

## `xv::trap::TickClock::TickClock`

```doxygen
@fn constexpr explicit xv::trap::TickClock::TickClock(unsigned initial_ticks = 0)
@brief 构造全局 tick 时钟，建立本重载要求的成员初值和借用关系。
@param initial_ticks 本次操作使用的 initialticks 值。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`ticks_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-1534a7aa73b688e2"></a>

## `xv::trap::TickClock::TickClock`

```doxygen
@fn xv::trap::TickClock::TickClock(const TickClock &)
@brief 全局 tick 时钟的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-f2bbe0fa1e621477"></a>

## `xv::trap::TickClock::operator=`

```doxygen
@fn TickClock &xv::trap::TickClock::operator=(const TickClock &)
@brief 全局 tick 时钟的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
