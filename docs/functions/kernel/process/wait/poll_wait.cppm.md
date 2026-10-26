# `kernel/process/wait/poll_wait.cppm`

开发职责：进程对象生命周期、组身份、调度、等待与信号。

源码：[打开实际文件](../../../../../kernel/process/wait/poll_wait.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.process:poll`。

直接模块依赖：`:process`, `xv.sync`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::process::PollWait::PollWait`](poll_wait.cppm.md#fn-6125f315d5c17491)
- [`xv::process::PollWait::PollWait`](poll_wait.cppm.md#fn-ce926e3856bed340)
- [`xv::process::PollWait::allocate_page`](poll_wait.cpp.md#fn-a6b19002bbc8ed4b)
- [`xv::process::PollWait::failed`](poll_wait.cppm.md#fn-9e37854e58be95a2)
- [`xv::process::PollWait::notify`](poll_wait.cpp.md#fn-ac78ce960fa8587b)
- [`xv::process::PollWait::operator=`](poll_wait.cppm.md#fn-f1da21008c755795)
- [`xv::process::PollWait::set_deadline`](poll_wait.cpp.md#fn-a963ef69e5d7511f)
- [`xv::process::PollWait::sleep`](poll_wait.cpp.md#fn-f79ddf34a080a9c4)
- [`xv::process::PollWait::table`](poll_wait.cpp.md#fn-ef1952a6c3f61b5f)
- [`xv::process::PollWait::triggered`](poll_wait.cpp.md#fn-58af055f0fb1e43c)
- [`xv::process::PollWait::watch`](poll_wait.cpp.md#fn-c0a508d830521c5d)
- [`xv::process::PollWait::~PollWait`](poll_wait.cpp.md#fn-000e85dd21110cb3)

<a id="fn-6125f315d5c17491"></a>

## `xv::process::PollWait::PollWait`

```doxygen
@fn constexpr xv::process::PollWait::PollWait(Process &process, WaitChannels &channels)
@brief 构造单次 poll 等待上下文，建立本重载要求的成员初值和借用关系。
@param process 被操作的稳定进程对象，存活期必须覆盖本次访问。
@param channels 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`process_` → `channels_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ce926e3856bed340"></a>

## `xv::process::PollWait::PollWait`

```doxygen
@fn xv::process::PollWait::PollWait(const PollWait &)
@brief 单次 poll 等待上下文的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-f1da21008c755795"></a>

## `xv::process::PollWait::operator=`

```doxygen
@fn PollWait &xv::process::PollWait::operator=(const PollWait &)
@brief 单次 poll 等待上下文的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-9e37854e58be95a2"></a>

## `xv::process::PollWait::failed`

```doxygen
@fn bool xv::process::PollWait::failed() const
@brief 单次 poll 等待上下文：读取或计算 `failed_`，返回当前查询结果。
@return 各分支实际返回：`failed_`。应结合上面的条件检查区分正常值、空值和错误值。
```
