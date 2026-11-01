# `kernel/process/storage/platform.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/storage/platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:storage_platform`。

## 函数导航

- [`xv::process::detail::allocate_kernel_stack`](kernel_platform.cpp.md#fn-0c73cf7a89069b02)
- [`xv::process::detail::allocate_process_page`](kernel_platform.cpp.md#fn-e1bcdb7d80c1c4c6)
- [`xv::process::detail::free_kernel_stack`](kernel_platform.cpp.md#fn-0ccd821dd7af8126)
- [`xv::process::detail::free_process_page`](kernel_platform.cpp.md#fn-470a48c1e3751448)
- [`xv::process::detail::storage_fail`](kernel_platform.cpp.md#fn-81d9f378847f3b25)

- [`xv::process::detail::allocate_process_object`](kernel_platform.cpp.md#fn-caa4a1fb931599c7)

- [`xv::process::detail::arm_deadline_timer`](kernel_platform.cpp.md#fn-ab4e82d5ecac01d7)

- [`xv::process::detail::free_process_object`](kernel_platform.cpp.md#fn-9ac05fbc453dc8b6)

- [`xv::process::detail::initialize_process_objects`](kernel_platform.cpp.md#fn-7d233fbcfe20643c)

- [`xv::process::detail::scheduling_time`](kernel_platform.cpp.md#fn-4fba22f386559889)

- [`xv::process::detail::synchronize_kernel_mappings`](kernel_platform.cpp.md#fn-41b310a196e93085)
