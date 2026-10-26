# `kernel/process/wait/sleep.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/sleep.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:wait_channels`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::prepare_interruptible_sleep`](../sleep.md#sleep-prepare-interruptible-sleep)
- [`xv::process::prepare_sleep`](../sleep.md#sleep-prepare-sleep)
- [`xv::process::sleep`](../sleep.md#sleep-sleep)
