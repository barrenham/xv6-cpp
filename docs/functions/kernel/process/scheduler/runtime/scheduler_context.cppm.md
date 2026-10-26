# `kernel/process/scheduler/runtime/scheduler_context.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../../kernel/process/scheduler/runtime/scheduler_context.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:scheduler_context`。

直接模块依赖：`:process`, `:scheduler`, `:ready_queue`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::SchedulerContext::begin_round`](scheduler_context.cpp.md#fn-3168ca64e91ead29)
- [`xv::process::SchedulerContext::dispatch`](scheduler_context.cpp.md#fn-717e81da3f2d8fa2)
- [`xv::process::SchedulerContext::idle`](scheduler_context.cpp.md#fn-70ec78efb2fe5d09)
- [`xv::process::SchedulerContext::make_runnable`](scheduler_context.cpp.md#fn-28e041bc6bad922e)
- [`xv::process::SchedulerContext::prepare_cpu`](scheduler_context.cpp.md#fn-8eb37916f4622bf5)
- [`xv::process::SchedulerContext::suspend`](scheduler_context.cpp.md#fn-cba1163ffee29c4e)
- [`xv::process::SchedulerContext::yield`](scheduler_context.cpp.md#fn-5bdf0a1333169edf)
