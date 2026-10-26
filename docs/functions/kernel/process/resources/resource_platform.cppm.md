# `kernel/process/resources/resource_platform.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/resources/resource_platform.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:resource_platform`。

直接模块依赖：`xv.fs`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::detail::release_file`](kernel_platform.cpp.md#fn-568d85df4de98c23)
- [`xv::process::detail::release_inode`](kernel_platform.cpp.md#fn-2f9bae0e0cdb8afe)
- [`xv::process::detail::resource_fail`](kernel_platform.cpp.md#fn-2efdaf57a891b47c)
- [`xv::process::detail::retain_file`](kernel_platform.cpp.md#fn-3c7590afb7fcdf81)
- [`xv::process::detail::retain_inode`](kernel_platform.cpp.md#fn-ceceb92a86a7781d)
