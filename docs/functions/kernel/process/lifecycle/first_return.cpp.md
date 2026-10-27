# `kernel/process/lifecycle/first_return.cpp`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/lifecycle/first_return.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process`。

直接模块依赖：`xv.fs`, `xv.trap`, `xv.print`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::first_return`](first_return.cpp.md#fn-f06f00214daff1d9)

<a id="fn-f06f00214daff1d9"></a>

## `xv::process::first_return`

```doxygen
@fn void xv::process::first_return()
@brief 在进程首次调度时准备文件系统并进入用户态。
@pre 调度器持有当前进程锁，本函数先释放该锁，再进行可能睡眠的文件系统初始化。
@details 初始进程恢复并挂载文件系统，然后从 XV_INIT_PATH 指定的 /sbin/init 装载首个用户程序。参数首项使用相同路径，装载结果写入 trapframe.a0；失败触发 panic。其他进程跳过首轮初始化并按自己的已准备上下文返回用户态。
@note first 标记用 acquire/release 发布初始化完成状态，确保其他核心观察到已就绪的系统服务。
@return 经 return_to_user 进入用户态，不普通返回到调度调用点。
```
