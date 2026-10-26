# `kernel/process/process_module.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../kernel/process/process_module.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:registers`, `:resources`, `:address_space`, `:process`, `:scheduler`, `:executable`, `:poll`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::claim_terminal`](lifecycle/service.cpp.md#fn-4884f3bab775fd24)
- [`xv::process::copy_in`](lifecycle/service.cpp.md#fn-d6c3ce5695ec8c2b)
- [`xv::process::copy_out`](lifecycle/service.cpp.md#fn-393041135005f370)
- [`xv::process::create_initial_process`](lifecycle/service.cpp.md#fn-b7634e2bbca815d9)
- [`xv::process::create_session`](lifecycle/service.cpp.md#fn-88fb19488751f21e)
- [`xv::process::deliver_signals`](lifecycle/service.cpp.md#fn-86b6b47851577869)
- [`xv::process::detach_terminal`](lifecycle/service.cpp.md#fn-7ada75984ab49466)
- [`xv::process::dump`](lifecycle/service.cpp.md#fn-e42abc0ba076c8f0)
- [`xv::process::exit_process`](service.md#service-exit-process)
- [`xv::process::fork_process`](service.md#service-fork-process)
- [`xv::process::get_process_group`](lifecycle/service.cpp.md#fn-c482d8945fdaf0ab)
- [`xv::process::get_session`](lifecycle/service.cpp.md#fn-39121e4d5139c870)
- [`xv::process::initialize`](service.md#service-initialize)
- [`xv::process::kill`](lifecycle/service.cpp.md#fn-42491b6e530470a7)
- [`xv::process::make_poll_wait`](lifecycle/service.cpp.md#fn-c10951196ee0a808)
- [`xv::process::may_read_terminal`](lifecycle/service.cpp.md#fn-a0836e0fefde5729)
- [`xv::process::poll_timer_tick`](lifecycle/service.cpp.md#fn-22490dbefd1fb8c0)
- [`xv::process::post_terminal_interrupt`](lifecycle/service.cpp.md#fn-b12e280ce26670d8)
- [`xv::process::prepare_interruptible_sleep`](sleep.md#sleep-prepare-interruptible-sleep)
- [`xv::process::prepare_sleep`](sleep.md#sleep-prepare-sleep)
- [`xv::process::send_signal`](lifecycle/service.cpp.md#fn-ac942543753e7e75)
- [`xv::process::service_signal_notifications`](lifecycle/service.cpp.md#fn-bdb81b83086795f2)
- [`xv::process::set_process_group`](lifecycle/service.cpp.md#fn-34091ebc76bb444c)
- [`xv::process::set_signal_disposition`](lifecycle/service.cpp.md#fn-bb3030848b31cf05)
- [`xv::process::set_terminal_foreground`](lifecycle/service.cpp.md#fn-2684c802094d44d8)
- [`xv::process::sleep`](sleep.md#sleep-sleep)
- [`xv::process::terminal_foreground`](lifecycle/service.cpp.md#fn-9d295666c0ed021d)
- [`xv::process::wait`](service.md#service-wait)
- [`xv::process::wait_for_child`](service.md#service-wait-for-child)
- [`xv::process::wakeup`](service.md#service-wakeup)

