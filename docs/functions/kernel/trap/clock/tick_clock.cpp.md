# `kernel/trap/clock/tick_clock.cpp`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/clock/tick_clock.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap`。

直接模块依赖：`:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::TickClock::initialize`](../tick-clock.md#clock-tickclock-initialize)
- [`xv::trap::TickClock::interrupt`](../tick-clock.md#clock-tickclock-interrupt)
- [`xv::trap::TickClock::pause`](../tick-clock.md#clock-tickclock-pause)
- [`xv::trap::TickClock::uptime`](../tick-clock.md#clock-tickclock-uptime)
