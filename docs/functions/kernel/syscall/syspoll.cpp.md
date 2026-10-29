# `kernel/syscall/syspoll.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/syspoll.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:arguments`, `:handlers`, `xv.process`, `xv.fs`, `xv.arch`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::PollFlow::complete`](syspoll.cpp.md#fn-1af6e5b92a09e329)
- [`xv::syscall::PollFlow::copy_results`](syspoll.cpp.md#fn-f37c560b975a24bb)
- [`xv::syscall::PollFlow::failed`](syspoll.cpp.md#fn-33aacb2ae25044b4)
- [`xv::syscall::PollFlow::killed`](syspoll.cpp.md#fn-ea3023eafd9591d9)
- [`xv::syscall::PollFlow::scan`](syspoll.cpp.md#fn-185ea3687e73f2e4)
- [`xv::syscall::PollFlow::sleep`](syspoll.cpp.md#fn-b27955ba944bc471)
- [`xv::syscall::PollFlow::success`](syspoll.cpp.md#fn-f9b8f0697a53fae9)
- [`xv::syscall::sys_poll`](syspoll.cpp.md#fn-9423b8d80c87e07b)

<a id="fn-9423b8d80c87e07b"></a>

## `xv::syscall::sys_poll`

```doxygen
@fn unsigned long xv::syscall::sys_poll()
@brief 系统调用入口：poll；从用户参数转入内核服务并返回 ABI 结果。
@details 先复制 pollfd 数组并保留 File 引用，建立单次 PollWait，首轮先登记全部依赖再查询，之后复查状态直到就绪、超时或信号。局部析构顺序保证先撤销订阅、再释放 File，避免端点已经释放而等待项仍引用它。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`ready`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
@details 编排由 PollFlow 的模板图声明；具名阶段负责业务操作，控制策略负责下一节点。原有等待协议、引用所有权和错误返回约定保留。
```

<a id="fn-1af6e5b92a09e329"></a>

## `xv::syscall::PollFlow::complete`

```doxygen
@fn static bool xv::syscall::PollFlow::complete(Context &c) noexcept
@brief 检查已有就绪事件、零超时或时限已到。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f37c560b975a24bb"></a>

## `xv::syscall::PollFlow::copy_results`

```doxygen
@fn static bool xv::syscall::PollFlow::copy_results(Context &c) noexcept
@brief 将扫描结果复制回用户描述符数组。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-33aacb2ae25044b4"></a>

## `xv::syscall::PollFlow::failed`

```doxygen
@fn static bool xv::syscall::PollFlow::failed(Context &c) noexcept
@brief 检查等待依赖登记是否失败。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-ea3023eafd9591d9"></a>

## `xv::syscall::PollFlow::killed`

```doxygen
@fn static bool xv::syscall::PollFlow::killed(Context &c) noexcept
@brief 检查当前进程是否已终止。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-185ea3687e73f2e4"></a>

## `xv::syscall::PollFlow::scan`

```doxygen
@fn static void xv::syscall::PollFlow::scan(Context &c) noexcept
@brief 直接扫描描述符、登记首轮等待依赖并统计就绪事件。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-b27955ba944bc471"></a>

## `xv::syscall::PollFlow::sleep`

```doxygen
@fn static void xv::syscall::PollFlow::sleep(Context &c) noexcept
@brief 结束依赖注册阶段并等待通知，然后回到扫描入口。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```

<a id="fn-f9b8f0697a53fae9"></a>

## `xv::syscall::PollFlow::success`

```doxygen
@fn static void xv::syscall::PollFlow::success(Context &c) noexcept
@brief 记录就绪描述符数量。
@details 入口已复制用户描述符并保留文件引用；PollWait 与 HeldFiles 在 sys_poll 返回时按原顺序析构。只有首轮注册依赖，后续轮次重扫状态。 下一节点由 Graph 的类型策略声明；本阶段不返回运行时跳转目标。
```
