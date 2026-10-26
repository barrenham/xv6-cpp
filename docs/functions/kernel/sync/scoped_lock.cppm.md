# `kernel/sync/scoped_lock.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../kernel/sync/scoped_lock.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:guard`。

直接模块依赖：`:lock`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::ScopedLock::ScopedLock`](scoped_lock.cppm.md#fn-0424c8f57b60d53f)
- [`xv::sync::ScopedLock::ScopedLock`](scoped_lock.cppm.md#fn-83b7fa32688fabd4)
- [`xv::sync::ScopedLock::ScopedLock`](scoped_lock.cppm.md#fn-390f56590e2cba06)
- [`xv::sync::ScopedLock::operator=`](scoped_lock.cppm.md#fn-a0e8a723c806f6a8)
- [`xv::sync::ScopedLock::~ScopedLock`](scoped_lock.cppm.md#fn-887258b47d4efbf7)

<a id="fn-0424c8f57b60d53f"></a>

## `xv::sync::ScopedLock::ScopedLock`

```doxygen
@fn explicit xv::sync::ScopedLock::ScopedLock(Lock &lock)
@brief 构造锁作用域守卫，建立本重载要求的成员初值和借用关系。
@param lock 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_` → `lock_.acquire`。
@note 源码中出现的锁操作：`lock_.acquire`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-83b7fa32688fabd4"></a>

## `xv::sync::ScopedLock::ScopedLock`

```doxygen
@fn xv::sync::ScopedLock::ScopedLock(Lock &lock, AdoptLock)
@brief 构造锁作用域守卫，建立本重载要求的成员初值和借用关系。
@param lock 指向或引用的对象/缓冲；其生命周期必须覆盖本次调用。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_`。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-887258b47d4efbf7"></a>

## `xv::sync::ScopedLock::~ScopedLock`

```doxygen
@fn xv::sync::ScopedLock::~ScopedLock()
@brief 结束锁作用域守卫的对象生命周期，并执行其显式清理或成员析构。
@details 调用/构造表达式的文本次序（分支内不一定全部执行）：`lock_.release`。
@note 源码中出现的锁操作：`lock_.release`。顺序包含分支和释放动作，不表示这些锁同时持有。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-390f56590e2cba06"></a>

## `xv::sync::ScopedLock::ScopedLock`

```doxygen
@fn xv::sync::ScopedLock::ScopedLock(const ScopedLock &)
@brief 锁作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-a0e8a723c806f6a8"></a>

## `xv::sync::ScopedLock::operator=`

```doxygen
@fn ScopedLock &xv::sync::ScopedLock::operator=(const ScopedLock &)
@brief 锁作用域守卫的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```
