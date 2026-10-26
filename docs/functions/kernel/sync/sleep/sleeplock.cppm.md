# `kernel/sync/sleep/sleeplock.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../../kernel/sync/sleep/sleeplock.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:sleep`。

直接模块依赖：`:spin`, `:poll`。依赖仅表示可见接口，不代表所有服务都在初始化时调用。

## 函数导航

- [`xv::sync::SleepLock::SleepLock`](sleeplock.cppm.md#fn-83ad73e507d3948f)
- [`xv::sync::SleepLock::acquire`](sleeplock.cpp.md#fn-0df6c25631ea54c1)
- [`xv::sync::SleepLock::acquire_interruptible`](sleeplock.cpp.md#fn-4bf66df4f890a6a5)
- [`xv::sync::SleepLock::available`](sleeplock.cpp.md#fn-cb0bae526473552b)
- [`xv::sync::SleepLock::holding`](sleeplock.cpp.md#fn-d8314f38391b20f3)
- [`xv::sync::SleepLock::init`](sleeplock.cpp.md#fn-7372bd87bf9732cf)
- [`xv::sync::SleepLock::kind`](sleeplock.cppm.md#fn-155a14e59e63b005)
- [`xv::sync::SleepLock::release`](sleeplock.cpp.md#fn-9c53ea1cb256794a)
- [`xv::sync::SleepLock::try_acquire`](sleeplock.cpp.md#fn-e399786b3d75952b)

<a id="fn-83ad73e507d3948f"></a>

## `xv::sync::SleepLock::SleepLock`

```doxygen
@fn constexpr xv::sync::SleepLock::SleepLock()
@brief 构造睡眠锁，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-155a14e59e63b005"></a>

## `xv::sync::SleepLock::kind`

```doxygen
@fn LockKind xv::sync::SleepLock::kind() const override
@brief 睡眠锁：读取或计算 `LockKind::Sleep`，返回当前查询结果。
@return 各分支实际返回：`LockKind::Sleep`。应结合上面的条件检查区分正常值、空值和错误值。
```
