# `kernel/sync/platform/platform.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/platform/platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:platform`。

## 函数导航

- [`xv::sync::detail::current_cpu`](kernel_bridge.cpp.md#fn-d83d20ff2ee4207e)
- [`xv::sync::detail::current_pid`](kernel_bridge.cpp.md#fn-c7137ed24713e0b5)
- [`xv::sync::detail::fail`](kernel_bridge.cpp.md#fn-d2c91545c0707296)
- [`xv::sync::detail::interrupted`](kernel_bridge.cpp.md#fn-34244a221cfe746d)
- [`xv::sync::detail::pop_interrupts_off`](kernel_bridge.cpp.md#fn-4b2a882b5e9104c1)
- [`xv::sync::detail::prepare_interruptible_sleep`](kernel_bridge.cpp.md#fn-f37ac8a65527566f)
- [`xv::sync::detail::prepare_sleep`](kernel_bridge.cpp.md#fn-78b07436e081d612)
- [`xv::sync::detail::push_interrupts_off`](kernel_bridge.cpp.md#fn-a6d34f68b527071c)
- [`xv::sync::detail::sleep_process`](kernel_bridge.cpp.md#fn-42fc3192bb67c511)
- [`xv::sync::detail::wake_processes`](kernel_bridge.cpp.md#fn-fed502f24b271bac)

