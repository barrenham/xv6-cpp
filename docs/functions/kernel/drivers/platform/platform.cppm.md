# `kernel/drivers/platform/platform.cppm`

开发职责：设备对象、MMIO、设备中断与终端数据流。

源码：[打开实际文件](../../../../../kernel/drivers/platform/platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.drivers:platform`。

## 函数导航

- [`xv::drivers::detail::allocate_dma_page`](kernel_platform.cpp.md#fn-3e2497f9a8bb8a76)
- [`xv::drivers::detail::claim_terminal`](kernel_platform.cpp.md#fn-eaf6516203ead07d)
- [`xv::drivers::detail::copy_in`](kernel_platform.cpp.md#fn-5c0e6bc6380432aa)
- [`xv::drivers::detail::copy_out`](kernel_platform.cpp.md#fn-1db6985a841bb5f6)
- [`xv::drivers::detail::detach_terminal`](kernel_platform.cpp.md#fn-3d096ebff7f651ec)
- [`xv::drivers::detail::dump_processes`](kernel_platform.cpp.md#fn-ce42a49f7c7724a2)
- [`xv::drivers::detail::fail`](kernel_platform.cpp.md#fn-bd791d74ae9d922d)
- [`xv::drivers::detail::free_dma_page`](kernel_platform.cpp.md#fn-2d8e5b711a2c2aec)
- [`xv::drivers::detail::free_terminal_page`](kernel_platform.cpp.md#fn-f60843cb4cde0d78)
- [`xv::drivers::detail::hangup_terminal`](kernel_platform.cpp.md#fn-c1af15eaa31d5e6f)
- [`xv::drivers::detail::io_barrier`](kernel_platform.cpp.md#fn-eec65ff52575da85)
- [`xv::drivers::detail::may_read_terminal`](kernel_platform.cpp.md#fn-39a74ec50ac9a282)
- [`xv::drivers::detail::panicked`](kernel_platform.cpp.md#fn-735b378b676359d6)
- [`xv::drivers::detail::panicking`](kernel_platform.cpp.md#fn-128e2bdfa3322a07)
- [`xv::drivers::detail::pop_interrupts_off`](kernel_platform.cpp.md#fn-7af9793254c5b352)
- [`xv::drivers::detail::post_terminal_interrupt`](kernel_platform.cpp.md#fn-5d2f829ce13fa9d7)
- [`xv::drivers::detail::prepare_interruptible_sleep`](kernel_platform.cpp.md#fn-1412ba43bdcc972f)
- [`xv::drivers::detail::prepare_sleep`](kernel_platform.cpp.md#fn-57dda9859ef34a3a)
- [`xv::drivers::detail::process_killed`](kernel_platform.cpp.md#fn-9ced86bffdf3c3f0)
- [`xv::drivers::detail::push_interrupts_off`](kernel_platform.cpp.md#fn-71423e5a4445cf2e)
- [`xv::drivers::detail::service_terminal_signals`](kernel_platform.cpp.md#fn-2151fe9eb501d5da)
- [`xv::drivers::detail::set_terminal_foreground`](kernel_platform.cpp.md#fn-d17648e30d4df93c)
- [`xv::drivers::detail::sleep_process`](kernel_platform.cpp.md#fn-e032c04f840cb18d)
- [`xv::drivers::detail::terminal_foreground`](kernel_platform.cpp.md#fn-52f3583ddfb3543d)
- [`xv::drivers::detail::terminal_milliseconds`](kernel_platform.cpp.md#fn-bd48ea235f0c5241)
- [`xv::drivers::detail::wake_processes`](kernel_platform.cpp.md#fn-63de6592afe7b624)
- [`xv::drivers::detail::zero_page`](kernel_platform.cpp.md#fn-20570dfe40006a2d)
