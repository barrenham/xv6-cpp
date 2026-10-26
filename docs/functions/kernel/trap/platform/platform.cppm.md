# `kernel/trap/platform/platform.cppm`

开发职责：陷阱快照、事件分派、设备服务及特权级返回。

源码：[打开实际文件](../../../../../kernel/trap/platform/platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.trap:platform`。

直接模块依赖：`:event`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::trap::detail::advance_user_pc`](../arch/riscv/kernel_platform.cpp.md#fn-b6c74a0b72429492)
- [`xv::trap::detail::arm_timer`](../arch/riscv/kernel_platform.cpp.md#fn-6d371a07cbcfe6cf)
- [`xv::trap::detail::deliver_signals`](../arch/riscv/kernel_platform.cpp.md#fn-4f041a4936f5176b)
- [`xv::trap::detail::dispatch_external_interrupt`](../arch/riscv/kernel_platform.cpp.md#fn-c6137e39ef8b7fbc)
- [`xv::trap::detail::dispatch_system_call`](../arch/riscv/kernel_platform.cpp.md#fn-4786b99366f6d39f)
- [`xv::trap::detail::enable_interrupts`](../arch/riscv/kernel_platform.cpp.md#fn-111982e6acd2b642)
- [`xv::trap::detail::enter_user`](../arch/riscv/kernel_platform.cpp.md#fn-c9c56524b5226794)
- [`xv::trap::detail::exit_current`](../arch/riscv/kernel_platform.cpp.md#fn-f003afe98a9ac7f7)
- [`xv::trap::detail::fatal`](../arch/riscv/kernel_platform.cpp.md#fn-c1ce930979d608fe)
- [`xv::trap::detail::hart_id`](../arch/riscv/kernel_platform.cpp.md#fn-64f75db4e245584e)
- [`xv::trap::detail::has_process`](../arch/riscv/kernel_platform.cpp.md#fn-7efdcba7acebcf8f)
- [`xv::trap::detail::install_kernel_vector`](../arch/riscv/kernel_platform.cpp.md#fn-13fcad96f6c4f2b3)
- [`xv::trap::detail::interrupted`](../arch/riscv/kernel_platform.cpp.md#fn-1841e13c7500b772)
- [`xv::trap::detail::killed`](../arch/riscv/kernel_platform.cpp.md#fn-d6ca1daf3715bdb9)
- [`xv::trap::detail::prepare_interruptible_sleep`](../arch/riscv/kernel_platform.cpp.md#fn-a4eda3906392ae77)
- [`xv::trap::detail::prepare_sleep`](../arch/riscv/kernel_platform.cpp.md#fn-b796ab17eb6c713b)
- [`xv::trap::detail::prepare_user_return`](../arch/riscv/kernel_platform.cpp.md#fn-1ed3457044a10998)
- [`xv::trap::detail::report_user_fault`](../arch/riscv/kernel_platform.cpp.md#fn-f43a07ef20a5a6b5)
- [`xv::trap::detail::request_kill`](../arch/riscv/kernel_platform.cpp.md#fn-de1ad36dd5d6359e)
- [`xv::trap::detail::resolve_page_fault`](../arch/riscv/kernel_platform.cpp.md#fn-e5148c82fe11c46e)
- [`xv::trap::detail::restore_kernel`](../arch/riscv/kernel_platform.cpp.md#fn-e72bdb404644cf01)
- [`xv::trap::detail::save_user_pc`](../arch/riscv/kernel_platform.cpp.md#fn-72d8410c76641146)
- [`xv::trap::detail::service_timer_devices`](../arch/riscv/kernel_platform.cpp.md#fn-8bf053cbd79e3fd0)
- [`xv::trap::detail::sleep`](../arch/riscv/kernel_platform.cpp.md#fn-8655e02476dd6fd7)
- [`xv::trap::detail::snapshot`](../arch/riscv/kernel_platform.cpp.md#fn-99ff6f6b07035421)
- [`xv::trap::detail::time`](../arch/riscv/kernel_platform.cpp.md#fn-21bfc0d68188d860)
- [`xv::trap::detail::user_page_table_token`](../arch/riscv/kernel_platform.cpp.md#fn-fb9cf283bc7dc87e)
- [`xv::trap::detail::wakeup`](../arch/riscv/kernel_platform.cpp.md#fn-372407f34cc73123)
- [`xv::trap::detail::yield_current`](../arch/riscv/kernel_platform.cpp.md#fn-b99841e868c4b5b2)

