# `kernel/syscall/syspoll.cpp`

开发职责：用户参数校验、描述符取得及系统调用返回 ABI。

源码：[打开实际文件](../../../../kernel/syscall/syspoll.cpp)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.syscall`。

直接模块依赖：`:arguments`, `:handlers`, `xv.process`, `xv.fs`, `xv.arch`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::syscall::sys_poll`](syspoll.cpp.md#fn-9423b8d80c87e07b)

<a id="fn-9423b8d80c87e07b"></a>

## `xv::syscall::sys_poll`

```doxygen
@fn unsigned long xv::syscall::sys_poll()
@brief 系统调用入口：poll；从用户参数转入内核服务并返回 ABI 结果。
@details 先复制 pollfd 数组并保留 File 引用，建立单次 PollWait，首轮先登记全部依赖再查询，之后复查状态直到就绪、超时或信号。局部析构顺序保证先撤销订阅、再释放 File，避免端点已经释放而等待项仍引用它。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`argument_address` → `argument_integer` → `xv::process::current` → `process->memory` → `copy_in` → `HeldFiles` → `xv::fs::files` → `release` → `process->files` → `get` → `retain` → `xv::process::make_poll_wait`。
@details 本函数可见的状态更新包括：`held.entries[i] = xv::fs::files().retain(file);`；`descriptor.revents = 0;`；`descriptor.revents = events & (static_cast<unsigned short>(descriptor.events) |`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`count < 0 || count > NOFILE`；`count && process->memory().copy_in(descriptors, address, count * sizeof(pollfd)) < 0`；`file`；`auto *file = process->files().get(descriptors[i].fd)`；`timeout > 0`。这些条件决定检查、重试或提前返回路径。
@note 此路径涉及用户地址与内核地址的边界。复制失败必须按当前分支退出，不能继续把用户地址作为普通内核指针使用。
@return 各分支实际返回：`-1`；`ready`。应结合上面的条件检查区分正常值、空值和错误值。
@note 系统调用处理函数使用内核 64 位返回通道；负错误值经无符号类型传回时保持位模式，由用户 ABI 按相应整型解释。
```
