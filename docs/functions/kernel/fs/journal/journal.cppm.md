# `kernel/fs/journal/journal.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/journal/journal.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:journal`。

直接模块依赖：`:cache`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Journal::Journal`](journal.cppm.md#fn-17149b6d55129134)
- [`xv::fs::Journal::Journal`](journal.cppm.md#fn-6cb3a5a184fdb474)
- [`xv::fs::Journal::begin`](journal.cpp.md#fn-68275ab1a63eaf6b)
- [`xv::fs::Journal::commit`](journal.cpp.md#fn-61fed5e757572a19)
- [`xv::fs::Journal::end`](journal.cpp.md#fn-9a8ba69510b2c5fb)
- [`xv::fs::Journal::initialize`](journal.cpp.md#fn-632dbb783eedc4e0)
- [`xv::fs::Journal::install`](journal.cpp.md#fn-a2705e4cf85be2f4)
- [`xv::fs::Journal::operator=`](journal.cppm.md#fn-cab0262ff7930705)
- [`xv::fs::Journal::read_header`](journal.cpp.md#fn-d538c5309149d8cd)
- [`xv::fs::Journal::record`](journal.cpp.md#fn-2bff0beebb0a1b13)
- [`xv::fs::Journal::recover`](journal.cpp.md#fn-a46bb87dec6e1fcb)
- [`xv::fs::Journal::sync`](journal.cpp.md#fn-5798daa238e10eb6)
- [`xv::fs::Journal::write_header`](journal.cpp.md#fn-95f9d303c4dca3ed)
- [`xv::fs::Journal::write_log`](journal.cpp.md#fn-0cdba2dc483b4403)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-e26e176641d6293e)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-1065f6193f43c5ac)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-b65a1bef83b0fe63)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-03718a79a3d72eff)
- [`xv::fs::Transaction::finish`](journal.cppm.md#fn-4108a82c8eef8888)
- [`xv::fs::Transaction::operator=`](journal.cppm.md#fn-93f6b3379a9a5409)
- [`xv::fs::Transaction::~Transaction`](journal.cppm.md#fn-b41989f478512a14)
- [`xv::fs::journal`](../service.cpp.md#fn-fde437052d31cdd9)

<a id="fn-17149b6d55129134"></a>

## `xv::fs::Journal::Journal`

```doxygen
@fn constexpr explicit xv::fs::Journal::Journal(BlockCache &cache)
@brief 构造文件系统重做日志，建立本重载要求的成员初值和借用关系。
@param cache 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`cache_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-6cb3a5a184fdb474"></a>

## `xv::fs::Journal::Journal`

```doxygen
@fn xv::fs::Journal::Journal(const Journal &)
@brief 文件系统重做日志的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-cab0262ff7930705"></a>

## `xv::fs::Journal::operator=`

```doxygen
@fn Journal &xv::fs::Journal::operator=(const Journal &)
@brief 文件系统重做日志的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-1065f6193f43c5ac"></a>

## `xv::fs::Transaction::Transaction`

```doxygen
@fn inline explicit xv::fs::Transaction::Transaction(Journal &journal)
@brief 构造日志事务作用域守卫，建立本重载要求的成员初值和借用关系。
@param journal 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`journal_` → `journal_->begin`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-e26e176641d6293e"></a>

## `xv::fs::Transaction::Transaction`

```doxygen
@fn inline xv::fs::Transaction::Transaction()
@brief 构造日志事务作用域守卫，建立本重载要求的成员初值和借用关系。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`Transaction` → `journal`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b41989f478512a14"></a>

## `xv::fs::Transaction::~Transaction`

```doxygen
@fn inline xv::fs::Transaction::~Transaction()
@brief 结束日志事务作用域守卫的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`finish`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-b65a1bef83b0fe63"></a>

## `xv::fs::Transaction::Transaction`

```doxygen
@fn xv::fs::Transaction::Transaction(const Transaction &)
@brief 日志事务作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-93f6b3379a9a5409"></a>

## `xv::fs::Transaction::operator=`

```doxygen
@fn Transaction &xv::fs::Transaction::operator=(const Transaction &)
@brief 日志事务作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-4108a82c8eef8888"></a>

## `xv::fs::Transaction::finish`

```doxygen
@fn inline void xv::fs::Transaction::finish()
@brief 日志事务作用域守卫：结束。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`journal_->end`。
@details 本函数可见的状态更新包括：`journal_ = nullptr;`。局部临时变量与对象字段应按作用域区分。
@details 需要特别跟踪的条件：`journal_`。这些条件决定检查、重试或提前返回路径。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
<a id="fn-03718a79a3d72eff"></a>

## `xv::fs::Transaction::Transaction`

```doxygen
@fn inline explicit xv::fs::Transaction::Transaction(bool persistent)
@brief 按持久性要求建立条件磁盘事务。
@details persistent 为 true 时调用全局 Journal::begin，否则保持空守卫。析构或 finish 只结束实际取得的事务；内存文件系统关闭使用 false，不申请日志容量。
```
