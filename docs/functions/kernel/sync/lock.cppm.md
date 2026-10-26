# `kernel/sync/lock.cppm`

开发职责：锁所有权、屏蔽中断与对象等待登记。

源码：[打开实际文件](../../../../kernel/sync/lock.cppm)。文档不会加入源码或生成额外 C++ 声明。

模块身份：`xv.sync:lock`。

## 函数导航

- [`xv::sync::Lock::Lock`](lock.cppm.md#fn-e74be0777e619cda)
- [`xv::sync::Lock::Lock`](lock.cppm.md#fn-79a2123e5b5f38c1)
- [`xv::sync::Lock::acquire`](lock.cppm.md#fn-d48dd1a3c416df06)
- [`xv::sync::Lock::holding`](lock.cppm.md#fn-ed0ad969c891d1a7)
- [`xv::sync::Lock::init`](lock.cppm.md#fn-0c66a2ca23a4482a)
- [`xv::sync::Lock::kind`](lock.cppm.md#fn-709209da8513b94d)
- [`xv::sync::Lock::name`](lock.cppm.md#fn-69ff09d837c5337a)
- [`xv::sync::Lock::operator=`](lock.cppm.md#fn-dbf250badd450697)
- [`xv::sync::Lock::release`](lock.cppm.md#fn-f76eb5cef010caf0)
- [`xv::sync::Lock::~Lock`](lock.cppm.md#fn-054c348e12181db7)

<a id="fn-79a2123e5b5f38c1"></a>

## `xv::sync::Lock::Lock`

```doxygen
@fn xv::sync::Lock::Lock(const Lock &)
@brief 锁所有权、屏蔽中断与对象等待登记的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-dbf250badd450697"></a>

## `xv::sync::Lock::operator=`

```doxygen
@fn Lock &xv::sync::Lock::operator=(const Lock &)
@brief 锁所有权、屏蔽中断与对象等待登记的禁用操作：在编译期拒绝此种复制或赋值。
@details 该成员显式声明为 deleted，不存在可在运行期调用的实现。它阻止绕开对象的引用/锁/所有权规则创建第二个拥有者。
@return 无运行期返回路径；使用该操作会导致编译失败。
```

<a id="fn-0c66a2ca23a4482a"></a>

## `xv::sync::Lock::init`

```doxygen
@fn virtual void xv::sync::Lock::init(const char *label)
@brief 锁所有权、屏蔽中断与对象等待登记：init。
@param label 保存在对象中的调试标签，字符串需在使用期间有效。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-d48dd1a3c416df06"></a>

## `xv::sync::Lock::acquire`

```doxygen
@fn virtual void xv::sync::Lock::acquire()
@brief 锁所有权、屏蔽中断与对象等待登记：取得锁所有权。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-f76eb5cef010caf0"></a>

## `xv::sync::Lock::release`

```doxygen
@fn virtual void xv::sync::Lock::release()
@brief 锁所有权、屏蔽中断与对象等待登记：释放当前引用或归还对象占用的资源。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-ed0ad969c891d1a7"></a>

## `xv::sync::Lock::holding`

```doxygen
@fn virtual bool xv::sync::Lock::holding() const
@brief 锁所有权、屏蔽中断与对象等待登记：检查当前执行者是否拥有此锁。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `bool () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-709209da8513b94d"></a>

## `xv::sync::Lock::kind`

```doxygen
@fn virtual LockKind xv::sync::Lock::kind() const
@brief 锁所有权、屏蔽中断与对象等待登记：种类。
@details 这是纯虚接口契约，当前类型不提供可执行函数体；具体语义由派生实现完成。应从实际注入的设备、调度器或存储对象继续跟踪调用，不应把接口声明当成同步实现。
@return 返回类型为 `LockKind () const`；本条目没有可独立分析的普通函数体，具体返回约定见对应实现及调用方。
```

<a id="fn-69ff09d837c5337a"></a>

## `xv::sync::Lock::name`

```doxygen
@fn const char *xv::sync::Lock::name() const
@brief 锁所有权、屏蔽中断与对象等待登记：读取或计算 `name_`，返回当前查询结果。
@return 各分支实际返回：`name_`。应结合上面的条件检查区分正常值、空值和错误值。
```

<a id="fn-e74be0777e619cda"></a>

## `xv::sync::Lock::Lock`

```doxygen
@fn constexpr xv::sync::Lock::Lock()
@brief 构造锁所有权、屏蔽中断与对象等待登记，建立本重载要求的成员初值和借用关系。
@details 使用编译器生成的默认成员行为。普通标量是否被初始化取决于成员默认初始化器和调用方的值初始化方式，不能把 default 一概当作清零。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```

<a id="fn-054c348e12181db7"></a>

## `xv::sync::Lock::~Lock`

```doxygen
@fn xv::sync::Lock::~Lock()
@brief 结束锁所有权、屏蔽中断与对象等待登记的对象生命周期，并执行其显式清理或成员析构。
@details 析构函数自身没有额外语句，成员对象仍按 C++ 析构顺序处理。对象存储的归还由拥有者或外层释放入口负责。
@return 无普通返回值；调用效果体现在受保护状态、资源所有权或控制流的变化中。
```
