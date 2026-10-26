# `kernel/sync/spin/spinlock.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/spin/spinlock.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:spin`。

直接模块依赖：`:lock`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::SpinLock::SpinLock`](spinlock.cppm.md#fn-607529f9f08a217b)
- [`xv::sync::SpinLock::acquire`](spinlock.cpp.md#fn-802a415222da16f6)
- [`xv::sync::SpinLock::holding`](spinlock.cpp.md#fn-53ef4002e320bbcd)
- [`xv::sync::SpinLock::init`](spinlock.cpp.md#fn-2ed907681aa58c53)
- [`xv::sync::SpinLock::kind`](spinlock.cppm.md#fn-3a61d81f0ca02960)
- [`xv::sync::SpinLock::release`](spinlock.cpp.md#fn-c91cfd40f4e7bacf)

<a id="fn-607529f9f08a217b"></a>

## `xv::sync::SpinLock::SpinLock`

```doxygen
@fn constexpr xv::sync::SpinLock::SpinLock()
@brief 构造自旋锁，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-3a61d81f0ca02960"></a>

## `xv::sync::SpinLock::kind`

```doxygen
@fn LockKind xv::sync::SpinLock::kind() const override
@brief 自旋锁：读取或计算 `LockKind::Spin`，返回当前查询结果。
@return 各分支实际返回：`LockKind::Spin`。应结合上面的条件检查区分正常值、空值和错误值。
```
