# `kernel/process/wait/wait_channels.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/wait_channels.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`:wait_channels`, `:storage_platform`, `:poll`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::WaitChannels::cancel`](../wait-channels.md#wait-waitchannels-cancel)
- [`xv::process::WaitChannels::index`](../wait-channels.md#wait-waitchannels-index)
- [`xv::process::WaitChannels::initialize`](../wait-channels.md#wait-waitchannels-initialize)
- [`xv::process::WaitChannels::prepare`](../wait-channels.md#wait-waitchannels-prepare)
- [`xv::process::WaitChannels::unlink`](../wait-channels.md#wait-waitchannels-unlink)
- [`xv::process::WaitChannels::wake`](../wait-channels.md#wait-waitchannels-wake)
