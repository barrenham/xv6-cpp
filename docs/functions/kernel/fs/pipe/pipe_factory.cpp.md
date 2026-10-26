# `kernel/fs/pipe/pipe_factory.cpp`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/pipe/pipe_factory.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs`。

直接模块依赖：`:pipe`, `:platform`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::create_pipe`](pipe_factory.cpp.md#fn-6abd7f37e1481270)

<a id="fn-6abd7f37e1481270"></a>

## `xv::fs::create_pipe`

```doxygen
@fn bool xv::fs::create_pipe(File **read_end, File **write_end)
@brief 文件描述、inode、路径、块缓存与日志事务：创建pipe。
@param read_end 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@param write_end 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`files` → `allocate` → `detail::allocate_page` → `release` → `new` → `pipe->initialize` → `bind_pipe`。
@details 需要特别跟踪的条件：`reader && writer`；`!storage`；`reader`；`writer`。这些条件决定检查、重试或提前返回路径。
@return 各分支实际返回：`false`；`true`。应结合上面的条件检查区分正常值、空值和错误值。
```
