# `kernel/fs/pipe/pipe_factory.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/pipe/pipe_factory.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:pipe`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::PipeFactoryFlow::allocate_files`](pipe_factory.cpp.md#fn-a2505fbece33de4e)
- [`xv::fs::PipeFactoryFlow::allocate_page`](pipe_factory.cpp.md#fn-99c39ce0a887d763)
- [`xv::fs::PipeFactoryFlow::both`](pipe_factory.cpp.md#fn-431df1f324c4bf6e)
- [`xv::fs::PipeFactoryFlow::construct`](pipe_factory.cpp.md#fn-ae51154b782e350e)
- [`xv::fs::PipeFactoryFlow::has_reader`](pipe_factory.cpp.md#fn-6ac8012a915ed262)
- [`xv::fs::PipeFactoryFlow::has_writer`](pipe_factory.cpp.md#fn-edb42d71f1692029)
- [`xv::fs::PipeFactoryFlow::publish`](pipe_factory.cpp.md#fn-99b108bbcdf315ce)
- [`xv::fs::PipeFactoryFlow::release_reader`](pipe_factory.cpp.md#fn-1b68eab67b49edc5)
- [`xv::fs::PipeFactoryFlow::release_writer`](pipe_factory.cpp.md#fn-b6c72f3083099ea2)
- [`xv::fs::create_pipe`](pipe_factory.cpp.md#fn-6abd7f37e1481270)

<a id="fn-6abd7f37e1481270"></a>

## `xv::fs::create_pipe`

```doxygen
@fn bool xv::fs::create_pipe(File **read_end, File **write_end)
@brief 文件描述、inode、路径、块缓存与日志事务：创建pipe。
@param read_end 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param write_end 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@return 完整构造并发布两端返回 true；失败清空输出并回收已取得的 File，返回 false。
@details 流程由 PipeFactoryFlow 的编译期图连接；具名业务阶段保留原有分配、锁、返回值与资源转移协议。
```

<a id="fn-a2505fbece33de4e"></a>

## `xv::fs::PipeFactoryFlow::allocate_files`

```doxygen
@fn static void xv::fs::PipeFactoryFlow::allocate_files(Context &c) noexcept
@brief 按原顺序分别尝试分配读端和写端 File。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-99c39ce0a887d763"></a>

## `xv::fs::PipeFactoryFlow::allocate_page`

```doxygen
@fn static bool xv::fs::PipeFactoryFlow::allocate_page(Context &c) noexcept
@brief 申请管道对象存储页。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-431df1f324c4bf6e"></a>

## `xv::fs::PipeFactoryFlow::both`

```doxygen
@fn static bool xv::fs::PipeFactoryFlow::both(Context &c) noexcept
@brief 检查两个 File 是否都分配成功。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-ae51154b782e350e"></a>

## `xv::fs::PipeFactoryFlow::construct`

```doxygen
@fn static void xv::fs::PipeFactoryFlow::construct(Context &c) noexcept
@brief 在页面中构造并初始化管道，再绑定两端后端。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-6ac8012a915ed262"></a>

## `xv::fs::PipeFactoryFlow::has_reader`

```doxygen
@fn static bool xv::fs::PipeFactoryFlow::has_reader(Context &c) noexcept
@brief 检查是否需要回收已分配读端。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-edb42d71f1692029"></a>

## `xv::fs::PipeFactoryFlow::has_writer`

```doxygen
@fn static bool xv::fs::PipeFactoryFlow::has_writer(Context &c) noexcept
@brief 检查是否需要回收已分配写端。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-99b108bbcdf315ce"></a>

## `xv::fs::PipeFactoryFlow::publish`

```doxygen
@fn static void xv::fs::PipeFactoryFlow::publish(Context &c) noexcept
@brief 向调用者发布两端 File 并记录成功。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-1b68eab67b49edc5"></a>

## `xv::fs::PipeFactoryFlow::release_reader`

```doxygen
@fn static void xv::fs::PipeFactoryFlow::release_reader(Context &c) noexcept
@brief 归还未发布的读端 File。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```

<a id="fn-b6c72f3083099ea2"></a>

## `xv::fs::PipeFactoryFlow::release_writer`

```doxygen
@fn static void xv::fs::PipeFactoryFlow::release_writer(Context &c) noexcept
@brief 归还未发布的写端 File。
@details 入口先清空调用者输出；两个 File 都分配后才申请页面，失败逐个归还已分配 File，成功才发布两端。 下一节点由 Graph 类型策略声明，不由本阶段返回运行时目标。
```
