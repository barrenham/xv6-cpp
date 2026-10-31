# `kernel/fs/journal/journal.cppm`

开发职责：文件描述、inode、路径、块缓存与日志事务。

源码：[打开实际文件](../../../../../kernel/fs/journal/journal.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.fs:journal`。

直接模块依赖：`:cache`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::fs::Journal::Journal`](journal.cppm.md#fn-6cb3a5a184fdb474)
- [`xv::fs::Journal::begin`](journal.cpp.md#fn-68275ab1a63eaf6b)
- [`xv::fs::Journal::end`](journal.cpp.md#fn-9a8ba69510b2c5fb)
- [`xv::fs::Journal::operator=`](journal.cppm.md#fn-cab0262ff7930705)
- [`xv::fs::Journal::record`](journal.cpp.md#fn-2bff0beebb0a1b13)
- [`xv::fs::Journal::sync`](journal.cpp.md#fn-5798daa238e10eb6)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-e26e176641d6293e)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-1065f6193f43c5ac)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-b65a1bef83b0fe63)
- [`xv::fs::Transaction::Transaction`](journal.cppm.md#fn-03718a79a3d72eff)
- [`xv::fs::Transaction::finish`](journal.cppm.md#fn-4108a82c8eef8888)
- [`xv::fs::Transaction::operator=`](journal.cppm.md#fn-93f6b3379a9a5409)
- [`xv::fs::Transaction::~Transaction`](journal.cppm.md#fn-b41989f478512a14)
- [`xv::fs::journal`](../service.cpp.md#fn-fde437052d31cdd9)

<a id="fn-6cb3a5a184fdb474"></a>

## `xv::fs::Journal::Journal`

```doxygen
@fn xv::fs::Journal::Journal(const Journal &)=delete
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-cab0262ff7930705"></a>

## `xv::fs::Journal::operator=`

```doxygen
@fn Journal &xv::fs::Journal::operator=(const Journal &)=delete
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
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
@brief 按固定顺序预留所需日志事务。
@details 显式 Journal 构造只保留指定日志；persistent 构造在需要时先系统盘再数据盘，默认构造选择持久事务。第二日志仅在启动恢复后发布。
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
@brief 按数据盘到系统盘的顺序结束预留。
@details 先归还可选第二日志再结束主日志，清空指针使重复 finish 和析构幂等；两个日志不是跨文件系统原子提交。
```
<a id="fn-03718a79a3d72eff"></a>

## `xv::fs::Transaction::Transaction`

```doxygen
@fn inline explicit xv::fs::Transaction::Transaction(bool persistent)
@brief 按固定顺序预留所需日志事务。
@details 显式 Journal 构造只保留指定日志；persistent 构造在需要时先系统盘再数据盘，默认构造选择持久事务。第二日志仅在启动恢复后发布。
```
<a id="fn-77b0a063f8d44c98"></a>

## `xv::fs::Journal::Journal`

```doxygen
@fn constexpr xv::fs::Journal::Journal()=default
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```

<a id="fn-fcb025f12f6226d1"></a>

## `xv::fs::Journal::~Journal`

```doxygen
@fn xv::fs::Journal::~Journal()=default
@brief 声明与具体磁盘格式无关的事务协议；由xv6或ext3后端实现。
@details 模块依赖、接口与当前限制见 docs/ext3.md、docs/devfs.md、docs/procfs.md 及 docs/directories.md；该函数的实际声明与实现由编译器USR绑定。
```
